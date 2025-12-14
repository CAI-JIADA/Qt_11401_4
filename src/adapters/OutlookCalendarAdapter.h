#pragma once

#include "CalendarAdapter.h"
#include <QNetworkAccessManager>
#include <QOAuthHttpServerReplyHandler>
#include <QOAuth2AuthorizationCodeFlow>
#include <QDesktopServices>

// Microsoft Outlook 適配器
class OutlookCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
    
public:
    explicit OutlookCalendarAdapter(QObject* parent = nullptr);
    ~OutlookCalendarAdapter() override;
    
    // 設定 OAuth 2.0 憑證 (Azure AD)
    void setCredentials(const QString& clientId, const QString& clientSecret, const QString& tenantId = "common");
    
    void authenticate() override;
    void fetchEvents(const QDateTime& start, const QDateTime& end) override;
    void fetchTasks() override;
    
private slots:
    void onAuthenticationGranted();
    void onAuthenticationError(const QString& error, const QString& errorDescription);
    void onEventsReplyFinished();
    void onTasksReplyFinished();
    
private:
    QNetworkAccessManager* m_networkManager;
    QOAuth2AuthorizationCodeFlow* m_oauth;
    QOAuthHttpServerReplyHandler* m_replyHandler;
    
    QString m_clientId;
    QString m_clientSecret;
    QString m_tenantId;
    QString m_accessToken;
    
    void setupOAuth();
    QList<CalendarEvent> parseEventsJson(const QByteArray& json);
    QList<Task> parseTasksJson(const QByteArray& json);
};
