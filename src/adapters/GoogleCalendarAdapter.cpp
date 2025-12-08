#include "GoogleCalendarAdapter.h"
#include <QDebug>

GoogleCalendarAdapter::GoogleCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
{
}

GoogleCalendarAdapter::~GoogleCalendarAdapter() = default;

void GoogleCalendarAdapter::authenticate() {
    qDebug() << "Google Calendar 適配器尚未實作";
    emit errorOccurred("Google Calendar 適配器尚未實作");
}

void GoogleCalendarAdapter::fetchEvents(const QDateTime& start, const QDateTime& end) {
    Q_UNUSED(start);
    Q_UNUSED(end);
    qDebug() << "Google Calendar fetchEvents 尚未實作";
}

void GoogleCalendarAdapter::fetchTasks() {
    qDebug() << "Google Calendar fetchTasks 尚未實作";
}
