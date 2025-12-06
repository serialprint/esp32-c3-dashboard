# GitHub'a Yükleme Rehberi

## Yöntem 1: GitHub CLI ile (Önerilen)

### 1. GitHub CLI Kurulumu
```bash
# Windows için (PowerShell)
winget install GitHub.cli
```

### 2. GitHub'a Giriş
```bash
gh auth login
```

### 3. Repository Oluştur ve Yükle
```bash
# Proje klasörüne git
cd C:\Users\rcp_u\Documents\PlatformIO\Projects\ST7789

# Git başlat
git init

# Dosyaları ekle
git add .

# İlk commit
git commit -m "Initial commit: ESP32-C3 Smart Dashboard v1.1.0"

# GitHub'da repository oluştur ve yükle
gh repo create esp32-c3-dashboard --public --source=. --remote=origin --push
```

---

## Yöntem 2: Manuel Git Komutları

### 1. Git Kurulumu
Git kurulu değilse: https://git-scm.com/download/win

### 2. GitHub'da Repository Oluştur
1. GitHub.com'a git
2. "New repository" butonuna tıkla
3. Repository adı: `esp32-c3-dashboard`
4. Public seç
5. "Create repository" tıkla

### 3. Terminal Komutları
```bash
# Proje klasörüne git
cd C:\Users\rcp_u\Documents\PlatformIO\Projects\ST7789

# Git başlat
git init

# Dosyaları ekle
git add .

# İlk commit
git commit -m "Initial commit: ESP32-C3 Smart Dashboard v1.1.0"

# GitHub repository'yi ekle (URL'yi kendi repository'nizle değiştirin)
git remote add origin https://github.com/KULLANICI_ADINIZ/esp32-c3-dashboard.git

# Ana branch'i ayarla
git branch -M main

# Yükle
git push -u origin main
```

**Not:** İlk push'ta GitHub kullanıcı adı ve şifre isteyecek. Şifre yerine **Personal Access Token** kullanın.

---

## Yöntem 3: Personal Access Token (Güvenli)

### 1. Token Oluştur
1. GitHub.com → Settings → Developer settings
2. Personal access tokens → Tokens (classic)
3. "Generate new token" → "Generate new token (classic)"
4. İsim: "ESP32 Dashboard Upload"
5. Süre: 90 gün (veya istediğiniz)
6. İzinler: `repo` seçin
7. "Generate token" tıkla
8. **Token'ı kopyalayın** (bir daha gösterilmeyecek!)

### 2. Token ile Push
```bash
git push -u origin main
# Username: GitHub kullanıcı adınız
# Password: Oluşturduğunuz token'ı yapıştırın
```

---

## Yöntem 4: VS Code ile (En Kolay)

### 1. VS Code'da Git Extension
- VS Code'da projeyi açın
- Sol menüden Source Control (Ctrl+Shift+G)
- "Initialize Repository" tıklayın

### 2. Commit ve Push
1. Değişiklikleri stage'e alın (+ işareti)
2. Commit mesajı yazın: "Initial commit"
3. Commit butonuna tıklayın
4. "Publish Branch" butonuna tıklayın
5. GitHub'da repository oluşturun

---

## .gitignore Dosyası

Proje köküne `.gitignore` dosyası ekleyin:

```
.pio
.vscode
*.bin
*.elf
*.hex
.DS_Store
```

---

## İlk Yükleme Sonrası

Repository oluşturulduktan sonra README'deki şu bilgileri güncelleyin:

1. GitHub kullanıcı adınızı değiştirin
2. Repository URL'ini güncelleyin
3. İletişim bilgilerinizi ekleyin

---

## Sorun Giderme

### "Authentication failed" hatası
- Personal Access Token kullanın (şifre değil)
- Token'ın `repo` iznine sahip olduğundan emin olun

### "Repository not found" hatası
- Repository URL'ini kontrol edin
- Repository'nin public olduğundan emin olun

### "Permission denied" hatası
- GitHub hesabınızın repository'ye erişim izni olduğundan emin olun

