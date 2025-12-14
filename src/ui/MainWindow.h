#pragma once

#include <QMainWindow>
#include <QTreeWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QGroupBox>
#include "core/CalendarManager.h"
#include "adapters/GoogleCalendarAdapter.h"
#include "adapters/OutlookCalendarAdapter.h"
#include "storage/DatabaseManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    
private slots:
    void onGoogleAuthClicked();
    void onOutlookAuthClicked();
    void onFetchEventsClicked();
    void onSearchTextChanged(const QString& text);
    void onEventSelected(QListWidgetItem* item);
    void onEventsUpdated(const QList<CalendarEvent>& events);
    void onErrorOccurred(const QString& error);
    void onGoogleAuthenticated();
    void onOutlookAuthenticated();
    
private:
    void setupUI();
    void updateEventList(const QList<CalendarEvent>& events);
    void showEventDetails(const CalendarEvent& event);
    void updateStatusBar(const QString& message);
    
    // UI 元件
    QWidget* m_centralWidget;
    QTreeWidget* m_calendarTree;
    QListWidget* m_eventList;
    QTextEdit* m_eventDetails;
    QPushButton* m_googleAuthBtn;
    QPushButton* m_outlookAuthBtn;
    QPushButton* m_fetchEventsBtn;
    QLineEdit* m_searchEdit;
    QDateEdit* m_startDateEdit;
    QDateEdit* m_endDateEdit;
    QComboBox* m_platformFilter;
    QLabel* m_statusLabel;
    
    // 核心元件
    CalendarManager* m_manager;
    GoogleCalendarAdapter* m_googleAdapter;
    OutlookCalendarAdapter* m_outlookAdapter;
    DatabaseManager* m_dbManager;
    
    // 資料
    QList<CalendarEvent> m_currentEvents;  // 所有事件
    QList<CalendarEvent> m_displayedEvents;  // 目前顯示的事件（已過濾）
    bool m_googleAuthenticated;
    bool m_outlookAuthenticated;
};
