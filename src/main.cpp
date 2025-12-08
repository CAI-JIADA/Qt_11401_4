#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "core/CalendarManager.h"
#include "adapters/AppleCalendarAdapter.h"
#include "storage/DatabaseManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Qt 多平台行事曆整合工具 ===";
    qDebug() << "示範：Apple Calendar (iCloud) 整合 - 無需 Mac!";
    qDebug() << "";
    
    // 初始化資料庫
    DatabaseManager dbManager;
    if (!dbManager.initialize("calendar.db")) {
        qCritical() << "資料庫初始化失敗！";
        return 1;
    }
    
    // 建立行事曆管理器
    CalendarManager manager;
    
    // 建立 Apple Calendar 適配器
    AppleCalendarAdapter* appleAdapter = new AppleCalendarAdapter(&manager);
    
    // 重要：請替換為您自己的 Apple ID 和應用程式專用密碼
    // 取得應用程式專用密碼的步驟：
    // 1. 前往 https://appleid.apple.com
    // 2. 登入您的 Apple ID
    // 3. 在「安全性」區段中選擇「應用程式專用密碼」
    // 4. 產生新密碼
    QString appleId = "your-apple-id@example.com";  // 請替換
    QString appPassword = "xxxx-xxxx-xxxx-xxxx";    // 請替換
    
    if (appleId == "your-apple-id@example.com") {
        qWarning() << "";
        qWarning() << "⚠️  請在 main.cpp 中設定您的 Apple ID 和應用程式專用密碼！";
        qWarning() << "";
        qWarning() << "步驟：";
        qWarning() << "1. 前往 https://appleid.apple.com";
        qWarning() << "2. 登入並進入「安全性」區段";
        qWarning() << "3. 產生「應用程式專用密碼」";
        qWarning() << "4. 在 main.cpp 中替換 appleId 和 appPassword";
        qWarning() << "";
        return 1;
    }
    
    appleAdapter->setCredentials(appleId, appPassword);
    
    // 將適配器加入管理器
    manager.addAdapter(appleAdapter);
    
    // 監聽事件
    QObject::connect(&manager, &CalendarManager::eventsUpdated,
                    [&dbManager](const QList<CalendarEvent>& events) {
        qDebug() << "";
        qDebug() << "=== 收到" << events.size() << "個事件 ===";
        
        for (const auto& event : events) {
            qDebug() << event.toString();
            
            // 儲存到本地資料庫
            dbManager.saveEvent(event);
        }
        
        qDebug() << "";
        qDebug() << "事件已儲存到本地資料庫";
    });
    
    QObject::connect(&manager, &CalendarManager::errorOccurred,
                    [](const QString& error) {
        qCritical() << "錯誤:" << error;
    });
    
    QObject::connect(appleAdapter, &AppleCalendarAdapter::authenticated,
                    [&manager]() {
        qDebug() << "";
        qDebug() << "✅ Apple Calendar 認證成功！";
        qDebug() << "";
        
        // 獲取未來 30 天的事件
        QDateTime start = QDateTime::currentDateTime();
        QDateTime end = start.addDays(30);
        
        qDebug() << "正在獲取事件 (" 
                 << start.toString("yyyy-MM-dd") 
                 << " 至 " 
                 << end.toString("yyyy-MM-dd") 
                 << ")...";
        
        manager.fetchAllEvents(start, end);
    });
    
    QObject::connect(appleAdapter, &AppleCalendarAdapter::authenticationFailed,
                    [](const QString& error) {
        qCritical() << "";
        qCritical() << "❌ 認證失敗:" << error;
        qCritical() << "";
        qCritical() << "請檢查：";
        qCritical() << "1. Apple ID 格式正確（email 格式）";
        qCritical() << "2. 使用的是「應用程式專用密碼」而非 Apple ID 密碼";
        qCritical() << "3. Apple ID 已啟用雙重認證";
        qCritical() << "4. 網路連線正常";
        qCritical() << "";
    });
    
    // 開始認證
    qDebug() << "開始連接 Apple Calendar (iCloud)...";
    appleAdapter->authenticate();
    
    // 設定 30 秒後自動結束（給足夠時間完成網路請求）
    QTimer::singleShot(30000, &app, [&app]() {
        qDebug() << "";
        qDebug() << "程式執行完畢";
        app.quit();
    });
    
    return app.exec();
}
