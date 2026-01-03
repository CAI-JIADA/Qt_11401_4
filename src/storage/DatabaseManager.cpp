#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
{
}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initialize(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    
    if (!m_db.open()) {
        qCritical() << "無法開啟資料庫:" << m_db.lastError().text();
        return false;
    }
    
    qDebug() << "資料庫已開啟:" << dbPath;
    
    return createTables();
}

bool DatabaseManager::createTables() {
    QSqlQuery query(m_db);
    
    // 建立事件表
    QString createEventsTable = R"(
        CREATE TABLE IF NOT EXISTS events (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            start_time DATETIME NOT NULL,
            end_time DATETIME,
            location TEXT,
            platform INTEGER,
            owner_id TEXT,
            is_all_day INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createEventsTable)) {
        qCritical() << "建立 events 表失敗:" << query.lastError().text();
        return false;
    }
    
    // 建立任務表
    QString createTasksTable = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            description TEXT,
            due_date DATETIME,
            platform INTEGER,
            owner_id TEXT,
            is_completed INTEGER DEFAULT 0,
            priority INTEGER DEFAULT 3,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createTasksTable)) {
        qCritical() << "建立 tasks 表失敗:" << query.lastError().text();
        return false;
    }
    
    // 建立修改紀錄表
    QString createChangeLogsTable = R"(
        CREATE TABLE IF NOT EXISTS change_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            entity_type TEXT NOT NULL,
            entity_id TEXT NOT NULL,
            action TEXT NOT NULL,
            changed_fields TEXT,
            old_values TEXT,
            new_values TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createChangeLogsTable)) {
        qCritical() << "建立 change_logs 表失敗:" << query.lastError().text();
        return false;
    }
    
    // 建立設定表
    QString createSettingsTable = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createSettingsTable)) {
        qCritical() << "建立 settings 表失敗:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "資料庫表已建立";
    return true;
}

bool DatabaseManager::saveEvent(const CalendarEvent& event) {
    // 檢查事件是否已存在，以判斷是新增或更新
    CalendarEvent oldEvent = getExistingEvent(event.id);
    bool isUpdate = !oldEvent.id.isEmpty();
    
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO events 
        (id, title, description, start_time, end_time, location, platform, owner_id, is_all_day)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(event.id);
    query.addBindValue(event.title);
    query.addBindValue(event.description);
    query.addBindValue(event.startTime);
    query.addBindValue(event.endTime);
    query.addBindValue(event.location);
    query.addBindValue(static_cast<int>(event.platform));
    query.addBindValue(event.ownerId);
    query.addBindValue(event.isAllDay ? 1 : 0);
    
    if (!query.exec()) {
        qWarning() << "儲存事件失敗:" << query.lastError().text();
        return false;
    }
    
    // 記錄修改
    ChangeLog log;
    log.entityType = "event";
    log.entityId = event.id;
    log.action = isUpdate ? "updated" : "created";
    log.timestamp = QDateTime::currentDateTime();
    
    if (isUpdate) {
        // 記錄變更的欄位
        QStringList changedFields;
        QJsonObject oldValues, newValues;
        
        if (oldEvent.title != event.title) {
            changedFields << "title";
            oldValues["title"] = oldEvent.title;
            newValues["title"] = event.title;
        }
        if (oldEvent.description != event.description) {
            changedFields << "description";
            oldValues["description"] = oldEvent.description;
            newValues["description"] = event.description;
        }
        if (oldEvent.location != event.location) {
            changedFields << "location";
            oldValues["location"] = oldEvent.location;
            newValues["location"] = event.location;
        }
        if (oldEvent.startTime != event.startTime) {
            changedFields << "startTime";
            oldValues["startTime"] = oldEvent.startTime.toString(Qt::ISODate);
            newValues["startTime"] = event.startTime.toString(Qt::ISODate);
        }
        if (oldEvent.endTime != event.endTime) {
            changedFields << "endTime";
            oldValues["endTime"] = oldEvent.endTime.toString(Qt::ISODate);
            newValues["endTime"] = event.endTime.toString(Qt::ISODate);
        }
        
        log.changedFields = changedFields.join(", ");
        log.oldValues = QString::fromUtf8(QJsonDocument(oldValues).toJson(QJsonDocument::Compact));
        log.newValues = QString::fromUtf8(QJsonDocument(newValues).toJson(QJsonDocument::Compact));
    } else {
        // 新增事件
        log.changedFields = "all";
        QJsonObject newValues;
        newValues["title"] = event.title;
        newValues["description"] = event.description;
        newValues["location"] = event.location;
        newValues["startTime"] = event.startTime.toString(Qt::ISODate);
        newValues["endTime"] = event.endTime.toString(Qt::ISODate);
        log.newValues = QString::fromUtf8(QJsonDocument(newValues).toJson(QJsonDocument::Compact));
    }
    
    saveChangeLog(log);
    
    return true;
}

