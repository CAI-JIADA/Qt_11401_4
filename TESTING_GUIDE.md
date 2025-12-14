# 測試指南 - CalDAV 認證修復

## 概述

我已經修復了 CalDAV 認證問題。問題的根本原因是程式直接使用從 email 提取的 username 來建構 URL，而沒有遵循 CalDAV 標準的服務發現流程。

## 主要變更

### 1. 實作完整的 CalDAV 服務發現流程
- ✅ Step 1: 查詢 `/.well-known/caldav` 來發現 principal URL
- ✅ Step 2: 從 principal 查詢 calendar-home-set
- ✅ Step 3: 查詢 calendar home 以列出行事曆
- ✅ Fallback: 如果發現失敗，仍使用原始方式

### 2. 安全性改進
- ✅ 移除了硬編碼的憑證（Apple ID 和應用程式專用密碼）
- ✅ 現在使用預設的佔位符，提醒使用者填入自己的憑證

### 3. 增強的錯誤處理和日誌
- ✅ 記錄每個步驟的詳細資訊
- ✅ 顯示 HTTP 狀態碼以便診斷
- ✅ 在發現失敗時自動嘗試 fallback

## 如何測試

### 準備步驟

1. **產生新的應用程式專用密碼**（建議）
   - 前往 https://appleid.apple.com
   - 登入您的 Apple ID
   - 進入「安全性」區段
   - 選擇「應用程式專用密碼」
   - 產生一個新的密碼（例如："CalendarTest"）
   - 記下密碼（格式：xxxx-xxxx-xxxx-xxxx）

2. **更新 main.cpp 中的憑證**
   ```cpp
   // 在 src/main.cpp 第 35-36 行
   QString appleId = "your-apple-id@example.com";  // 改為您的 Apple ID
   QString appPassword = "xxxx-xxxx-xxxx-xxxx";    // 改為應用程式專用密碼
   ```

3. **重新編譯**
   ```bash
   cd build
   cmake --build . --config Release
   ```

### 執行測試

```bash
cd build/Release
.\CalendarIntegration.exe
```

### 預期成功輸出

您應該看到類似以下的訊息（不再出現 403 Forbidden 錯誤）：

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
發現 3 個行事曆
  - 工作: https://caldav.icloud.com:443/12345678/calendars/work/
  - 個人: https://caldav.icloud.com:443/12345678/calendars/home/
  - 家庭: https://caldav.icloud.com:443/12345678/calendars/family/

✅ Apple Calendar 認證成功！

正在獲取事件 (2024-12-14 至 2025-01-13)...
```

### 關鍵改變

**之前的錯誤 URL（錯誤）：**
```
https://caldav.icloud.com:443/redmaqic/calendars/
```
這是直接從 email `redmaqic@icloud.com` 提取的 username。

**現在的正確 URL（應該會是類似）：**
```
https://caldav.icloud.com:443/12345678/calendars/
```
這是透過服務發現從 iCloud 伺服器取得的實際路徑。數字 ID 會因每個使用者而異。

## 故障排除

### 如果仍然出現 403 Forbidden

1. **檢查應用程式專用密碼**
   - 確認密碼沒有輸入錯誤
   - 確認密碼沒有被撤銷
   - 嘗試重新產生一個新的密碼

2. **檢查 Apple ID 設定**
   - 確認已啟用雙重認證
   - 嘗試在其他 CalDAV 客戶端（如 Thunderbird）測試相同的憑證

3. **檢查網路**
   - 確認可以連接到 caldav.icloud.com
   - 檢查防火牆設定
   - 嘗試在瀏覽器訪問 https://caldav.icloud.com/

### 如果出現其他錯誤

請將完整的輸出日誌貼給我，包括：
- 所有 "發送 PROPFIND 請求..." 訊息
- HTTP 狀態碼
- 任何錯誤訊息

## 技術說明

### 為什麼之前的方法會失敗？

1. **簡單的 username 提取不可靠**
   - iCloud 內部使用數字 ID，不是 email username
   - 不同的使用者可能有不同的 URL 結構

2. **缺少標準的發現流程**
   - CalDAV 規範要求使用服務發現
   - iCloud 完全支援標準的發現端點

3. **直接訪問可能的路徑會被拒絕**
   - iCloud 可能會拒絕未經發現流程的直接訪問
   - 這是一種安全措施

### 修復如何運作？

1. **遵循標準**：實作 RFC 6764 和 RFC 4791 定義的 CalDAV 服務發現
2. **動態發現**：不再假設 URL 結構，而是從伺服器取得
3. **容錯能力**：如果發現失敗，仍有 fallback 機制

## 需要的文件

- ✅ CALDAV_FIX.md - 詳細的技術說明和修復內容
- ✅ TESTING_GUIDE.md - 本測試指南（這個文件）
- ✅ 更新的原始碼檔案

## 下一步

1. 測試修復是否有效
2. 如果成功，您可以：
   - 開始獲取和顯示行事曆事件
   - 測試其他功能（如事件搜尋、任務管理等）
3. 如果失敗，請提供詳細的日誌輸出以便進一步診斷

## 聯絡資訊

如果測試過程中遇到任何問題，請提供：
1. 完整的程式輸出（包括所有除錯訊息）
2. HTTP 狀態碼
3. 是否已確認 Apple ID 和密碼正確
4. 是否可以用其他 CalDAV 客戶端連接

祝測試順利！🎉
