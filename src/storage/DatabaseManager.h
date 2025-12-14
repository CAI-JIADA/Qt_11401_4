#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "core/CalendarEvent.h"

// 資料庫管理器 - 本地儲存
class DatabaseManager : public QObject {
    Q_OBJECT
    
public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager() override;
    
    // 初始化資料庫
    bool initialize(const QString& dbPath = "calendar.db");
    
    // 事件操作
    bool saveEvent(const CalendarEvent& event);
    bool deleteEvent(const QString& eventId);
    QList<CalendarEvent> loadEvents();
    
    // 任務操作
    bool saveTask(const Task& task);
    bool deleteTask(const QString& taskId);
    QList<Task> loadTasks();
    
private:
    QSqlDatabase m_db;
    
    bool createTables();
};
