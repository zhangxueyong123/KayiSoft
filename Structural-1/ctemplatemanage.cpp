#include "ctemplatemanage.h"
#include <iostream>
#include <fstream>

QString g_strListJson;
QString g_strTreeJson;
QString g_strFirst;
QString g_strSecond;
CTemplateManage* CTemplateManage::m_pSinglePtr = nullptr;
CTemplateManage::CTemplateManage(const QString &strUrl, CNetworkAccessManager *network)
    :m_strUrl(strUrl),
      m_network(network)
{
    
    QTimer timer;
    int nTimeout = 1000;
    timer.setInterval(nTimeout);  // 设置超时时间
    timer.setSingleShot(true);

 
    if (nTimeout > 0) {
        QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
        timer.start();
    }
}

CTemplateManage *CTemplateManage::GetSingle()
{
    return m_pSinglePtr;
}

CTemplateManage *CTemplateManage::GetSingle(const QString &strUrl, CNetworkAccessManager *network)
{
    if (nullptr == m_pSinglePtr)
    {
        m_pSinglePtr = new CTemplateManage(strUrl, network);
    }
    return m_pSinglePtr;
}

std::vector<QString> CTemplateManage::GetTemplateNameList(const QString &strFirst, const QString &strSecond)
{
    std::vector<QString> templateNameList;

    auto vecTemplateList =  GetTemplateList(strFirst, strSecond);
    for(auto &itVec : vecTemplateList)
    {
        templateNameList.push_back(itVec.keyEx.strLabelEx);
    }
    return templateNameList;
}
//获取同级目录下有哪些模板如（CT-左肺情况下有多少模板）
std::vector<stTableKey> CTemplateManage::GetTemplateList(const QString &strFirst, const QString &strSecond)
{
    if(strFirst.isEmpty())
    {
        return std::vector<stTableKey>();
    }
    auto itFindFirst = m_mapTemplateList.find(strFirst);
    bool bFind = true;
    if(itFindFirst == m_mapTemplateList.end())
    {
        bFind = false;
       // return templateList;
    }
    if(bFind)
    {
        auto itFindSecond = itFindFirst->second.find(strSecond);
        if(itFindSecond == itFindFirst->second.end())
        {
            bFind = false;
        }
        else
        {
            return itFindSecond->second;
        }
    }
    if(!bFind)
    {
        return LoadTemplateList(strFirst, strSecond);
    }
    return std::vector<stTableKey>();
}

QString CTemplateManage::GetTemplateDataByName(const QString &strFirst, const QString &strSecond, const QString &strName)
{
    ////测试
    //return GetTemplateDataById(strName);
    //测试
    QString templateData;
    if(strFirst.isEmpty() || strName.isEmpty())
    {
        return templateData;
    }
    
    auto vecTemplateList =  GetTemplateList(strFirst, strSecond);
    for(auto &itVec : vecTemplateList)
    {
        if(itVec.strKey == strName)
        {
            return GetTemplateDataById(itVec.keyEx.strLabelEx);
        }
    }
    return templateData;
}

QString CTemplateManage::GetTemplateDataById(const QString &strId)
{
    if(strId.isEmpty())
    {
        return "";
    }
    //如果在缓存中，则返回；不在，就去后端中获取。
    for(auto &itMap : m_mapTemplate)
    {
        if(itMap.first.strKey == strId)
        {
            //qDebug()<< itMap.second;
            return itMap.second;
        }
    }

    return LoadTemplateData(strId);
}

