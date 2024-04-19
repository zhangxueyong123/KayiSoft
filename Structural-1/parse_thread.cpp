#include "parse_thread.h"

#include <QEventLoop>
#include <QNetworkReply>

HttpThread::HttpThread()
{
    QString strAuthorization = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJuYW1laWQiOiJhZDBiNGI0ZS1kZjViLTQ0ODMtYTZlZS02YjE4NjA2MTZmMDYiLCJ1bmlxdWVfbmFtZSI6IueuoeeQhuWRmCIsIndpbmFjY291bnRuYW1lIjoiYWRtaW4iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9tb2JpbGVwaG9uZSI6IjE1ODU3NzcyNTQwIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbENvZGUiOiI0NzAwMDU5MzAzMzAzMDIxMUExMDAxIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbE5hbWUiOiLmn5Dmn5DljLvnp5HlpKflrabpmYTlsZ7nrKzkuozljLvpmaIiLCJLYXlpQ2xvdWRDbGFpbVR5cGVzLUhvc3BpdGFsSWQiOiI5OTlGRkNGQzgyQ0E0QjNCOTY0NjNGNUYzNzE3NDVCMCIsIktheWlDbG91ZENsYWltVHlwZXMtRGVwYXJ0bWVudElkIjoiNGZjYmE0ZDktMmE3Yy00ZTZkLThlNDQtOTliZTRkNzY1YTk2IiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1EZXBhcnRtZW50TmFtZSI6IuaUvuWwhOenkSIsImh0dHA6Ly9zY2hlbWFzLm1pY3Jvc29mdC5jb20vd3MvMjAwOC8wNi9pZGVudGl0eS9jbGFpbXMvZXhwaXJhdGlvbiI6IjQvNy8yMDI0IDg6MDg6MDEgQU0iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9oYXNoIjoiOTUwNGQ0M2UtOTJlZC00ZjcxLTg4NTgtNGY4MjI4YWYzODM5IiwibmJmIjoxNzEyNDcwMDgxLCJleHAiOjE3MTI0NzcyODEsImlhdCI6MTcxMjQ3MDA4MSwiaXNzIjoiS2F5aUNsb3VkLlVzZXJSaWdodFNlcnZpY2UuV2ViQXBpLklzc3VlciIsImF1ZCI6IktheWlDbG91ZC5Vc2VyUmlnaHRTZXJ2aWNlLldlYkFwaS5BdWRpZW5jZSJ9.NNrnfmnwCcJ92NAhs7dAY_XeNHnuYiv1jLLd7fnEU0w";

    //m_pHttpMgr.setRawHeader("Authorization", strAuthorization.toLatin1());
}


HttpThread::~HttpThread()
{
    m_stoped = true;
    m_waitCondition.wakeOne();
    quit();
    wait();
    m_stoped = true;
}


void HttpThread::addQuery(QString cmd)
{
    QMutexLocker lock(&m_mutex);
    m_taskList.append(cmd);
    m_waitCondition.wakeOne();
}

void HttpThread::stopRun()
{
    m_stoped = true;
    m_waitCondition.wakeOne();

    quit();
    wait();
}
void HttpThread::setStructuralData(StructuralData *da)
{
    m_data = da;
}
void HttpThread::setVecId(std::vector<QString>id)
{
    m_id = id;
    for (auto it : id)
        m_idList.append(it);
}
void HttpThread::setNetworkAccessManager(CNetworkAccessManager*net)
{
    //m_pHttpMgr = net;
}
void HttpThread::slotReplyFinished(QNetworkReply* reply)
{

}

void HttpThread::run()
{
    m_taskList.clear();
    m_stoped = false;
    m_pHttpMgr = new QNetworkAccessManager();
    QNetworkRequest requestInfo;
    QString strAuthorization = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJuYW1laWQiOiJhZDBiNGI0ZS1kZjViLTQ0ODMtYTZlZS02YjE4NjA2MTZmMDYiLCJ1bmlxdWVfbmFtZSI6IueuoeeQhuWRmCIsIndpbmFjY291bnRuYW1lIjoiYWRtaW4iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9tb2JpbGVwaG9uZSI6IjE1ODU3NzcyNTQwIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbENvZGUiOiI0NzAwMDU5MzAzMzAzMDIxMUExMDAxIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbE5hbWUiOiLmn5Dmn5DljLvnp5HlpKflrabpmYTlsZ7nrKzkuozljLvpmaIiLCJLYXlpQ2xvdWRDbGFpbVR5cGVzLUhvc3BpdGFsSWQiOiI5OTlGRkNGQzgyQ0E0QjNCOTY0NjNGNUYzNzE3NDVCMCIsIktheWlDbG91ZENsYWltVHlwZXMtRGVwYXJ0bWVudElkIjoiNGZjYmE0ZDktMmE3Yy00ZTZkLThlNDQtOTliZTRkNzY1YTk2IiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1EZXBhcnRtZW50TmFtZSI6IuaUvuWwhOenkSIsImh0dHA6Ly9zY2hlbWFzLm1pY3Jvc29mdC5jb20vd3MvMjAwOC8wNi9pZGVudGl0eS9jbGFpbXMvZXhwaXJhdGlvbiI6IjQvNy8yMDI0IDg6MDg6MDEgQU0iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9oYXNoIjoiOTUwNGQ0M2UtOTJlZC00ZjcxLTg4NTgtNGY4MjI4YWYzODM5IiwibmJmIjoxNzEyNDcwMDgxLCJleHAiOjE3MTI0NzcyODEsImlhdCI6MTcxMjQ3MDA4MSwiaXNzIjoiS2F5aUNsb3VkLlVzZXJSaWdodFNlcnZpY2UuV2ViQXBpLklzc3VlciIsImF1ZCI6IktheWlDbG91ZC5Vc2VyUmlnaHRTZXJ2aWNlLldlYkFwaS5BdWRpZW5jZSJ9.NNrnfmnwCcJ92NAhs7dAY_XeNHnuYiv1jLLd7fnEU0w";
    requestInfo.setRawHeader("Authorization", strAuthorization.toLatin1());
    //requestInfo.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json;charset=utf-8"));
    //requestInfo.setRawHeader("Authorization", "");//Authorization
    while (1) {
        QMutexLocker lock(&m_mutex);
        if (m_taskList.isEmpty())
            m_waitCondition.wait(&m_mutex);
        else {
            auto data = m_taskList.takeFirst();
            lock.unlock();
            requestInfo.setUrl(data);
            QNetworkReply* reply = m_pHttpMgr->get(requestInfo);
            QEventLoop eventLoop;
            connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
            eventLoop.exec();       //block until finish
            if (reply->error() != QNetworkReply::NoError)
            {
                qDebug() << qPrintable(reply->errorString());
            }
            //请求返回的结果
            QByteArray responseByte = reply->readAll();
            m_data->AddChildByJson(responseByte, m_idList.takeFirst());
            signReplySuccess(responseByte);
        }

        if (m_stoped) {
            break;
        }
    }
}
