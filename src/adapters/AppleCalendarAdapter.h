#pragma once

#include "CalendarAdapter.h"
#include "CalDAVClient.h"
#include <QDateTime>

// Apple Calendar (iCloud) 適配器 - 使用 CalDAV 協議
class AppleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
    
public:
    explicit AppleCalendarAdapter(QObject* parent = nullptr);
    ~AppleCalendarAdapter() override;
    
    // 設定認證資訊
    void setCredentials(const QString& appleId, const QString& appPassword);
    
    // CalendarAdapter 介面實作
    void authenticate() override;
    void fetchEvents(const QDateTime& start, const QDateTime& end) override;
    void fetchTasks() override;
    
private slots:
    void onCalendarsListed(const QList<CalendarInfo>& calendars);
    void onEventsReceived(const QList<CalendarEvent>& events);
    void onCalDAVError(const QString& error);
    
private:
    CalDAVClient* m_caldavClient;
    QList<CalendarInfo> m_calendars;
    QString m_appleId;
    QString m_appPassword;
};
