# 測試指南 - Qt 多平台行事曆整合工具

本文件說明如何測試 Google Calendar 和 Microsoft Outlook 整合功能。

---

## 目錄

1. [環境需求](#環境需求)
2. [準備工作](#準備工作)
3. [建置步驟](#建置步驟)
4. [Google Calendar 測試步驟](#google-calendar-測試步驟)
5. [Microsoft Outlook 測試步驟](#microsoft-outlook-測試步驟)
6. [UI 測試步驟](#ui-測試步驟)
7. [常見問題排除](#常見問題排除)
8. [測試檢查清單](#測試檢查清單)

---

## 環境需求

### 必要軟體

| 軟體 | 版本 | 用途 | 取得方式 |
|------|------|------|----------|
| Qt | 6.2+ | 開發框架 | https://www.qt.io/download |
| CMake | 3.16+ | 建置系統 | https://cmake.org/download/ |
| C++ 編譯器 | C++17 | 編譯程式 | GCC 9+, Clang 10+, MSVC 2019+ |
| Git | 2.0+ | 版本控制 | https://git-scm.com/ |

### 支援平台

- ✅ Windows 10/11
- ✅ Linux (Ubuntu 20.04+, Fedora 35+, 其他主流發行版)
- ✅ macOS 11+

---

## 準備工作

### 1. 安裝 Qt 6

#### Windows

```powershell
# 下載 Qt Online Installer
# https://www.qt.io/download-qt-installer

# 安裝時選擇以下元件：
# - Qt 6.x for MSVC 2019 64-bit
# - Qt Network
# - Qt NetworkAuth
# - Qt Widgets
# - Qt SQL
# - CMake
```

#### Linux (Ubuntu/Debian)

```bash
# 安裝 Qt 6
sudo apt update
sudo apt install -y qt6-base-dev qt6-base-dev-tools qt6-networkauth-dev cmake build-essential

# 驗證安裝
qmake6 --version
cmake --version
```

#### macOS

```bash
# 使用 Homebrew
brew install qt@6 cmake

# 設定環境變數
echo 'export PATH="/usr/local/opt/qt@6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc

# 驗證安裝
qmake --version
```

---

## 建置步驟

### 1. 克隆專案

```bash
# 克隆專案
git clone https://github.com/CAI-JIADA/Qt_11401_4.git
cd Qt_11401_4
```

### 2. 建置專案

#### Windows

```powershell
# 建立建置目錄
mkdir build
cd build

# 執行 CMake（替換為您的 Qt 安裝路徑）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2019_64"

# 建置（Qt DLL 會自動部署到執行檔目錄）
cmake --build . --config Release

# 執行檔位置
cd Release
./CalendarIntegration.exe
```

#### Linux

```bash
# 建立建置目錄
mkdir build
cd build

# 執行 CMake
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/qt6

# 建置（使用多核心加速）
make -j$(nproc)

# 執行
./CalendarIntegration
```

#### macOS

```bash
# 建立建置目錄
mkdir build
cd build

# 執行 CMake
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@6

# 建置
make -j$(sysctl -n hw.ncpu)

# 執行
./CalendarIntegration
```

---

## Google Calendar 測試步驟

### 前置準備：設定 Google Cloud 專案

#### 步驟 1：建立 Google Cloud 專案

1. 前往 https://console.cloud.google.com/
2. 登入您的 Google 帳號
3. 點選「新增專案」
4. 輸入專案名稱：「Calendar Integration」
5. 點選「建立」

#### 步驟 2：啟用 Google Calendar API

1. 在左側選單中選擇「API 和服務」→「程式庫」
2. 搜尋「Google Calendar API」
3. 點選並啟用
4. 同樣啟用「Google Tasks API」（可選）

#### 步驟 3：建立 OAuth 2.0 憑證

1. 前往「API 和服務」→「憑證」
2. 點選「+ 建立憑證」→「OAuth 用戶端 ID」
3. 設定 OAuth 同意畫面（首次需要）
   - 選擇「外部」使用者類型
   - 填寫應用程式名稱和聯絡資訊
   - 新增範圍：`calendar.readonly` 和 `tasks.readonly`
   - 新增測試使用者（您的 Google 帳號）
4. 建立 OAuth 用戶端 ID
   - 類型：「桌面應用程式」
   - 名稱：「Calendar Integration Desktop」
5. 記錄 Client ID 和 Client Secret

#### 步驟 4：設定環境變數

設定環境變數以儲存 OAuth 憑證（不要直接寫在程式碼中）：

**Linux/macOS:**
```bash
export GOOGLE_CLIENT_ID="YOUR_CLIENT_ID.apps.googleusercontent.com"
export GOOGLE_CLIENT_SECRET="YOUR_CLIENT_SECRET"
```

**Windows (PowerShell):**
```powershell
$env:GOOGLE_CLIENT_ID="YOUR_CLIENT_ID.apps.googleusercontent.com"
$env:GOOGLE_CLIENT_SECRET="YOUR_CLIENT_SECRET"
```

**Windows (CMD):**
```cmd
set GOOGLE_CLIENT_ID=YOUR_CLIENT_ID.apps.googleusercontent.com
set GOOGLE_CLIENT_SECRET=YOUR_CLIENT_SECRET
```

將 `YOUR_CLIENT_ID` 和 `YOUR_CLIENT_SECRET` 替換為您在步驟 3 中取得的實際憑證。

**永久設定（可選）：**

Linux/macOS：將 export 指令加入 `~/.bashrc` 或 `~/.zshrc`

Windows：在「系統內容」→「環境變數」中設定

#### 步驟 5：重新建置並執行

#### 測試 1：OAuth 認證

1. 執行程式
2. 點選「連接 Google Calendar」
3. 瀏覽器會自動開啟 Google 授權頁面
4. 登入並點選「允許」
5. 確認程式顯示「✓ Google Calendar 已連接」

**預期結果：**
- ✅ 瀏覽器成功開啟授權頁面
- ✅ 認證成功，按鈕變為綠色
- ✅ 「獲取事件」按鈕變為可用

#### 測試 2：獲取事件

1. 在 Google Calendar 建立幾個測試事件
2. 在程式中點選「獲取事件」
3. 確認事件列表顯示 Google 事件（藍色）
4. 點選事件查看詳情

**預期結果：**
- ✅ 成功獲取事件
- ✅ 事件資訊正確
- ✅ 事件以藍色顯示

---

## Microsoft Outlook 測試步驟

### 前置準備：設定 Azure AD 應用程式

#### 步驟 1：註冊應用程式

1. 前往 https://portal.azure.com/
2. 搜尋「Azure Active Directory」
3. 選擇「應用程式註冊」→「+ 新增註冊」
4. 填寫資訊：
   - 名稱：「Calendar Integration」
   - 帳戶類型：「任何組織目錄及個人帳戶」
   - 重新導向 URI：`http://localhost:8081`
5. 記錄 Application ID 和 Directory ID

#### 步驟 2：建立密碼

1. 選擇「憑證及祕密」→「+ 新增用戶端密碼」
2. 設定描述和到期時間
3. 複製並保存密碼值（只顯示一次）

#### 步驟 3：設定權限

1. 選擇「API 權限」→「+ 新增權限」
2. 選擇「Microsoft Graph」→「委派的權限」
3. 新增權限：
   - `Calendars.Read`
   - `Calendars.Read.Shared`
   - `Tasks.Read`
   - `User.Read`

#### 步驟 4：設定環境變數

設定環境變數以儲存 Azure AD OAuth 憑證：

**Linux/macOS:**
```bash
export OUTLOOK_CLIENT_ID="YOUR_APPLICATION_CLIENT_ID"
export OUTLOOK_CLIENT_SECRET="YOUR_CLIENT_SECRET"
```

**Windows (PowerShell):**
```powershell
$env:OUTLOOK_CLIENT_ID="YOUR_APPLICATION_CLIENT_ID"
$env:OUTLOOK_CLIENT_SECRET="YOUR_CLIENT_SECRET"
```

**Windows (CMD):**
```cmd
set OUTLOOK_CLIENT_ID=YOUR_APPLICATION_CLIENT_ID
set OUTLOOK_CLIENT_SECRET=YOUR_CLIENT_SECRET
```

將憑證替換為您在步驟 1 和步驟 2 中取得的實際值。

#### 步驟 5：重新建置並執行

#### 測試 1：OAuth 認證

1. 執行程式
2. 點選「連接 Microsoft Outlook」
3. 瀏覽器會開啟 Microsoft 授權頁面
4. 登入並點選「接受」
5. 確認程式顯示「✓ Outlook 已連接」

**預期結果：**
- ✅ 瀏覽器成功開啟授權頁面
- ✅ 認證成功，按鈕變為藍色
- ✅ 「獲取事件」按鈕變為可用

#### 測試 2：獲取事件

1. 在 Outlook Calendar 建立測試事件
2. 在程式中點選「獲取事件」
3. 確認事件列表顯示 Outlook 事件（深藍色）

**預期結果：**
- ✅ 成功獲取事件
- ✅ 事件以深藍色顯示

---

## UI 測試步驟

### 測試 1：視窗佈局

- 檢查主視窗大小：1200x800
- 檢查三欄佈局：左側（帳號）、中間（列表）、右側（詳情）
- 檢查選單列和狀態列

**預期結果：**
- ✅ 所有 UI 元件正確顯示
- ✅ 佈局合理美觀

### 測試 2：互動功能

- 測試搜尋功能：即時過濾事件
- 測試平台篩選：切換 Google/Outlook/全部
- 測試日期選擇器：選擇日期範圍
- 測試事件詳情：點選事件顯示詳情

**預期結果：**
- ✅ 所有互動正常
- ✅ 回饋及時

### 測試 3：雙平台整合

1. 連接 Google 和 Outlook
2. 獲取事件
3. 使用平台篩選切換顯示

**預期結果：**
- ✅ 可同時使用兩個平台
- ✅ 事件混合顯示
- ✅ 平台篩選正確

---

## 常見問題排除

### Q1: 找不到 Qt NetworkAuth

**解決方案：**
```bash
# Linux
sudo apt install qt6-networkauth-dev

# macOS
brew reinstall qt@6

# Windows
# 使用 Qt Maintenance Tool 安裝
```

### Q2: OAuth 回調失敗

**原因：** 埠被佔用或防火牆阻擋

**解決方案：**
- 確認埠 8080 (Google) 和 8081 (Outlook) 可用
- 檢查防火牆設定
- 確認回調 URI 設定正確

### Q3: 無法獲取事件

**可能原因：**
- Access Token 過期
- API 權限不足
- 網路連線問題

**解決方案：**
- 重新認證
- 檢查 API 權限
- 查看除錯輸出

---

## 測試檢查清單

### 環境設定
- [ ] Qt 6 已安裝（含 NetworkAuth 模組）
- [ ] CMake 已安裝
- [ ] 已取得 Google OAuth 憑證
- [ ] 已註冊 Azure AD 應用程式

### 建置測試
- [ ] CMake 設定成功
- [ ] 專案建置成功
- [ ] 執行檔正常啟動

### Google Calendar
- [ ] OAuth 認證成功
- [ ] 獲取事件成功
- [ ] 事件資訊正確
- [ ] 搜尋功能正常

### Microsoft Outlook
- [ ] OAuth 認證成功
- [ ] 獲取事件成功
- [ ] 事件資訊正確

### UI 測試
- [ ] 佈局正確
- [ ] 互動正常
- [ ] 雙平台整合正常
- [ ] 狀態更新正確

---

## 參考資源

- [Google Calendar API](https://developers.google.com/calendar/api)
- [Microsoft Graph API](https://learn.microsoft.com/en-us/graph/api/resources/calendar)
- [Qt NetworkAuth](https://doc.qt.io/qt-6/qtnetworkauth-index.html)

---

**最後更新**: 2024 年 12 月

**版本**: 2.0.0

**注意**：Apple Calendar 整合已完成但目前因設備問題暫時停用。
