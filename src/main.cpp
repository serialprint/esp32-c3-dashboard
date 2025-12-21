#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include "logo.h"
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <DHT_U.h>

#define VERSION_TEXT "v1.2.1"   // 🔥 SAG ALTA GÖRÜNECEK VERSİYON

// =====================================================
// 🔹 Wi-Fi Bilgileri
// =====================================================
const char* ssid = "SUPERONLINE_Wi-Fi_8133";
const char* password = "XdUfXtSXk5NZ";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;
const int daylightOffset_sec = 0;

// 🔹 TFT Pinleri
#define TFT_CS   10
#define TFT_DC    7
#define TFT_RST   5
#define TFT_SCLK  4
#define TFT_MOSI  6

// 🔹 DHT11 Pin
#define DHT_PIN   2   // GPIO2 - DHT11 DATA pini

// 🔹 Encoder Pinleri
#define ENCODER_CLK   8   // GPIO8 - CLK pini
#define ENCODER_DT    9   // GPIO9 - DT pini  
#define ENCODER_SW    3   // GPIO3 - Switch/Button pini

// 🔹 Backlight Pin (PWM)
#define TFT_BACKLIGHT 1   // GPIO1 - Backlight PWM pini
#define LEDC_FREQ 5000    // PWM frekansı (Hz)
#define LEDC_RESOLUTION 8 // 8-bit çözünürlük (0-255)
#define USE_PWM true      // PWM kullanılsın

#define TFT_WIDTH  320
#define TFT_HEIGHT 172

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHT_PIN, DHT11);

// =====================================================
unsigned long lastTimeUpdate = 0;
const unsigned long timeInterval = 1000;

String prevTime = "";
String prevDate = "";
String prevTemp = "";
String prevHum = "";
String ipAddress = "";
int prevOTAPercent = -1;  // OTA progress takibi için

// 🔹 WiFi Yeniden Bağlanma
unsigned long lastWiFiCheck = 0;
const unsigned long wifiCheckInterval = 10000;  // 10 saniyede bir kontrol (CPU'yu yormaz)
bool wifiReconnecting = false;
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;  // 5 saniyede bir yeniden bağlanma dene

// 🔹 Encoder Değişkenleri
volatile int encoderPosition = 0;
volatile int lastEncoderPosition = 0;
int lastCLKState = HIGH;
bool encoderButtonPressed = false;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;  // Debounce süresi artırıldı

// 🔹 Menü Sistemi
int currentMenuPage = 0;  // 0: Ana sayfa, 1: Ayarlar, 2: Parlaklık, 3: WiFi Bilgileri, 4: İstatistikler, 5: Sistem Bilgileri
int menuItem = 0;

// 🔹 İstatistikler
float minTemp = 999.0;
float maxTemp = -999.0;
float minHum = 999.0;
float maxHum = -999.0;
float sumTemp = 0.0;
float sumHum = 0.0;
unsigned long readingCount = 0;
unsigned long wifiConnectedTime = 0;  // WiFi bağlantı zamanı (millis)
bool wifiWasConnected = false;

// 🔹 Sistem Bilgileri
unsigned long systemStartTime = 0;  // Sistem başlangıç zamanı

// 🔹 Parlaklık Kontrolü
int brightness = 128;  // 0-255 arası (varsayılan: %50)
const int brightnessMin = 20;
const int brightnessMax = 255;

const char* otaName = "sp_dashboard";
const char* otaPass = "1234";

// =======================================================
//  🟦 OTA BAŞLATMA
// =======================================================
void startOTA() {

  Serial.println("OTA yukleme servisi baslatiliyor...");

  if (!MDNS.begin(otaName)) {
    Serial.println("mDNS baslatilamadi!");
  }

  ArduinoOTA.setHostname(otaName);
  ArduinoOTA.setPassword(otaPass);

  ArduinoOTA
      .onStart([]() {
        Serial.println("OTA Basladi!");
        prevOTAPercent = -1;  // Reset progress tracking
        tft.fillScreen(ST77XX_BLACK);
        
        // Başlık
        tft.setCursor(10, 30);
        tft.setTextColor(ST77XX_CYAN);
        tft.setTextSize(2);
        tft.println("OTA Yukleniyor");
        
        // Progress bar çerçevesi
        tft.drawRect(10, 70, TFT_WIDTH - 20, 25, ST77XX_WHITE);
      })
      .onEnd([]() {
        Serial.println("OTA Tamamlandi!");
        prevOTAPercent = -1;  // Reset
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(10, 70);
        tft.setTextColor(ST77XX_GREEN);
        tft.setTextSize(2);
        tft.println("OTA Tamamlandi!");
        delay(2000);
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        int percent = (progress * 100) / total;
        
        // Progress bar genişliği (çerçeve içinde)
        int barWidth = TFT_WIDTH - 22;
        int filledWidth = (progress * barWidth) / total;
        
        // Sadece yeni eklenen kısmı çiz (kırpma önleme)
        if (prevOTAPercent != percent) {
          int prevFilledWidth = (prevOTAPercent >= 0) ? (prevOTAPercent * barWidth) / 100 : 0;
          
          // Sadece yeni eklenen kısmı yeşil yap
          if (filledWidth > prevFilledWidth) {
            tft.fillRect(11 + prevFilledWidth, 71, filledWidth - prevFilledWidth, 23, ST77XX_GREEN);
          }
          
          // Yüzde bilgisini sadece değiştiğinde güncelle
          String percentStr = String(percent) + "% tamamlandi";
          tft.setTextSize(2);
          tft.setTextColor(ST77XX_WHITE);
          
          int16_t x1, y1;
          uint16_t w, h;
          tft.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
          
          int x = (TFT_WIDTH - w) / 2;
          int y = 105;
          
          tft.fillRect(x - 2, y - 2, w + 4, h + 4, ST77XX_BLACK);
          tft.setCursor(x, y);
          tft.println(percentStr);
          
          prevOTAPercent = percent;
        }
      })
      .onError([](ota_error_t error) {
        Serial.printf("Hata[%u]\n", error);
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(10, 70);
        tft.setTextColor(ST77XX_RED);
        tft.setTextSize(2);
        tft.print("OTA HATASI: ");
        tft.println(error);
      });

  ArduinoOTA.begin();
  Serial.println("OTA Hazir!");
}

