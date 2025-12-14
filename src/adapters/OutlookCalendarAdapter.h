#pragma once

#include "CalendarAdapter.h"

// Microsoft Outlook 適配器 (佔位符)
class OutlookCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
    
public:
    explicit OutlookCalendarAdapter(QObject* parent = nullptr);
    ~OutlookCalendarAdapter() override;
    
    void authenticate() override;
    void fetchEvents(const QDateTime& start, const QDateTime& end) override;
    void fetchTasks() override;
};
