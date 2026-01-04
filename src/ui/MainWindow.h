#pragma once

#include <QMainWindow>
#include "core/CalendarManager.h"
#include "adapters/GoogleCalendarAdapter.h"
#include "adapters/OutlookCalendarAdapter.h"
#include "storage/DatabaseManager.h"

class QListWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    void setupConnections();
    void updateEventList(const QList<CalendarEvent>& events);
    void showEventDetails(const CalendarEvent& event);
    void updateStatusBar(const QString& message);
    
    // UI
    Ui::MainWindow *ui;
    
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