// =======================================================
// 🟦 VERSİYON YAZISI (SAĞ-ALT)
// =======================================================
void drawVersionText() {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  tft.getTextBounds(VERSION_TEXT, 0, 0, &x1, &y1, &w, &h);

  int x = TFT_WIDTH - w - 6;
  int y = TFT_HEIGHT - h - 6;

  tft.fillRect(x, y, w + 2, h + 2, ST77XX_BLACK);
  tft.setCursor(x, y);
  tft.println(VERSION_TEXT);
}

// =======================================================
//  🟦 AÇILIŞ LOGOSU
// =======================================================
void showSplashScreen() {
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  int x = (tft.width() - 170) / 2;
  int y = (tft.height() - 172) / 2;

  tft.drawRGBBitmap(x, y, epd_bitmap_SP, 170, 172);
  delay(3000);

  tft.fillScreen(ST77XX_BLACK);
}

// =======================================================
void drawTimeAndDate(struct tm &timeinfo) {
  char bufDate[32];
  char bufTime[16];
  strftime(bufDate, sizeof(bufDate), "%d.%m.%Y", &timeinfo);
  strftime(bufTime, sizeof(bufTime), "%H:%M:%S", &timeinfo);

  String curTime = String(bufTime);
  String curDate = String(bufDate);

  int16_t x1, y1;
  uint16_t w, h;

  // Tarih
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.getTextBounds(bufDate, 0, 0, &x1, &y1, &w, &h);

  int dateX = (TFT_WIDTH - w) / 2;
  int dateY = 14;

  if (curDate != prevDate) {
    tft.fillRect(0, dateY, TFT_WIDTH, h + 4, ST77XX_BLACK);
    tft.setCursor(dateX, dateY);
    tft.println(curDate);
    prevDate = curDate;
  }

  // Saat
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.getTextBounds(bufTime, 0, 0, &x1, &y1, &w, &h);

  int timeX = (TFT_WIDTH - w) / 2;
  int timeY = dateY + h + 8;

  if (curTime != prevTime) {
    tft.fillRect(0, timeY, TFT_WIDTH, h + 6, ST77XX_BLACK);
    tft.setCursor(timeX, timeY);
    tft.println(curTime);
    prevTime = curTime;
  }
}

// =======================================================
// 🟦 MENÜ BUTONU
// =======================================================
void drawMenuButton() {
  // Menü butonu - alt ortada (yazı olarak)
  tft.setTextSize(3);  // TextSize 2'den 3'e çıkarıldı
  tft.setTextColor(ST77XX_CYAN);
  
  String menuText = "MENU";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(menuText, 0, 0, &x1, &y1, &w, &h);
  
  // Alt ortada konumlandır
  int x = (TFT_WIDTH - w) / 2;
  int y = TFT_HEIGHT - h - 8;
  
  // Önceki yazıyı temizle
  tft.fillRect(x - 2, y - 2, w + 4, h + 4, ST77XX_BLACK);
  
  // Menü yazısını çiz
  tft.setCursor(x, y);
  tft.println(menuText);
}

void drawLogo(const unsigned char *bitmap, int w, int h) {
  int x = TFT_WIDTH - w - 12;
  int y = 10;
  tft.fillRect(x, y, w, h, ST77XX_BLACK);
  tft.drawBitmap(x, y, bitmap, w, h, ST77XX_WHITE);
}

void updateLogoByRSSI() {
  int rssi = WiFi.RSSI();

  if (rssi >= -60)
    drawLogo(epd_bitmap_High, 17, 13);
  else if (rssi >= -80)
    drawLogo(epd_bitmap_Mid, 17, 13);
  else
    drawLogo(epd_bitmap_Low, 17, 13);
}

// =======================================================
void drawIPAddress() {

  if (ipAddress != "") {

    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    int16_t x1, y1;
    uint16_t w, h;

    tft.getTextBounds(ipAddress.c_str(), 0, 0, &x1, &y1, &w, &h);

    int x = 6;
    int y = TFT_HEIGHT - h - 6;  // Bir alt satıra indirildi

    tft.fillRect(x, y, w + 2, h + 4, ST77XX_BLACK);
    tft.setCursor(x, y);
    tft.println(ipAddress);
  }
}

