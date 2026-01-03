#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QMetaType>
#include "core/CalendarEvent.h"

// 修改紀錄結構
struct ChangeLog {
    int id;
    QString entityType;  // "event" 或 "task"
    QString entityId;
    QString action;      // "created", "updated", "deleted"
    QString changedFields;
    QString oldValues;
    QString newValues;
    QDateTime timestamp;
    
    QString toString() const;
};

Q_DECLARE_METATYPE(ChangeLog)

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
    
    // 修改紀錄操作
    bool saveChangeLog(const ChangeLog& log);
    QList<ChangeLog> loadChangeLogs(int limit = 100);
    QList<ChangeLog> loadChangeLogsForEntity(const QString& entityType, const QString& entityId);
    
    // 設定操作
    bool saveSetting(const QString& key, const QString& value);
    QString loadSetting(const QString& key, const QString& defaultValue = QString());
    
private:
    QSqlDatabase m_db;
    
    bool createTables();
    CalendarEvent getExistingEvent(const QString& eventId);
};
