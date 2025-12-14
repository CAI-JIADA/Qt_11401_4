# 測試指南 - Qt 多平台行事曆整合工具

本文件說明如何測試 Apple Calendar (iCloud) 整合功能。

---

## 目錄

1. [環境需求](#環境需求)
2. [準備工作](#準備工作)
3. [建置步驟](#建置步驟)
4. [測試步驟](#測試步驟)
5. [常見問題排除](#常見問題排除)
6. [測試檢查清單](#測試檢查清單)

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

### Apple 帳號需求

- ✅ 有效的 Apple ID 帳號（免費）
- ✅ 已啟用雙重認證
- ❌ **不需要** Apple Developer Program ($99/年)
- ❌ **不需要** Mac 電腦

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
# - Qt SQL
# - CMake
```

#### Linux (Ubuntu/Debian)

```bash
# 安裝 Qt 6
sudo apt update
sudo apt install -y qt6-base-dev qt6-base-dev-tools cmake build-essential

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

### 2. 取得 Apple 應用程式專用密碼

這是測試的**關鍵步驟**！

#### 步驟：

1. **登入 Apple ID 帳號管理**
   - 前往 https://appleid.apple.com
   - 使用您的 Apple ID 登入

2. **進入安全性設定**
   - 在帳號頁面中，找到「安全性」(Security) 區段
   - 確認已啟用「雙重認證」(Two-Factor Authentication)
   - 如果尚未啟用，請先啟用雙重認證

3. **產生應用程式專用密碼**
   - 在「安全性」區段中，找到「應用程式專用密碼」(App-Specific Passwords)
   - 點選「產生密碼」(Generate Password)
   - 輸入密碼用途的標籤，例如：「Calendar Integration Test」
   - 系統會顯示一組格式為 `xxxx-xxxx-xxxx-xxxx` 的密碼
   - **重要**：立即複製並儲存這個密碼，因為它只會顯示一次

4. **記錄認證資訊**
   ```
   Apple ID: your-email@example.com
   應用程式專用密碼: abcd-efgh-ijkl-mnop
   ```

#### 注意事項

- ⚠️ 應用程式專用密碼與您的 Apple ID 密碼不同
- ⚠️ 絕對不要在程式中使用 Apple ID 密碼
- ✅ 應用程式專用密碼可以隨時撤銷
- ✅ 可以為不同的應用產生多個密碼

---

## 建置步驟

### 1. 克隆專案

```bash
# 克隆專案
git clone https://github.com/CAI-JIADA/Calender.git
cd Calender

# 切換到開發分支（如果需要）
git checkout copilot/integrate-apple-calendar-without-mac-again
```

### 2. 設定認證資訊

編輯 `src/main.cpp`，找到以下行並替換為您的資訊：

```cpp
// 第 29-30 行
QString appleId = "your-apple-id@example.com";  // 替換為您的 Apple ID
QString appPassword = "xxxx-xxxx-xxxx-xxxx";    // 替換為應用程式專用密碼
```

例如：

```cpp
QString appleId = "john.doe@icloud.com";
QString appPassword = "abcd-efgh-ijkl-mnop";
```

### 3. 建置專案

#### Windows

```powershell
# 建立建置目錄
mkdir build
cd build

# 執行 CMake（替換為您的 Qt 安裝路徑）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2019_64"

# 建置
cmake --build . --config Release

# 執行檔位置
cd Release
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

# 執行檔位置：./CalendarIntegration
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

# 執行檔位置：./CalendarIntegration
```

### 4. 執行程式

```bash
# Windows
.\CalendarIntegration.exe

# Linux/macOS
./CalendarIntegration
```

---

## 測試步驟

### 測試 1：基本連線測試

**目的**：驗證程式能否成功連接到 iCloud CalDAV 伺服器。

#### 步驟：

1. 執行程式：`./CalendarIntegration`

2. 觀察輸出，應該看到：
   ```
   === Qt 多平台行事曆整合工具 ===
   示範：Apple Calendar (iCloud) 整合 - 無需 Mac!
   
   資料庫已開啟: calendar.db
   資料庫表已建立
   已新增平台適配器
   開始連接 Apple Calendar (iCloud)...
   開始 Apple Calendar 認證...
   ```

3. 如果成功，會看到：
   ```
   ✅ Apple Calendar 認證成功！
   發現 X 個行事曆:
     - 行事曆名稱: URL
   ```

4. 如果失敗，會看到錯誤訊息並提示可能的原因

#### 預期結果：

- ✅ 成功連接到 caldav.icloud.com
- ✅ 成功通過 HTTP Basic 認證
- ✅ 成功列出使用者的行事曆

#### 可能的錯誤：

| 錯誤訊息 | 原因 | 解決方案 |
|---------|------|---------|
| `401 Unauthorized` | 密碼錯誤 | 確認使用應用程式專用密碼 |
| `SSL 憑證驗證失敗` | SSL 問題 | 確認系統時間正確，更新 OpenSSL |
| `Network timeout` | 網路問題 | 檢查網路連線，防火牆設定 |

### 測試 2：行事曆列表測試

**目的**：驗證程式能否正確解析 CalDAV PROPFIND 回應。

#### 步驟：

1. 執行程式後，檢查輸出的行事曆列表

2. 預期輸出範例：
   ```
   發現 3 個行事曆:
     - 工作: https://caldav.icloud.com/12345678/calendars/work/
     - 個人: https://caldav.icloud.com/12345678/calendars/personal/
     - 家庭: https://caldav.icloud.com/12345678/calendars/family/
   ```

3. 驗證：
   - 行事曆名稱是否正確
   - 行事曆 URL 是否完整
   - 數量是否符合您在 iCloud 中的行事曆數量

#### 預期結果：

- ✅ 列出所有 iCloud 行事曆
- ✅ 行事曆名稱正確（中文、英文都支援）
- ✅ URL 格式正確

### 測試 3：事件查詢測試

**目的**：驗證程式能否獲取並解析行事曆事件。

#### 準備：

在測試前，請在您的 iCloud Calendar 中建立一些測試事件：

1. 前往 https://www.icloud.com/calendar
2. 建立以下測試事件：
   - **事件 1**: 一般事件（有開始和結束時間）
     - 標題：「測試事件 1」
     - 時間：明天 10:00-11:00
     - 地點：「辦公室」
   
   - **事件 2**: 全天事件
     - 標題：「全天事件測試」
     - 時間：後天（勾選「全天」）
   
   - **事件 3**: 多天事件
     - 標題：「出差」
     - 時間：一週後，持續 3 天

#### 步驟：

1. 執行程式

2. 程式會自動查詢未來 30 天的事件

3. 觀察輸出：
   ```
   正在獲取事件 (2024-01-01 至 2024-01-31)...
   收到 3 個事件
   
   === 收到 3 個事件 ===
   Event: 測試事件 1 (2024-01-02T10:00:00 - 2024-01-02T11:00:00) at 辦公室 [非全天]
   Event: 全天事件測試 (2024-01-03T00:00:00 - 2024-01-03T23:59:59) at  [全天]
   Event: 出差 (2024-01-10T00:00:00 - 2024-01-12T23:59:59) at  [全天]
   
   事件已儲存到本地資料庫
   ```

#### 驗證項目：

- ✅ 事件標題正確
- ✅ 開始/結束時間正確
- ✅ 全天事件標記正確
- ✅ 地點資訊正確（如果有）
- ✅ 事件已儲存到本地資料庫

### 測試 4：本地資料庫測試

**目的**：驗證事件能否正確儲存到本地 SQLite 資料庫。

#### 步驟：

1. 執行程式並成功獲取事件後

2. 檢查資料庫檔案：
   ```bash
   # 應該在執行目錄中看到 calendar.db
   ls -lh calendar.db
   ```

3. 使用 SQLite 工具檢查內容：
   ```bash
   # 安裝 sqlite3（如果尚未安裝）
   # Ubuntu: sudo apt install sqlite3
   # macOS: brew install sqlite3
   # Windows: 下載自 https://www.sqlite.org/download.html
   
   # 查詢資料庫
   sqlite3 calendar.db
   ```

4. 在 SQLite prompt 中執行：
   ```sql
   -- 查看表結構
   .schema events
   
   -- 查詢所有事件
   SELECT id, title, start_time, is_all_day FROM events;
   
   -- 統計事件數量
   SELECT COUNT(*) FROM events;
   
   -- 退出
   .quit
   ```

#### 預期結果：

- ✅ calendar.db 檔案已建立
- ✅ events 表包含所有查詢到的事件
- ✅ 資料欄位完整且正確

### 測試 5：錯誤處理測試

**目的**：驗證程式的錯誤處理機制。

#### 測試 5.1：錯誤的認證資訊

1. 修改 `main.cpp` 中的密碼為錯誤值
2. 執行程式
3. 預期看到：
   ```
   ❌ 認證失敗: PROPFIND 錯誤: Error transferring ... - server replied: Unauthorized
   
   請檢查：
   1. Apple ID 格式正確（email 格式）
   2. 使用的是「應用程式專用密碼」而非 Apple ID 密碼
   3. Apple ID 已啟用雙重認證
   4. 網路連線正常
   ```

#### 測試 5.2：網路中斷

1. 執行程式前先中斷網路連線
2. 執行程式
3. 預期看到網路錯誤訊息

#### 測試 5.3：無效的 Apple ID 格式

1. 修改 `main.cpp` 中的 Apple ID 為無效格式（例如：沒有 @ 符號）
2. 執行程式
3. 預期看到：
   ```
   錯誤: 無效的 Apple ID 格式
   ```

#### 預期結果：

- ✅ 錯誤訊息清晰明確
- ✅ 提供除錯建議
- ✅ 程式不會崩潰

---

## 常見問題排除

### Q1: 編譯錯誤 - 找不到 Qt 模組

**錯誤訊息**：
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**解決方案**：
```bash
# 確認 Qt 已正確安裝
qmake --version  # 或 qmake6 --version

# 設定 CMAKE_PREFIX_PATH
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6

# Windows 範例
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2019_64"

# Linux 範例
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/qt6

# macOS 範例
cmake .. -DCMAKE_PREFIX_PATH=/usr/local/opt/qt@6
```

### Q2: 執行時錯誤 - 找不到 Qt 函式庫

**錯誤訊息** (Windows)：
```
無法啟動此程式，因為您的電腦遺失 Qt6Core.dll
```

**解決方案**：

#### Windows
```powershell
# 方法 1: 使用 windeployqt
cd build/Release
windeployqt CalendarIntegration.exe

# 方法 2: 手動複製 DLL
# 從 C:/Qt/6.5.3/msvc2019_64/bin/ 複製以下檔案到執行檔目錄：
# - Qt6Core.dll
# - Qt6Network.dll
# - Qt6Sql.dll
```

#### Linux
```bash
# 設定 LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
./CalendarIntegration
```

#### macOS
```bash
# 使用 macdeployqt
macdeployqt CalendarIntegration.app

# 或設定 DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=/usr/local/opt/qt@6/lib:$DYLD_LIBRARY_PATH
./CalendarIntegration
```

### Q3: 401 Unauthorized 錯誤

**症狀**：程式執行後顯示認證失敗。

**可能原因與解決方案**：

1. **使用了 Apple ID 密碼而非應用程式專用密碼**
   - ✅ 解決：重新產生應用程式專用密碼並使用

2. **應用程式專用密碼輸入錯誤**
   - ✅ 解決：仔細檢查密碼格式（xxxx-xxxx-xxxx-xxxx）

3. **Apple ID 尚未啟用雙重認證**
   - ✅ 解決：前往 appleid.apple.com 啟用雙重認證

4. **應用程式專用密碼已被撤銷**
   - ✅ 解決：重新產生新的密碼

### Q4: SSL 憑證驗證失敗

**錯誤訊息**：
```
SSL 錯誤: The certificate is not trusted
```

**解決方案**：

1. **更新系統憑證**
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt install ca-certificates
   sudo update-ca-certificates
   
   # Fedora/CentOS
   sudo yum update ca-certificates
   
   # macOS
   # 系統會自動管理憑證
   ```

2. **檢查系統時間**
   ```bash
   # 確認系統時間正確
   date
   
   # 如果不正確，同步時間
   # Linux
   sudo ntpdate time.nist.gov
   
   # Windows
   # 設定 > 時間與語言 > 日期與時間 > 立即同步
   ```

3. **更新 OpenSSL**
   ```bash
   # Ubuntu/Debian
   sudo apt update
   sudo apt upgrade openssl
   
   # macOS
   brew upgrade openssl
   ```

### Q5: 沒有列出任何行事曆

**症狀**：程式執行成功但顯示「發現 0 個行事曆」。

**解決方案**：

1. **確認 iCloud 帳號中有行事曆**
   - 前往 https://www.icloud.com/calendar
   - 確認至少有一個行事曆存在
   - 如果沒有，建立一個新的行事曆

2. **檢查行事曆權限**
   - 確認行事曆不是被隱藏或刪除的

3. **查看詳細除錯資訊**
   - 修改程式碼啟用除錯模式
   - 查看 PROPFIND 的原始回應

### Q6: 程式卡住不動

**症狀**：程式執行後沒有任何輸出或反應。

**可能原因與解決方案**：

1. **網路連線問題**
   - 檢查防火牆是否阻擋 443 埠
   - 確認可以存取 caldav.icloud.com

2. **等待時間不足**
   - 網路請求需要時間，等待至少 10-30 秒

3. **啟用除錯輸出**
   ```bash
   # 設定 Qt 除錯環境變數
   export QT_LOGGING_RULES="qt.network.*=true"
   ./CalendarIntegration
   ```

---

## 測試檢查清單

使用此檢查清單確保所有功能都已測試：

### 環境設定

- [ ] Qt 6 已安裝（`qmake --version`）
- [ ] CMake 已安裝（`cmake --version`）
- [ ] C++ 編譯器已安裝
- [ ] Apple ID 已啟用雙重認證
- [ ] 已產生應用程式專用密碼
- [ ] 已在 iCloud Calendar 中建立測試事件

### 建置測試

- [ ] CMake 設定成功
- [ ] 專案建置成功（無編譯錯誤）
- [ ] 執行檔已產生

### 功能測試

- [ ] **連線測試**：程式能成功連接 caldav.icloud.com
- [ ] **認證測試**：HTTP Basic 認證成功
- [ ] **行事曆列表**：能列出所有 iCloud 行事曆
- [ ] **事件查詢**：能獲取指定日期範圍的事件
- [ ] **事件解析**：事件資訊正確（標題、時間、地點）
- [ ] **全天事件**：全天事件正確識別
- [ ] **資料庫儲存**：事件成功儲存到 SQLite
- [ ] **中文支援**：中文事件標題正確顯示

### 錯誤處理測試

- [ ] **錯誤密碼**：顯示清晰的錯誤訊息
- [ ] **網路中斷**：正確處理網路錯誤
- [ ] **無效格式**：輸入驗證正常
- [ ] **SSL 錯誤**：安全性檢查正常

### 跨平台測試

- [ ] **Windows**：在 Windows 上測試成功
- [ ] **Linux**：在 Linux 上測試成功
- [ ] **macOS**：在 macOS 上測試成功（可選）

---

## 進階測試

### 效能測試

測試大量事件的處理效能：

1. 在 iCloud Calendar 中建立 100+ 個事件
2. 執行程式並測量：
   - 查詢時間
   - 解析時間
   - 資料庫寫入時間

### 並發測試

測試多個適配器同時運作：

```cpp
// 在 main.cpp 中同時使用多個平台
AppleCalendarAdapter* appleAdapter = new AppleCalendarAdapter(&manager);
GoogleCalendarAdapter* googleAdapter = new GoogleCalendarAdapter(&manager);

manager.addAdapter(appleAdapter);
manager.addAdapter(googleAdapter);
```

### 長時間執行測試

測試程式的穩定性：

```cpp
// 修改 main.cpp 中的計時器
// 原本: 30 秒
// 改為: 3600 秒 (1 小時)
QTimer::singleShot(3600000, &app, [&app]() {
    app.quit();
});
```

---

## 測試報告範本

測試完成後，請填寫此測試報告：

```
測試日期：____________________
測試人員：____________________
測試平台：□ Windows  □ Linux  □ macOS

### 測試結果

✅ = 通過  ❌ = 失敗  ⚠️ = 部分通過

| 測試項目 | 結果 | 備註 |
|---------|------|------|
| 環境設定 | ☐ | |
| 專案建置 | ☐ | |
| CalDAV 連線 | ☐ | |
| 行事曆列表 | ☐ | |
| 事件查詢 | ☐ | |
| 資料庫儲存 | ☐ | |
| 錯誤處理 | ☐ | |

### 發現的問題

1. 
2. 
3. 

### 建議改進

1. 
2. 
3. 
```

---

## 技術支援

如果遇到問題，請：

1. **檢查文件**：詳細閱讀 [IMPLEMENTATION.md](IMPLEMENTATION.md)
2. **查看 FAQ**：IMPLEMENTATION.md 中的常見問題章節
3. **啟用除錯**：使用 Qt 除錯工具獲取更多資訊
4. **提交 Issue**：在 GitHub 上提交詳細的問題報告

---

## 參考資源

- [IMPLEMENTATION.md](IMPLEMENTATION.md) - 完整實作文件
- [README.md](README.md) - 專案說明
- [Qt 6 文件](https://doc.qt.io/qt-6/)
- [CalDAV RFC 4791](https://tools.ietf.org/html/rfc4791)
- [Apple 應用程式專用密碼說明](https://support.apple.com/zh-tw/HT204397)

---

**最後更新**: 2024 年 12 月

**版本**: 1.0.0
