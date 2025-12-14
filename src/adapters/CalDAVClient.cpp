#include "CalDAVClient.h"
#include <QXmlStreamReader>
#include <QDebug>
#include <QSslError>
#include <QRegularExpression>

CalDAVClient::CalDAVClient(QObject* parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_server("caldav.icloud.com")
    , m_port(443)
    , m_principalUrl("")
    , m_calendarHomeUrl("")
    , m_discoveryInProgress(false)
{
    // 設定 SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // 連接信號
    connect(m_manager, &QNetworkAccessManager::authenticationRequired,
            this, &CalDAVClient::handleAuthenticationRequired);
    connect(m_manager, &QNetworkAccessManager::sslErrors,
            this, &CalDAVClient::handleSslErrors);
}

CalDAVClient::~CalDAVClient() = default;

void CalDAVClient::setCredentials(const QString& username, const QString& appPassword) {
    m_username = username;
    m_password = appPassword;
}

void CalDAVClient::setServer(const QString& server, quint16 port) {
    m_server = server;
    m_port = port;
    m_baseUrl = QString("https://%1:%2").arg(m_server).arg(m_port);
}

void CalDAVClient::discoverService() {
    if (m_discoveryInProgress) {
        qWarning() << "服務發現已在進行中，跳過重複請求";
        return;
    }
    
    m_discoveryInProgress = true;
    qDebug() << "開始 CalDAV 服務發現...";
    QUrl url(m_baseUrl + "/.well-known/caldav");
    
    QByteArray propfindXml = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<d:propfind xmlns:d=\"DAV:\">"
        "  <d:prop>"
        "    <d:current-user-principal />"
        "  </d:prop>"
        "</d:propfind>";
    
    sendPropfindForPrincipal(url, propfindXml, 0);
}

void CalDAVClient::discoverCalendars() {
    QString targetUrl;
    bool shouldQueryCalendars = false;
    
    // 優先使用已發現的 calendar home URL
    if (!m_calendarHomeUrl.isEmpty()) {
        targetUrl = m_baseUrl + m_calendarHomeUrl;
        qDebug() << "使用已發現的 calendar home URL:" << targetUrl;
        shouldQueryCalendars = true;
        m_discoveryInProgress = false;  // 發現完成
    }
    // 如果有 principal URL 但還沒有 calendar home URL，查詢 calendar home
    else if (!m_principalUrl.isEmpty() && m_discoveryInProgress) {
        qDebug() << "使用 principal URL 查找 calendar home...";
        
        QByteArray propfindXml = 
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<d:propfind xmlns:d=\"DAV:\" "
            "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
            "  <d:prop>"
            "    <c:calendar-home-set />"
            "  </d:prop>"
            "</d:propfind>";
        
        sendPropfindForCalendarHome(QUrl(m_baseUrl + m_principalUrl), propfindXml, 0);
        return;  // 等待 calendar home 回應
    }
    // Fallback: 使用從 email 提取的 username
    else if (!m_discoveryInProgress) {
        QString username = m_username.left(m_username.indexOf('@'));
        if (username.isEmpty()) {
            emit errorOccurred("無效的 Apple ID 格式");
            return;
        }
        
        targetUrl = m_baseUrl + "/" + username + "/calendars/";
        qDebug() << "使用 fallback URL:" << targetUrl;
        shouldQueryCalendars = true;
    }
    else {
        // 不應該到達這裡，但為了安全起見
        qWarning() << "discoverCalendars() 處於未預期的狀態";
        return;
    }
    
    // 如果確定要查詢行事曆，發送請求
    if (shouldQueryCalendars) {
        QByteArray propfindXml = 
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
            "<d:propfind xmlns:d=\"DAV:\" "
            "xmlns:c=\"urn:ietf:params:xml:ns:caldav\" "
            "xmlns:ical=\"http://apple.com/ns/ical/\">"
            "  <d:prop>"
            "    <d:resourcetype />"
            "    <d:displayname />"
            "    <ical:calendar-color />"
            "    <c:calendar-description />"
            "  </d:prop>"
            "</d:propfind>";
        
        sendPropfind(QUrl(targetUrl), propfindXml, 1);
    }
}

void CalDAVClient::listCalendars() {
    discoverCalendars();
}

void CalDAVClient::getEvents(const QString& calendarUrl,
                             const QDateTime& start,
                             const QDateTime& end) {
    QByteArray reportXml = QString(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<c:calendar-query xmlns:d=\"DAV:\" "
        "xmlns:c=\"urn:ietf:params:xml:ns:caldav\">"
        "  <d:prop>"
        "    <d:getetag />"
        "    <c:calendar-data />"
        "  </d:prop>"
        "  <c:filter>"
        "    <c:comp-filter name=\"VCALENDAR\">"
        "      <c:comp-filter name=\"VEVENT\">"
        "        <c:time-range start=\"%1\" end=\"%2\" />"
        "      </c:comp-filter>"
        "    </c:comp-filter>"
        "  </c:filter>"
        "</c:calendar-query>")
        .arg(start.toString(Qt::ISODate))
        .arg(end.toString(Qt::ISODate))
        .toUtf8();
    
    sendReport(QUrl(calendarUrl), reportXml);
}

void CalDAVClient::sendPropfind(const QUrl& url, const QByteArray& xml, int depth) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/xml; charset=utf-8");
    request.setRawHeader("Depth", QByteArray::number(depth));
    request.setRawHeader("User-Agent", "Qt CalDAV Client/1.0");
    
    QNetworkReply* reply = m_manager->sendCustomRequest(request, "PROPFIND", xml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "PROPFIND 回應:" << response;
            
            QList<CalendarInfo> calendars = parseCalendarList(response);
            if (!calendars.isEmpty()) {
                emit calendarsListed(calendars);
            } else {
                qWarning() << "未找到任何行事曆";
                emit errorOccurred("未找到任何行事曆");
            }
        } else {
            QString errorMsg = QString("PROPFIND 錯誤: %1").arg(reply->errorString());
            qWarning() << errorMsg;
            qWarning() << "HTTP 狀態碼:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qWarning() << "回應內容:" << reply->readAll();
            emit errorOccurred(errorMsg);
        }
        reply->deleteLater();
    });
}

