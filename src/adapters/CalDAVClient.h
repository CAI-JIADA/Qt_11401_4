#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QSslConfiguration>
#include <QDateTime>
#include <QUrl>
#include "CalendarEvent.h"

// 行事曆資訊結構
struct CalendarInfo {
    QString url;           // CalDAV URL
    QString displayName;   // 顯示名稱
    QString description;   // 描述
    QString color;         // 顏色 (hex)
    QStringList supportedComponents; // 支援的元件 (VEVENT, VTODO 等)
};

// CalDAV 客戶端
class CalDAVClient : public QObject {
    Q_OBJECT
    
public:
    explicit CalDAVClient(QObject* parent = nullptr);
    ~CalDAVClient() override;
    
    // 設定認證資訊
    void setCredentials(const QString& username, const QString& appPassword);
    
    // 設定伺服器
    void setServer(const QString& server, quint16 port = 443);
    
    // 服務發現
    void discoverService();
    void discoverCalendars();
    
    // 行事曆操作
    void listCalendars();
    void getEvents(const QString& calendarUrl, 
                   const QDateTime& start, 
                   const QDateTime& end);
    
signals:
    void serviceDiscovered(const QString& principalUrl);
    void calendarsListed(const QList<CalendarInfo>& calendars);
    void eventsReceived(const QList<CalendarEvent>& events);
    void operationSucceeded();
    void errorOccurred(const QString& error);
    
private slots:
    void handleAuthenticationRequired(QNetworkReply* reply, 
                                      QAuthenticator* authenticator);
    void handleSslErrors(QNetworkReply* reply, 
                        const QList<QSslError>& errors);
    
private:
    QNetworkAccessManager* m_manager;
    QString m_username;
    QString m_password;
    QString m_baseUrl;
    QString m_server;
    quint16 m_port;
    
    // CalDAV 請求方法
    void sendPropfind(const QUrl& url, const QByteArray& xml, int depth = 1);
    void sendReport(const QUrl& url, const QByteArray& xml);
    
    // XML 處理
    QList<CalendarInfo> parseCalendarList(const QByteArray& xml);
    QList<CalendarEvent> parseEventsFromCalDAV(const QByteArray& xml);
    QString parsePrincipalUrl(const QByteArray& xml);
};