// =======================================================
// 🟦 DHT11 SICAKLIK VE NEM GÖSTERİMİ
// =======================================================
void drawDHT11Data() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 okuma hatasi!");
    return;
  }

  // İstatistikleri güncelle
  if (temperature < minTemp) minTemp = temperature;
  if (temperature > maxTemp) maxTemp = temperature;
  if (humidity < minHum) minHum = humidity;
  if (humidity > maxHum) maxHum = humidity;
  sumTemp += temperature;
  sumHum += humidity;
  readingCount++;

  String tempStr = String(temperature, 1) + " C";
  String humStr = String(humidity, 1) + "%";

  // Sıcaklık güncellemesi - SOL ÜST
  if (tempStr != prevTemp) {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(tempStr, 0, 0, &x1, &y1, &w, &h);

    int x = 6;
    int y = 6;

    tft.fillRect(x, y, w + 4, h + 4, ST77XX_BLACK);
    tft.setCursor(x, y);
    tft.println(tempStr);
    prevTemp = tempStr;
  }

  // Nem güncellemesi - SOL ÜST (Sıcaklığın altında)
  if (humStr != prevHum) {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(humStr, 0, 0, &x1, &y1, &w, &h);

    int x = 6;
    int y = 20;  // Sıcaklığın hemen altında

    tft.fillRect(x, y, w + 4, h + 4, ST77XX_BLACK);
    tft.setCursor(x, y);
    tft.println(humStr);
    prevHum = humStr;
  }
}

// =======================================================
// 🟦 İSTATİSTİKLER SAYFASI
// =======================================================
void showStatisticsMenu(bool reset = false) {
  static bool firstDraw = true;
  static String prevUptimeStr = "";
  static String prevWifiUptimeStr = "";
  
  // Değişkenleri fonksiyonun başında tanımla
  int lineHeight = 18;
  int yPos = 35;
  
  if (reset) {
    firstDraw = true;
    prevUptimeStr = "";
    prevWifiUptimeStr = "";
    return;
  }
  
  if (firstDraw) {
    tft.fillScreen(ST77XX_BLACK);
    firstDraw = false;
    
    // Başlık (ortalanmış)
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    String title = "ISTATISTIKLER";
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int titleX = (TFT_WIDTH - w) / 2;
    tft.setCursor(titleX, 10);
    tft.println(title);
    
    // İlk çizimde tüm verileri göster
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    
    yPos = 35;  // Başlangıç pozisyonu
    
    // Ortalama Sıcaklık
    if (readingCount > 0) {
      float avgTemp = sumTemp / readingCount;
      String avgTempStr = "Ort. Sicaklik: " + String(avgTemp, 1) + " C";
      tft.setCursor(10, yPos);
      tft.println(avgTempStr);
      yPos += lineHeight;
      
      // Maksimum/Minimum Sıcaklık
      String tempRangeStr = "Sicaklik: " + String(minTemp, 1) + " / " + String(maxTemp, 1) + " C";
      tft.setCursor(10, yPos);
      tft.println(tempRangeStr);
      yPos += lineHeight;
      
      // Ortalama Nem
      float avgHum = sumHum / readingCount;
      String avgHumStr = "Ort. Nem: " + String(avgHum, 1) + " %";
      tft.setCursor(10, yPos);
      tft.println(avgHumStr);
      yPos += lineHeight;
      
      // Maksimum/Minimum Nem
      String humRangeStr = "Nem: " + String(minHum, 1) + " / " + String(maxHum, 1) + " %";
      tft.setCursor(10, yPos);
      tft.println(humRangeStr);
      yPos += lineHeight;
    } else {
      String noDataStr = "Henuz veri yok";
      tft.setCursor(10, yPos);
      tft.println(noDataStr);
      yPos += lineHeight * 2;
    }
    
    // Talimat (ortalanmış)
    tft.setTextColor(ST77XX_CYAN);
    String instructionText = "Buton ile geri don";
    tft.getTextBounds(instructionText, 0, 0, &x1, &y1, &w, &h);
    int instX = (TFT_WIDTH - w) / 2;
    tft.setCursor(instX, 155);
    tft.println(instructionText);
    
    // İlk çizimde süreleri de göster
    prevUptimeStr = "";
    prevWifiUptimeStr = "";
  }
  
  // Sadece süreleri güncelle (canlı)
  // İstatistik verilerinin yüksekliğini hesapla
  yPos = 35;  // Başlangıç pozisyonu
  if (readingCount > 0) {
    yPos += lineHeight * 4;  // 4 satır istatistik
  } else {
    yPos += lineHeight * 2;  // "Henuz veri yok" mesajı
  }
  
  // Çalışma Süresi (Uptime) - CANLI GÜNCELLEME
  unsigned long uptimeSeconds = (millis() - systemStartTime) / 1000;
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  String uptimeStr = "Calisma Suresi: " + String(days) + "g " + String(hours) + "s " + String(minutes) + "d";
  
  if (uptimeStr != prevUptimeStr) {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
    tft.setCursor(10, yPos);
    tft.println(uptimeStr);
    prevUptimeStr = uptimeStr;
  }
  yPos += lineHeight;
  
  // WiFi Bağlantı Süresi - CANLI GÜNCELLEME
  String wifiUptimeStr;
  if (WiFi.status() == WL_CONNECTED && wifiWasConnected) {
    unsigned long wifiUptimeSeconds = (millis() - wifiConnectedTime) / 1000;
    unsigned long wifiDays = wifiUptimeSeconds / 86400;
    unsigned long wifiHours = (wifiUptimeSeconds % 86400) / 3600;
    unsigned long wifiMinutes = (wifiUptimeSeconds % 3600) / 60;
    wifiUptimeStr = "WiFi Baglanti: " + String(wifiDays) + "g " + String(wifiHours) + "s " + String(wifiMinutes) + "d";
    
    if (wifiUptimeStr != prevWifiUptimeStr) {
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_WHITE);
      tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
      tft.setCursor(10, yPos);
      tft.println(wifiUptimeStr);
      prevWifiUptimeStr = wifiUptimeStr;
    }
  } else {
    wifiUptimeStr = "WiFi Baglanti: BAGLI DEGIL";
    if (wifiUptimeStr != prevWifiUptimeStr) {
      tft.setTextSize(1);
      tft.setTextColor(ST77XX_RED);
      tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
      tft.setCursor(10, yPos);
      tft.println(wifiUptimeStr);
      prevWifiUptimeStr = wifiUptimeStr;
    }
  }
}

