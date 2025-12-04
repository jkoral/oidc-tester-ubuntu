#ifndef JWTDECODER_H
#define JWTDECODER_H

#include <QString>
#include <QJsonObject>

class JWTDecoder
{
public:
    static QString decodeJWT(const QString& token);
    static QString formatTokenDetails(const QString& token);
    
private:
    static QByteArray decodeBase64URLSafe(const QString& input);
    static QString formatJSON(const QJsonObject& json, const QString& indent = "  ");
};

#endif // JWTDECODER_H