bool DatabaseManager::deleteEvent(const QString& eventId) {
    // 在刪除前獲取事件資料用於記錄
    CalendarEvent oldEvent = getExistingEvent(eventId);
    
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM events WHERE id = ?");
    query.addBindValue(eventId);
    
    if (!query.exec()) {
        qWarning() << "刪除事件失敗:" << query.lastError().text();
        return false;
    }
    
    // 記錄刪除
    if (!oldEvent.id.isEmpty()) {
        ChangeLog log;
        log.entityType = "event";
        log.entityId = eventId;
        log.action = "deleted";
        log.timestamp = QDateTime::currentDateTime();
        log.changedFields = "all";
        
        QJsonObject oldValues;
        oldValues["title"] = oldEvent.title;
        oldValues["description"] = oldEvent.description;
        oldValues["location"] = oldEvent.location;
        oldValues["startTime"] = oldEvent.startTime.toString(Qt::ISODate);
        oldValues["endTime"] = oldEvent.endTime.toString(Qt::ISODate);
        log.oldValues = QString::fromUtf8(QJsonDocument(oldValues).toJson(QJsonDocument::Compact));
        
        saveChangeLog(log);
    }
    
    return true;
}

QList<CalendarEvent> DatabaseManager::loadEvents() {
    QList<CalendarEvent> events;
    
    QSqlQuery query("SELECT * FROM events ORDER BY start_time", m_db);
    
    while (query.next()) {
        CalendarEvent event;
        event.id = query.value("id").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.startTime = query.value("start_time").toDateTime();
        event.endTime = query.value("end_time").toDateTime();
        event.location = query.value("location").toString();
        event.platform = static_cast<Platform>(query.value("platform").toInt());
        event.ownerId = query.value("owner_id").toString();
        event.isAllDay = query.value("is_all_day").toInt() != 0;
        
        events.append(event);
    }
    
    qDebug() << "載入" << events.size() << "個事件";
    return events;
}

bool DatabaseManager::saveTask(const Task& task) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO tasks 
        (id, title, description, due_date, platform, owner_id, is_completed, priority)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(task.id);
    query.addBindValue(task.title);
    query.addBindValue(task.description);
    query.addBindValue(task.dueDate);
    query.addBindValue(static_cast<int>(task.platform));
    query.addBindValue(task.ownerId);
    query.addBindValue(task.isCompleted ? 1 : 0);
    query.addBindValue(task.priority);
    
    if (!query.exec()) {
        qWarning() << "儲存任務失敗:" << query.lastError().text();
        return false;
    }
    
    return true;
}

