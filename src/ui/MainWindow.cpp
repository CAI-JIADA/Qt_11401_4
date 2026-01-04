#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QListWidgetItem>
#include <QColor>
#include <QIcon>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_googleAuthenticated(false)
    , m_outlookAuthenticated(false)
{
    ui->setupUi(this);
    
    // 設定日期編輯器的預設值
    ui->startDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDate(QDate::currentDate().addDays(30));
    // 初始化資料庫
    m_dbManager = new DatabaseManager(this);
    if (!m_dbManager->initialize("calendar.db")) {
        QMessageBox::critical(this, "錯誤", "資料庫初始化失敗！");
    }
    
    // 初始化行事曆管理器
    m_manager = new CalendarManager(this);
    
    // 初始化適配器
    m_googleAdapter = new GoogleCalendarAdapter(this);
    m_outlookAdapter = new OutlookCalendarAdapter(this);
    
    // 連接信號
    connect(m_manager, &CalendarManager::eventsUpdated,
            this, &MainWindow::onEventsUpdated);
    connect(m_manager, &CalendarManager::errorOccurred,
            this, &MainWindow::onErrorOccurred);
    
    connect(m_googleAdapter, &GoogleCalendarAdapter::authenticated,
            this, &MainWindow::onGoogleAuthenticated);
    connect(m_outlookAdapter, &OutlookCalendarAdapter::authenticated,
            this, &MainWindow::onOutlookAuthenticated);
    
    setupConnections();
    
    updateStatusBar("就緒 - 請先進行帳號認證");
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setupConnections() {
    // 連接按鈕信號
    connect(ui->googleAuthBtn, &QPushButton::clicked,
            this, &MainWindow::onGoogleAuthClicked);
    connect(ui->outlookAuthBtn, &QPushButton::clicked,
            this, &MainWindow::onOutlookAuthClicked);
    connect(ui->fetchEventsBtn, &QPushButton::clicked,
            this, &MainWindow::onFetchEventsClicked);
    
    // 連接搜尋信號
    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    
    // 連接列表選擇信號
    connect(ui->eventList, &QListWidget::itemClicked,
            this, &MainWindow::onEventSelected);
    
    // 連接選單動作
    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QMessageBox::about(this, "關於",
            "Qt 多平台行事曆整合工具\n\n"
            "整合 Google Calendar 和 Microsoft Outlook 的行事曆管理工具\n\n"
            "版本: 1.0.0");
    });
}

void MainWindow::onGoogleAuthClicked() {
    updateStatusBar("正在連接 Google Calendar...");
    
    // 從環境變數讀取 OAuth 憑證
    QString clientId = qEnvironmentVariable("GOOGLE_CLIENT_ID");
    QString clientSecret = qEnvironmentVariable("GOOGLE_CLIENT_SECRET");
    
    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        QMessageBox::warning(this, "設定錯誤",
            "請設定 Google OAuth 2.0 環境變數\n\n"
            "設定方式：\n"
            "export GOOGLE_CLIENT_ID=\"your_client_id.apps.googleusercontent.com\"\n"
            "export GOOGLE_CLIENT_SECRET=\"your_client_secret\"\n\n"
            "取得步驟：\n"
            "1. 前往 Google Cloud Console\n"
            "2. 建立專案並啟用 Calendar API\n"
            "3. 建立 OAuth 2.0 憑證\n"
            "4. 設定環境變數後重新啟動程式");
        updateStatusBar("就緒");
        return;
    }
    
    m_googleAdapter->setCredentials(clientId, clientSecret);
    m_googleAdapter->authenticate();
}

void MainWindow::onOutlookAuthClicked() {
    updateStatusBar("正在連接 Microsoft Outlook...");
    
    // 從環境變數讀取 OAuth 憑證
    QString clientId = qEnvironmentVariable("OUTLOOK_CLIENT_ID");
    QString clientSecret = qEnvironmentVariable("OUTLOOK_CLIENT_SECRET");
    
    if (clientId.isEmpty() || clientSecret.isEmpty()) {
        QMessageBox::warning(this, "設定錯誤",
            "請設定 Microsoft Azure AD 環境變數\n\n"
            "設定方式：\n"
            "export OUTLOOK_CLIENT_ID=\"your_application_client_id\"\n"
            "export OUTLOOK_CLIENT_SECRET=\"your_client_secret\"\n\n"
            "取得步驟：\n"
            "1. 前往 Azure Portal\n"
            "2. 註冊應用程式\n"
            "3. 設定 API 權限 (Calendars.Read, Tasks.Read)\n"
            "4. 建立 Client Secret\n"
            "5. 設定環境變數後重新啟動程式");
        updateStatusBar("就緒");
        return;
    }
    
    m_outlookAdapter->setCredentials(clientId, clientSecret);
    m_outlookAdapter->authenticate();
}

void MainWindow::onGoogleAuthenticated() {
    m_googleAuthenticated = true;
    ui->googleAuthBtn->setText("✓ Google Calendar 已連接");
    ui->googleAuthBtn->setEnabled(false);
    ui->googleAuthBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 6px; }");
    
    m_manager->addAdapter(m_googleAdapter);
    
    // 更新行事曆樹狀圖
    QTreeWidgetItem* googleItem = new QTreeWidgetItem(ui->calendarTree);
    googleItem->setText(0, "Google Calendar");
    googleItem->setIcon(0, QIcon());
    
    ui->fetchEventsBtn->setEnabled(true);
    updateStatusBar("Google Calendar 認證成功");
}

