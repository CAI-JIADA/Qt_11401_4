#include "OutlookCalendarAdapter.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDateTime>
#include <QAbstractOAuth>

OutlookCalendarAdapter::OutlookCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_oauth(nullptr)
    , m_replyHandler(nullptr)
    , m_tenantId("common")
{
}

OutlookCalendarAdapter::~OutlookCalendarAdapter() = default;

void OutlookCalendarAdapter::setCredentials(const QString& clientId, const QString& clientSecret, const QString& tenantId) {
    m_clientId = clientId;
    m_clientSecret = clientSecret;
    m_tenantId = tenantId;
    setupOAuth();
}

void OutlookCalendarAdapter::setupOAuth() {
    if (m_oauth) {
        delete m_oauth;
    }
    
    m_oauth = new QOAuth2AuthorizationCodeFlow(this);
    m_oauth->setScope("https://graph.microsoft.com/Calendars.Read https://graph.microsoft.com/Tasks.Read https://graph.microsoft.com/User.Read");
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
    
    // Azure AD 端點
    QString authUrl = QString("https://login.microsoftonline.com/%1/oauth2/v2.0/authorize").arg(m_tenantId);
    QString tokenUrl = QString("https://login.microsoftonline.com/%1/oauth2/v2.0/token").arg(m_tenantId);
    
    m_oauth->setAuthorizationUrl(QUrl(authUrl));
    m_oauth->setAccessTokenUrl(QUrl(tokenUrl));
    m_oauth->setClientIdentifier(m_clientId);
    m_oauth->setClientIdentifierSharedKey(m_clientSecret);
    
    // 設定本地回調處理器
    if (m_replyHandler) {
        delete m_replyHandler;
    }
    m_replyHandler = new QOAuthHttpServerReplyHandler(8081, this);
    m_replyHandler->setCallbackPath("/");  // 明確設定回調路徑為根路徑
    m_oauth->setReplyHandler(m_replyHandler);
    
    // 明確設定 redirect_uri 參數以確保與 Azure AD 註冊的 URI 完全匹配
    m_oauth->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization || 
            stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            // 確保 redirect_uri 在授權和令牌交換階段都使用正確的格式（包含結尾斜線）
            parameters->replace("redirect_uri", "http://localhost:8081/");
        }
    });
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::granted,
            this, &OutlookCalendarAdapter::onAuthenticationGranted);
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::error,
            this, &OutlookCalendarAdapter::onAuthenticationError);
}

void OutlookCalendarAdapter::authenticate() {
    if (!m_oauth) {
        emit authenticationFailed("OAuth 未初始化，請先設定 Client ID 和 Client Secret");
        return;
    }
    
    qDebug() << "開始 Microsoft Outlook OAuth 2.0 認證流程...";
    qDebug() << "瀏覽器將會開啟授權頁面";
    m_oauth->grant();
}

void OutlookCalendarAdapter::onAuthenticationGranted() {
    m_accessToken = m_oauth->token();
    qDebug() << "Microsoft Outlook 認證成功！";
    emit authenticated();
}

void OutlookCalendarAdapter::onAuthenticationError(const QString& error, const QString& errorDescription) {
    QString errorMsg = QString("認證錯誤: %1 - %2").arg(error, errorDescription);
    qDebug() << errorMsg;
    emit authenticationFailed(errorMsg);
}

void OutlookCalendarAdapter::fetchEvents(const QDateTime& start, const QDateTime& end) {
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("尚未認證，請先呼叫 authenticate()");
        return;
    }
    
    // 構建 Microsoft Graph API 請求
    QUrl url("https://graph.microsoft.com/v1.0/me/calendarview");
    QUrlQuery query;
    query.addQueryItem("startDateTime", start.toUTC().toString(Qt::ISODate));
    query.addQueryItem("endDateTime", end.toUTC().toString(Qt::ISODate));
    query.addQueryItem("$orderby", "start/dateTime");
    url.setQuery(query);
    
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OutlookCalendarAdapter::onEventsReplyFinished);
}

