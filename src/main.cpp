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

#define VERSION_TEXT "v1.1.0"   // 🔥 SAG ALTA GÖRÜNECEK VERSİYON

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

#define TFT_WIDTH  320
#define TFT_HEIGHT 172

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
DHT dht(DHT_PIN, DHT11);

// =====================================================
unsigned long lastTimeUpdate = 0;
const unsigned long timeInterval = 1000;

String prevTime = "";
String prevDate = "";
String prevRSSI = "";
String prevTemp = "";
String prevHum = "";
String ipAddress = "";
int prevOTAPercent = -1;  // OTA progress takibi için

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
void drawWiFiSignal() {
  int rssi = WiFi.RSSI();
  String rssiStr = String(rssi) + " dBm";

  if (rssiStr != prevRSSI) {

    tft.setTextSize(2);
    tft.setTextColor(ST77XX_YELLOW);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(rssiStr, 0, 0, &x1, &y1, &w, &h);

    int x = (TFT_WIDTH - w) / 2;
    int y = (TFT_HEIGHT / 2) + 20;

    tft.fillRect(0, y, TFT_WIDTH, h + 6, ST77XX_BLACK);
    tft.setCursor(x, y);
    tft.println(rssiStr);

    prevRSSI = rssiStr;
  }
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
void updateTimeIfNeeded() {
  unsigned long now = millis();

  if (now - lastTimeUpdate >= timeInterval) {
    lastTimeUpdate = now;

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      drawTimeAndDate(timeinfo);
      drawWiFiSignal();
      updateLogoByRSSI();
      drawDHT11Data();      // 🔥 DHT11 VERİLERİNİ GÖSTER
      drawIPAddress();
      drawVersionText();   // 🔥 VERSİYON SÜREKLİ GÜNCELLENİR
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

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(TFT_HEIGHT, TFT_WIDTH);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // DHT11 başlatma
  dht.begin();
  Serial.println("DHT11 baslatildi!");

  showSplashScreen();
  connectWiFiAndNTP();

  drawVersionText();  // Açılışta da yazılsın
  startOTA();
}

// =======================================================
void loop() {
  ArduinoOTA.handle();
  updateTimeIfNeeded();
}
