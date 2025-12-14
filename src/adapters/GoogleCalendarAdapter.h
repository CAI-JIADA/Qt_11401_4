#pragma once

#include "CalendarAdapter.h"
#include <QNetworkAccessManager>
#include <QOAuthHttpServerReplyHandler>
#include <QOAuth2AuthorizationCodeFlow>
#include <QDesktopServices>

// Google Calendar 適配器
class GoogleCalendarAdapter : public CalendarAdapter {
    Q_OBJECT
    
public:
    explicit GoogleCalendarAdapter(QObject* parent = nullptr);
    ~GoogleCalendarAdapter() override;
    
    // 設定 OAuth 2.0 憑證
    void setCredentials(const QString& clientId, const QString& clientSecret);
    
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
    QString m_accessToken;
    
    void setupOAuth();
    QList<CalendarEvent> parseEventsJson(const QByteArray& json);
    QList<Task> parseTasksJson(const QByteArray& json);
};
