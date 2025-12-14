#include "AppleCalendarAdapter.h"
#include <QDebug>

AppleCalendarAdapter::AppleCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_caldavClient(new CalDAVClient(this))
{
    // 連接 CalDAV 客戶端信號
    connect(m_caldavClient, &CalDAVClient::calendarsListed,
            this, &AppleCalendarAdapter::onCalendarsListed);
    connect(m_caldavClient, &CalDAVClient::eventsReceived,
            this, &AppleCalendarAdapter::onEventsReceived);
    connect(m_caldavClient, &CalDAVClient::errorOccurred,
            this, &AppleCalendarAdapter::onCalDAVError);
}

AppleCalendarAdapter::~AppleCalendarAdapter() = default;

void AppleCalendarAdapter::setCredentials(const QString& appleId, 
                                          const QString& appPassword) {
    m_appleId = appleId;
    m_appPassword = appPassword;
    m_caldavClient->setCredentials(appleId, appPassword);
    m_caldavClient->setServer("caldav.icloud.com", 443);
}

void AppleCalendarAdapter::authenticate() {
    qDebug() << "開始 Apple Calendar 認證...";
    
    // 使用服務發現來找到正確的 CalDAV 端點
    m_caldavClient->discoverService();
}

void AppleCalendarAdapter::fetchEvents(const QDateTime& start, const QDateTime& end) {
    qDebug() << "獲取 Apple Calendar 事件...";
    
    if (m_calendars.isEmpty()) {
        qWarning() << "尚未發現行事曆，先執行 authenticate()";
        emit errorOccurred("尚未認證或發現行事曆");
        return;
    }
    
    // 從第一個行事曆獲取事件
    if (!m_calendars.isEmpty()) {
        m_caldavClient->getEvents(m_calendars.first().url, start, end);
    }
}

void AppleCalendarAdapter::fetchTasks() {
    qDebug() << "Apple Calendar 任務功能尚未實作";
    // TODO: 實作 VTODO 支援
    emit tasksReceived(QList<Task>());
}

void AppleCalendarAdapter::onCalendarsListed(const QList<CalendarInfo>& calendars) {
    qDebug() << "發現" << calendars.size() << "個行事曆";
    
    m_calendars = calendars;
    
    for (const auto& calendar : calendars) {
        qDebug() << "  -" << calendar.displayName << ":" << calendar.url;
    }
    
    emit authenticated();
}

void AppleCalendarAdapter::onEventsReceived(const QList<CalendarEvent>& events) {
    qDebug() << "收到" << events.size() << "個事件";
    
    for (const auto& event : events) {
        qDebug() << "  -" << event.title;
    }
    
    emit eventsReceived(events);
}

void AppleCalendarAdapter::onCalDAVError(const QString& error) {
    qWarning() << "CalDAV 錯誤:" << error;
    emit authenticationFailed(error);
    emit errorOccurred(error);
}