void OutlookCalendarAdapter::onEventsReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QList<CalendarEvent> events = parseEventsJson(data);
        qDebug() << "獲取到" << events.size() << "個 Outlook 事件";
        emit eventsReceived(events);
    } else {
        QString error = QString("獲取事件失敗: %1").arg(reply->errorString());
        qDebug() << error;
        emit errorOccurred(error);
    }
    
    reply->deleteLater();
}

void OutlookCalendarAdapter::fetchTasks() {
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("尚未認證，請先呼叫 authenticate()");
        return;
    }
    
    // 構建 Microsoft Graph API 請求 (Microsoft To Do)
    QUrl url("https://graph.microsoft.com/v1.0/me/todo/lists");
    
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OutlookCalendarAdapter::onTasksReplyFinished);
}

void OutlookCalendarAdapter::onTasksReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QList<Task> tasks = parseTasksJson(data);
        qDebug() << "獲取到" << tasks.size() << "個 Microsoft To Do 任務";
        emit tasksReceived(tasks);
    } else {
        QString error = QString("獲取任務失敗: %1").arg(reply->errorString());
        qDebug() << error;
        emit errorOccurred(error);
    }
    
    reply->deleteLater();
}

QList<CalendarEvent> OutlookCalendarAdapter::parseEventsJson(const QByteArray& json) {
    QList<CalendarEvent> events;
    
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return events;
    
    QJsonObject root = doc.object();
    QJsonArray items = root["value"].toArray();
    
    for (const QJsonValue& value : items) {
        QJsonObject item = value.toObject();
        
        CalendarEvent event;
        event.id = item["id"].toString();
        event.title = item["subject"].toString();
        
        // 解析 body
        QJsonObject bodyObj = item["body"].toObject();
        event.description = bodyObj["content"].toString();
        
        // 解析 location
        QJsonObject locationObj = item["location"].toObject();
        event.location = locationObj["displayName"].toString();
        
        event.platform = Platform::Outlook;
        
        // 解析開始時間
        QJsonObject startObj = item["start"].toObject();
        event.startTime = QDateTime::fromString(startObj["dateTime"].toString(), Qt::ISODate);
        event.isAllDay = item["isAllDay"].toBool();
        
        // 解析結束時間
        QJsonObject endObj = item["end"].toObject();
        event.endTime = QDateTime::fromString(endObj["dateTime"].toString(), Qt::ISODate);
        
        // 解析參與者
        QJsonArray attendees = item["attendees"].toArray();
        for (const QJsonValue& attendee : attendees) {
            QJsonObject emailAddress = attendee.toObject()["emailAddress"].toObject();
            event.attendees.append(emailAddress["address"].toString());
        }
        
        // 解析重複規則
        if (item.contains("recurrence") && !item["recurrence"].isNull()) {
            QJsonObject recurrenceObj = item["recurrence"].toObject();
            QJsonObject patternObj = recurrenceObj["pattern"].toObject();
            event.recurrenceRule = patternObj["type"].toString();
        }
        
        events.append(event);
    }
    
    return events;
}

QList<Task> OutlookCalendarAdapter::parseTasksJson(const QByteArray& json) {
    QList<Task> tasks;
    
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return tasks;
    
    QJsonObject root = doc.object();
    QJsonArray items = root["value"].toArray();
    
    // 這裡獲取的是任務列表，需要進一步獲取每個列表中的任務
    // 簡化實作，僅解析列表資訊
    for (const QJsonValue& value : items) {
        QJsonObject item = value.toObject();
        
        Task task;
        task.id = item["id"].toString();
        task.title = item["displayName"].toString();
        task.platform = Platform::Outlook;
        task.isCompleted = false;
        
        tasks.append(task);
    }
    
    return tasks;
}
