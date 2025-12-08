#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

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
    
    qDebug() << "資料庫表已建立";
    return true;
}

bool DatabaseManager::saveEvent(const CalendarEvent& event) {
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
    
    return true;
}

bool DatabaseManager::deleteEvent(const QString& eventId) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM events WHERE id = ?");
    query.addBindValue(eventId);
    
    if (!query.exec()) {
        qWarning() << "刪除事件失敗:" << query.lastError().text();
        return false;
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
