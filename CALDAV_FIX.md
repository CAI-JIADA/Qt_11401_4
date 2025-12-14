# CalDAV 認證問題修復說明

## 問題分析

原始錯誤訊息：
```
PROPFIND 錯誤: Error transferring https://caldav.icloud.com:443/redmaqic/calendars/ - server replied: Forbidden
```

### 根本原因

1. **跳過服務發現步驟**：程式直接使用從 Apple ID 提取的 username (`redmaqic`) 來建構 CalDAV URL，而沒有先透過標準的 CalDAV 服務發現協議來找到正確的端點。

2. **URL 可能不正確**：直接從 email 提取的 username 可能不是 iCloud 內部使用的實際用戶識別碼，導致 403 Forbidden 錯誤。

3. **缺少完整的發現流程**：根據 CalDAV 規範 (RFC 6764)，正確的流程應該是：
   - Step 1: 查詢 `/.well-known/caldav` 或根路徑
   - Step 2: 從回應中取得 `current-user-principal`
   - Step 3: 查詢 principal 以取得 `calendar-home-set`
   - Step 4: 查詢 calendar home 以列出所有行事曆

## 修復內容

### 1. 新增狀態追蹤
在 `CalDAVClient.h` 中新增：
```cpp
QString m_principalUrl;      // 儲存發現的 principal URL
QString m_calendarHomeUrl;   // 儲存發現的 calendar home URL
```

### 2. 分離請求處理方法
新增專門的方法來處理不同階段的 PROPFIND 請求：
- `sendPropfindForPrincipal()` - 處理 principal 發現
- `sendPropfindForCalendarHome()` - 處理 calendar home 發現
- `sendPropfind()` - 處理行事曆列表查詢

### 3. 實作完整的服務發現流程

#### Step 1: Principal 發現
```cpp
void CalDAVClient::discoverService() {
    qDebug() << "開始 CalDAV 服務發現...";
    QUrl url(m_baseUrl + "/.well-known/caldav");
    
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\">"
        "  <d:prop>"
        "    <d:current-user-principal />"
        "  </d:prop>"
        "</d:propfind>";
    
    sendPropfindForPrincipal(url, propfindXml, 0);
}
```

#### Step 2: Calendar Home 發現
```cpp
// 在 discoverCalendars() 中
if (!m_principalUrl.isEmpty()) {
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\" "
        "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "  <d:prop>"
        "    <c:calendar-home-set />"
        "  </d:prop>"
        "</d:propfind>";
    
    sendPropfindForCalendarHome(QUrl(m_baseUrl + m_principalUrl), propfindXml, 0);
    return;
}
```

#### Step 3: 行事曆列表查詢
```cpp
// 當 calendar home URL 已知時
if (!m_calendarHomeUrl.isEmpty()) {
    targetUrl = m_baseUrl + m_calendarHomeUrl;
    // ... 發送 PROPFIND 查詢行事曆
}
```

### 4. Fallback 機制
如果服務發現失敗，仍然會嘗試使用原始的方式（從 email 提取 username）：
```cpp
// 最後才使用從 email 提取的 username (fallback)
else {
    QString username = m_username.left(m_username.indexOf('@'));
    targetUrl = m_baseUrl + "/" + username + "/calendars/";
    qDebug() << "使用 fallback URL:" << targetUrl;
}
```

### 5. 更新認證流程
在 `AppleCalendarAdapter::authenticate()` 中，改為呼叫服務發現：
```cpp
void AppleCalendarAdapter::authenticate() {
    qDebug() << "開始 Apple Calendar 認證...";
    
    // 使用服務發現來找到正確的 CalDAV 端點
    m_caldavClient->discoverService();
}
```

### 6. 增強錯誤處理
- 記錄 HTTP 狀態碼以便診斷
- 在發現失敗時自動嘗試 fallback 方式
- 提供更詳細的錯誤訊息

## 測試步驟

### 1. 重新編譯專案
```bash
cd build
cmake --build . --config Release
```

### 2. 確認憑證設定
在 `src/main.cpp` 中檢查：
```cpp
QString appleId = "your-apple-id@icloud.com";  // 您的 Apple ID
QString appPassword = "xxxx-xxxx-xxxx-xxxx";    // 應用程式專用密碼
```

