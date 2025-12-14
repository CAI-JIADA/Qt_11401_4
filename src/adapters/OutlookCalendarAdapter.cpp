#include "OutlookCalendarAdapter.h"
#include <QDebug>

OutlookCalendarAdapter::OutlookCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
{
}

OutlookCalendarAdapter::~OutlookCalendarAdapter() = default;

void OutlookCalendarAdapter::authenticate() {
    qDebug() << "Outlook 適配器尚未實作";
    emit errorOccurred("Outlook 適配器尚未實作");
}

void OutlookCalendarAdapter::fetchEvents(const QDateTime& start, const QDateTime& end) {
    Q_UNUSED(start);
    Q_UNUSED(end);
    qDebug() << "Outlook fetchEvents 尚未實作";
}

void OutlookCalendarAdapter::fetchTasks() {
    qDebug() << "Outlook fetchTasks 尚未實作";
}
