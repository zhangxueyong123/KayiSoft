#include "CNetWork.h"

#include <QTimer>
#include <QEventLoop>

CNetWork::CNetWork()
{
}


/**********************************************************
 * 函数名称:    localHostIp
 * 函数说明:    获取本机IP
 * 函数参数:    QAbstractSocket::NetworkLayerProtocol networkLayerProtocol
 * 返 回 值:    QString
 * 创建日期:    2022/07/29
 * 创 建 人:    npc
 **********************************************************/
QString CNetWork::localHostIp(QAbstractSocket::NetworkLayerProtocol networkLayerProtocol)
{
	QString ip;

	QList <QHostAddress> list = QNetworkInterface::allAddresses();
	foreach(QHostAddress address, list){
		if (address.protocol() == networkLayerProtocol) {
			ip = address.toString();
			break;
		}
	}

	return ip;
}

/**********************************************************
 * 函数名称:    httpMultiPartAddFileField
 * 函数说明:    
 * 函数参数:    QHttpMultiPart * multiPart
 * 函数参数:    const QString & key
 * 函数参数:    const QString & file_path
 * 返 回 值:    void
 * 创建日期:    2022/10/18
 * 创 建 人:    npc
 **********************************************************/
void CNetWork::httpMultiPartAddFileField(QHttpMultiPart* multiPart, const QString& key, const QString& filePath)
{
	if (multiPart) {
		QHttpPart httpPart;
		QString header = QString("form-data;name=\"%1\";filename=\"%2\"").arg(key).arg(filePath);
		httpPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(header));
		httpPart.setHeader(QNetworkRequest::ContentTypeHeader, QMimeDatabase().mimeTypeForFile(filePath).name());

		QFile* file = new QFile(filePath, multiPart);
		file->open(QIODevice::ReadOnly);
		httpPart.setBodyDevice(file);

		multiPart->append(httpPart);
	}
}

/**********************************************************
 * 函数名称:    httpMultiPartAddTextField
 * 函数说明:    
 * 函数参数:    QHttpMultiPart * multiPart
 * 函数参数:    const QString & key
 * 函数参数:    const QString & value
 * 返 回 值:    void
 * 创建日期:    2022/10/18
 * 创 建 人:    npc
 **********************************************************/
void CNetWork::httpMultiPartAddTextField(QHttpMultiPart* multiPart, const QString& key, const QByteArray& value)
{
	if (multiPart) {
		QHttpPart httpPart;
		httpPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + key + "\""));
		httpPart.setBody(value);
		multiPart->append(httpPart);
	}
}

/***************************************************************
 * 函数名：waitForNetworkReplyFinished
 * 功能：等待网络请求返回
 * 参数：名称                类型                   描述
 * 返回值：
 * 作者：npc
 ***************************************************************/
int CNetWork::waitForNetworkReplyFinished(QNetworkReply* reply, int nTimeout)
{
    QTimer timer;
    timer.setInterval(nTimeout);  // 设置超时时间
    timer.setSingleShot(true);

    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));

    if (nTimeout > 0) {
        QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
        timer.start();
    }

    eventLoop.exec();

    if (nTimeout > 0) {
        if (timer.isActive()) {
            timer.stop();
        } else { /* 处理超时 */
            QObject::disconnect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
            reply->abort();
            reply->deleteLater();

            //qDebug() << QString("URL(%1):%2, ERROR(timeout)").arg(reply->operation()).arg(reply->url().toString());

            return -2;
        }
    }

    if (!(reply->error() == QNetworkReply::NoError ||
          reply->error() == QNetworkReply::ContentNotFoundError)) {
        return -1;
    }

    return 0;
}

/**
 * @brief CNetWork::enableRequestToSupportHttps 使网络请求支持https
 * @param networkRequest
 */
void CNetWork::enableRequestToSupportHttps(QNetworkRequest& networkRequest)
{
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfig.setProtocol(QSsl::TlsV1SslV3);
    networkRequest.setSslConfiguration(sslConfig);
}

int CNetWork::netWorkGet(CNetworkAccessManager *network, QString url, QString& data)
{
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply* reply = network->get(request);
    int ret = waitForNetworkReplyFinished(reply, 3000);
    if (ret == 0) {
        data = reply->readAll();
    }

    return ret;
}
