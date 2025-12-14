#pragma once

#include <QObject>
#include <QList>
#include "CalendarEvent.h"
#include "adapters/CalendarAdapter.h"

// 行事曆管理器 - 統一管理所有平台的行事曆
class CalendarManager : public QObject {
    Q_OBJECT
    
public:
    explicit CalendarManager(QObject* parent = nullptr);
    ~CalendarManager() override;
    
    // 新增平台適配器
    void addAdapter(CalendarAdapter* adapter);
    
    // 獲取所有事件
    void fetchAllEvents(const QDateTime& start, const QDateTime& end);
    
    // 獲取所有任務
    void fetchAllTasks();
    
    // 搜尋事件
    QList<CalendarEvent> searchEvents(const QString& query) const;
    
signals:
    void eventsUpdated(const QList<CalendarEvent>& events);
    void tasksUpdated(const QList<Task>& tasks);
    void errorOccurred(const QString& error);
    
private slots:
    void onAdapterEventsReceived(const QList<CalendarEvent>& events);
    void onAdapterTasksReceived(const QList<Task>& tasks);
    void onAdapterError(const QString& error);
    
private:
    QList<CalendarAdapter*> m_adapters;
    QList<CalendarEvent> m_allEvents;
    QList<Task> m_allTasks;
};
