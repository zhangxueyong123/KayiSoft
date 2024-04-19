#ifndef CNETWORKACCESSMANAGER_H
#define CNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QNetworkReply>


class CNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    CNetworkAccessManager(QObject *parent = Q_NULLPTR);

    void setEnabledRequestToSupportHttps(bool enabled);
    void setRawHeader(const QByteArray &headerName, const QByteArray &headerValue);

    QNetworkReply* deleteResource(QNetworkRequest &request);
    QNetworkReply* get(QNetworkRequest &request);
    QNetworkReply* post(QNetworkRequest &request, QIODevice *data);
    QNetworkReply* post(QNetworkRequest &request, const QByteArray &data);
    QNetworkReply* post(QNetworkRequest &request, QHttpMultiPart *multiPart);
    QNetworkReply* put(QNetworkRequest &request, QIODevice *data);
    QNetworkReply* put(QNetworkRequest &request, const QByteArray &data);
    QNetworkReply* put(QNetworkRequest &request, QHttpMultiPart *multiPart);

signals:
    void signal_replyError(QString errorinfo);

protected slots:
    void slot_replyFinished(QNetworkReply* reply);

private:
    void prework(QNetworkRequest &request);


private:
    bool m_bEnbledHttps;

    QSslConfiguration m_sslConfig;

    QHash<QByteArray, QByteArray> m_hashRawHeaders;
};

#endif // CNETWORKACCESSMANAGER_H