stTableKey CTemplateManage::AddTemplateData(const QString &strFirst, const QString &strSecond, const QString &strName, const QString &strData)
{
    stTableKey key("", -1);
    if(strFirst.isEmpty() || strName.isEmpty() || strData.isEmpty())
    {
        return key;
    }
    m_addName = strName;
    m_currentFirst = strFirst;
    m_currentSecond = strSecond;
    QJsonObject dataObject;
    //如果在缓存中，则此为修改，上传的json需要包含ID；不在，就去后端中获取。
    auto itFindFirst = m_mapTemplateList.find(strFirst);
    bool bFind = true;
    if (itFindFirst == m_mapTemplateList.end())
    {
        bFind = false;
        // return templateList;
    }
    if (bFind)
    {
        auto itFindSecond = itFindFirst->second.find(strSecond);
        if (itFindSecond == itFindFirst->second.end())
        {

        }
        else
        {
            bFind = false;
            dataObject["ID"] = "";
            for (auto it : itFindSecond->second)
            {
                if (it.strKey == strName)
                    dataObject["ID"] = it.keyEx.strLabelEx;
            }  

        }
    }


    dataObject["FirstId"] = strFirst;
    dataObject["SecondId"] = strSecond;
    dataObject["Title"] = strName;
    dataObject["TreeJson"] = strData;
    return AddTemplateData(dataObject, dataObject["ID"] == "" ? 1 : 0);
}
//与前端交互
void CTemplateManage::DeleteTemplateData(const QString &strId)
{
    if( strId.isEmpty())
    {
        return ;
    }
    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot?id=665aba1c-0cd6-42e8-b12f-6e3c505ea698

    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot?id=a220c4aa-7ca9-4dee-80dd-4b6b5cbee163
    QString url = m_strUrl + "/api/v1/sr_snapshot?id=" + strId;

    QNetworkRequest request = QNetworkRequest(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_network->deleteResource(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply,this]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 成功响应
            QByteArray responseData = reply->readAll();
            QString str = responseData;
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            if (!doc.isNull())
            {
                if (doc.isObject())
                {
                    QJsonObject jsonObj = doc.object();
                    // 现在可以使用jsonObj访问JSON对象中的数据
                    // 例如：获取名为"exampleKey"的值
                    QJsonValue value = jsonObj.value("Success");
                    if (value != true)
                    {
                        signSettingInfo("解析失败", 1);
                        return;
                    }
            
                    value = jsonObj.value("StatusCode");
                    if (value != 200)
                    {
                        signSettingInfo("HTTP状态码有误", 1);
                        return;
                    }
                    signSettingInfo("删除成功", 0);
                    signChangeComboboxItem( m_delName,0);
                    return;
                }
            }
        }
        else 
        {
            // 出错处理
            qDebug() << "Error:" << reply->errorString();
        }
        reply->deleteLater();
        });
}

void CTemplateManage::DeleteTemplateData(const QString &strFirst, const QString &strSecond, const QString &strName)
{
    if( strFirst.isEmpty() || strName.isEmpty())
    {
        return ;
    }
    m_delName = strName;
    m_currentFirst = strFirst;
    m_currentSecond = strSecond;
    auto vecTemplateList =  GetTemplateList(strFirst, strSecond);
    for(auto &itVec : vecTemplateList)
    {
        if(itVec.strKey == strName)
        {
            DeleteTemplateData(itVec.keyEx.strLabelEx);
            //从缓存中删除
            auto itFindFirst = m_mapTemplateList.find(strFirst);
            bool bFind = true;
            if (itFindFirst == m_mapTemplateList.end())
            {
                bFind = false;
                // return templateList;
            }
            if (bFind)
            {
                auto itFindSecond = itFindFirst->second.find(strSecond);
                if (itFindSecond == itFindFirst->second.end())
                {

                }
                else
                {
                    bFind = false;
                    for (auto it = itFindSecond->second.begin(); it != itFindSecond->second.end(); ) 
                    {
                        if (it->strKey == strName) 
                        {
                            it = itFindSecond->second.erase(it);  // 使用迭代器直接删除元素，并更新迭代器位置
                            break;
                        }
                        else 
                        {
                            ++it;  // 只有在不删除元素时才移动迭代器
                        }
                    }

                }
            }
            break;
            //return GetTemplateDataById(itVec.strKey);
        }
    }
}

void CTemplateManage::Clear(const QString &strFirst, const QString &strSecond)
{
    if(strFirst.isEmpty() && strSecond.isEmpty())
    {
        m_mapTemplate.clear();
        m_mapTemplateList.clear();
        return;
    }
    auto ClearSecond = [](TemplateSecondListMap *pSecondMap, const QString &strSecond, CTemplateManage *pThis)
    {
        if(strSecond.isEmpty())
        {
            auto itMap = pSecondMap->begin();
            while(itMap != pSecondMap->end())
            {
               // for(auto &itVec : itMap->secon)
                pThis->ClearTemplateData(itMap->second);
                ++itMap;
            }
            pSecondMap->clear();
        }
        else
        {
            auto itMap = pSecondMap->find(strSecond);
            if(itMap != pSecondMap->end())
            {
                pThis->ClearTemplateData(itMap->second);
                pSecondMap->erase(itMap);
            }
        }
    };
    if(strFirst.isEmpty())
    {
        for(auto &itFirstList : m_mapTemplateList)
        {
            ClearSecond(&itFirstList.second,strSecond, this);
        }
    }
    else
    {
        auto itFind = m_mapTemplateList.find(strFirst);
        if(itFind != m_mapTemplateList.end())
        {

            ClearSecond(&(itFind->second),strSecond, this);
        }
    }
}

