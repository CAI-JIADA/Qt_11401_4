#include "DatabaseManager.h"
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QDebug>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent)
    , m_connectionName(QUuid::createUuid().toString())
{
}

bool DatabaseManager::initialize(const QString& dbPath)
{
    QString path = dbPath;
    if (path.isEmpty()) {
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataDir);
        path = dataDir + "/calendar.db";
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(path);

    if (!m_database.open()) {
        emit errorOccurred(tr("Failed to open database: %1").arg(m_database.lastError().text()));
        return false;
    }

    if (!createTables()) {
        return false;
    }

    if (!createFTSIndex()) {
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::isOpen() const
{
    return m_database.isOpen();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // 行事曆表
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS calendars (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            platform TEXT NOT NULL,
            owner_id TEXT,
            is_shared INTEGER DEFAULT 0,
            color TEXT,
            last_sync TEXT
        )
    )")) {
        emit errorOccurred(tr("Failed to create calendars table: %1").arg(query.lastError().text()));
        return false;
    }

    // 事件表
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS events (
            id TEXT PRIMARY KEY,
            calendar_id TEXT,
            title TEXT NOT NULL,
            description TEXT,
            start_time TEXT NOT NULL,
            end_time TEXT,
            location TEXT,
            platform TEXT NOT NULL,
            owner_id TEXT,
            is_all_day INTEGER DEFAULT 0,
            attendees TEXT,
            recurrence_rule TEXT,
            color TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )")) {
        emit errorOccurred(tr("Failed to create events table: %1").arg(query.lastError().text()));
        return false;
    }

    // 任務表
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id TEXT PRIMARY KEY,
            calendar_id TEXT,
            title TEXT NOT NULL,
            description TEXT,
            due_date TEXT,
            platform TEXT NOT NULL,
            owner_id TEXT,
            is_completed INTEGER DEFAULT 0,
            priority INTEGER DEFAULT 3,
            tags TEXT,
            created_at TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )")) {
        emit errorOccurred(tr("Failed to create tasks table: %1").arg(query.lastError().text()));
        return false;
    }

    // 建立索引
    query.exec("CREATE INDEX IF NOT EXISTS idx_events_start_time ON events(start_time)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_events_platform ON events(platform)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_tasks_due_date ON tasks(due_date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_tasks_platform ON tasks(platform)");

    return true;
}

bool DatabaseManager::createFTSIndex()
{
    QSqlQuery query(m_database);

    // 使用 FTS5 建立全文搜索索引
    if (!query.exec(R"(
        CREATE VIRTUAL TABLE IF NOT EXISTS events_fts USING fts5(
            title,
            description,
            location,
            content='events',
            content_rowid='rowid'
        )
    )")) {
        // FTS5 可能不可用，記錄錯誤訊息後繼續執行
        qWarning() << "FTS5 not available, full-text search will use LIKE fallback:" 
                   << query.lastError().text();
        return true;
    }

    // 建立觸發器以保持 FTS 索引同步
    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS events_ai AFTER INSERT ON events BEGIN
            INSERT INTO events_fts(rowid, title, description, location)
            VALUES (NEW.rowid, NEW.title, NEW.description, NEW.location);
        END
    )");

    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS events_ad AFTER DELETE ON events BEGIN
            INSERT INTO events_fts(events_fts, rowid, title, description, location)
            VALUES ('delete', OLD.rowid, OLD.title, OLD.description, OLD.location);
        END
    )");

    query.exec(R"(
        CREATE TRIGGER IF NOT EXISTS events_au AFTER UPDATE ON events BEGIN
            INSERT INTO events_fts(events_fts, rowid, title, description, location)
            VALUES ('delete', OLD.rowid, OLD.title, OLD.description, OLD.location);
            INSERT INTO events_fts(rowid, title, description, location)
            VALUES (NEW.rowid, NEW.title, NEW.description, NEW.location);
        END
    )");

    return true;
}

bool DatabaseManager::saveCalendar(const QString& id, const QString& name, Platform platform,
                                    const QString& ownerId, bool isShared, const QString& color)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO calendars (id, name, platform, owner_id, is_shared, color)
        VALUES (:id, :name, :platform, :owner_id, :is_shared, :color)
    )");
    query.bindValue(":id", id);
    query.bindValue(":name", name);
    query.bindValue(":platform", platformToString(platform));
    query.bindValue(":owner_id", ownerId);
    query.bindValue(":is_shared", isShared ? 1 : 0);
    query.bindValue(":color", color);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool DatabaseManager::deleteCalendar(const QString& id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM calendars WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool DatabaseManager::saveEvent(const CalendarEvent& event)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO events
        (id, title, description, start_time, end_time, location, platform,
         owner_id, is_all_day, attendees, recurrence_rule, color, updated_at)
        VALUES
        (:id, :title, :description, :start_time, :end_time, :location, :platform,
         :owner_id, :is_all_day, :attendees, :recurrence_rule, :color, :updated_at)
    )");
    query.bindValue(":id", event.id);
    query.bindValue(":title", event.title);
    query.bindValue(":description", event.description);
    query.bindValue(":start_time", event.startTime.toString(Qt::ISODate));
    query.bindValue(":end_time", event.endTime.toString(Qt::ISODate));
    query.bindValue(":location", event.location);
    query.bindValue(":platform", platformToString(event.platform));
    query.bindValue(":owner_id", event.ownerId);
    query.bindValue(":is_all_day", event.isAllDay ? 1 : 0);
    query.bindValue(":attendees", event.attendees.join(","));
    query.bindValue(":recurrence_rule", event.recurrenceRule);
    query.bindValue(":color", event.color.name());
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool DatabaseManager::deleteEvent(const QString& id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM events WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

QList<CalendarEvent> DatabaseManager::getAllEvents() const
{
    QList<CalendarEvent> events;
    QSqlQuery query(m_database);
    query.exec("SELECT * FROM events ORDER BY start_time");

    while (query.next()) {
        events.append(eventFromQuery(query));
    }

    return events;
}

QList<CalendarEvent> DatabaseManager::getEventsByDateRange(const QDate& start, const QDate& end) const
{
    QList<CalendarEvent> events;
    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT * FROM events
        WHERE date(start_time) <= :end AND date(end_time) >= :start
        ORDER BY start_time
    )");
    query.bindValue(":start", start.toString(Qt::ISODate));
    query.bindValue(":end", end.toString(Qt::ISODate));
    query.exec();

    while (query.next()) {
        events.append(eventFromQuery(query));
    }

    return events;
}

