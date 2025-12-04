#include "OIDCManager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QTcpSocket>
#include <QDateTime>
#include <QRandomGenerator>
#include <QCryptographicHash>

OIDCManager::OIDCManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_callbackServer(new QTcpServer(this))
    , m_promptLogin(false)
{
    m_redirectURI = QString("http://localhost:%1/callback").arg(CALLBACK_PORT);
    connect(m_callbackServer, &QTcpServer::newConnection, this, &OIDCManager::onNewConnection);
}

OIDCManager::~OIDCManager()
{
    if (m_callbackServer->isListening()) {
        m_callbackServer->close();
    }
}

void OIDCManager::startAuthentication(const QString& issuerURL,
                                     const QString& clientID,
                                     const QString& clientSecret,
                                     const QString& scopes,
                                     const QString& acrValue,
                                     const QString& loginHint,
                                     bool promptLogin,
                                     const QString& responseType,
                                     const QString& extraParams)
{
    m_issuerURL = issuerURL;
    m_clientID = clientID;
    m_clientSecret = clientSecret;
    m_scopes = scopes;
    m_acrValue = acrValue;
    m_loginHint = loginHint;
    m_promptLogin = promptLogin;
    m_responseType = responseType;
    m_extraParams = extraParams;
    
    // Generate state for CSRF protection
    m_state = QString::number(QRandomGenerator::global()->generate64(), 16);
    
    // Generate PKCE code verifier and challenge
    QByteArray verifierBytes;
    for (int i = 0; i < 32; ++i) {
        verifierBytes.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    m_codeVerifier = verifierBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    
    QByteArray challengeBytes = QCryptographicHash::hash(m_codeVerifier.toUtf8(), QCryptographicHash::Sha256);
    m_codeChallenge = challengeBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    
    emit progressUpdated("Fetching OIDC discovery document...");
    emit logMessage(QString("Started OIDC authentication at %1").arg(QDateTime::currentDateTime().toString()));
    
    // Start callback server
    if (!m_callbackServer->listen(QHostAddress::LocalHost, CALLBACK_PORT)) {
        emit errorOccurred(QString("Failed to start callback server on port %1").arg(CALLBACK_PORT));
        return;
    }
    
    emit logMessage(QString("Callback server listening on port %1").arg(CALLBACK_PORT));
    
    // Fetch discovery document
    QString discoveryURL = issuerURL + "/.well-known/openid-configuration";
    QNetworkRequest request(discoveryURL);
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &OIDCManager::onDiscoveryFinished);
}

void OIDCManager::cancelAuthentication()
{
    if (m_callbackServer->isListening()) {
        m_callbackServer->close();
    }
    emit logMessage("Authentication cancelled by user");
}

void OIDCManager::onDiscoveryFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Failed to fetch discovery document: %1").arg(reply->errorString()));
        emit logMessage(QString("Discovery error: %1").arg(reply->errorString()));
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isObject()) {
        emit errorOccurred("Failed to parse discovery document.");
        emit logMessage("Failed to parse discovery document.");
        return;
    }
    
    QJsonObject json = doc.object();
    m_authorizationEndpoint = json["authorization_endpoint"].toString();
    m_tokenEndpoint = json["token_endpoint"].toString();
    
    if (m_authorizationEndpoint.isEmpty() || m_tokenEndpoint.isEmpty()) {
        emit errorOccurred("Discovery document missing required endpoints.");
        emit logMessage("Discovery document missing authorization_endpoint or token_endpoint.");
        return;
    }
    
    emit logMessage(QString("Fetched discovery document - Auth endpoint: %1, Token endpoint: %2")
                   .arg(m_authorizationEndpoint, m_tokenEndpoint));
    
    emit progressUpdated("Building authorization URL...");
    
    QUrl authURL = buildAuthorizationURL(m_authorizationEndpoint);
    
    emit logMessage(QString("Starting authentication with URL: %1").arg(authURL.toString()));
    emit progressUpdated("Opening browser for authentication...");
    
    // Open browser
    if (!QDesktopServices::openUrl(authURL)) {
        emit errorOccurred("Failed to open browser.");
        emit logMessage("Failed to open browser.");
        return;
    }
    
    emit progressUpdated("Waiting for authentication completion...");
}

QUrl OIDCManager::buildAuthorizationURL(const QString& authEndpoint)
{
    QUrl url(authEndpoint);
    QUrlQuery query;

    query.addQueryItem("client_id", m_clientID);
    query.addQueryItem("redirect_uri", m_redirectURI);
    query.addQueryItem("response_type", m_responseType);
    query.addQueryItem("scope", m_scopes);
    query.addQueryItem("state", m_state);
    query.addQueryItem("code_challenge", m_codeChallenge);
    query.addQueryItem("code_challenge_method", "S256");

    if (m_acrValue != "None" && !m_acrValue.isEmpty()) {
        // Extract ACR value from display string if needed
        QString acrRaw = m_acrValue;
        if (m_acrValue.contains("(")) {
            int start = m_acrValue.indexOf('(') + 1;
            int end = m_acrValue.indexOf(')');
            if (start > 0 && end > start) {
                acrRaw = m_acrValue.mid(start, end - start);
            }
        }
        query.addQueryItem("acr_values", acrRaw);
    }

    if (!m_loginHint.isEmpty()) {
        query.addQueryItem("login_hint", m_loginHint);
    }

    if (m_promptLogin) {
        query.addQueryItem("prompt", "login");
    }

    // Add extra parameters
    if (!m_extraParams.isEmpty()) {
        QStringList pairs = m_extraParams.split('&');
        for (const QString& pair : pairs) {
            QStringList kv = pair.split('=');
            if (kv.size() == 2) {
                query.addQueryItem(kv[0], kv[1]);
            }
        }
    }

    url.setQuery(query);
    return url;
}