// =======================================================
// 🟦 SİSTEM BİLGİLERİ SAYFASI
// =======================================================
void showSystemInfoMenu(bool reset = false) {
  static bool firstDraw = true;
  static unsigned long lastUpdate = 0;
  
  if (reset) {
    firstDraw = true;
    lastUpdate = 0;
    return;
  }
  
  // 2 saniyede bir güncelle
  unsigned long now = millis();
  if (!firstDraw && (now - lastUpdate < 2000)) {
    return;
  }
  lastUpdate = now;
  
  if (firstDraw) {
    tft.fillScreen(ST77XX_BLACK);
    firstDraw = false;
  }
  
  // Başlık (ortalanmış)
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  String title = "SISTEM BILGILERI";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int titleX = (TFT_WIDTH - w) / 2;
  tft.fillRect(titleX - 5, 8, w + 10, h + 4, ST77XX_BLACK);
  tft.setCursor(titleX, 10);
  tft.println(title);
  
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  
  int lineHeight = 18;
  int yPos = 35;
  
  // CPU Frekansı (ESP32-C3 için)
  uint32_t cpuFreq = ESP.getCpuFreqMHz();
  String cpuStr = "CPU Frekans: " + String(cpuFreq) + " MHz";
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(cpuStr);
  yPos += lineHeight;
  
  // Bellek Kullanımı
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t totalHeap = ESP.getHeapSize();
  uint32_t usedHeap = totalHeap - freeHeap;
  float heapPercent = (float)usedHeap / totalHeap * 100.0;
  String heapStr = "Bellek: " + String(usedHeap / 1024) + "/" + String(totalHeap / 1024) + " KB (" + String(heapPercent, 1) + "%)";
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(heapStr);
  yPos += lineHeight;
  
  // Chip ID
  uint64_t chipid = ESP.getEfuseMac();
  String chipIdStr = "Chip ID: " + String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
  chipIdStr.toUpperCase();
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(chipIdStr);
  yPos += lineHeight;
  
  // Firmware Versiyonu
  String versionStr = "Firmware: " + String(VERSION_TEXT);
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(versionStr);
  yPos += lineHeight;
  
  // Uptime (Çalışma Süresi)
  unsigned long uptimeSeconds = (millis() - systemStartTime) / 1000;
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long minutes = (uptimeSeconds % 3600) / 60;
  String uptimeStr = "Uptime: " + String(days) + "g " + String(hours) + "s " + String(minutes) + "d";
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(uptimeStr);
  yPos += lineHeight;
  
  // WiFi Durumu
  String wifiStatus = "WiFi: ";
  if (WiFi.status() == WL_CONNECTED) {
    wifiStatus += "BAGLI";
    tft.setTextColor(ST77XX_GREEN);
  } else {
    wifiStatus += "BAGLI DEGIL";
    tft.setTextColor(ST77XX_RED);
  }
  tft.fillRect(10, yPos, TFT_WIDTH - 20, lineHeight, ST77XX_BLACK);
  tft.setCursor(10, yPos);
  tft.println(wifiStatus);
  tft.setTextColor(ST77XX_WHITE);
  
  // Talimat (ortalanmış)
  tft.setTextColor(ST77XX_CYAN);
  String instructionText = "Buton ile geri don";
  tft.getTextBounds(instructionText, 0, 0, &x1, &y1, &w, &h);
  int instX = (TFT_WIDTH - w) / 2;
  tft.fillRect(instX - 5, 155, w + 10, h + 4, ST77XX_BLACK);
  tft.setCursor(instX, 155);
  tft.println(instructionText);
}

// =======================================================
// 🟦 ENCODER INTERRUPT HANDLER
// =======================================================
void IRAM_ATTR encoderISR() {
  int CLKState = digitalRead(ENCODER_CLK);
  int DTState = digitalRead(ENCODER_DT);
  
  if (CLKState != lastCLKState) {
    if (DTState != CLKState) {
      encoderPosition++;  // Saat yönünde
    } else {
      encoderPosition--;  // Saat yönünün tersi
    }
  }
  lastCLKState = CLKState;
}

