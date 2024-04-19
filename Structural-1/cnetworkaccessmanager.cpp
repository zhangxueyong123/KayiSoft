#include "cnetworkaccessmanager.h"

#include <QMetaEnum>
#include <QVariant>

CNetworkAccessManager::CNetworkAccessManager(QObject *parent):
    QNetworkAccessManager(parent),
    m_bEnbledHttps(false)
{
    m_sslConfig = QSslConfiguration::defaultConfiguration();
    m_sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    m_sslConfig.setProtocol(QSsl::TlsV1SslV3);

    connect(this, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slot_replyFinished(QNetworkReply*)));
}

void CNetworkAccessManager::setEnabledRequestToSupportHttps(bool enabled)
{
    m_bEnbledHttps = enabled;
}

void CNetworkAccessManager::prework(QNetworkRequest &request)
{
    if (m_bEnbledHttps){
        request.setSslConfiguration(m_sslConfig);
    }

    auto iter = m_hashRawHeaders.begin();
    while (iter != m_hashRawHeaders.end()){
        request.setRawHeader(iter.key(), iter.value());
        iter++;
    }


//    if (this->networkAccessible() != QNetworkAccessManager::Accessible){
//        qDebug() << "QNetworkAccessManager::NotAccessible";
//        //this->clearAccessCache();
//        this->setNetworkAccessible(QNetworkAccessManager::Accessible);
//    }
}


QNetworkReply* CNetworkAccessManager::deleteResource(QNetworkRequest &request)
{
    prework(request);

    return QNetworkAccessManager::deleteResource(request);
}

QNetworkReply* CNetworkAccessManager::get(QNetworkRequest &request)
{
    prework(request);

    return QNetworkAccessManager::get(request);
}

QNetworkReply* CNetworkAccessManager::post(QNetworkRequest &request, QIODevice *data)
{
    prework(request);

    return QNetworkAccessManager::post(request, data);
}

QNetworkReply* CNetworkAccessManager::post(QNetworkRequest &request, const QByteArray &data)
{
    prework(request);

    return QNetworkAccessManager::post(request, data);
}

QNetworkReply* CNetworkAccessManager::post(QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    prework(request);

    return QNetworkAccessManager::post(request, multiPart);
}

QNetworkReply* CNetworkAccessManager::put(QNetworkRequest &request, QIODevice *data)
{
    prework(request);

    return QNetworkAccessManager::put(request, data);
}

QNetworkReply* CNetworkAccessManager::put(QNetworkRequest &request, const QByteArray &data)
{
    prework(request);

    return QNetworkAccessManager::put(request, data);
}

QNetworkReply* CNetworkAccessManager::put(QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    prework(request);

    return QNetworkAccessManager::put(request, multiPart);
}

void CNetworkAccessManager::slot_replyFinished(QNetworkReply* reply)
{
    bool bNoError = true;
    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    switch (httpCode){
//    case 302:
//    {
//        emit signal_replyError(QString("错误码:%1,登录超时").arg(httpCode));
//    }
//        break;
//    case 401:
//    case 403:
//    {
//        emit signal_replyError(QString("错误码:%1,权限不足").arg(httpCode));
//    }
//        break;
//    default:
//        bNoError = true;
//        break;
//    }


    if (reply->error() != QNetworkReply::NoError || bNoError == false) {
        qDebug() << QString("URL(%1):%2, ERROR(%3 %4):%5").arg(reply->operation()).arg(reply->url().toString())
                    .arg(QMetaEnum::fromType<QNetworkReply::NetworkError>().valueToKey(reply->error()))
                    .arg(httpCode).arg(reply->errorString());
    }
}

void CNetworkAccessManager::setRawHeader(const QByteArray &headerName, const QByteArray &headerValue)
{
    m_hashRawHeaders.insert(headerName, headerValue);
}
