#include "GoogleCalendarAdapter.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDateTime>
#include <QAbstractOAuth>

GoogleCalendarAdapter::GoogleCalendarAdapter(QObject* parent)
    : CalendarAdapter(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_oauth(nullptr)
    , m_replyHandler(nullptr)
{
}

GoogleCalendarAdapter::~GoogleCalendarAdapter() = default;

void GoogleCalendarAdapter::setCredentials(const QString& clientId, const QString& clientSecret) {
    m_clientId = clientId;
    m_clientSecret = clientSecret;
    setupOAuth();
}

void GoogleCalendarAdapter::setupOAuth() {
    if (m_oauth) {
        delete m_oauth;
    }
    
    m_oauth = new QOAuth2AuthorizationCodeFlow(this);
    m_oauth->setScope("https://www.googleapis.com/auth/calendar.readonly https://www.googleapis.com/auth/tasks.readonly");
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
    
    m_oauth->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
    m_oauth->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
    m_oauth->setClientIdentifier(m_clientId);
    m_oauth->setClientIdentifierSharedKey(m_clientSecret);
    
    // 設定本地回調處理器
    if (m_replyHandler) {
        delete m_replyHandler;
    }
    m_replyHandler = new QOAuthHttpServerReplyHandler(8080, this);
    m_replyHandler->setCallbackPath("/");  // 明確設定回調路徑為根路徑
    m_oauth->setReplyHandler(m_replyHandler);
    
    // 明確設定 redirect_uri 參數以確保與 Google Cloud 註冊的 URI 完全匹配
    m_oauth->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization || 
            stage == QAbstractOAuth::Stage::RequestingAccessToken) {
            // 確保 redirect_uri 在授權和令牌交換階段都使用正確的格式（包含結尾斜線）
            parameters->replace("redirect_uri", "http://localhost:8080/");
        }
    });
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::granted,
            this, &GoogleCalendarAdapter::onAuthenticationGranted);
    
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::error,
            this, &GoogleCalendarAdapter::onAuthenticationError);
}

void GoogleCalendarAdapter::authenticate() {
    if (!m_oauth) {
        emit authenticationFailed("OAuth 未初始化，請先設定 Client ID 和 Client Secret");
        return;
    }
    
    qDebug() << "開始 Google Calendar OAuth 2.0 認證流程...";
    qDebug() << "瀏覽器將會開啟授權頁面";
    m_oauth->grant();
}

void GoogleCalendarAdapter::onAuthenticationGranted() {
    m_accessToken = m_oauth->token();
    qDebug() << "Google Calendar 認證成功！";
    emit authenticated();
}

void GoogleCalendarAdapter::onAuthenticationError(const QString& error, const QString& errorDescription) {
    QString errorMsg = QString("認證錯誤: %1 - %2").arg(error, errorDescription);
    qDebug() << errorMsg;
    emit authenticationFailed(errorMsg);
}

void GoogleCalendarAdapter::fetchEvents(const QDateTime& start, const QDateTime& end) {
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("尚未認證，請先呼叫 authenticate()");
        return;
    }
    
    // 構建 Google Calendar API 請求
    QUrl url("https://www.googleapis.com/calendar/v3/calendars/primary/events");
    QUrlQuery query;
    query.addQueryItem("timeMin", start.toUTC().toString(Qt::ISODate));
    query.addQueryItem("timeMax", end.toUTC().toString(Qt::ISODate));
    query.addQueryItem("singleEvents", "true");
    query.addQueryItem("orderBy", "startTime");
    url.setQuery(query);
    
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &GoogleCalendarAdapter::onEventsReplyFinished);
}

void GoogleCalendarAdapter::onEventsReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QList<CalendarEvent> events = parseEventsJson(data);
        qDebug() << "獲取到" << events.size() << "個 Google Calendar 事件";
        emit eventsReceived(events);
    } else {
        QString error = QString("獲取事件失敗: %1").arg(reply->errorString());
        qDebug() << error;
        emit errorOccurred(error);
    }
    
    reply->deleteLater();
}

void GoogleCalendarAdapter::fetchTasks() {
    if (m_accessToken.isEmpty()) {
        emit errorOccurred("尚未認證，請先呼叫 authenticate()");
        return;
    }
    
    // 構建 Google Tasks API 請求
    QUrl url("https://tasks.googleapis.com/tasks/v1/lists/@default/tasks");
    
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_accessToken).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &GoogleCalendarAdapter::onTasksReplyFinished);
}

void GoogleCalendarAdapter::onTasksReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QList<Task> tasks = parseTasksJson(data);
        qDebug() << "獲取到" << tasks.size() << "個 Google Tasks 任務";
        emit tasksReceived(tasks);
    } else {
        QString error = QString("獲取任務失敗: %1").arg(reply->errorString());
        qDebug() << error;
        emit errorOccurred(error);
    }
    
    reply->deleteLater();
}

QList<CalendarEvent> GoogleCalendarAdapter::parseEventsJson(const QByteArray& json) {
    QList<CalendarEvent> events;
    
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return events;
    
    QJsonObject root = doc.object();
    QJsonArray items = root["items"].toArray();
    
    for (const QJsonValue& value : items) {
        QJsonObject item = value.toObject();
        
        CalendarEvent event;
        event.id = item["id"].toString();
        event.title = item["summary"].toString();
        event.description = item["description"].toString();
        event.location = item["location"].toString();
        event.platform = Platform::Google;
        
        // 解析開始時間
        QJsonObject startObj = item["start"].toObject();
        if (startObj.contains("dateTime")) {
            event.startTime = QDateTime::fromString(startObj["dateTime"].toString(), Qt::ISODate);
            event.isAllDay = false;
        } else if (startObj.contains("date")) {
            QDate date = QDate::fromString(startObj["date"].toString(), Qt::ISODate);
            event.startTime = QDateTime(date, QTime(0, 0));
            event.isAllDay = true;
        }
        
        // 解析結束時間
        QJsonObject endObj = item["end"].toObject();
        if (endObj.contains("dateTime")) {
            event.endTime = QDateTime::fromString(endObj["dateTime"].toString(), Qt::ISODate);
        } else if (endObj.contains("date")) {
            QDate date = QDate::fromString(endObj["date"].toString(), Qt::ISODate);
            event.endTime = QDateTime(date, QTime(23, 59, 59));
        }
        
        // 解析參與者
        QJsonArray attendees = item["attendees"].toArray();
        for (const QJsonValue& attendee : attendees) {
            event.attendees.append(attendee.toObject()["email"].toString());
        }
        
        // 解析重複規則
        QJsonArray recurrence = item["recurrence"].toArray();
        if (!recurrence.isEmpty()) {
            event.recurrenceRule = recurrence[0].toString();
        }
        
        events.append(event);
    }
    
    return events;
}

QList<Task> GoogleCalendarAdapter::parseTasksJson(const QByteArray& json) {
    QList<Task> tasks;
    
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) return tasks;
    
    QJsonObject root = doc.object();
    QJsonArray items = root["items"].toArray();
    
    for (const QJsonValue& value : items) {
        QJsonObject item = value.toObject();
        
        Task task;
        task.id = item["id"].toString();
        task.title = item["title"].toString();
        task.description = item["notes"].toString();
        task.platform = Platform::Google;
        
        // 解析到期日期
        if (item.contains("due")) {
            task.dueDate = QDateTime::fromString(item["due"].toString(), Qt::ISODate);
        }
        
        // 解析完成狀態
        QString status = item["status"].toString();
        task.isCompleted = (status == "completed");
        
        tasks.append(task);
    }
    
    return tasks;
}