// =======================================================
// 🟦 ENCODER BAŞLATMA
// =======================================================
void initEncoder() {
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  
  lastCLKState = digitalRead(ENCODER_CLK);
  
  // Interrupt ayarları
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  
  Serial.println("Encoder baslatildi!");
}

// =======================================================
// 🟦 ENCODER BUTON KONTROLÜ
// =======================================================
void checkEncoderButton() {
  int buttonState = digitalRead(ENCODER_SW);
  
  if (buttonState == LOW && (millis() - lastButtonPress) > debounceDelay) {
    encoderButtonPressed = true;
    lastButtonPress = millis();
    Serial.println("Encoder buton basildi!");
  }
}

// =======================================================
// 🟦 PARLAKLIK AYARLAMA
// =======================================================
void setBrightness(int level) {
  if (level < brightnessMin) level = brightnessMin;
  if (level > brightnessMax) level = brightnessMax;
  brightness = level;
  
  if (USE_PWM) {
    // PWM kontrolü (çalışmıyorsa bu kısım atlanır)
    ledcWrite(TFT_BACKLIGHT, brightness);
  } else {
    // Basit açık/kapalı kontrolü
    // %50'nin üzerinde açık, altında kapalı
    if (brightness > (brightnessMin + brightnessMax) / 2) {
      digitalWrite(TFT_BACKLIGHT, HIGH);  // Açık
    } else {
      digitalWrite(TFT_BACKLIGHT, LOW);   // Kapalı
    }
  }
  
  // Debug bilgisi (sadece değiştiğinde)
  static int lastReportedBrightness = -1;
  if (brightness != lastReportedBrightness) {
    Serial.print("Parlaklik: ");
    Serial.print(brightness);
    Serial.print(" (");
    Serial.print(map(brightness, brightnessMin, brightnessMax, 0, 100));
    Serial.print("%), Pin=GPIO");
    Serial.print(TFT_BACKLIGHT);
    if (USE_PWM) {
      Serial.print(", PWM=");
      Serial.print(brightness);
    }
    Serial.println();
    lastReportedBrightness = brightness;
  }
}

// =======================================================
// 🟦 PARLAKLIK AYAR MENÜSÜ
// =======================================================
void showBrightnessMenu(bool reset = false) {
  static int lastDrawnBrightness = -1;
  static bool firstDraw = true;
  
  if (reset) {
    firstDraw = true;
    lastDrawnBrightness = -1;
    return;
  }
  
  if (firstDraw) {
    tft.fillScreen(ST77XX_BLACK);
    
    // Başlık (ortalanmış)
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    String title = "PARLAKLIK";
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int titleX = (TFT_WIDTH - w) / 2;
    tft.setCursor(titleX, 10);
    tft.println(title);
    
    // Progress bar çerçevesi
    tft.drawRect(20, 60, TFT_WIDTH - 40, 20, ST77XX_WHITE);
    
    // Talimat (ortalanmış) - aynı değişkenleri kullan
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);
    String instructionText = "Encoder ile ayarla, buton ile geri don";
    tft.getTextBounds(instructionText, 0, 0, &x1, &y1, &w, &h);
    int instX = (TFT_WIDTH - w) / 2;
    tft.setCursor(instX, 140);
    tft.println(instructionText);
    
    firstDraw = false;
  }
  
  // Sadece parlaklık değiştiğinde güncelle
  if (brightness != lastDrawnBrightness) {
    // Progress bar doldurma
    int barWidth = TFT_WIDTH - 42;
    int filledWidth = map(brightness, brightnessMin, brightnessMax, 0, barWidth);
    
    // Önceki bar'ı tamamen temizle (her zaman)
    if (lastDrawnBrightness >= 0) {
      int prevFilledWidth = map(lastDrawnBrightness, brightnessMin, brightnessMax, 0, barWidth);
      // Tüm bar alanını temizle (artma veya azalma fark etmez)
      if (prevFilledWidth > 0) {
        tft.fillRect(21, 61, prevFilledWidth, 18, ST77XX_BLACK);
      }
    }
    
    // Yeni bar'ı çiz
    if (filledWidth > 0) {
      tft.fillRect(21, 61, filledWidth, 18, ST77XX_YELLOW);
    }
    
    // Yüzde gösterimi
    int percent = map(brightness, brightnessMin, brightnessMax, 0, 100);
    String percentStr = String(percent) + "%";
    
    tft.setTextSize(3);
    tft.setTextColor(ST77XX_WHITE);
    
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(percentStr, 0, 0, &x1, &y1, &w, &h);
    
    // Maksimum genişliği hesapla (%100 en geniş)
    String maxPercentStr = "100%";
    int16_t mx1, my1;
    uint16_t mw, mh;
    tft.getTextBounds(maxPercentStr, 0, 0, &mx1, &my1, &mw, &mh);
    
    int x = (TFT_WIDTH - w) / 2;
    int y = 100;
    
    // Önceki yüzdeyi temizle - maksimum genişlik kullan (iz kalmaması için)
    // Yüzde alanının tamamını temizle
    tft.fillRect((TFT_WIDTH - mw) / 2 - 5, y - 3, mw + 10, mh + 6, ST77XX_BLACK);
    
    // Yeni yüzdeyi çiz
    tft.setCursor(x, y);
    tft.println(percentStr);
    
    lastDrawnBrightness = brightness;
  }
}

