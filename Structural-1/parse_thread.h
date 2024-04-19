#ifndef HTTPTHREAD_H
#define HTTPTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QNetworkAccessManager>
#include "cnetworkaccessmanager.h"
#include "structuraldata.h"
class HttpThread : public QThread
{
    Q_OBJECT
public:
    HttpThread();
    ~HttpThread();
    void addQuery(QString cmd);
    void stopRun();
    void setStructuralData(StructuralData*);
    void setVecId(std::vector<QString>);
    void setNetworkAccessManager(CNetworkAccessManager*);
signals:
    void signReplySuccess(QByteArray arr);

    void signReplyFinished();
public slots:
    void slotReplyFinished(QNetworkReply* reply);
    // QThread interface
protected:
    void run();

private:
    QStringList     m_taskList;
    bool            m_stoped = 1;
    QMutex          m_mutex;
    QMutex          m_mutexReply;
    QWaitCondition  m_waitCondition;
    QWaitCondition  m_waitReplyCondition;
    QNetworkAccessManager *m_pHttpMgr;
    QNetworkReply* reply;
    StructuralData* m_data;
    std::vector<QString>    m_id;
    QStringList         m_idList;
};

#endif // HTTPTHREAD_H
