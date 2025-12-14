#include "CalendarEvent.h"

QString CalendarEvent::toString() const {
    return QString("Event: %1 (%2 - %3) at %4 [%5]")
        .arg(title)
        .arg(startTime.toString(Qt::ISODate))
        .arg(endTime.toString(Qt::ISODate))
        .arg(location)
        .arg(isAllDay ? "全天" : "非全天");
}

QString Task::toString() const {
    return QString("Task: %1 (Due: %2, Priority: %3) [%4]")
        .arg(title)
        .arg(dueDate.toString(Qt::ISODate))
        .arg(priority)
        .arg(isCompleted ? "已完成" : "未完成");
}