// =======================================================
// 🟦 AYARLAR MENÜSÜ
// =======================================================
void showSettingsMenu() {
  tft.fillScreen(ST77XX_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, 10);
  tft.println("AYARLAR");
  
  String menuItems[] = {
    "Parlaklik",
    "WiFi Ayarlari",
    "Istatistikler",
    "Sistem Bilgileri",
    "Geri Don"
  };
  
  for (int i = 0; i < 5; i++) {
    if (i == menuItem) {
      // Seçili item: CYAN arka plan, WHITE text (ana sayfa uyumlu)
      tft.fillRect(15, 38 + (i * 25), TFT_WIDTH - 30, 22, ST77XX_CYAN);
      tft.setTextColor(ST77XX_BLACK);
    } else {
      tft.setTextColor(ST77XX_WHITE);
    }
    tft.setCursor(20, 40 + (i * 25));
    tft.println(menuItems[i]);
  }
}

// =======================================================
// 🟦 WIFI BİLGİLERİ SAYFASI
// =======================================================
void showWiFiInfoMenu(bool reset = false) {
  static bool firstDraw = true;
  static String prevSSID = "";
  static String prevIP = "";
  static int prevRSSI = -999;
  
  if (reset) {
    firstDraw = true;
    prevSSID = "";
    prevIP = "";
    prevRSSI = -999;
    return;
  }
  
  if (firstDraw) {
    tft.fillScreen(ST77XX_BLACK);
    
    // Başlık (ortalanmış)
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_CYAN);
    String title = "WIFI BILGILERI";
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    int titleX = (TFT_WIDTH - w) / 2;
    tft.setCursor(titleX, 10);
    tft.println(title);
    
    // Alt kısım - Geri dön talimatı (ortalanmış)
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);
    String backText = "Buton ile geri don";
    tft.getTextBounds(backText, 0, 0, &x1, &y1, &w, &h);
    int backX = (TFT_WIDTH - w) / 2;
    tft.setCursor(backX, TFT_HEIGHT - 15);
    tft.println(backText);
    
    firstDraw = false;
  }
  
  // WiFi durumu kontrolü
  if (WiFi.status() != WL_CONNECTED) {
    if (firstDraw || prevRSSI != -999) {
      tft.fillRect(10, 40, TFT_WIDTH - 20, 100, ST77XX_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_RED);
      String errorText = "BAGLI DEGIL!";
      int16_t x1, y1;
      uint16_t w, h;
      tft.getTextBounds(errorText, 0, 0, &x1, &y1, &w, &h);
      int errorX = (TFT_WIDTH - w) / 2;
      tft.setCursor(errorX, 50);
      tft.println(errorText);
      prevRSSI = -999;
    }
    return;
  }
  
  tft.setTextSize(1);
  int yPos = 45;
  int lineHeight = 22;  // Satır aralığı optimize edildi
  
  // SSID - Tam yazılsın (kısaltma kaldırıldı)
  String ssidStr = WiFi.SSID();
  if (ssidStr != prevSSID) {
    // Önceki SSID'yi temizle (daha geniş alan)
    tft.fillRect(10, yPos, TFT_WIDTH - 20, 18, ST77XX_BLACK);
    
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, yPos);
    tft.print("SSID: ");
    
    // SSID uzunluğunu kontrol et ve gerekirse alt satıra geç
    tft.setTextColor(ST77XX_YELLOW);
    int16_t x1, y1;
    uint16_t labelW, labelH;
    tft.getTextBounds("SSID: ", 0, 0, &x1, &y1, &labelW, &labelH);
    
    int ssidX = 10 + labelW;
    int maxWidth = TFT_WIDTH - ssidX - 10;
    
    // SSID'nin genişliğini kontrol et
    uint16_t ssidW, ssidH;
    tft.getTextBounds(ssidStr, 0, 0, &x1, &y1, &ssidW, &ssidH);
    
    if (ssidW > maxWidth) {
      // Uzun SSID - alt satıra geç
      tft.setCursor(10, yPos);
      tft.println("SSID:");
      tft.setCursor(10, yPos + lineHeight);
      tft.println(ssidStr);
      yPos += lineHeight;  // Ekstra satır eklendi
    } else {
      // Normal SSID - aynı satırda
      tft.setCursor(ssidX, yPos);
      tft.println(ssidStr);
    }
    prevSSID = ssidStr;
  }
  yPos += lineHeight;
  
  // IP Adresi
  String ipStr = WiFi.localIP().toString();
  if (ipStr != prevIP) {
    tft.fillRect(10, yPos, TFT_WIDTH - 20, 18, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, yPos);
    tft.print("IP: ");
    tft.setTextColor(ST77XX_CYAN);
    tft.println(ipStr);
    prevIP = ipStr;
  }
  yPos += lineHeight;
  
  // RSSI (Sinyal Gücü) - Sürekli güncellenir
  int rssi = WiFi.RSSI();
  if (rssi != prevRSSI) {
    tft.fillRect(10, yPos, TFT_WIDTH - 20, 18, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(10, yPos);
    tft.print("Sinyal: ");
    tft.setTextColor(ST77XX_YELLOW);
    String rssiStr = String(rssi) + " dBm";
    tft.print(rssiStr);
    
    // Sinyal seviyesi gösterimi
    if (rssi >= -60) {
      tft.setTextColor(ST77XX_GREEN);
      tft.println(" (Iyi)");
    } else if (rssi >= -80) {
      tft.setTextColor(ST77XX_YELLOW);
      tft.println(" (Orta)");
    } else {
      tft.setTextColor(ST77XX_RED);
      tft.println(" (Zayif)");
    }
    prevRSSI = rssi;
  }
}

