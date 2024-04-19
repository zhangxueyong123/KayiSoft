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
        if(itVec.keyEx.strLabelEx == strName)
        {
            return GetTemplateDataById(itVec.strKey);
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
            qDebug()<< itMap.second;
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
    QJsonObject dataObject;
    //如果在缓存中，则此为修改，上传的json需要包含ID；不在，就去后端中获取。
    for (auto& itMap : m_mapTemplate)
    {
        if (itMap.first.strKey == strName)
        {
            qDebug() << itMap.second;
            dataObject["ID"] = itMap.first.keyEx.strLabelEx;

        }
    }

    dataObject["FirstId"] = strFirst;
    dataObject["SecondId"] = strSecond;
    dataObject["Title"] = strName;
    dataObject["TreeJson"] = strData;
    return AddTemplateData(dataObject);
}

void CTemplateManage::DeleteTemplateData(const QString &strId)
{
    if( strId.isEmpty())
    {
        return ;
    }
}

void CTemplateManage::DeleteTemplateData(const QString &strFirst, const QString &strSecond, const QString &strName)
{
    if( strFirst.isEmpty() || strName.isEmpty())
    {
        return ;
    }

    auto vecTemplateList =  GetTemplateList(strFirst, strSecond);
    for(auto &itVec : vecTemplateList)
    {
        if(itVec.keyEx.strLabelEx == strName)
        {
            DeleteTemplateData(itVec.strKey);
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
QEventLoop loop;

std::vector<stTableKey> CTemplateManage::LoadTemplateList(const QString &strFirst, const QString &strSecond)
{
    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot/list?firstid=4756628e-34c2-408f-b510-9f3f7785cb4f&secondid=2aa8215f-bbb1-422a-b591-b9c9e4897443
    QString url = m_strUrl + "/api/v1/sr_snapshot/list?firstid="+ strFirst + "&secondid="+ strSecond;

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    handleFinish = false;
    QNetworkReply* reply = m_network->get(request);
 
   
    SignalForwarder* forwarder = new SignalForwarder(strFirst,strSecond);
    QObject::connect(reply, &QNetworkReply::finished, forwarder, &SignalForwarder::handleSignal);
    QObject::connect(forwarder, &SignalForwarder::forwardSignal, this,&CTemplateManage::handleTemplateListReplay);
    std::vector<stTableKey> result;
    loop.exec();

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
    //for(auto it:vec)
    //{
    //    result.push_back(it);
    //}
        

  
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
                                stTableKey k(itemObj["ID"].toString(),1);
                                k.keyEx.strLabelEx = itemObj["Title"].toString();
                                it->second.push_back(k);
                            }
                            else
                            {
                                std::vector<stTableKey> temp;
                                iter->second.insert(std::pair(second, temp));
                                goto loop2;
                            }
                        }
                        else
                        {
                            // not found
                            TemplateSecondListMap temp;
                            m_mapTemplateList.insert(std::pair(first, temp));
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
    loop.quit();
}


//从后端返回模板数据，测试
QString CTemplateManage::LoadTemplateData(const QString &strId)
{
    ////读取测试json  该json为GetSaveReportJson返回完整json
    //std::ifstream file("C:\\code\\json1.txt", std::ios::binary);
    //if (file.is_open())
    //{
    //    //读取全部文件数据
    //    file.seekg(0, std::ios::end);
    //    int nLength = static_cast<int>(file.tellg());
    //    file.seekg(0, std::ios::beg);
    //    char* pBuffer = new char[nLength + 1];
    //    file.read(pBuffer, nLength);
    //    file.close();
    //    pBuffer[nLength] = '\0';
    //    QString strJson(pBuffer);
    //    delete[]pBuffer;
    //    pBuffer = nullptr;
    //    QJsonParseError error;
    //    //解析json
    //    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(), &error);
    //    if (error.error == QJsonParseError::NoError)
    //    {
    //        if (jsonDocument.isObject())
    //        {
    //            auto result = jsonDocument.toVariant().toMap();
    //            if (!result.empty())
    //            {
    //                auto itFind = result.find("TreeJson");
    //                //获取treejson  保存在2t.txt
    //                if (itFind != result.end() && !itFind->isNull())
    //                {
    //                    g_strTreeJson = itFind->toString();
    //                    std::ofstream outfile("d:\\2t.txt");
    //                    outfile << g_strTreeJson.toStdString() << std::endl;
    //                    outfile.close();
    //                }
    //                //ListJson
    //                itFind = result.find("ReportJson");
    //                if (itFind != result.end() && !itFind->isNull())
    //                {
    //                    g_strListJson = itFind->toString();
    //                    std::ofstream outfile("d:\\3t.txt");
    //                    outfile << g_strListJson.toStdString() << std::endl;
    //                    outfile.close();
    //                }
    //                //FirstId
    //                itFind = result.find("FirstId");
    //                if (itFind != result.end() && !itFind->isNull())
    //                {
    //                    g_strFirst = itFind->toString();
    //                }
    //                //SecondId
    //                itFind = result.find("SecondId");
    //                if (itFind != result.end() && !itFind->isNull())
    //                {
    //                    g_strSecond = itFind->toString();
    //                }
    //            }
    //        }
    //    }
    //}

    //https://hidos-ris.kayisoft.dev/api/v1/sr_snapshot/file?id=e9102148-6802-4a04-be5d-9a3416ddbb88
    QString url = m_strUrl + "/api/v1/sr_snapshot/file?id="+ strId;

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_network->get(request);
 
    
    connect(reply, &QNetworkReply::finished, this, &CTemplateManage::handleTemplateJsonReplay);
    loop.exec();
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
    loop.quit();
}

stTableKey CTemplateManage::AddTemplateData(const QJsonObject &json)
{
    // 将JSON对象转换为JSON文档
    QJsonDocument doc(json);
    // 将JSON文档转换为字节流
    QByteArray data =  doc.toJson();
    qDebug() << "dataJson = " << data.data();
    QString url = m_strUrl + "/api/v1/sr_snapshot";

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = m_network->post(request, data);
    // 处理回复
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            // 成功响应
            QByteArray responseData = reply->readAll();
            //qDebug() << "Response received:" << responseData;
        }
        else {
            // 出错处理
            qDebug() << "Error:" << reply->errorString();
        }
        reply->deleteLater();
        });

    return stTableKey("", -1);
}
