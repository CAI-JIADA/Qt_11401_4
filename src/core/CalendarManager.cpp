#include "CalendarManager.h"
#include <QDebug>

CalendarManager::CalendarManager(QObject* parent)
    : QObject(parent)
{
}

CalendarManager::~CalendarManager() = default;

void CalendarManager::addAdapter(CalendarAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    m_adapters.append(adapter);
    
    // 連接適配器信號
    connect(adapter, &CalendarAdapter::eventsReceived,
            this, &CalendarManager::onAdapterEventsReceived);
    connect(adapter, &CalendarAdapter::tasksReceived,
            this, &CalendarManager::onAdapterTasksReceived);
    connect(adapter, &CalendarAdapter::errorOccurred,
            this, &CalendarManager::onAdapterError);
    
    qDebug() << "已新增平台適配器";
}

void CalendarManager::fetchAllEvents(const QDateTime& start, const QDateTime& end) {
    qDebug() << "從所有平台獲取事件...";
    
    m_allEvents.clear();
    
    for (auto* adapter : m_adapters) {
        adapter->fetchEvents(start, end);
    }
}

void CalendarManager::fetchAllTasks() {
    qDebug() << "從所有平台獲取任務...";
    
    m_allTasks.clear();
    
    for (auto* adapter : m_adapters) {
        adapter->fetchTasks();
    }
}

QList<CalendarEvent> CalendarManager::searchEvents(const QString& query) const {
    QList<CalendarEvent> results;
    
    for (const auto& event : m_allEvents) {
        if (event.title.contains(query, Qt::CaseInsensitive) ||
            event.description.contains(query, Qt::CaseInsensitive) ||
            event.location.contains(query, Qt::CaseInsensitive)) {
            results.append(event);
        }
    }
    
    return results;
}

void CalendarManager::onAdapterEventsReceived(const QList<CalendarEvent>& events) {
    qDebug() << "收到" << events.size() << "個事件";
    
    m_allEvents.append(events);
    emit eventsUpdated(m_allEvents);
}

void CalendarManager::onAdapterTasksReceived(const QList<Task>& tasks) {
    qDebug() << "收到" << tasks.size() << "個任務";
    
    m_allTasks.append(tasks);
    emit tasksUpdated(m_allTasks);
}

void CalendarManager::onAdapterError(const QString& error) {
    qWarning() << "適配器錯誤:" << error;
    emit errorOccurred(error);
}