void MainWindow::onOutlookAuthenticated() {
    m_outlookAuthenticated = true;
    ui->outlookAuthBtn->setText("✓ Outlook 已連接");
    ui->outlookAuthBtn->setEnabled(false);
    ui->outlookAuthBtn->setStyleSheet("QPushButton { background-color: #0078D4; color: white; padding: 6px; }");
    
    m_manager->addAdapter(m_outlookAdapter);
    
    // 更新行事曆樹狀圖
    QTreeWidgetItem* outlookItem = new QTreeWidgetItem(ui->calendarTree);
    outlookItem->setText(0, "Microsoft Outlook");
    outlookItem->setIcon(0, QIcon());
    
    ui->fetchEventsBtn->setEnabled(true);
    updateStatusBar("Microsoft Outlook 認證成功");
}

void MainWindow::onFetchEventsClicked() {
    if (!m_googleAuthenticated && !m_outlookAuthenticated) {
        QMessageBox::warning(this, "警告", "請先進行至少一個帳號的認證");
        return;
    }
    
    QDateTime start(ui->startDateEdit->date(), QTime(0, 0));
    QDateTime end(ui->endDateEdit->date(), QTime(23, 59, 59));
    
    updateStatusBar(QString("正在獲取事件 (%1 至 %2)...")
                   .arg(start.toString("yyyy-MM-dd"))
                   .arg(end.toString("yyyy-MM-dd")));
    
    m_manager->fetchAllEvents(start, end);
}

void MainWindow::onSearchTextChanged(const QString& text) {
    if (text.isEmpty()) {
        updateEventList(m_currentEvents);
        return;
    }
    
    // 簡單的搜尋過濾
    QList<CalendarEvent> filtered;
    for (const auto& event : m_currentEvents) {
        if (event.title.contains(text, Qt::CaseInsensitive) ||
            event.description.contains(text, Qt::CaseInsensitive) ||
            event.location.contains(text, Qt::CaseInsensitive)) {
            filtered.append(event);
        }
    }
    
    updateEventList(filtered);
}

void MainWindow::onEventSelected(QListWidgetItem* item) {
    if (!item) return;
    
    // 使用顯示的事件列表，而非所有事件
    int index = ui->eventList->row(item);
    if (index >= 0 && index < m_displayedEvents.size()) {
        showEventDetails(m_displayedEvents[index]);
    }
}

void MainWindow::onEventsUpdated(const QList<CalendarEvent>& events) {
    m_currentEvents = events;
    updateEventList(events);
    
    // 儲存到資料庫
    for (const auto& event : events) {
        m_dbManager->saveEvent(event);
    }
    
    updateStatusBar(QString("已獲取 %1 個事件").arg(events.size()));
}

void MainWindow::onErrorOccurred(const QString& error) {
    QMessageBox::critical(this, "錯誤", error);
    updateStatusBar(QString("錯誤: %1").arg(error));
}

void MainWindow::updateEventList(const QList<CalendarEvent>& events) {
    ui->eventList->clear();
    m_displayedEvents.clear();  // 清除顯示的事件列表
    
    QString platformFilter = ui->platformFilter->currentText();
    
    for (const auto& event : events) {
        // 平台篩選
        if (platformFilter != "全部") {
            QString platform;
            switch (event.platform) {
                case Platform::Google: platform = "Google"; break;
                case Platform::Outlook: platform = "Outlook"; break;
                default: platform = "Unknown"; break;
            }
            if (platform != platformFilter) {
                continue;
            }
        }
        
        // 將事件加入顯示列表
        m_displayedEvents.append(event);
        
        QString itemText = QString("%1 - %2")
            .arg(event.startTime.toString("yyyy-MM-dd hh:mm"))
            .arg(event.title);
        
        QListWidgetItem* item = new QListWidgetItem(itemText);
        
        // 根據平台設定顏色
        switch (event.platform) {
            case Platform::Google:
                item->setForeground(QColor("#4285F4"));
                break;
            case Platform::Outlook:
                item->setForeground(QColor("#0078D4"));
                break;
            default:
                break;
        }
        
        ui->eventList->addItem(item);
    }
}

void MainWindow::showEventDetails(const CalendarEvent& event) {
    QString platformName;
    switch (event.platform) {
        case Platform::Google: platformName = "Google Calendar"; break;
        case Platform::Outlook: platformName = "Microsoft Outlook"; break;
        default: platformName = "Unknown"; break;
    }
    
    QString details = QString(
        "<h2>%1</h2>"
        "<p><b>平台:</b> %2</p>"
        "<p><b>開始時間:</b> %3</p>"
        "<p><b>結束時間:</b> %4</p>"
        "<p><b>全天事件:</b> %5</p>"
        "<p><b>地點:</b> %6</p>"
        "<p><b>描述:</b></p>"
        "<p>%7</p>"
    )
        .arg(event.title)
        .arg(platformName)
        .arg(event.startTime.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(event.endTime.toString("yyyy-MM-dd hh:mm:ss"))
        .arg(event.isAllDay ? "是" : "否")
        .arg(event.location.isEmpty() ? "無" : event.location)
        .arg(event.description.isEmpty() ? "無" : event.description);
    
    if (!event.attendees.isEmpty()) {
        details += "<p><b>參與者:</b></p><ul>";
        for (const QString& attendee : event.attendees) {
            details += QString("<li>%1</li>").arg(attendee);
        }
        details += "</ul>";
    }
    
    ui->eventDetails->setHtml(details);
}

void MainWindow::updateStatusBar(const QString& message) {
    statusBar()->showMessage(message, 3000);
}
