# Qt 多平台行事曆整合工具 - 實作指南

## 專案概述

本專案旨在使用 Qt 框架開發一個跨平台的行事曆整合工具，可以將 Google Calendar 和 Microsoft Outlook 等不同平台的待辦事項整合到統一的介面中。

---

## 目錄

1. [系統架構](#系統架構)
2. [支援平台整合](#支援平台整合)
3. [功能需求](#功能需求)
4. [技術實作建議](#技術實作建議)
5. [所需工具與依賴](#所需工具與依賴)
6. [使用者介面設計](#使用者介面設計)
7. [安全性考量](#安全性考量)
8. [開發時程建議](#開發時程建議)
9. [參考資源](#參考資源)

---

## 系統架構

### 整體架構圖

```
┌─────────────────────────────────────────────────────────────┐
│                      使用者介面 (Qt QML/Widgets)             │
│                   (桌面版 + 行動裝置版)                       │
├─────────────────────────────────────────────────────────────┤
│                      業務邏輯層                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   事件管理   │  │   日程管理   │  │   搜索引擎   │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
├─────────────────────────────────────────────────────────────┤
│                      資料同步層                              │
│  ┌──────────────┐                   ┌──────────────┐       │
│  │ Google API   │                   │ Outlook API  │       │
│  │   適配器     │                   │   適配器     │       │
│  └──────────────┘                   └──────────────┘       │
├─────────────────────────────────────────────────────────────┤
│                      本地儲存層                              │
│           (SQLite / JSON 快取)                              │
└─────────────────────────────────────────────────────────────┘
```

### 核心模組

1. **平台適配器模組** - 處理不同平台 API 的差異
2. **資料同步模組** - 管理雲端與本地資料同步
3. **事件管理模組** - 統一管理事件和日程
4. **搜索模組** - 提供全文搜索功能
5. **UI 模組** - 跨平台使用者介面

---

## 支援平台整合

### 1. Google Calendar

#### API 資訊
- **API 名稱**: Google Calendar API v3
- **文件連結**: https://developers.google.com/calendar/api
- **認證方式**: OAuth 2.0

#### 整合步驟
1. 在 Google Cloud Console 建立專案
2. 啟用 Google Calendar API
3. 設定 OAuth 2.0 憑證
4. 實作 OAuth 2.0 授權流程

#### 所需權限範圍 (Scopes)
```
https://www.googleapis.com/auth/calendar.readonly
https://www.googleapis.com/auth/calendar.events.readonly
https://www.googleapis.com/auth/tasks.readonly
```

#### Qt 實作建議
```cpp
// 使用 Qt Network 模組進行 API 呼叫
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

class GoogleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    void authenticate();
    void fetchEvents(const QDate& startDate, const QDate& endDate);
    void fetchTasks();
    
signals:
    void eventsReceived(const QList<CalendarEvent>& events);
    void tasksReceived(const QList<Task>& tasks);
};
```

---

### 2. Microsoft Outlook

#### API 資訊
- **API 名稱**: Microsoft Graph API
- **文件連結**: https://learn.microsoft.com/en-us/graph/api/resources/calendar
- **認證方式**: OAuth 2.0 (Azure AD)

#### 整合步驟
1. 在 Azure Portal 註冊應用程式
2. 設定 API 權限
3. 實作 OAuth 2.0 授權流程

#### 所需權限
```
Calendars.Read
Calendars.Read.Shared
Tasks.Read
User.Read
```

#### Qt 實作建議
```cpp
class OutlookCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
public:
    void authenticate();
    void fetchEvents(const QDateTime& start, const QDateTime& end);
    void fetchSharedCalendars();
    void fetchTasks();
    
private:
    QString m_accessToken;
    QNetworkAccessManager* m_networkManager;
};
```

---

## 功能需求

### 1. 待辦事項整合

#### 資料模型

```cpp
// 平台類型列舉
enum class Platform {
    Google,
    Outlook
};

// 統一的事件資料結構
struct CalendarEvent {
    QString id;
    QString title;
    QString description;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    Platform platform;
    QString ownerId;
    bool isAllDay;
    QStringList attendees;
    QString recurrenceRule;
    QColor color;
};

// 任務/待辦事項
struct Task {
    QString id;
    QString title;
    QString description;
    QDateTime dueDate;
    Platform platform;
    QString ownerId;
    bool isCompleted;
    int priority;         // 1-5
    QStringList tags;
};
```

### 2. 讀取他人行事曆

#### 共享行事曆支援

| 平台 | 共享方式 | 實作方法 |
|------|----------|----------|
| Google | 行事曆共享連結 / 邀請 | 使用 `calendarList.list` API |
| Outlook | 共享行事曆 / 群組行事曆 | Microsoft Graph `calendarView` |

```cpp
class SharedCalendarManager {
public:
    void addSharedCalendar(const QString& url, Platform platform);
    void removeSharedCalendar(const QString& calendarId);
    QList<CalendarEvent> getSharedEvents();
    void syncSharedCalendars();
};
```

### 3. 搜索功能

#### 搜索範圍
- 事件標題
- 事件描述
- 地點
- 參與者
- 標籤

```cpp
class SearchEngine {
public:
    // 基本搜索
    QList<CalendarEvent> search(const QString& query);
    
    // 進階搜索
    QList<CalendarEvent> advancedSearch(const SearchCriteria& criteria);
    
    // 日期範圍搜索
    QList<CalendarEvent> searchByDateRange(
        const QDate& start, 
        const QDate& end,
        const QString& query = QString()
    );
    
    // 平台篩選搜索
    QList<CalendarEvent> searchByPlatform(
        const QString& query,
        const QStringList& platforms
    );
};

struct SearchCriteria {
    QString keyword;
    QDate startDate;
    QDate endDate;
    QStringList platforms;
    QStringList owners;
    bool includeCompleted;
};
```

---

## 技術實作建議

### Qt 版本與模組

#### 建議使用 Qt 6.x
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(CalendarIntegration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    Network
    NetworkAuth
    Sql
    Concurrent
)
```

### 專案結構

```
CalendarIntegration/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── CalendarEvent.h
│   │   ├── Task.h
│   │   ├── CalendarManager.cpp
│   │   └── CalendarManager.h
│   ├── adapters/
│   │   ├── CalendarAdapter.h
│   │   ├── GoogleCalendarAdapter.cpp
│   │   ├── GoogleCalendarAdapter.h
│   │   ├── OutlookCalendarAdapter.cpp
│   │   └── OutlookCalendarAdapter.h
│   ├── sync/
│   │   ├── SyncManager.cpp
│   │   └── SyncManager.h
│   ├── search/
│   │   ├── SearchEngine.cpp
│   │   └── SearchEngine.h
│   ├── storage/
│   │   ├── DatabaseManager.cpp
│   │   └── DatabaseManager.h
│   └── ui/
│       ├── MainWindow.cpp
│       ├── MainWindow.h
│       └── qml/
│           ├── main.qml
│           ├── CalendarView.qml
│           ├── EventList.qml
│           └── SearchBar.qml
├── resources/
│   ├── icons/
│   └── qml.qrc
└── tests/
    ├── test_adapters.cpp
    └── test_search.cpp
```

### 本地資料庫設計 (SQLite)

```sql
-- 行事曆表
CREATE TABLE calendars (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    platform TEXT NOT NULL,
    owner_id TEXT,
    is_shared BOOLEAN DEFAULT FALSE,
    color TEXT,
    last_sync DATETIME
);

-- 事件表
CREATE TABLE events (
    id TEXT PRIMARY KEY,
    calendar_id TEXT NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    start_time DATETIME NOT NULL,
    end_time DATETIME,
    location TEXT,
    is_all_day BOOLEAN DEFAULT FALSE,
    recurrence_rule TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (calendar_id) REFERENCES calendars(id)
);

-- 任務表
CREATE TABLE tasks (
    id TEXT PRIMARY KEY,
    calendar_id TEXT NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    due_date DATETIME,
    is_completed BOOLEAN DEFAULT FALSE,
    priority INTEGER DEFAULT 3,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (calendar_id) REFERENCES calendars(id)
);

-- 搜索索引 (使用 FTS5)
CREATE VIRTUAL TABLE events_fts USING fts5(
    title, 
    description, 
    location,
    content='events',
    content_rowid='rowid'
);
```

---

## 所需工具與依賴

### 開發環境

| 工具 | 版本建議 | 用途 |
|------|----------|------|
| Qt | 6.5+ | 主要開發框架 |
| Qt Creator | 12.0+ | IDE |
| CMake | 3.16+ | 建置系統 |
| C++ Compiler | C++17 支援 | GCC 9+, Clang 10+, MSVC 2019+ |
| Git | 2.0+ | 版本控制 |

### 外部函式庫

```cmake
# 第三方依賴
# 1. JSON 處理 (Qt 內建)
# 2. OAuth 2.0 (Qt Network Auth)
# 3. SQLite (Qt SQL)
```

### 平台 SDK

#### Google
```bash
# 不需要額外 SDK，使用 REST API
# 需要設定 Google Cloud 專案
```

#### Microsoft
```bash
# 使用 Microsoft Graph REST API
# 需要設定 Azure AD 應用程式
```

### 行動裝置建置

#### Android
```bash
# 需要安裝
- Android SDK
- Android NDK
- Java JDK 11+
```

#### iOS
```bash
# 需要
- Xcode
- Apple Developer 帳號
- macOS 開發環境
```

---

## 使用者介面設計

### 桌面版 (Qt Widgets / QML)

```qml
// main.qml - 主介面範例
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1200
    height: 800
    visible: true
    title: "行事曆整合工具"
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 側邊欄 - 行事曆列表
        Rectangle {
            Layout.preferredWidth: 250
            Layout.fillHeight: true
            color: "#f5f5f5"
            
            CalendarSidebar {
                anchors.fill: parent
            }
        }
        
        // 主內容區
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // 搜索列
            SearchBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                onSearch: (query) => calendarManager.search(query)
            }
            
            // 行事曆視圖
            CalendarView {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
        
        // 右側欄 - 事件詳情
        EventDetailPanel {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            visible: selectedEvent !== null
        }
    }
}
```

### 行動版調整

```qml
// 響應式設計
Item {
    property bool isMobile: width < 600
    
    StackView {
        id: stackView
        anchors.fill: parent
        visible: isMobile
        
        initialItem: calendarListPage
    }
    
    SplitView {
        anchors.fill: parent
        visible: !isMobile
        
        // 桌面版分欄佈局
    }
}
```

### UI 元件清單

1. **CalendarView** - 月/週/日 檢視
2. **EventList** - 事件列表
3. **SearchBar** - 搜索欄
4. **EventDetailPanel** - 事件詳情面板
5. **CalendarSidebar** - 行事曆選擇器
6. **PlatformAccountManager** - 帳號管理
7. **SyncStatusIndicator** - 同步狀態指示

---

## 安全性考量

### OAuth Token 管理

```cpp
#include <optional>

class SecureTokenStorage {
public:
    // 使用系統金鑰鏈儲存 token
    bool saveToken(const QString& service, const QString& token);
    
    // 使用 std::optional 處理 token 可能不存在的情況
    std::optional<QString> loadToken(const QString& service);
    
    bool deleteToken(const QString& service);
    
private:
    // 平台特定實作
    #ifdef Q_OS_MACOS
        // 使用 Keychain
    #elif defined(Q_OS_WIN)
        // 使用 Windows Credential Manager
    #elif defined(Q_OS_LINUX)
        // 使用 libsecret / KWallet
    #endif
};

// 使用範例
void authenticateUser() {
    SecureTokenStorage storage;
    auto token = storage.loadToken("google_calendar");
    if (token.has_value()) {
        // Token 存在，使用它
        useToken(token.value());
    } else {
        // Token 不存在，需要重新認證
        performOAuthFlow();
    }
}
```

### 資料加密

```cpp
// 本地敏感資料加密
class DataEncryption {
public:
    QByteArray encrypt(const QByteArray& data, const QByteArray& key);
    QByteArray decrypt(const QByteArray& data, const QByteArray& key);
};
```

### 安全建議

1. **永遠不要**在程式碼中硬編碼 API 金鑰
2. 使用 HTTPS 進行所有網路通訊
3. 實作 token 自動更新機制
4. 定期清理過期的 token
5. 加密本地儲存的敏感資料

---

## 開發時程建議

### 第一階段 - 基礎架構 

- [ ] 建立專案結構
- [ ] 實作資料模型
- [ ] 建立本地資料庫
- [ ] 基本 UI 框架

### 第二階段 - 平台整合

- [ ] Google Calendar 整合 (OAuth 2.0)
- [ ] Microsoft Outlook 整合 (Microsoft Graph API)

### 第三階段 - 核心功能 

- [ ] 事件同步功能
- [ ] 搜索功能實作
- [ ] 共享行事曆支援
- [ ] 離線模式

### 第四階段 - UI 完善

- [ ] 響應式 UI
- [ ] 行動裝置優化
- [ ] 主題與自訂設定

### 第五階段 - 測試與發布 

- [ ] 單元測試
- [ ] 整合測試
- [ ] 效能優化
- [ ] 文件撰寫

---

## 參考資源

### API 文件
- [Google Calendar API](https://developers.google.com/calendar/api/v3/reference)
- [Microsoft Graph API](https://learn.microsoft.com/en-us/graph/api/resources/calendar)

### Qt 文件
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [Qt Network Auth](https://doc.qt.io/qt-6/qtnetworkauth-index.html)
- [Qt Network](https://doc.qt.io/qt-6/qtnetwork-index.html)
- [Qt QML](https://doc.qt.io/qt-6/qtqml-index.html)

### 範例專案
- [Qt OAuth2 Examples](https://doc.qt.io/qt-6/qtnetworkauth-index.html#examples)
- [Qt Calendar Example](https://doc.qt.io/qt-6/qtwidgets-widgets-calendarwidget-example.html)
- [Qt Network Examples](https://doc.qt.io/qt-6/qtnetwork-examples.html)

---

## 授權與貢獻

本專案採用 MIT 授權。歡迎提交 Issue 和 Pull Request。

---

*最後更新: 2024*