void CTemplateManage::ClearTemplateData(const QString &strId)
{
     auto itFind = m_mapTemplate.begin();
   // auto itFind = m_mapTemplate.find(strId);
    while(itFind != m_mapTemplate.end())
    {
        if(itFind->first.strKey == strId)
        {
            m_mapTemplate.erase(itFind);
            return;
        }
        ++itFind;
    }
}

void CTemplateManage::ClearTemplateData(const std::vector<stTableKey> &dataList)
{
    for(auto &itList : dataList)
    {
        ClearTemplateData(itList.strKey);

    }
}


std::vector<stTableKey> CTemplateManage::LoadTemplateList(const QString &strFirst, const QString &strSecond)
{
    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot/list?firstid=4756628e-34c2-408f-b510-9f3f7785cb4f&secondid=2aa8215f-bbb1-422a-b591-b9c9e4897443
    QString url = m_strUrl + "/api/v1/sr_snapshot/list?firstid="+ strFirst + "&secondid="+ strSecond;

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_network->get(request);
 
   
    SignalForwarder* forwarder = new SignalForwarder(strFirst,strSecond);
    QObject::connect(reply, &QNetworkReply::finished, forwarder, &SignalForwarder::handleSignal);
    QObject::connect(forwarder, &SignalForwarder::forwardSignal, this,&CTemplateManage::handleTemplateListReplay);
    std::vector<stTableKey> result;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));

    eventLoop.exec();


    
    timer.start(1000);
    //auto vec = m_mapTemplateList.find(strFirst)->second.find(strSecond)->second;
    auto iter = m_mapTemplateList.find(strFirst);// ->second.find(strSecond)->second;
    if (iter != m_mapTemplateList.end())
    {
        auto it = iter->second.find(strSecond);
        if (it != iter->second.end())
        {
            for (auto vec : it->second)
            {
                result.push_back(vec);
            }
        }
    }

    return result;
}

void CTemplateManage::handleTemplateListReplay(QNetworkReply* reply, QString first, QString second)
{
    /*auto reply = qobject_cast<QNetworkReply*>(sender());*/


    if (reply->error() == QNetworkReply::NoError)
    {
        // 成功响应
        QByteArray responseData = reply->readAll();
        QString str = responseData;
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (!doc.isNull())
        {
            if (doc.isObject())
            {
                QJsonObject jsonObj = doc.object();
                // 现在可以使用jsonObj访问JSON对象中的数据
                // 例如：获取名为"exampleKey"的值
                QJsonValue value = jsonObj.value("Success");
                if (value != true)
                {
                    qDebug() << "exampleKey:" << value.toString();
                    return;
                }

                // 如果数据包含数组，比如一个名为"items"的数组
                if (jsonObj.contains("Context") && jsonObj["Context"].isArray())
                {
                    QJsonArray jsonArray = jsonObj["Context"].toArray();
                    for (int i = 0; i < jsonArray.size(); ++i)
                    {
                        QJsonObject itemObj = jsonArray[i].toObject();

                        // 假设每个项都有一个"name"属性
                        //qDebug() << "template Title:" << itemObj["Title"].toString();
                        //qDebug() << "template ID:" << itemObj["ID"].toString();
                        loop:
                        auto iter = m_mapTemplateList.find(first);
                        if (iter != m_mapTemplateList.end())
                        {
                            // 找到了
                            loop2:
                            auto it = iter->second.find(second);
                            if (it != iter->second.end())
                            {
                                //找到了
                                stTableKey k(itemObj["Title"].toString(),1);
                                k.keyEx.strLabelEx = itemObj["ID"].toString();
                                //m_mapTemplate.insert(std::pair(k, itemObj["Title"].toString()));
                                it->second.push_back(k);
                            }
                            else
                            {
                                std::vector<stTableKey> temp;
                                iter->second.insert(std::pair<QString, std::vector<stTableKey>>(second, temp));
                                goto loop2;
                            }
                        }
                        else
                        {
                            // not found
                            TemplateSecondListMap temp;
                            m_mapTemplateList.insert(std::pair<QString, TemplateSecondListMap>(first, temp));
                            goto loop;
                        }


                    }
                }
            }
        }
        else
            qDebug() << "failed";
    }
    else
        // 出错处理
        qDebug() << "Error:" << reply->errorString();
    reply->deleteLater();
    eventLoop.quit();
}


