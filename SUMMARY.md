# CalDAV 認證問題修復 - 完整摘要

## 問題描述

您在執行 CalendarIntegration 程式時遇到以下錯誤：

```
CalDAV 錯誤: "PROPFIND 錯誤: Error transferring https://caldav.icloud.com:443/redmaqic/calendars/ - server replied: Forbidden"
認證失敗: "PROPFIND 錯誤: Error transferring https://caldav.icloud.com:443/redmaqic/calendars/ - server replied: Forbidden"
```

## 問題根源

程式直接使用從 Apple ID email (`redmaqic@icloud.com`) 提取的 username (`redmaqic`) 來建構 CalDAV URL：
```
https://caldav.icloud.com:443/redmaqic/calendars/
```

但是 iCloud 的 CalDAV 服務實際上使用**數字 ID** 而非 username，所以這個 URL 是錯誤的，導致伺服器回應 403 Forbidden。

正確的 URL 格式應該類似：
```
https://caldav.icloud.com:443/12345678/calendars/
```

其中 `12345678` 是您帳號的實際數字 ID，這個 ID 必須透過 CalDAV 服務發現協議來取得。

## 修復方案

我已經實作了完整的 **CalDAV 服務發現流程**（RFC 6764 和 RFC 4791），包含以下步驟：

### 修復流程

```
1. 查詢 /.well-known/caldav
   ↓
2. 取得 principal URL (例如: /12345678/principal/)
   ↓
3. 查詢 principal 以取得 calendar-home-set
   ↓
4. 取得 calendar home URL (例如: /12345678/calendars/)
   ↓
5. 列出所有行事曆
```

### 如果服務發現失敗
程式會自動回退到原始方式（使用 username），確保向下相容性。

## 修改的檔案

1. **src/adapters/CalDAVClient.h**
   - 新增狀態追蹤變數
   - 新增服務發現相關方法

2. **src/adapters/CalDAVClient.cpp**
   - 實作完整的服務發現流程
   - 新增防止無限遞迴的保護機制
   - 改進錯誤處理和日誌記錄

3. **src/adapters/AppleCalendarAdapter.cpp**
   - 更新認證流程以使用服務發現

4. **src/main.cpp**
   - 移除硬編碼的憑證（改為佔位符）

5. **文件**
   - `CALDAV_FIX.md` - 技術詳細說明
   - `TESTING_GUIDE.md` - 測試指南

## 如何測試修復

### 步驟 1: 更新憑證

在 `src/main.cpp` 中，找到並更新以下兩行：

```cpp
QString appleId = "your-apple-id@example.com";  // 改為您的 Apple ID
QString appPassword = "xxxx-xxxx-xxxx-xxxx";    // 改為應用程式專用密碼
```

**注意：** 如果您尚未產生應用程式專用密碼，請：
1. 前往 https://appleid.apple.com
2. 登入並進入「安全性」區段
3. 產生新的「應用程式專用密碼」

### 步驟 2: 重新編譯

```bash
cd build
cmake --build . --config Release
```

### 步驟 3: 執行測試

```bash
cd build/Release
.\CalendarIntegration.exe
```

## 預期結果

### 成功的輸出應該類似：

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
發現 principal URL: /12345678/principal/
使用 principal URL 查找 calendar home...
發送 PROPFIND 請求以發現 calendar home: https://caldav.icloud.com:443/12345678/principal/
Calendar Home PROPFIND 回應: <?xml version="1.0" encoding="UTF-8"?>...
發現 calendar home URL: /12345678/calendars/
使用已發現的 calendar home URL: https://caldav.icloud.com:443/12345678/calendars/
PROPFIND 回應: <?xml version="1.0" encoding="UTF-8"?>...
發現 3 個行事曆
  - 工作: /12345678/calendars/work/
  - 個人: /12345678/calendars/home/
  - 家庭: /12345678/calendars/family/

✅ Apple Calendar 認證成功！

正在獲取事件 (2024-12-14 至 2025-01-13)...
```

### 關鍵差異

**修復前（錯誤）：**
```
https://caldav.icloud.com:443/redmaqic/calendars/
```
↓ 403 Forbidden

**修復後（正確）：**
```
https://caldav.icloud.com:443/12345678/calendars/
```
↓ 成功！

## 如果仍然失敗

### 檢查清單

1. **憑證檢查**
   - ✓ Apple ID 格式正確（email）
   - ✓ 使用應用程式專用密碼（不是 Apple ID 密碼）
   - ✓ 密碼未被撤銷
   - ✓ Apple ID 已啟用雙重認證

2. **網路檢查**
   - ✓ 可以連接到 caldav.icloud.com
   - ✓ 防火牆未封鎖 port 443
   - ✓ 代理伺服器設定正確（如果有）

3. **其他測試**
   - 嘗試在其他 CalDAV 客戶端（如 Thunderbird）使用相同憑證
   - 檢查 iCloud 網頁版是否正常運作

### 需要幫助？

如果修復後仍然失敗，請提供：
1. **完整的程式輸出**（包括所有除錯訊息）
2. **HTTP 狀態碼**（如果有）
3. **確認您已正確設定憑證**
4. **是否可以用其他 CalDAV 客戶端連接**

## 技術亮點

### 改進項目

✅ **標準合規**: 完整實作 RFC 6764 和 RFC 4791  
✅ **自動發現**: 不再假設 URL 結構  
✅ **錯誤處理**: 改進的錯誤訊息和日誌  
✅ **容錯機制**: Fallback 到原始方式  
✅ **安全性**: 移除硬編碼憑證  
✅ **防護機制**: 防止無限遞迴  

### 程式碼品質

✅ **清晰的流程**: 明確的狀態管理  
✅ **詳細的日誌**: 便於診斷問題  
✅ **完整的文件**: 技術文件和測試指南  

## 後續步驟

成功認證後，您可以：

1. **查看行事曆列表**: 程式會顯示所有 iCloud 行事曆
2. **獲取事件**: 自動獲取未來 30 天的事件
3. **本地儲存**: 事件會儲存到 SQLite 資料庫
4. **繼續開發**: 實作更多功能（如事件編輯、新增等）

## 相關文件

- `CALDAV_FIX.md` - 詳細的技術文件
- `TESTING_GUIDE.md` - 完整的測試指南
- `IMPLEMENTATION.md` - 原始實作指南
- `TESTING.md` - 原始測試文件

## 總結

這個修復解決了 CalDAV 認證問題的根本原因，並遵循了業界標準協議。程式現在會正確地發現您的 iCloud CalDAV 端點，而不是盲目地假設 URL 結構。

祝測試順利！如有任何問題，請隨時提供日誌輸出以便進一步診斷。🎉
