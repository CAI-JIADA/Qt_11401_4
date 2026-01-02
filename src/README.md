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
│   ├── GoogleCalendarAdapter.h/cpp     # Google Calendar
│   └── OutlookCalendarAdapter.h/cpp    # Outlook
└── storage/                    # 儲存模組
    └── DatabaseManager.h/cpp  # SQLite 資料庫管理
```

## 模組說明

### Core（核心模組）

- **CalendarEvent**: 定義統一的事件和任務資料結構
- **CalendarManager**: 管理多個平台適配器，協調事件查詢和儲存

### Adapters（適配器模組）

- **CalendarAdapter**: 所有平台適配器的抽象基類
- **GoogleCalendarAdapter**: Google Calendar 適配器
- **OutlookCalendarAdapter**: Microsoft Outlook 適配器

### Storage（儲存模組）

- **DatabaseManager**: SQLite 本地資料庫管理，提供事件和任務的持久化儲存

## 主要類別關係

```
CalendarManager
    ├── GoogleCalendarAdapter (CalendarAdapter)
    └── OutlookCalendarAdapter (CalendarAdapter)

DatabaseManager (獨立)
```

## 快速開始

1. 檢視 [TESTING.md](../TESTING.md) 了解建置和測試步驟
2. 使用 CMake 建置專案
3. 執行 CalendarIntegration

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

### 資料流

1. 使用者執行程式
2. 各適配器透過 OAuth 2.0 連接到對應的服務
3. 獲取事件並解析為 CalendarEvent 物件
4. 事件透過信號傳遞給 CalendarManager
5. DatabaseManager 將事件儲存到本地資料庫

## 授權

MIT License