//从后端返回模板数据，测试
QString CTemplateManage::LoadTemplateData(const QString &strId)
{
    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot/file?id=e9102148-6802-4a04-be5d-9a3416ddbb88
    QString url = m_strUrl + "/api/v1/sr_snapshot/file?id="+ strId;

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_network->get(request);
 
    
    connect(reply, &QNetworkReply::finished, this, &CTemplateManage::handleTemplateJsonReplay);
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));

    eventLoop.exec();
    timer.start(1000);
    //m_mapTemplate.insert(std::pair(, g_strTreeJson));
    //切换显示页面
    return g_strTreeJson;
}
void CTemplateManage::handleTemplateJsonReplay()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        // 成功响应

        auto responseData = reply->readAll();
        QString str = responseData;
        /*qDebug() << "Response received:" << g_strTreeJson;*/

        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (!doc.isNull()) 
        {
            if (doc.isObject()) 
            {
                QJsonObject jsonObj = doc.object();
                // 现在可以使用jsonObj访问JSON对象中的数据
                // 例如：获取名为"exampleKey"的值
                auto context = jsonObj["Context"].toString();
                QJsonParseError error;
                //解析json
                QJsonDocument jsonDocument = QJsonDocument::fromJson(context.toUtf8(), &error);

                if (error.error == QJsonParseError::NoError)
                {
                    if (jsonDocument.isObject())
                    {
                        auto result = jsonDocument.toVariant().toMap();
                        if (!result.empty())
                        {
                            auto itFind = result.find("TreeJson");
                            //获取json 
                            if (itFind != result.end() && !itFind->isNull())
                            {
                                g_strTreeJson = itFind->toString();
                                
                            }
                           
                        }
                    }
                }

            }
        }
        else {
            qDebug() << "Received JSON is invalid.";
        }
    }
    else {
        // 出错处理
        qDebug() << "Error:" << reply->errorString();
    }
    reply->deleteLater();
    eventLoop.quit();
}
//1 新增  0 修改
stTableKey CTemplateManage::AddTemplateData(const QJsonObject &json,bool isAdd)
{
    // 将JSON对象转换为JSON文档
    QJsonDocument doc(json);
    // 将JSON文档转换为字节流
    QByteArray data =  doc.toJson();
    QString s = data;
    QString url = m_strUrl + "/api/v1/sr_snapshot";

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_network->post(request, data);
    // 处理回复
    QObject::connect(reply, &QNetworkReply::finished, [reply,this,isAdd]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 成功响应
            QByteArray responseData = reply->readAll();
            QString str = responseData;
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            if (!doc.isNull())
            {
                if (doc.isObject())
                {
                    QJsonObject jsonObj = doc.object();
                    // 现在可以使用jsonObj访问JSON对象中的数据
                    // 例如：获取名为"exampleKey"的值
                    QJsonValue value = jsonObj.value("Success");
                    if (value != true)
                    {
                        signSettingInfo("解析失败", 1);
                        return;
                    }

                    value = jsonObj.value("StatusCode");
                    if (value != 200)
                    {
                        signSettingInfo("HTTP状态码有误", 1);
                        return;
                    }
                    //解析ID
                    
                    QJsonDocument document = QJsonDocument::fromJson(str.toUtf8());
                    QJsonObject rootObject = document.object();

                    QJsonObject context = rootObject.value("Context").toObject();
                    QString id = context.value("ID").toString();
                    //添加缓存中的内容
                    auto it = m_mapTemplateList.find(m_currentFirst);
                    if (it != m_mapTemplateList.end())
                    {
                        auto itt = it->second.find(m_currentSecond);
                        if (itt != it->second.end())
                        {
                           
                            stTableKey temp(m_addName,-1);
                            temp.keyEx.strLabelEx = id;
                            itt->second.push_back(temp);
                        }
                    }
                    signSettingInfo(isAdd?"添加成功":"修改成功", 0);
                    signChangeComboboxItem(m_addName,1);
                    return;
                }
            }

        }
        else {
            // 出错处理
            qDebug() << "Error:" << reply->errorString();
        }
        reply->deleteLater();
        });

    return stTableKey("", -1);
}

