# 測試指南 - Qt 多平台行事曆整合工具

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
   - 注意：桌面應用程式類型會自動設定 `http://localhost` 作為授權重新導向 URI
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
   - 重新導向 URI：`http://localhost:8081/`（注意：必須包含結尾的斜線 `/`）
5. 記錄 Application ID 和 Directory ID

**重要提示**：重新導向 URI 必須設定為 `http://localhost:8081/`（包含結尾斜線），否則會出現 `invalid_request: The provided value for the input parameter 'redirect_uri' is not valid` 錯誤。

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

> 💡 **遇到 OAuth 認證錯誤？** 請查看詳細的 [**OAuth 認證問題排除指南 (OAUTH_TROUBLESHOOTING.md)**](./OAUTH_TROUBLESHOOTING.md)

### Q1: Google Calendar 顯示 "錯誤 400: redirect_uri_mismatch"

**症狀：** 當您嘗試登入 Google Calendar 時看到「已封鎖存取權：這個應用程式的要求無效」和「發生錯誤 400： redirect_uri_mismatch」

**快速解決方案：**
1. 前往 [Google Cloud Console 憑證頁面](https://console.cloud.google.com/apis/credentials)
2. 編輯您的 OAuth 2.0 用戶端 ID
3. 在「授權的重新導向 URI」中確保有：`http://localhost:8080/`（**必須包含結尾的 /** ）
4. 儲存並重新啟動應用程式

**詳細說明：** 請參閱 [OAUTH_TROUBLESHOOTING.md - Google Calendar 錯誤](./OAUTH_TROUBLESHOOTING.md)

---

### Q2: 找不到 Qt NetworkAuth

**解決方案：**
```bash
# Linux
sudo apt install qt6-networkauth-dev

# macOS
brew reinstall qt@6

# Windows
# 使用 Qt Maintenance Tool 安裝
```

### Q3: OAuth 回調失敗

**原因：** 埠被佔用或防火牆阻擋

**解決方案：**
- 確認埠 8080 (Google) 和 8081 (Outlook) 可用
- 檢查防火牆設定
- 確認回調 URI 設定正確

### Q4: Microsoft Outlook 顯示 "invalid_request: redirect_uri is not valid"

**快速解決方案：**
1. 前往 [Azure Portal](https://portal.azure.com/)
2. 開啟您的應用程式註冊 → 選擇「驗證」
3. 確認「重新導向 URI」設定為 `http://localhost:8081/`（**必須包含結尾斜線**）
4. 如果已經存在 `http://localhost:8081`（無斜線），請刪除並重新新增 `http://localhost:8081/`（含斜線）
5. 點選「儲存」並重新啟動應用程式

**詳細說明：** 請參閱 [OAUTH_TROUBLESHOOTING.md - Microsoft Outlook 錯誤](./OAUTH_TROUBLESHOOTING.md)

**注意事項：**
- Microsoft Azure AD 對重新導向 URI 的格式非常嚴格，必須完全匹配（包括是否有結尾斜線）
- 應用程式已設定使用 `http://localhost:8081/` 作為重新導向 URI
- 必須確保 Azure AD 註冊的 URI 與程式碼中設定的完全一致

### Q5: 無法獲取事件

**可能原因：**
- Access Token 過期
- API 權限不足
- 網路連線問題

**解決方案：**
- 重新認證
- 檢查 API 權限
- 查看除錯輸出

---

## 參考資源

- [Google Calendar API](https://developers.google.com/calendar/api)
- [Microsoft Graph API](https://learn.microsoft.com/en-us/graph/api/resources/calendar)
- [Qt NetworkAuth](https://doc.qt.io/qt-6/qtnetworkauth-index.html)

---

**最後更新**: 2026 年 1 月

**版本**: 2.0.0
