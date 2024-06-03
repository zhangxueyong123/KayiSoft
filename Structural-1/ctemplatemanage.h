#ifndef CTEMPLATEMANAGE_H
#define CTEMPLATEMANAGE_H

/******C/S模板保存数据*****/

#include "structdef.h"
#include <map>
#include "CNetWork.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
//struct TemplateParameter
//{
//    QString strFirstId;
//    QString strSecondId;
//    QString strTreeJson;
//   // QString strTemplateName;
//};
//using TemplateParameterList = std::vector<QString>;  //用于保存加载的模板数据
using TemplateParameterMap = std::map<stTableKey, QString>; //用于保存加载的模板数据
using TemplateSecondListMap = std::map<QString, std::vector<stTableKey>>; //用于保存加载的模板列表  key为secondid
using TemplateSecondListMapIter = std::map<QString, std::vector<stTableKey>>::iterator;
using TemplateFirstListMap= std::map<QString, TemplateSecondListMap>; //用于保存加载的模板列表  key为firstid
class CTemplateManage:public QObject
{
    Q_OBJECT
public:
    CTemplateManage(const QString &strUrl, CNetworkAccessManager *network);
    static CTemplateManage* GetSingle();
    static CTemplateManage* GetSingle(const QString &strUrl, CNetworkAccessManager *network);
    //获取模板名列表
    std::vector<QString>  GetTemplateNameList(const QString &strFirst, const QString &strSecond);
    //获取模板列表
    std::vector<stTableKey>  GetTemplateList(const QString &strFirst, const QString &strSecond);
    //获取模板数据
    QString GetTemplateDataByName(const QString &strFirst, const QString &strSecond, const QString &strName);
    QString GetTemplateDataById(const QString &strId);
    //添加数据
    stTableKey AddTemplateData(const QString &strFirst, const QString &strSecond, const QString &strName,
                               const QString &strData);
    //删除数据
    void DeleteTemplateData(const QString &strId);
    void DeleteTemplateData(const QString &strFirst, const QString &strSecond, const QString &strName);

    void Clear(const QString &strFirst, const QString &strSecond);

    void updataToken(QString token);
signals:
    void signSettingInfo(QString title,bool flag);
    void signChangeComboboxItem(QString str, bool f);//0是删 1是增
public slots:
    void handleTemplateListReplay(QNetworkReply* reply, QString first, QString second);

    void handleTemplateJsonReplay();
private:
     void ClearTemplateData(const QString &strId);
     void ClearTemplateData(const std::vector<stTableKey>& dataList);
    std::vector<stTableKey> LoadTemplateList(const QString &strFirst, const QString &strSecond); //通过接口获取列表
    QString LoadTemplateData(const QString &strId);
    stTableKey AddTemplateData(const QJsonObject &json,bool isAdd); //通过接口保存数据
    TemplateParameterMap m_mapTemplate; //模板数据
    TemplateFirstListMap m_mapTemplateList; // 模板列表
   // std::map<QString, std::map>
    static CTemplateManage* m_pSinglePtr ;
    QString m_strUrl;
    CNetworkAccessManager *m_network = nullptr;
    QNetworkReply* m_currentReply;
    QString m_currentFirst;
    QString m_currentSecond;
    QString m_addName = "";
    QString m_delName = "";
    QTimer timer;
    int nTimeout = 1000;
    QEventLoop eventLoop;
};

class SignalForwarder : public QObject {
    Q_OBJECT
public:
    SignalForwarder(QString first, QString second, QObject* parent = nullptr) : QObject(parent), m_currentFirst(first) , m_currentSecond(second){}

public slots:
    void handleSignal() {
        auto reply = qobject_cast<QNetworkReply*>(sender());
        emit forwardSignal(reply,m_currentFirst, m_currentSecond);
    }

signals:
    void forwardSignal(QNetworkReply*,QString,QString);

private:
    QString m_currentFirst;
    QString m_currentSecond;
};


#endif // CTEMPLATEMANAGE_H
