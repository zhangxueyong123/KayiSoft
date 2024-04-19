#pragma once

#include "cnetworkaccessmanager.h"

#include <QtNetwork>

class CNetWork
{
public:
    CNetWork();

    static QString localHostIp(QAbstractSocket::NetworkLayerProtocol networkLayerProtocol = QAbstractSocket::IPv4Protocol);

	static void httpMultiPartAddFileField(QHttpMultiPart* multiPart, const QString& key, const QString& filePath);
	static void httpMultiPartAddTextField(QHttpMultiPart* multiPart, const QString& key, const QByteArray& value);
    static int waitForNetworkReplyFinished(QNetworkReply* reply, int nTimeout = -1);
    static void enableRequestToSupportHttps(QNetworkRequest& networkRequest);

    static int netWorkGet(CNetworkAccessManager* network, QString url, QString& data);
};
