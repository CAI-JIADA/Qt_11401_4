#pragma once

#include <QObject>
#include <QList>
#include "CalendarEvent.h"

// 抽象基類 - 所有平台適配器的介面
class CalendarAdapter : public QObject {
    Q_OBJECT
    
public:
    explicit CalendarAdapter(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~CalendarAdapter() = default;
    
    // 認證
    virtual void authenticate() = 0;
    
    // 獲取事件
    virtual void fetchEvents(const QDateTime& start, const QDateTime& end) = 0;
    
    // 獲取任務
    virtual void fetchTasks() = 0;
    
signals:
    void authenticated();
    void authenticationFailed(const QString& error);
    void eventsReceived(const QList<CalendarEvent>& events);
    void tasksReceived(const QList<Task>& tasks);
    void errorOccurred(const QString& error);
};