bool DatabaseManager::deleteTask(const QString& taskId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM tasks WHERE id = ?");
    query.addBindValue(taskId);
    
    if (!query.exec()) {
        qWarning() << "刪除任務失敗:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<Task> DatabaseManager::loadTasks() {
    QList<Task> tasks;
    
    QSqlQuery query("SELECT * FROM tasks ORDER BY due_date", m_db);
    
    while (query.next()) {
        Task task;
        task.id = query.value("id").toString();
        task.title = query.value("title").toString();
        task.description = query.value("description").toString();
        task.dueDate = query.value("due_date").toDateTime();
        task.platform = static_cast<Platform>(query.value("platform").toInt());
        task.ownerId = query.value("owner_id").toString();
        task.isCompleted = query.value("is_completed").toInt() != 0;
        task.priority = query.value("priority").toInt();
        
        tasks.append(task);
    }
    
    qDebug() << "載入" << tasks.size() << "個任務";
    return tasks;
}

// 修改紀錄操作
bool DatabaseManager::saveChangeLog(const ChangeLog& log) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO change_logs 
        (entity_type, entity_id, action, changed_fields, old_values, new_values, timestamp)
        VALUES (?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(log.entityType);
    query.addBindValue(log.entityId);
    query.addBindValue(log.action);
    query.addBindValue(log.changedFields);
    query.addBindValue(log.oldValues);
    query.addBindValue(log.newValues);
    query.addBindValue(log.timestamp);
    
    if (!query.exec()) {
        qWarning() << "儲存修改紀錄失敗:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QList<ChangeLog> DatabaseManager::loadChangeLogs(int limit) {
    QList<ChangeLog> logs;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM change_logs ORDER BY timestamp DESC LIMIT ?");
    query.addBindValue(limit);
    
    if (!query.exec()) {
        qWarning() << "載入修改紀錄失敗:" << query.lastError().text();
        return logs;
    }
    
    while (query.next()) {
        ChangeLog log;
        log.id = query.value("id").toInt();
        log.entityType = query.value("entity_type").toString();
        log.entityId = query.value("entity_id").toString();
        log.action = query.value("action").toString();
        log.changedFields = query.value("changed_fields").toString();
        log.oldValues = query.value("old_values").toString();
        log.newValues = query.value("new_values").toString();
        log.timestamp = query.value("timestamp").toDateTime();
        
        logs.append(log);
    }
    
    qDebug() << "載入" << logs.size() << "筆修改紀錄";
    return logs;
}

QList<ChangeLog> DatabaseManager::loadChangeLogsForEntity(const QString& entityType, const QString& entityId) {
    QList<ChangeLog> logs;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM change_logs WHERE entity_type = ? AND entity_id = ? ORDER BY timestamp DESC");
    query.addBindValue(entityType);
    query.addBindValue(entityId);
    
    if (!query.exec()) {
        qWarning() << "載入特定實體的修改紀錄失敗:" << query.lastError().text();
        return logs;
    }
    
    while (query.next()) {
        ChangeLog log;
        log.id = query.value("id").toInt();
        log.entityType = query.value("entity_type").toString();
        log.entityId = query.value("entity_id").toString();
        log.action = query.value("action").toString();
        log.changedFields = query.value("changed_fields").toString();
        log.oldValues = query.value("old_values").toString();
        log.newValues = query.value("new_values").toString();
        log.timestamp = query.value("timestamp").toDateTime();
        
        logs.append(log);
    }
    
    return logs;
}

// 設定操作
bool DatabaseManager::saveSetting(const QString& key, const QString& value) {
    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT OR REPLACE INTO settings (key, value, updated_at)
        VALUES (?, ?, ?)
    )");
    
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(QDateTime::currentDateTime());
    
    if (!query.exec()) {
        qWarning() << "儲存設定失敗:" << query.lastError().text();
        return false;
    }
    
    return true;
}

QString DatabaseManager::loadSetting(const QString& key, const QString& defaultValue) {
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM settings WHERE key = ?");
    query.addBindValue(key);
    
    if (!query.exec()) {
        qWarning() << "載入設定失敗:" << query.lastError().text();
        return defaultValue;
    }
    
    if (query.next()) {
        return query.value(0).toString();
    }
    
    return defaultValue;
}

// 輔助方法
CalendarEvent DatabaseManager::getExistingEvent(const QString& eventId) {
    CalendarEvent event;
    
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM events WHERE id = ?");
    query.addBindValue(eventId);
    
    if (!query.exec()) {
        return event;
    }
    
    if (query.next()) {
        event.id = query.value("id").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.startTime = query.value("start_time").toDateTime();
        event.endTime = query.value("end_time").toDateTime();
        event.location = query.value("location").toString();
        event.platform = static_cast<Platform>(query.value("platform").toInt());
        event.ownerId = query.value("owner_id").toString();
        event.isAllDay = query.value("is_all_day").toInt() != 0;
    }
    
    return event;
}

QString ChangeLog::toString() const {
    return QString("ChangeLog[%1] %2 %3 at %4: %5")
        .arg(id)
        .arg(entityType)
        .arg(action)
        .arg(timestamp.toString(Qt::ISODate))
        .arg(changedFields);
}
