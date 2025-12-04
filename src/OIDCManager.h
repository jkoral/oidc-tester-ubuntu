#ifndef OIDCMANAGER_H
#define OIDCMANAGER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QTcpServer>
#include <QMap>

class OIDCManager : public QObject
{
    Q_OBJECT

public:
    explicit OIDCManager(QObject *parent = nullptr);
    ~OIDCManager();

    void startAuthentication(const QString& issuerURL,
                            const QString& clientID,
                            const QString& clientSecret,
                            const QString& scopes,
                            const QString& acrValue,
                            const QString& loginHint,
                            bool promptLogin,
                            const QString& responseType,
                            const QString& extraParams);
    
    void cancelAuthentication();

signals:
    void progressUpdated(const QString& message);
    void errorOccurred(const QString& error);
    void tokensReceived(const QString& tokens);
    void logMessage(const QString& message);

private slots:
    void onDiscoveryFinished();
    void onTokenExchangeFinished();
    void onNewConnection();
    void onReadyRead();

private:
    QUrl buildAuthorizationURL(const QString& authEndpoint);
    void exchangeCodeForTokens(const QString& code, const QString& tokenEndpoint);
    void handleAuthCallback(const QUrl& url);
    QString parseCallbackURL(const QString& urlString);
    
    QNetworkAccessManager* m_networkManager;
    QTcpServer* m_callbackServer;
    
    QString m_issuerURL;
    QString m_clientID;
    QString m_clientSecret;
    QString m_scopes;
    QString m_acrValue;
    QString m_loginHint;
    bool m_promptLogin;
    QString m_responseType;
    QString m_extraParams;
    QString m_redirectURI;
    QString m_authorizationEndpoint;
    QString m_tokenEndpoint;
    QString m_state;
    QString m_codeVerifier;
    QString m_codeChallenge;
    
    static const int CALLBACK_PORT = 8080;
};

#endif // OIDCMANAGER_H