void CalDAVClient::sendPropfindForPrincipal(const QUrl& url, const QByteArray& xml, int depth) {
    qDebug() << "發送 PROPFIND 請求以發現 principal URL:" << url.toString();
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/xml; charset=utf-8");
    request.setRawHeader("Depth", QByteArray::number(depth));
    request.setRawHeader("User-Agent", "Qt CalDAV Client/1.0");
    
    QNetworkReply* reply = m_manager->sendCustomRequest(request, "PROPFIND", xml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "Principal PROPFIND 回應:" << response;
            
            QString principalUrl = parsePrincipalUrl(response);
            if (!principalUrl.isEmpty()) {
                m_principalUrl = principalUrl;
                qDebug() << "發現 principal URL:" << principalUrl;
                emit serviceDiscovered(principalUrl);
                
                // 繼續發現 calendar home
                discoverCalendars();
            } else {
                qWarning() << "無法從回應中解析 principal URL，使用 fallback";
                m_discoveryInProgress = false;
                discoverCalendars();
            }
        } else {
            QString errorMsg = QString("Principal 發現失敗: %1").arg(reply->errorString());
            qWarning() << errorMsg;
            qWarning() << "HTTP 狀態碼:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            
            // 如果發現失敗，嘗試使用 fallback 方式
            qWarning() << "服務發現失敗，嘗試使用 fallback 方式...";
            m_discoveryInProgress = false;
            discoverCalendars();
        }
        reply->deleteLater();
    });
}

