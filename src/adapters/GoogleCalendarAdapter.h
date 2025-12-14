#pragma once

#include "CalendarAdapter.h"

// Google Calendar 適配器 (佔位符)
class GoogleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
    
public:
    explicit GoogleCalendarAdapter(QObject* parent = nullptr);
    ~GoogleCalendarAdapter() override;
    
    void authenticate() override;
    void fetchEvents(const QDateTime& start, const QDateTime& end) override;
    void fetchTasks() override;
};