void OIDCManager::onNewConnection()
{
    QTcpSocket* socket = m_callbackServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &OIDCManager::onReadyRead);
}

void OIDCManager::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString request = QString::fromUtf8(socket->readAll());

    // Parse the HTTP request to get the callback URL
    QStringList lines = request.split("\r\n");
    if (lines.isEmpty()) {
        socket->close();
        socket->deleteLater();
        return;
    }

    QString firstLine = lines[0];
    QStringList parts = firstLine.split(' ');
    if (parts.size() < 2) {
        socket->close();
        socket->deleteLater();
        return;
    }

    QString path = parts[1];
    QString fullURL = QString("http://localhost:%1%2").arg(CALLBACK_PORT).arg(path);

    // Send HTTP response
    QString response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n"
                      "\r\n"
                      "<html><body>"
                      "<h1>Authentication Complete</h1>"
                      "<p>You can close this window and return to the OIDC Tester application.</p>"
                      "</body></html>";

    socket->write(response.toUtf8());
    socket->flush();
    socket->close();
    socket->deleteLater();

    // Stop listening for more connections
    m_callbackServer->close();

    // Handle the callback
    emit logMessage(QString("Authentication complete. Parsing tokens from callback URL: %1").arg(fullURL));
    emit progressUpdated("Authentication complete. Parsing tokens...");
    handleAuthCallback(QUrl(fullURL));
}

void OIDCManager::handleAuthCallback(const QUrl& url)
{
    QUrlQuery query(url);

    QString code = query.queryItemValue("code");
    QString idToken = query.queryItemValue("id_token");
    QString accessToken = query.queryItemValue("access_token");
    QString error = query.queryItemValue("error");
    QString state = query.queryItemValue("state");

    // Verify state
    if (state != m_state) {
        emit errorOccurred("State mismatch - possible CSRF attack");
        emit logMessage("State mismatch in callback");
        return;
    }

    // Check for errors
    if (!error.isEmpty()) {
        emit errorOccurred(QString("Authentication error: %1").arg(error));
        emit logMessage(QString("Authentication error: %1").arg(error));
        return;
    }

    // If we have direct tokens (implicit flow), display them
    if (!idToken.isEmpty() || !accessToken.isEmpty()) {
        QString result;
        if (!idToken.isEmpty()) {
            result += QString("ID Token: %1\n").arg(idToken);
        }
        if (!accessToken.isEmpty()) {
            result += QString("Access Token: %1\n").arg(accessToken);
        }
        emit tokensReceived(result);
        emit logMessage("Direct tokens received from callback");
        return;
    }

    // If we have an authorization code, exchange it for tokens
    if (!code.isEmpty()) {
        emit logMessage(QString("Authorization code received: %1").arg(code));
        emit progressUpdated("Exchanging authorization code for tokens...");
        exchangeCodeForTokens(code, m_tokenEndpoint);
    } else {
        emit errorOccurred("No authorization code or tokens received in callback.");
        emit logMessage("No authorization code or tokens found in callback URL");
    }
}

void OIDCManager::exchangeCodeForTokens(const QString& code, const QString& tokenEndpoint)
{
    QNetworkRequest request(tokenEndpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QUrlQuery postData;
    postData.addQueryItem("grant_type", "authorization_code");
    postData.addQueryItem("code", code);
    postData.addQueryItem("client_id", m_clientID);
    postData.addQueryItem("redirect_uri", m_redirectURI);
    postData.addQueryItem("code_verifier", m_codeVerifier);

    if (!m_clientSecret.isEmpty()) {
        postData.addQueryItem("client_secret", m_clientSecret);
    }

    emit logMessage(QString("Exchanging authorization code at token endpoint: %1").arg(tokenEndpoint));

    QNetworkReply* reply = m_networkManager->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, &OIDCManager::onTokenExchangeFinished);
}

void OIDCManager::onTokenExchangeFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    reply->deleteLater();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    emit logMessage(QString("Token endpoint response status: %1").arg(statusCode));

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Token exchange error: %1").arg(reply->errorString()));
        emit logMessage(QString("Token exchange error: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isObject()) {
        emit errorOccurred("Failed to parse token response.");
        emit logMessage("Failed to parse token response.");
        return;
    }

    QJsonObject json = doc.object();
    QString result;

    if (json.contains("access_token")) {
        result += QString("Access Token: %1\n").arg(json["access_token"].toString());
        emit logMessage("Access token received");
    }

    if (json.contains("id_token")) {
        result += QString("ID Token: %1\n").arg(json["id_token"].toString());
        emit logMessage("ID token received");
    }

    if (json.contains("refresh_token")) {
        result += QString("Refresh Token: %1\n").arg(json["refresh_token"].toString());
        emit logMessage("Refresh token received");
    }

    if (json.contains("token_type")) {
        result += QString("Token Type: %1\n").arg(json["token_type"].toString());
    }

    if (json.contains("expires_in")) {
        result += QString("Expires In: %1 seconds\n").arg(json["expires_in"].toInt());
    }

    if (result.isEmpty()) {
        emit tokensReceived("No tokens found in response.");
    } else {
        emit tokensReceived(result);
        emit logMessage("Token exchange completed successfully");
    }
}

