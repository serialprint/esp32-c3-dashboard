# ESP32-C3 Smart Dashboard

<div align="center">

![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--C3-green.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

**Akıllı Dijital Dashboard Projesi**

ESP32-C3 Super Mini ile ST7789 TFT ekran kullanarak gerçek zamanlı veri gösterimi yapan bir dashboard projesi.

[Özellikler](#-özellikler) • [Kurulum](#-kurulum) • [Kullanım](#-kullanım) • [Donanım](#-donanım) • [Geliştirme](#-geliştirme)

</div>

---

## 📋 İçindekiler

- [Özellikler](#-özellikler)
- [Ekran Görüntüleri](#-ekran-görüntüleri)
- [Donanım Gereksinimleri](#-donanım-gereksinimleri)
- [Kurulum](#-kurulum)
- [Yapılandırma](#-yapılandırma)
- [Kullanım](#-kullanım)
- [Pin Bağlantıları](#-pin-bağlantıları)
- [Proje Yapısı](#-proje-yapısı)
- [Özellik Detayları](#-özellik-detayları)
- [Sorun Giderme](#-sorun-giderme)
- [Geliştirme](#-geliştirme)
- [Katkıda Bulunma](#-katkıda-bulunma)
- [Lisans](#-lisans)

---

## ✨ Özellikler

### 🎯 Ana Özellikler

- **📺 ST7789 TFT Ekran Desteği**
  - 320x172 piksel çözünürlük
  - Optimize edilmiş grafik gösterimi
  - Düşük güç tüketimi

- **🌡️ DHT11 Sensör Entegrasyonu**
  - Gerçek zamanlı sıcaklık ölçümü
  - Nem ölçümü
  - Otomatik veri güncelleme

- **📶 WiFi Bağlantısı**
  - Otomatik WiFi bağlantısı
  - Sinyal gücü gösterimi (RSSI)
  - IP adresi gösterimi
  - Dinamik WiFi ikonları

- **🕐 NTP Zaman Senkronizasyonu**
  - Otomatik zaman senkronizasyonu
  - GMT+3 zaman dilimi desteği
  - Tarih ve saat gösterimi

- **🔄 OTA (Over-The-Air) Güncelleme**
  - Kablosuz firmware güncelleme
  - İlerleme çubuğu ve yüzde gösterimi
  - Güvenli güncelleme mekanizması

- **🎨 Modern Arayüz**
  - Temiz ve okunabilir tasarım
  - Renkli bilgi gösterimi
  - Optimize edilmiş ekran güncellemeleri

### 📊 Ekranda Gösterilen Bilgiler

- **Tarih ve Saat** (Merkez, üst)
  - Tarih: Cyan renk, 2x boyut
  - Saat: Beyaz renk, 3x boyut

- **Sıcaklık** (Sol üst)
  - Beyaz renk, 1x boyut
  - Format: "XX.X C"

- **Nem** (Sol üst, sıcaklığın altında)
  - Beyaz renk, 1x boyut
  - Format: "XX.X%"

- **WiFi Sinyal Gücü** (Merkez)
  - Sarı renk, 2x boyut
  - Format: "XX dBm"
  - Dinamik sinyal ikonları

- **IP Adresi** (Sol alt)
  - Beyaz renk, 1x boyut

- **Versiyon Bilgisi** (Sağ alt)
  - Beyaz renk, 1x boyut
  - Format: "vX.X.X"

---

## 🖼️ Ekran Görüntüleri

### Ana Ekran
```
┌─────────────────────────────────┐
│  25.5 C                          │
│  60.0%                           │
│                                  │
│     01.01.2024                   │
│     14:30:45                     │
│                                  │
│        -65 dBm                   │
│                                  │
│                                  │
│ 192.168.1.100            v1.1.0  │
└─────────────────────────────────┘
```

### OTA Güncelleme Ekranı
```
┌─────────────────────────────────┐
│                                  │
│      OTA Yukleniyor             │
│                                  │
│  ┌─────────────────────────┐    │
│  │████████████░░░░░░░░░░░░░│    │
│  └─────────────────────────┘    │
│                                  │
│      45% tamamlandi              │
│                                  │
└─────────────────────────────────┘
```

---

## 🔧 Donanım Gereksinimleri

### Gerekli Bileşenler

| Bileşen | Miktar | Açıklama |
|---------|--------|----------|
| ESP32-C3 Super Mini | 1 | Ana mikrodenetleyici |
| ST7789 TFT Ekran | 1 | 320x172 piksel |
| DHT11 Sensör | 1 | Sıcaklık ve nem sensörü |
| 4.7kΩ - 10kΩ Direnç | 1 | DHT11 pull-up (modülde olabilir) |
| Jumper Kablolar | - | Bağlantı için |
| Breadboard | 1 | Prototipleme için (opsiyonel) |

### Önerilen Modüller

- **DHT11 Modülü**: Pull-up direnci dahil
- **ST7789 Modülü**: SPI bağlantılı

---

## 📦 Kurulum

### 1. PlatformIO Kurulumu

PlatformIO IDE veya PlatformIO Core kurulu olmalıdır.

**PlatformIO IDE:**
- [VS Code Extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide) kurun

**PlatformIO Core:**
```bash
pip install platformio
```

### 2. Projeyi Klonlayın

```bash
git clone https://github.com/kullaniciadi/esp32-c3-dashboard.git
cd esp32-c3-dashboard
```

### 3. Bağımlılıkları Yükleyin

PlatformIO otomatik olarak `platformio.ini` dosyasındaki kütüphaneleri yükleyecektir:

```bash
pio pkg install
```

### 4. Yapılandırma

`src/main.cpp` dosyasında WiFi bilgilerinizi güncelleyin:

```cpp
const char* ssid = "WiFi_Adiniz";
const char* password = "WiFi_Sifreniz";
```

### 5. Derleme ve Yükleme

```bash
# Projeyi derle
pio run

# ESP32'ye yükle
pio run -t upload

# Seri monitörü aç
pio device monitor
```

---

## ⚙️ Yapılandırma

### WiFi Ayarları

`src/main.cpp` dosyasında:

```cpp
const char* ssid = "WiFi_Adiniz";
const char* password = "WiFi_Sifreniz";
```

### NTP Ayarları

```cpp
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3 * 3600;  // GMT+3 için
const int daylightOffset_sec = 0;
```

### OTA Ayarları

```cpp
const char* otaName = "sp_dashboard";
const char* otaPass = "1234";  // Güvenlik için değiştirin!
```

### Pin Ayarları

```cpp
// TFT Pinleri
#define TFT_CS   10
#define TFT_DC    7
#define TFT_RST   5
#define TFT_SCLK  4
#define TFT_MOSI  6

// DHT11 Pin
#define DHT_PIN   2
```

---

## 🚀 Kullanım

### İlk Çalıştırma

1. Donanım bağlantılarını yapın (Pin Bağlantıları bölümüne bakın)
2. WiFi bilgilerini yapılandırın
3. Kodu derleyip ESP32'ye yükleyin
4. ESP32 açıldığında:
   - Splash ekranı gösterilir (3 saniye)
   - WiFi'ye bağlanır
   - NTP'den zaman alınır
   - Dashboard ekranı görüntülenir

### OTA Güncelleme

1. ESP32'yi WiFi'ye bağlayın
2. Arduino IDE veya PlatformIO'dan OTA yükleme seçeneğini kullanın
3. Hostname: `sp_dashboard`
4. Şifre: `1234` (yapılandırmada değiştirdiyseniz onu kullanın)
5. Güncelleme sırasında ekranda ilerleme görüntülenir

### Seri Monitör

```bash
pio device monitor
```

Seri monitörde şu bilgiler görüntülenir:
- WiFi bağlantı durumu
- IP adresi
- DHT11 okuma durumu
- OTA güncelleme durumu

---

## 🔌 Pin Bağlantıları

### ESP32-C3 Super Mini Pinout

```
ESP32-C3 Super Mini
┌─────────────────┐
│                 │
│  [USB-C]        │
│                 │
│  GPIO4  ────┐   │
│  GPIO5  ────┤   │
│  GPIO6  ────┤   │
│  GPIO7  ────┤   │
│  GPIO10 ────┤   │
│  GPIO2  ────┘   │
│                 │
└─────────────────┘
```

### ST7789 TFT Ekran Bağlantıları

| ST7789 | ESP32-C3 | Açıklama |
|--------|----------|----------|
| VCC | 3.3V | Güç |
| GND | GND | Toprak |
| CS | GPIO10 | Chip Select |
| DC | GPIO7 | Data/Command |
| RST | GPIO5 | Reset |
| SCLK | GPIO4 | SPI Clock |
| MOSI | GPIO6 | SPI Data |
| BLK | 3.3V | Backlight (opsiyonel) |

### DHT11 Bağlantıları

| DHT11 | ESP32-C3 | Açıklama |
|-------|----------|----------|
| VCC | 3.3V | Güç |
| GND | GND | Toprak |
| DATA | GPIO2 | Veri (4.7kΩ pull-up gerekli) |

**Not:** DHT11 modülü kullanıyorsanız pull-up direnci genellikle modülde mevcuttur.

### Bağlantı Şeması

```
ESP32-C3          ST7789          DHT11
─────────         ──────          ─────
3.3V      ──────── VCC
          ──────── BLK
GND       ──────── GND    ──────── GND
GPIO10    ──────── CS
GPIO7     ──────── DC
GPIO5     ──────── RST
GPIO4     ──────── SCLK
GPIO6     ──────── MOSI
GPIO2     ──────────────────────── DATA
3.3V      ──────────────────────── VCC
```

---

## 📁 Proje Yapısı

```
esp32-c3-dashboard/
│
├── src/
│   ├── main.cpp          # Ana program dosyası
│   └── logo.h           # Logo bitmap verileri
│
├── include/             # Header dosyaları (boş)
├── lib/                 # Kütüphaneler (boş)
├── test/                # Test dosyaları (boş)
│
├── platformio.ini       # PlatformIO yapılandırması
├── README.md            # Bu dosya
└── .gitignore           # Git ignore dosyası
```

### Dosya Açıklamaları

- **`src/main.cpp`**: Ana program kodu, tüm fonksiyonlar ve loop
- **`src/logo.h`**: WiFi sinyal ikonları ve splash ekran bitmap'leri
- **`platformio.ini`**: PlatformIO yapılandırması, kütüphaneler ve build ayarları

---

## 🔍 Özellik Detayları

### Ekran Güncelleme Optimizasyonu

Proje, ekran performansını optimize etmek için sadece değişen kısımları günceller:

- Tarih: Sadece tarih değiştiğinde güncellenir
- Saat: Her saniye güncellenir
- Sıcaklık/Nem: Değer değiştiğinde güncellenir
- WiFi Sinyal: Değer değiştiğinde güncellenir

### OTA Güncelleme Mekanizması

- Progress bar sadece yeni eklenen kısmı çizer (kırpma yok)
- Yüzde bilgisi gerçek zamanlı gösterilir
- Hata durumunda kırmızı hata mesajı

### WiFi Sinyal İkonları

Sinyal gücüne göre dinamik ikonlar:
- **Yüksek** (≥ -60 dBm): Tam sinyal ikonu
- **Orta** (-60 to -80 dBm): Orta sinyal ikonu
- **Düşük** (< -80 dBm): Düşük sinyal ikonu

---

## 🐛 Sorun Giderme

### WiFi Bağlanmıyor

**Sorun:** ESP32 WiFi'ye bağlanamıyor

**Çözümler:**
- WiFi SSID ve şifresini kontrol edin
- WiFi sinyal gücünü kontrol edin
- Serial Monitor'da hata mesajlarını kontrol edin
- ESP32'yi resetleyin

### DHT11 Okuma Hatası

**Sorun:** "DHT11 okuma hatasi!" mesajı

**Çözümler:**
- DHT11 bağlantılarını kontrol edin
- Pull-up direncinin (4.7kΩ-10kΩ) bağlı olduğundan emin olun
- DHT11'in 3.3V ile beslendiğinden emin olun
- GPIO2 pininin doğru bağlandığını kontrol edin
- DHT11'in 2 saniye aralıklarla okunması gerektiğini unutmayın

### Ekran Görüntülenmiyor

**Sorun:** ST7789 ekranı çalışmıyor

**Çözümler:**
- Tüm SPI bağlantılarını kontrol edin
- CS, DC, RST pinlerinin doğru bağlandığını kontrol edin
- Ekranın 3.3V ile beslendiğinden emin olun
- `platformio.ini`'deki pin tanımlarını kontrol edin

### OTA Güncelleme Çalışmıyor

**Sorun:** OTA güncelleme başlamıyor

**Çözümler:**
- ESP32'nin WiFi'ye bağlı olduğundan emin olun
- OTA hostname ve şifresini kontrol edin
- Aynı ağda olduğunuzdan emin olun
- Firewall ayarlarını kontrol edin

### NTP Zaman Alınamıyor

**Sorun:** Saat gösterilmiyor veya yanlış

**Çözümler:**
- WiFi bağlantısını kontrol edin
- NTP sunucusuna erişilebildiğinden emin olun
- GMT offset değerini kontrol edin
- İnternet bağlantısını kontrol edin

---

## 🛠️ Geliştirme

### Yeni Özellik Ekleme

1. **Yeni Sensör Ekleme:**
   ```cpp
   // Pin tanımı
   #define NEW_SENSOR_PIN 3
   
   // Okuma fonksiyonu
   void readNewSensor() {
     // Sensör okuma kodu
   }
   
   // Ekrana yazdırma
   void drawNewSensorData() {
     // Ekran çizim kodu
   }
   ```

2. **Yeni Ekran Modu:**
   - `updateTimeIfNeeded()` fonksiyonunu genişletin
   - Buton/encoder ile mod değiştirme ekleyin

3. **Web Sunucusu Ekleme:**
   - ESPAsyncWebServer kütüphanesini ekleyin
   - API endpoint'leri oluşturun

### Kod Yapısı

Ana fonksiyonlar:
- `setup()`: Başlangıç ayarları
- `loop()`: Ana döngü
- `drawTimeAndDate()`: Tarih/saat çizimi
- `drawDHT11Data()`: Sensör verileri
- `drawWiFiSignal()`: WiFi bilgileri
- `startOTA()`: OTA başlatma
- `updateTimeIfNeeded()`: Zamanlı güncellemeler

### Test Etme

```bash
# Kod derleme
pio run

# Test yükleme
pio run -t upload

# Seri monitör
pio device monitor
```

---

## 🤝 Katkıda Bulunma

Katkılarınızı bekliyoruz! Lütfen:

1. Fork yapın
2. Feature branch oluşturun (`git checkout -b feature/AmazingFeature`)
3. Değişikliklerinizi commit edin (`git commit -m 'Add some AmazingFeature'`)
4. Branch'inizi push edin (`git push origin feature/AmazingFeature`)
5. Pull Request açın

### Katkı Kuralları

- Kod standartlarına uyun
- Yorum satırlarını Türkçe veya İngilizce yazın
- Test edilmiş kod gönderin
- README'yi güncelleyin

---

## 📝 Versiyon Geçmişi

### v1.1.0 (Mevcut)
- ✅ DHT11 sensör desteği eklendi
- ✅ Sıcaklık ve nem gösterimi
- ✅ OTA ilerleme ekranı iyileştirildi
- ✅ Ekran optimizasyonları

### v1.0.3
- ✅ İlk stabil sürüm
- ✅ WiFi ve NTP desteği
- ✅ Temel dashboard arayüzü

---

## 📄 Lisans

Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için `LICENSE` dosyasına bakın.

---

## 👤 Yazar

**Kullanıcı Adı**
- GitHub: [@kullaniciadi](https://github.com/kullaniciadi)
- Instagram: [@kullaniciadi](https://instagram.com/kullaniciadi)

---

## 🙏 Teşekkürler

- [Adafruit](https://www.adafruit.com/) - GFX ve ST7789 kütüphaneleri
- [PlatformIO](https://platformio.org/) - Geliştirme ortamı
- [ESP32 Community](https://www.espressif.com/) - Desteği için

---

## 📞 İletişim

Sorularınız veya önerileriniz için:
- Issue açın: [GitHub Issues](https://github.com/kullaniciadi/esp32-c3-dashboard/issues)
- E-posta: your.email@example.com

---

<div align="center">

**⭐ Beğendiyseniz yıldız vermeyi unutmayın! ⭐**

Made with ❤️ by [Your Name]

</div>