// =======================================================
// 🟦 ENCODER İLE MENÜ KONTROLÜ
// =======================================================
void handleEncoderNavigation() {
  // Encoder pozisyon değişikliği kontrolü
  if (encoderPosition != lastEncoderPosition) {
    int diff = encoderPosition - lastEncoderPosition;
    
    if (currentMenuPage == 0) {
      // Ana sayfada encoder ile menü item seçimi yapılabilir
      // Şimdilik sadece log
      Serial.print("Encoder pozisyon: ");
      Serial.println(encoderPosition);
    } else if (currentMenuPage == 1) {
      // Ayarlar menüsünde item seçimi
      menuItem += diff;
      if (menuItem < 0) menuItem = 0;
      if (menuItem > 4) menuItem = 4;
      showSettingsMenu();
    } else if (currentMenuPage == 2) {
      // Parlaklık ayarı menüsünde
      brightness += diff * 5;  // Her adımda 5 artır/azalt
      if (brightness < brightnessMin) brightness = brightnessMin;
      if (brightness > brightnessMax) brightness = brightnessMax;
      setBrightness(brightness);
      showBrightnessMenu();
    }
    
    lastEncoderPosition = encoderPosition;
  }
  
  // Buton kontrolü
  checkEncoderButton();
  
  if (encoderButtonPressed) {
    encoderButtonPressed = false;
    
    if (currentMenuPage == 0) {
      // Ana sayfadan ayarlar menüsüne geç
      currentMenuPage = 1;
      menuItem = 0;
      showSettingsMenu();
    } else if (currentMenuPage == 1) {
      // Ayarlar menüsünde buton basıldı
      if (menuItem == 0) {
        // "Parlaklik" seçildi - parlaklık menüsüne geç
        currentMenuPage = 2;
        showBrightnessMenu(true);  // Reset
        showBrightnessMenu();      // İlk çizim
      } else if (menuItem == 1) {
        // "WiFi Ayarlari" seçildi - WiFi bilgileri sayfasına geç
        currentMenuPage = 3;
        showWiFiInfoMenu(true);  // Reset
        showWiFiInfoMenu();      // İlk çizim
      } else if (menuItem == 2) {
        // "Istatistikler" seçildi - İstatistikler sayfasına geç
        currentMenuPage = 4;
        showStatisticsMenu(true);  // Reset
        showStatisticsMenu();      // İlk çizim
      } else if (menuItem == 3) {
        // "Sistem Bilgileri" seçildi - Sistem bilgileri sayfasına geç
        currentMenuPage = 5;
        showSystemInfoMenu(true);  // Reset
        showSystemInfoMenu();      // İlk çizim
      } else if (menuItem == 4) {
        // "Geri Don" seçildi - ana sayfaya dön
        currentMenuPage = 0;
        tft.fillScreen(ST77XX_BLACK);
        
        // Önceki değerleri sıfırla ki tekrar çizilsin
        prevTime = "";
        prevDate = "";
        prevTemp = "";
        prevHum = "";
        
        // lastTimeUpdate'i sıfırla ki hemen güncellensin
        lastTimeUpdate = 0;
        
        // Ana sayfa çizimlerini hemen yeniden çiz
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
          drawTimeAndDate(timeinfo);
          updateLogoByRSSI();
          drawDHT11Data();
          drawIPAddress();
          drawMenuButton();     // 🔥 MENÜ BUTONU
          drawVersionText();
        }
      } else {
        // Diğer menü item'ları için (ileride fonksiyonellik eklenebilir)
        Serial.print("Menu item secildi: ");
        Serial.println(menuItem);
      }
    } else if (currentMenuPage == 2) {
      // Parlaklık menüsünden ayarlar menüsüne geri dön
      currentMenuPage = 1;
      menuItem = 0;
      // showBrightnessMenu() içindeki static değişkeni sıfırlamak için
      // fonksiyonu bir kez daha çağırmadan önce ekranı temizleyelim
      showSettingsMenu();
    } else if (currentMenuPage == 3) {
      // WiFi bilgileri sayfasından ayarlar menüsüne geri dön
      currentMenuPage = 1;
      menuItem = 1;  // WiFi Ayarlari seçili kalsın
      showWiFiInfoMenu(true);  // Reset
      showSettingsMenu();
    } else if (currentMenuPage == 4) {
      // İstatistikler sayfasından ayarlar menüsüne geri dön
      currentMenuPage = 1;
      menuItem = 2;  // Istatistikler seçili kalsın
      showStatisticsMenu(true);  // Reset
      showSettingsMenu();
    } else if (currentMenuPage == 5) {
      // Sistem bilgileri sayfasından ayarlar menüsüne geri dön
      currentMenuPage = 1;
      menuItem = 3;  // Sistem Bilgileri seçili kalsın
      showSystemInfoMenu(true);  // Reset
      showSettingsMenu();
    }
  }
}