void CalDAVClient::sendPropfindForCalendarHome(const QUrl& url, const QByteArray& xml, int depth) {
    qDebug() << "發送 PROPFIND 請求以發現 calendar home:" << url.toString();
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/xml; charset=utf-8");
    request.setRawHeader("Depth", QByteArray::number(depth));
    request.setRawHeader("User-Agent", "Qt CalDAV Client/1.0");
    
    QNetworkReply* reply = m_manager->sendCustomRequest(request, "PROPFIND", xml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "Calendar Home PROPFIND 回應:" << response;
            
            QString calendarHomeUrl = parseCalendarHomeUrl(response);
            if (!calendarHomeUrl.isEmpty()) {
                m_calendarHomeUrl = calendarHomeUrl;
                qDebug() << "發現 calendar home URL:" << calendarHomeUrl;
                
                // 標記發現完成，繼續列出行事曆
                m_discoveryInProgress = false;
                discoverCalendars();
            } else {
                qWarning() << "無法從回應中解析 calendar home URL，使用 fallback";
                m_discoveryInProgress = false;
                discoverCalendars();
            }
        } else {
            QString errorMsg = QString("Calendar Home 發現失敗: %1").arg(reply->errorString());
            qWarning() << errorMsg;
            
            // 如果發現失敗，嘗試使用 fallback 方式
            qWarning() << "Calendar home 發現失敗，嘗試使用 fallback 方式...";
            m_discoveryInProgress = false;
            discoverCalendars();
        }
        reply->deleteLater();
    });
}

void CalDAVClient::sendReport(const QUrl& url, const QByteArray& xml) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, 
                     "application/xml; charset=utf-8");
    request.setRawHeader("Depth", "1");
    request.setRawHeader("User-Agent", "Qt CalDAV Client/1.0");
    
    QNetworkReply* reply = m_manager->sendCustomRequest(request, "REPORT", xml);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "REPORT 回應:" << response;
            
            QList<CalendarEvent> events = parseEventsFromCalDAV(response);
            emit eventsReceived(events);
        } else {
            emit errorOccurred(QString("REPORT 錯誤: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    });
}

void CalDAVClient::handleAuthenticationRequired(QNetworkReply* reply,
                                                QAuthenticator* authenticator) {
    Q_UNUSED(reply);
    authenticator->setUser(m_username);
    authenticator->setPassword(m_password);
}

void CalDAVClient::handleSslErrors(QNetworkReply* reply,
                                   const QList<QSslError>& errors) {
    // 記錄 SSL 錯誤
    for (const QSslError& error : errors) {
        qWarning() << "SSL 錯誤:" << error.errorString();
    }
    
    // 對於 iCloud，應該要求完全有效的 SSL 憑證
    emit errorOccurred(QString("SSL 憑證驗證失敗: %1")
        .arg(errors.isEmpty() ? "Unknown" : errors.first().errorString()));
}

QList<CalendarInfo> CalDAVClient::parseCalendarList(const QByteArray& xml) {
    QList<CalendarInfo> calendars;
    
    QXmlStreamReader reader(xml);
    CalendarInfo currentCalendar;
    bool inResponse = false;
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement()) {
            if (reader.name() == QStringLiteral("response")) {
                inResponse = true;
                currentCalendar = CalendarInfo();
            } else if (inResponse && reader.name() == QStringLiteral("href")) {
                currentCalendar.url = reader.readElementText();
            } else if (inResponse && reader.name() == QStringLiteral("displayname")) {
                currentCalendar.displayName = reader.readElementText();
            } else if (inResponse && reader.name() == QStringLiteral("calendar-description")) {
                currentCalendar.description = reader.readElementText();
            } else if (inResponse && reader.name() == QStringLiteral("calendar-color")) {
                currentCalendar.color = reader.readElementText();
            }
        } else if (reader.isEndElement()) {
            if (reader.name() == QStringLiteral("response")) {
                if (!currentCalendar.displayName.isEmpty()) {
                    calendars.append(currentCalendar);
                }
                inResponse = false;
            }
        }
    }
    
    if (reader.hasError()) {
        qWarning() << "XML 解析錯誤:" << reader.errorString();
    }
    
    return calendars;
}