QList<CalendarEvent> DatabaseManager::getEventsByPlatform(Platform platform) const
{
    QList<CalendarEvent> events;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM events WHERE platform = :platform ORDER BY start_time");
    query.bindValue(":platform", platformToString(platform));
    query.exec();

    while (query.next()) {
        events.append(eventFromQuery(query));
    }

    return events;
}

CalendarEvent DatabaseManager::eventFromQuery(const QSqlQuery& query) const
{
    CalendarEvent event;
    event.id = query.value("id").toString();
    event.title = query.value("title").toString();
    event.description = query.value("description").toString();
    event.startTime = QDateTime::fromString(query.value("start_time").toString(), Qt::ISODate);
    event.endTime = QDateTime::fromString(query.value("end_time").toString(), Qt::ISODate);
    event.location = query.value("location").toString();
    event.platform = stringToPlatform(query.value("platform").toString());
    event.ownerId = query.value("owner_id").toString();
    event.isAllDay = query.value("is_all_day").toBool();
    event.attendees = query.value("attendees").toString().split(",", Qt::SkipEmptyParts);
    event.recurrenceRule = query.value("recurrence_rule").toString();
    event.color = QColor(query.value("color").toString());
    return event;
}

bool DatabaseManager::saveTask(const Task& task)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT OR REPLACE INTO tasks
        (id, title, description, due_date, platform, owner_id, is_completed, priority, tags)
        VALUES
        (:id, :title, :description, :due_date, :platform, :owner_id, :is_completed, :priority, :tags)
    )");
    query.bindValue(":id", task.id);
    query.bindValue(":title", task.title);
    query.bindValue(":description", task.description);
    query.bindValue(":due_date", task.dueDate.toString(Qt::ISODate));
    query.bindValue(":platform", platformToString(task.platform));
    query.bindValue(":owner_id", task.ownerId);
    query.bindValue(":is_completed", task.isCompleted ? 1 : 0);
    query.bindValue(":priority", task.priority);
    query.bindValue(":tags", task.tags.join(","));

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

bool DatabaseManager::deleteTask(const QString& id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM tasks WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    emit dataChanged();
    return true;
}

QList<Task> DatabaseManager::getAllTasks() const
{
    QList<Task> tasks;
    QSqlQuery query(m_database);
    query.exec("SELECT * FROM tasks ORDER BY is_completed, priority, due_date");

    while (query.next()) {
        tasks.append(taskFromQuery(query));
    }

    return tasks;
}

QList<Task> DatabaseManager::getTasksByPlatform(Platform platform) const
{
    QList<Task> tasks;
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM tasks WHERE platform = :platform ORDER BY is_completed, priority, due_date");
    query.bindValue(":platform", platformToString(platform));
    query.exec();

    while (query.next()) {
        tasks.append(taskFromQuery(query));
    }

    return tasks;
}

Task DatabaseManager::taskFromQuery(const QSqlQuery& query) const
{
    Task task;
    task.id = query.value("id").toString();
    task.title = query.value("title").toString();
    task.description = query.value("description").toString();
    task.dueDate = QDateTime::fromString(query.value("due_date").toString(), Qt::ISODate);
    task.platform = stringToPlatform(query.value("platform").toString());
    task.ownerId = query.value("owner_id").toString();
    task.isCompleted = query.value("is_completed").toBool();
    task.priority = query.value("priority").toInt();
    task.tags = query.value("tags").toString().split(",", Qt::SkipEmptyParts);
    return task;
}

QList<CalendarEvent> DatabaseManager::searchEvents(const QString& query) const
{
    QList<CalendarEvent> events;
    QSqlQuery sqlQuery(m_database);

    // 嘗試使用 FTS5 搜索
    sqlQuery.prepare(R"(
        SELECT e.* FROM events e
        JOIN events_fts f ON e.rowid = f.rowid
        WHERE events_fts MATCH :query
        ORDER BY e.start_time
    )");
    sqlQuery.bindValue(":query", query);

    if (sqlQuery.exec()) {
        while (sqlQuery.next()) {
            events.append(eventFromQuery(sqlQuery));
        }
    } else {
        // 回退到 LIKE 搜索
        sqlQuery.prepare(R"(
            SELECT * FROM events
            WHERE title LIKE :query OR description LIKE :query OR location LIKE :query
            ORDER BY start_time
        )");
        sqlQuery.bindValue(":query", QString("%%%1%%").arg(query));
        sqlQuery.exec();

        while (sqlQuery.next()) {
            events.append(eventFromQuery(sqlQuery));
        }
    }

    return events;
}

bool DatabaseManager::updateLastSync(const QString& calendarId, const QDateTime& syncTime)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE calendars SET last_sync = :sync_time WHERE id = :id");
    query.bindValue(":id", calendarId);
    query.bindValue(":sync_time", syncTime.toString(Qt::ISODate));

    if (!query.exec()) {
        emit errorOccurred(query.lastError().text());
        return false;
    }

    return true;
}
