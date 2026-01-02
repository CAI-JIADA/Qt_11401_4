#pragma once

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QColor>

// 平台類型列舉
enum class Platform {
    Google,
    Outlook
};

// 統一的事件資料結構
class CalendarEvent {
public:
    CalendarEvent() = default;
    
    QString id;
    QString title;
    QString description;
    QDateTime startTime;
    QDateTime endTime;
    QString location;
    Platform platform;
    QString ownerId;
    bool isAllDay = false;
    QStringList attendees;
    QString recurrenceRule;
    QColor color;
    
    // 轉換為字串以便除錯
    QString toString() const;
};

// 任務/待辦事項
class Task {
public:
    Task() = default;
    
    QString id;
    QString title;
    QString description;
    QDateTime dueDate;
    Platform platform;
    QString ownerId;
    bool isCompleted = false;
    int priority = 3; // 1-5
    QStringList tags;
    
    QString toString() const;
};
