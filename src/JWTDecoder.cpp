#include "JWTDecoder.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

QByteArray JWTDecoder::decodeBase64URLSafe(const QString& input)
{
    QString base64 = input;
    base64.replace('-', '+');
    base64.replace('_', '/');
    
    // Pad if necessary
    int rem = base64.length() % 4;
    if (rem > 0) {
        base64.append(QString(4 - rem, '='));
    }
    
    return QByteArray::fromBase64(base64.toUtf8());
}

QString JWTDecoder::decodeJWT(const QString& token)
{
    QStringList segments = token.split('.');
    if (segments.size() != 3) {
        return "Invalid JWT format.";
    }
    
    QByteArray payloadData = decodeBase64URLSafe(segments[1]);
    QJsonDocument doc = QJsonDocument::fromJson(payloadData);
    
    if (doc.isNull() || !doc.isObject()) {
        return "Failed to decode JWT payload.";
    }
    
    return QString("Decoded JWT: %1\n").arg(QString::fromUtf8(doc.toJson(QJsonDocument::Indented)));
}

QString JWTDecoder::formatTokenDetails(const QString& token)
{
    QStringList segments = token.split('.');
    if (segments.size() != 3) {
        return "Invalid JWT format.\n";
    }
    
    QString result;
    
    // Decode header
    QByteArray headerData = decodeBase64URLSafe(segments[0]);
    QJsonDocument headerDoc = QJsonDocument::fromJson(headerData);
    if (!headerDoc.isNull() && headerDoc.isObject()) {
        result += "Header:\n";
        result += formatJSON(headerDoc.object());
        result += "\n";
    }
    
    // Decode payload
    QByteArray payloadData = decodeBase64URLSafe(segments[1]);
    QJsonDocument payloadDoc = QJsonDocument::fromJson(payloadData);
    if (!payloadDoc.isNull() && payloadDoc.isObject()) {
        result += "Payload:\n";
        result += formatJSON(payloadDoc.object());
        result += "\n";
    }
    
    // Show signature info
    result += QString("Signature: %1\n").arg(segments[2]);
    
    return result;
}

QString JWTDecoder::formatJSON(const QJsonObject& json, const QString& indent)
{
    QString result;
    QStringList keys = json.keys();
    keys.sort();
    
    for (const QString& key : keys) {
        QJsonValue value = json[key];
        
        if (value.isString()) {
            result += QString("%1%2: \"%3\"\n").arg(indent, key, value.toString());
        } else if (value.isDouble()) {
            result += QString("%1%2: %3\n").arg(indent, key).arg(value.toDouble());
        } else if (value.isArray()) {
            QJsonDocument doc(value.toArray());
            result += QString("%1%2: %3\n").arg(indent, key, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        } else if (value.isObject()) {
            QJsonDocument doc(value.toObject());
            result += QString("%1%2: %3\n").arg(indent, key, QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
        } else {
            result += QString("%1%2: %3\n").arg(indent, key, value.toVariant().toString());
        }
    }
    
    return result;
}

