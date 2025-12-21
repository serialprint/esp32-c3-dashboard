# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2024-12-XX

### Added
- 🔄 Rotary encoder desteği eklendi
- 📱 Menü sistemi eklendi (Ayarlar menüsü)
- 💡 Parlaklık kontrolü (PWM) eklendi
  - Encoder ile parlaklık ayarı (0-100%)
  - Progress bar ile görsel gösterim
  - Gerçek zamanlı PWM kontrolü
- 📶 WiFi bilgileri sayfası eklendi
  - SSID gösterimi
  - IP adresi gösterimi
  - Sinyal gücü (RSSI) gösterimi ve seviye bilgisi
  - Otomatik güncelleme
- 🔁 Otomatik WiFi yeniden bağlanma
  - Bağlantı koptuğunda otomatik yeniden bağlanma
  - Non-blocking, CPU dostu
  - 10 saniyede bir durum kontrolü
- 🎨 Menü butonu eklendi (ana sayfada)
  - Alt ortada konumlandırılmış
  - Kullanıcı dostu görünüm

### Changed
- 🎨 Menü renkleri ana sayfa ile uyumlu hale getirildi
  - CYAN başlıklar
  - WHITE normal text
  - YELLOW vurgular
- 📱 WiFi sinyal gücü ana sayfadan kaldırıldı
  - WiFi bilgileri artık menüde gösteriliyor
- 🎯 UI iyileştirmeleri
  - Tüm başlıklar ortalandı
  - "Geri dön" yazıları ortalandı
  - Progress bar temizleme optimizasyonu
  - SSID tam görüntüleme (kısaltma kaldırıldı)

### Fixed
- 🔧 Progress bar iz kalma sorunu düzeltildi
- 🔧 WiFi bilgileri sayfasında yazı boyutu optimize edildi
- 🔧 Menü sayfaları arası geçiş sorunları düzeltildi
- 🔧 Parlaklık menüsünde yüzde gösterimi iz kalma sorunu düzeltildi

## [1.1.0] - 2024-01-XX

### Added
- DHT11 sensör desteği eklendi
- Sıcaklık ve nem gösterimi
- OTA ilerleme ekranı iyileştirildi
- Progress bar optimizasyonu (kırpma önlendi)
- Yüzde bilgisi gösterimi

### Changed
- Ekran güncelleme optimizasyonları
- Sıcaklık ve nem konumları güncellendi
- IP adresi konumu değiştirildi
- Renk şemaları güncellendi

### Fixed
- OTA progress bar kırpma sorunu düzeltildi
- Derece simgesi görüntüleme sorunu düzeltildi

## [1.0.3] - 2024-01-XX

### Added
- İlk stabil sürüm
- WiFi bağlantısı
- NTP zaman senkronizasyonu
- ST7789 TFT ekran desteği
- OTA güncelleme desteği
- Temel dashboard arayüzü
- WiFi sinyal gücü gösterimi
- Dinamik WiFi ikonları
- Versiyon bilgisi gösterimi

---

[1.2.0]: https://github.com/serialprint/esp32-c3-dashboard/releases/tag/v1.2.0
[1.1.0]: https://github.com/serialprint/esp32-c3-dashboard/releases/tag/v1.1.0
[1.0.3]: https://github.com/serialprint/esp32-c3-dashboard/releases/tag/v1.0.3