### 3. 執行程式
```bash
cd build/Release
.\CalendarIntegration.exe
```

### 4. 預期輸出
成功的情況下，您應該看到類似以下的輸出：

```
=== Qt 多平台行事曆整合工具 ===
示範：Apple Calendar (iCloud) 整合 - 無需 Mac!

資料庫已開啟: "calendar.db"
資料庫表已建立
已新增平台適配器
開始連接 Apple Calendar (iCloud)...
開始 Apple Calendar 認證...
開始 CalDAV 服務發現...
發送 PROPFIND 請求以發現 principal URL: https://caldav.icloud.com:443/.well-known/caldav
Principal PROPFIND 回應: <?xml version="1.0" encoding="UTF-8"?>...
發現 principal URL: /123456789/principal/
使用 principal URL 查找 calendar home...
發送 PROPFIND 請求以發現 calendar home: https://caldav.icloud.com:443/123456789/principal/
Calendar Home PROPFIND 回應: <?xml version="1.0" encoding="UTF-8"?>...
發現 calendar home URL: /123456789/calendars/
使用已發現的 calendar home URL: https://caldav.icloud.com:443/123456789/calendars/
PROPFIND 回應: <?xml version="1.0" encoding="UTF-8"?>...
發現 3 個行事曆
  - 工作: https://caldav.icloud.com:443/123456789/calendars/work/
  - 個人: https://caldav.icloud.com:443/123456789/calendars/home/
  - 家庭: https://caldav.icloud.com:443/123456789/calendars/family/

✅ Apple Calendar 認證成功！
```

### 5. 故障排除

如果仍然出現錯誤：

#### 錯誤 1: 403 Forbidden (與之前相同)
可能原因：
- 應用程式專用密碼錯誤或已撤銷
- Apple ID 未啟用雙重認證
- 網路問題或防火牆封鎖

解決方法：
1. 重新產生應用程式專用密碼
2. 確認 Apple ID 已啟用雙重認證
3. 檢查網路連線

#### 錯誤 2: 401 Unauthorized
可能原因：
- 認證資訊未正確傳送

解決方法：
- 檢查 `handleAuthenticationRequired()` 是否被正確呼叫
- 確認 username 和 password 已正確設定

#### 錯誤 3: SSL 錯誤
可能原因：
- 系統時間不正確
- SSL 憑證信任鏈問題

解決方法：
- 確認系統時間正確
- 更新系統根憑證

## 技術細節

### CalDAV 服務發現協議

根據 RFC 6764 和 RFC 4791，CalDAV 服務發現的標準流程：

1. **Initial Discovery** (`/.well-known/caldav`)
   - 請求：PROPFIND with `current-user-principal`
   - 回應：Principal URL (例如 `/123456789/principal/`)

2. **Principal Properties**
   - 請求：PROPFIND to Principal URL with `calendar-home-set`
   - 回應：Calendar Home URL (例如 `/123456789/calendars/`)

3. **Calendar Collection**
   - 請求：PROPFIND to Calendar Home URL with Depth: 1
   - 回應：所有行事曆的列表和屬性

### iCloud CalDAV 特殊性

Apple 的 iCloud CalDAV 服務有以下特點：

1. **需要應用程式專用密碼**：不能使用 Apple ID 密碼
2. **使用數字 ID**：實際的 principal URL 使用數字 ID，不是 email username
3. **嚴格的 SSL 要求**：需要有效的 SSL 憑證
4. **支援服務發現**：完整支援 `.well-known/caldav` 端點

## 參考資源

- [RFC 4791 - CalDAV](https://www.rfc-editor.org/rfc/rfc4791)
- [RFC 6764 - CalDAV Service Discovery](https://www.rfc-editor.org/rfc/rfc6764)
- [Apple CalDAV Documentation](https://developer.apple.com/library/archive/documentation/NetworkingInternet/Conceptual/RemoteNotificationsPG/APNSOverview.html)

## 變更檔案清單

- `src/adapters/CalDAVClient.h` - 新增狀態追蹤和新方法宣告
- `src/adapters/CalDAVClient.cpp` - 實作完整的服務發現流程
- `src/adapters/AppleCalendarAdapter.cpp` - 更新認證流程以使用服務發現