QList<CalendarEvent> CalDAVClient::parseEventsFromCalDAV(const QByteArray& xml) {
    QList<CalendarEvent> events;
    
    QXmlStreamReader reader(xml);
    QString currentIcsData;
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement() && reader.name() == QStringLiteral("calendar-data")) {
            currentIcsData = reader.readElementText();
            
            // 簡單的 iCalendar 解析
            if (currentIcsData.contains("BEGIN:VEVENT")) {
                CalendarEvent event;
                event.platform = Platform::Apple;
                
                // 提取標題
                QRegularExpression summaryRegex("SUMMARY:([^\r\n]*)");
                QRegularExpressionMatch match = summaryRegex.match(currentIcsData);
                if (match.hasMatch()) {
                    event.title = match.captured(1);
                }
                
                // 提取開始時間
                QRegularExpression dtStartRegex("DTSTART[^:]*:([^\r\n]*)");
                match = dtStartRegex.match(currentIcsData);
                if (match.hasMatch()) {
                    QString dtStart = match.captured(1);
                    if (dtStart.endsWith('Z')) {
                        event.startTime = QDateTime::fromString(dtStart, "yyyyMMddTHHmmss'Z'");
                        event.startTime.setTimeSpec(Qt::UTC);
                    } else if (dtStart.contains('T')) {
                        event.startTime = QDateTime::fromString(dtStart, "yyyyMMddTHHmmss");
                    } else {
                        QDate date = QDate::fromString(dtStart, "yyyyMMdd");
                        event.startTime = QDateTime(date, QTime(0, 0, 0));
                        event.isAllDay = true;
                    }
                }
                
                // 提取結束時間
                QRegularExpression dtEndRegex("DTEND[^:]*:([^\r\n]*)");
                match = dtEndRegex.match(currentIcsData);
                if (match.hasMatch()) {
                    QString dtEnd = match.captured(1);
                    if (dtEnd.endsWith('Z')) {
                        event.endTime = QDateTime::fromString(dtEnd, "yyyyMMddTHHmmss'Z'");
                        event.endTime.setTimeSpec(Qt::UTC);
                    } else if (dtEnd.contains('T')) {
                        event.endTime = QDateTime::fromString(dtEnd, "yyyyMMddTHHmmss");
                    } else {
                        QDate date = QDate::fromString(dtEnd, "yyyyMMdd");
                        event.endTime = QDateTime(date, QTime(23, 59, 59));
                    }
                }
                
                // 提取地點
                QRegularExpression locationRegex("LOCATION:([^\r\n]*)");
                match = locationRegex.match(currentIcsData);
                if (match.hasMatch()) {
                    event.location = match.captured(1);
                }
                
                if (!event.title.isEmpty()) {
                    events.append(event);
                }
            }
        }
    }
    
    return events;
}

QString CalDAVClient::parsePrincipalUrl(const QByteArray& xml) {
    QXmlStreamReader reader(xml);
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement() && 
            reader.name() == QStringLiteral("current-user-principal")) {
            reader.readNext();
            if (reader.isStartElement() && reader.name() == QStringLiteral("href")) {
                return reader.readElementText();
            }
        }
    }
    
    return QString();
}

QString CalDAVClient::parseCalendarHomeUrl(const QByteArray& xml) {
    QXmlStreamReader reader(xml);
    
    while (!reader.atEnd()) {
        reader.readNext();
        
        if (reader.isStartElement() && 
            reader.name() == QStringLiteral("calendar-home-set")) {
            reader.readNext();
            if (reader.isStartElement() && reader.name() == QStringLiteral("href")) {
                return reader.readElementText();
            }
        }
    }
    
    return QString();
}