// =======================================================
void updateTimeIfNeeded() {
  unsigned long now = millis();

  if (now - lastTimeUpdate >= timeInterval) {
    lastTimeUpdate = now;

    if (currentMenuPage == 0) {
      // Ana sayfa çizimleri
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        drawTimeAndDate(timeinfo);
        updateLogoByRSSI();
        drawDHT11Data();      // 🔥 DHT11 VERİLERİNİ GÖSTER
        drawIPAddress();
        drawMenuButton();     // 🔥 MENÜ BUTONU
        drawVersionText();   // 🔥 VERSİYON SÜREKLİ GÜNCELLENİR
      }
    } else if (currentMenuPage == 3) {
      // WiFi bilgileri sayfası - sinyal gücü güncellensin
      showWiFiInfoMenu();
    } else if (currentMenuPage == 4) {
      // İstatistikler sayfası - güncelle
      showStatisticsMenu();
    } else if (currentMenuPage == 5) {
      // Sistem bilgileri sayfası - güncelle
      showSystemInfoMenu();
    }
  }
}

// =======================================================
// 🟦 WIFI DURUM KONTROLÜ VE YENİDEN BAĞLANMA (Non-blocking)
// =======================================================
void checkWiFiConnection() {
  unsigned long now = millis();
  
  // Her 10 saniyede bir kontrol et (CPU'yu yormaz)
  if (now - lastWiFiCheck >= wifiCheckInterval) {
    lastWiFiCheck = now;
    
    // WiFi durumunu kontrol et
    if (WiFi.status() != WL_CONNECTED) {
      if (!wifiReconnecting) {
        wifiReconnecting = true;
        lastReconnectAttempt = 0;  // Hemen dene
        wifiWasConnected = false;  // Bağlantı koptu
        Serial.println("WiFi baglantisi koptu! Yeniden baglanma baslatiliyor...");
      }
      
      // 5 saniyede bir yeniden bağlanmayı dene (non-blocking)
      if (now - lastReconnectAttempt >= reconnectInterval) {
        lastReconnectAttempt = now;
        
        Serial.print("WiFi yeniden baglanma denemesi... ");
        
        // WiFi'yi durdur ve yeniden başlat (hızlı, blocking değil)
        WiFi.disconnect();
        delay(100);  // Kısa delay, blocking değil
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        Serial.println("baslatildi (non-blocking)");
      }
    } else {
      // Bağlantı var
      if (!wifiWasConnected) {
        // İlk bağlantı zamanını kaydet
        wifiConnectedTime = millis();
        wifiWasConnected = true;
      }
      if (wifiReconnecting) {
        wifiReconnecting = false;
        Serial.println("WiFi YENİDEN BAGLANDI!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        
        ipAddress = WiFi.localIP().toString();
        
        // WiFi bağlantı zamanını güncelle (yeniden bağlandı)
        wifiConnectedTime = millis();
        wifiWasConnected = true;
        
        // NTP'yi yeniden yapılandır (hızlı, blocking değil)
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        
        // Önceki değerleri sıfırla ki ekran yeniden çizilsin
        prevTime = "";
        prevDate = "";
      }
    }
  }
}

// =======================================================
void connectWiFiAndNTP() {

  Serial.print("Wifi baglaniyor: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nWiFi BAGLANDI!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  ipAddress = WiFi.localIP().toString();
  
  // WiFi bağlantı zamanını kaydet
  wifiConnectedTime = millis();
  wifiWasConnected = true;

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("NTP alinamadi...");
    delay(500);
  }
}

// =======================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  
  // Sistem başlangıç zamanını kaydet
  systemStartTime = millis();

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(TFT_HEIGHT, TFT_WIDTH);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // DHT11 başlatma
  dht.begin();
  Serial.println("DHT11 baslatildi!");

  // Backlight PWM başlatma
  Serial.print("Backlight pin'i ayarlaniyor: GPIO");
  Serial.println(TFT_BACKLIGHT);
  Serial.print("PWM Frekans: ");
  Serial.print(LEDC_FREQ);
  Serial.print(" Hz, Cozunurluk: ");
  Serial.print(LEDC_RESOLUTION);
  Serial.println(" bit");
  
  // Pin'i OUTPUT olarak ayarla
  pinMode(TFT_BACKLIGHT, OUTPUT);
  digitalWrite(TFT_BACKLIGHT, LOW);  // Önce LOW
  delay(10);
  
  if (USE_PWM) {
    // LEDC PWM başlat
    ledcAttach(TFT_BACKLIGHT, LEDC_FREQ, LEDC_RESOLUTION);
    delay(10);
    Serial.println("PWM modu aktif - LEDC baslatildi");
  } else {
    // Digital modu (açık/kapalı)
    Serial.println("Digital modu aktif (acik/kapali)");
  }
  
  // Varsayılan parlaklığa ayarla
  setBrightness(brightness);
  
  Serial.println("Backlight baslatildi!");

  // Encoder başlatma
  initEncoder();

  showSplashScreen();
  connectWiFiAndNTP();

  drawVersionText();  // Açılışta da yazılsın
  startOTA();
}

// =======================================================
void loop() {
  ArduinoOTA.handle();
  updateTimeIfNeeded();
  handleEncoderNavigation();  // 🔥 ENCODER KONTROLÜ
  checkWiFiConnection();      // 🔥 WIFI YENİDEN BAĞLANMA (Non-blocking, CPU dostu)
}
