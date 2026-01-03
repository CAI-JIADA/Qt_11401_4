# 新功能說明

本更新新增了兩個主要功能：

## 1. 修改紀錄 / 版本歷史

### 功能說明
系統會自動記錄所有事件的新增、更新和刪除操作，讓您可以追蹤資料的變更歷史。

### 如何使用

1. 開啟應用程式
2. 點選選單列的「檢視」→「修改紀錄 / 版本歷史」
3. 會開啟修改紀錄視窗，顯示最近 100 筆變更記錄

### 記錄內容包含

- **時間**: 變更發生的時間
- **類型**: 事件類型（event 或 task）
- **操作**: 新增、更新或刪除
- **實體ID**: 被變更的事件 ID
- **變更欄位**: 哪些欄位被修改
- **詳細資訊**: 變更前後的值

### 操作說明

- 點擊任意記錄列，會彈出對話框顯示完整的變更詳情
- 點擊「重新整理」按鈕可以重新載入最新的記錄
- 變更記錄會自動儲存在本地資料庫中

### 記錄時機

系統會在以下情況自動記錄：
- 從 Google Calendar 或 Outlook 獲取新事件時（記錄為「新增」）
- 更新已存在的事件時（記錄為「更新」，並記錄變更的欄位）
- 刪除事件時（記錄為「刪除」，保留刪除前的資料）

---

## 2. 自訂背景圖片

### 功能說明
您可以上傳自己喜歡的圖片作為應用程式的背景，打造個性化的使用體驗。

### 如何使用

#### 設定背景圖片

1. 開啟應用程式
2. 點選選單列的「設定」→「更改背景圖片」
3. 在檔案選擇對話框中選擇圖片檔案（支援 PNG、JPG、JPEG、BMP、GIF）
4. 確認後，背景會立即套用

#### 重設背景

1. 點選選單列的「設定」→「重設背景」
2. 確認後，背景會恢復為預設樣式

### 支援的圖片格式

- PNG (.png)
- JPEG (.jpg, .jpeg)
- BMP (.bmp)
- GIF (.gif)

### 建議

- 使用解析度適中的圖片，避免圖片過大影響載入速度
- 背景圖片會自動淡化（添加半透明白色遮罩），以確保文字清晰可讀
- 圖片會自動拉伸以覆蓋整個視窗，保持原始比例

### 技術細節

- 背景圖片路徑儲存在本地資料庫的 `settings` 表中
- 使用 Qt 的 StyleSheet 機制套用背景
- 圖片使用 `background-size: cover` 自動拉伸至視窗同比例
- 添加 50% 不透明度的白色遮罩層來淡化背景圖片
- 每次啟動應用程式時會自動載入已儲存的背景設定

---

## 資料庫架構變更

### 新增的資料表

#### 1. change_logs（修改紀錄表）

```sql
CREATE TABLE change_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    entity_type TEXT NOT NULL,      -- "event" 或 "task"
    entity_id TEXT NOT NULL,        -- 實體的 ID
    action TEXT NOT NULL,           -- "created", "updated", "deleted"
    changed_fields TEXT,            -- 變更的欄位列表
    old_values TEXT,                -- 變更前的值（JSON 格式）
    new_values TEXT,                -- 變更後的值（JSON 格式）
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

#### 2. settings（設定表）

```sql
CREATE TABLE settings (
    key TEXT PRIMARY KEY,
    value TEXT,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

### 更新的方法

#### DatabaseManager 新增的方法

- `saveChangeLog(const ChangeLog& log)`: 儲存修改紀錄
- `loadChangeLogs(int limit = 100)`: 載入最近的修改紀錄
- `loadChangeLogsForEntity(const QString& entityType, const QString& entityId)`: 載入特定實體的修改紀錄
- `saveSetting(const QString& key, const QString& value)`: 儲存設定
- `loadSetting(const QString& key, const QString& defaultValue)`: 讀取設定

---

## 測試步驟

### 測試修改紀錄功能

1. 啟動應用程式
2. 連接 Google Calendar 或 Outlook
3. 獲取一些事件
4. 開啟「修改紀錄 / 版本歷史」視窗
5. 確認可以看到事件被記錄為「新增」操作
6. 記錄應包含事件的標題、時間等資訊

### 測試背景圖片功能

1. 啟動應用程式
2. 準備一張測試圖片（例如：1920x1080 的風景照）
3. 點選「設定」→「更改背景圖片」
4. 選擇測試圖片
5. 確認背景圖片成功套用
6. 重新啟動應用程式，確認背景圖片保持不變
7. 測試「重設背景」功能，確認能恢復預設樣式

---

## 技術實作摘要

### 修改紀錄系統

- 在 `DatabaseManager::saveEvent()` 中加入變更偵測邏輯
- 比對舊值與新值，記錄變更的欄位
- 使用 JSON 格式儲存變更前後的值
- 提供專用的 UI 對話框（ChangeLogDialog）顯示記錄

### 背景圖片系統

- 使用 Qt 的 Settings 機制儲存圖片路徑
- 透過 QFileDialog 提供友善的檔案選擇介面
- 使用 CSS StyleSheet 套用背景圖片
- 在應用程式啟動時自動載入並套用已儲存的背景

---

**版本**: 1.1.0  
**更新日期**: 2026-01-03
