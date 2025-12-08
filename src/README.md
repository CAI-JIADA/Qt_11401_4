# 原始碼說明

本目錄包含 Qt 多平台行事曆整合工具的原始碼。

## 目錄結構

```
src/
├── main.cpp                    # 程式入口點
├── core/                       # 核心模組
│   ├── CalendarEvent.h/cpp    # 事件資料結構
│   └── CalendarManager.h/cpp  # 行事曆管理器
├── adapters/                   # 平台適配器
│   ├── CalendarAdapter.h      # 適配器基類
│   ├── AppleCalendarAdapter.h/cpp      # Apple Calendar (完整實作)
│   ├── CalDAVClient.h/cpp     # CalDAV 客戶端
│   ├── GoogleCalendarAdapter.h/cpp     # Google Calendar (佔位符)
│   └── OutlookCalendarAdapter.h/cpp    # Outlook (佔位符)
└── storage/                    # 儲存模組
    └── DatabaseManager.h/cpp  # SQLite 資料庫管理
```

## 模組說明

### Core（核心模組）

- **CalendarEvent**: 定義統一的事件和任務資料結構
- **CalendarManager**: 管理多個平台適配器，協調事件查詢和儲存

### Adapters（適配器模組）

- **CalendarAdapter**: 所有平台適配器的抽象基類
- **AppleCalendarAdapter**: Apple Calendar (iCloud) 完整實作，使用 CalDAV
- **CalDAVClient**: CalDAV 協議客戶端，處理 HTTP 請求和 XML 解析
- **GoogleCalendarAdapter**: Google Calendar 適配器（待實作）
- **OutlookCalendarAdapter**: Microsoft Outlook 適配器（待實作）

### Storage（儲存模組）

- **DatabaseManager**: SQLite 本地資料庫管理，提供事件和任務的持久化儲存

## 主要類別關係

```
CalendarManager
    ├── AppleCalendarAdapter (CalendarAdapter)
    │   └── CalDAVClient
    ├── GoogleCalendarAdapter (CalendarAdapter)
    └── OutlookCalendarAdapter (CalendarAdapter)

DatabaseManager (獨立)
```

## 快速開始

1. 檢視 [TESTING.md](../TESTING.md) 了解建置和測試步驟
2. 編輯 `main.cpp` 設定您的 Apple ID 認證資訊
3. 使用 CMake 建置專案
4. 執行 CalendarIntegration

## 擴展指南

### 新增其他平台支援

1. 建立新的適配器類別繼承 `CalendarAdapter`
2. 實作 `authenticate()`, `fetchEvents()`, `fetchTasks()` 方法
3. 在 `main.cpp` 中建立並註冊適配器實例

範例：

```cpp
class NewPlatformAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    void authenticate() override {
        // 實作認證邏輯
        emit authenticated();
    }
    
    void fetchEvents(const QDateTime& start, const QDateTime& end) override {
        // 實作事件獲取邏輯
        emit eventsReceived(events);
    }
    
    void fetchTasks() override {
        // 實作任務獲取邏輯
        emit tasksReceived(tasks);
    }
};
```

## 技術細節

### CalDAV 實作

AppleCalendarAdapter 使用標準的 CalDAV 協議：

- **PROPFIND**: 發現行事曆列表
- **REPORT**: 查詢事件
- **HTTP Basic Auth**: 使用應用程式專用密碼認證

### 資料流

1. 使用者執行程式
2. AppleCalendarAdapter 透過 CalDAVClient 連接 iCloud
3. CalDAVClient 發送 PROPFIND 請求列出行事曆
4. CalDAVClient 發送 REPORT 請求獲取事件
5. 解析 iCalendar 格式的事件資料
6. 事件透過信號傳遞給 CalendarManager
7. DatabaseManager 將事件儲存到本地資料庫

## 授權

MIT License
