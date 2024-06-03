#include "mymeasureparameter.h"
#include "CNetWork.h"
//#include "CGlobalVar.h"
//#include "CCore.h"
#include "QtQml/qjsengine.h"
#include <QFontMetrics>
#include <fstream>
#include<typeinfo>
MyMeasureParameterManage* MyMeasureParameterManage::m_pSinglePtr = nullptr;
MyMeasureParameter::MyMeasureParameter(const QString &strUrl, CNetworkAccessManager *network, QObject *parent)
    :  m_url(strUrl), m_network(network)/*,QObject(parent)*/
{

}



QStringList MyMeasureParameter::getMatchingStrings(QString str, QString pattern)
{
    QStringList list;

    if (!str.isEmpty()) {
        QRegExp rx(pattern);

        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1) {
            QString s = rx.cap(0);
            if (!s.isEmpty()){
                list << s;
            }
            pos += rx.matchedLength();
        }

        list.removeDuplicates();
    }

    return list;
}

void MyMeasureParameter::getMeasureParameters(const QString & tableName, const QString & systemCode, const QString & hospitalCode, const QString & sex, const QString & strAge)
{
    m_hashMeasureParameterInfo.clear();
    if (!tableName.isEmpty()){
        QUrl url(QString("%1/api/v1/reportdictionary/measureparameterlist").arg(m_url));
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("systemcode", systemCode);
        urlQuery.addQueryItem("hospitalcode", hospitalCode);
        if(!sex.isEmpty())
        {
            urlQuery.addQueryItem("sex", sex);
        }
        urlQuery.addQueryItem("parametertype", tableName);
        url.setQuery(urlQuery);

        int age = 0;
        if (strAge.contains(QRegExp("^\\d+(Y|y|岁|年)"))){
            age = strAge.left(strAge.count()-1).toInt();
        } else if (strAge.contains(QRegExp("^\\d+$"))){
            age = strAge.toInt();
        }

        QNetworkRequest request = QNetworkRequest(url);
        QNetworkReply* reply = m_network->get(request);
        if (CNetWork::waitForNetworkReplyFinished(reply, 3000) != -2) {
            QVariant var;

            QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                QJsonObject jsonObjMain = jsonDoc.object();
                if (jsonObjMain.contains("Context")){
                    if (jsonObjMain.value("Success").toBool()){
                        QJsonValue value = jsonObjMain.value("Context");
                        if (!value.isNull()){
                            var = value;
                        }
                    }
                } else {
                    var = jsonObjMain;
                }
            }

            if (var.isValid()){
                QJsonArray jsonArray = var.toJsonValue().toArray();
                foreach (auto iter, jsonArray){
                    QJsonObject jsonObj = iter.toObject();
                    int  nId = jsonObj.value("ParameterId").toInt();

                    if (!m_hashMeasureParameterInfo.contains(nId)){
                        if ((!jsonObj.value("Enabled").toVariant().toBool()) ||
                                (!jsonObj.value("Startage").isNull() && age < jsonObj.value("Startage").toInt()) ||
                                (!jsonObj.value("Endage").isNull() && age > jsonObj.value("Endage").toInt())){ /* 未启用，或年龄不符 */
                            continue;
                        }

//                        if (jsonObj.value("ParameterNormal").toString().isEmpty() ||
//                                (jsonObj.value("ParameterSmallreminder").toString().isEmpty() &&
//                                 jsonObj.value("ParameterBigreminder").toString().isEmpty())){ /* 正常范围为空，或无超范围提醒 */
//                            continue;
//                        }

                        MY_MEASURE_PARAMETER_INFO info;
                        info.jsonInfo = jsonObj;
                        info.parameterId = nId;
                        info.fieldName = jsonObj.value("ParameterCode").toString();
                        info.formula = jsonObj.value("ParameterFormula").toString();
                        info.dependentParameters = getMatchingStrings(info.formula, "[a-zA-Z_]+\\w*");
                        QString normal = jsonObj.value("ParameterNormal").toString();
                        QStringList ranges = normal.split("~", QString::SkipEmptyParts);
                        if (ranges.count() == 2){
                            info.normalMin = ranges.front().toDouble();
                            info.normalMax = ranges.back().toDouble();
                            info.parameterBigreminder = jsonObj.value("ParameterBigreminder").toString();
                            info.parameterSmallreminder = jsonObj.value("ParameterSmallreminder").toString();
                            info.parameterNormalreminder = jsonObj.value("ParameterNormalreminder").toString();

                        }
                        m_hashMeasureParameterInfo.insert(info.parameterId, info);
                    }
                }
            }
        }
    }
}

MY_MEASURE_PARAMETER_INFO MyMeasureParameter::GetMeasureParameter(int nCode)
{
    //  MY_MEASURE_PARAMETER_INFO info;
    auto itFind = m_hashMeasureParameterInfo.find(nCode);
    if(itFind != m_hashMeasureParameterInfo.end())
    {
        return m_hashMeasureParameterInfo[nCode];
    }
    return MY_MEASURE_PARAMETER_INFO();
}

std::map<int, MY_MEASURE_PARAMETER_INFO>  MyMeasureParameter::CheckNeedMath(const QString &fieldName)
{
    std::map<int, MY_MEASURE_PARAMETER_INFO>  map;
  //  auto values = m_hashMeasureParameterInfo.values();
    for(auto &itHash : m_hashMeasureParameterInfo)
    {
        for(auto &itList : itHash.dependentParameters)
        {
            if(itList == fieldName)
            {
                map[itHash.parameterId] = itHash;
                break;
            }
        }
    }
    return map;
}

std::map<int, MY_MEASURE_PARAMETER_INFO> MyMeasureParameter::CheckNeedMath(int nId)
{
    auto itFind = m_hashMeasureParameterInfo.find(nId);
    if(itFind == m_hashMeasureParameterInfo.end())
    {
        return std::map<int, MY_MEASURE_PARAMETER_INFO>();
    }
  //  QString fieldName = itFind->fieldName;
//    std::map<int, MY_MEASURE_PARAMETER_INFO>  map;
//    auto values = m_hashMeasureParameterInfo.values();
//    for(auto &itHash : values)
//    {
//        for(auto &itList : itHash.dependentParameters)
//        {
//            if(itList == fieldName)
//            {
//                map[itHash.parameterId] = itHash;
//                break;
//            }
//        }
//    }
    return CheckNeedMath(itFind->fieldName);
}

int MyMeasureParameter::GetID(const QString &fieldName)
{
    int nId = -1;
    for(auto &itMap : m_hashMeasureParameterInfo)
    {
        if(itMap.fieldName == fieldName )
        {
            return itMap.parameterId;
        }
    }
    return nId;
}

MyMeasureParameterManage::MyMeasureParameterManage(const QString &strUrl, CNetworkAccessManager *network, QObject *parent)
    :m_strUrl(strUrl),
      m_pNetwork(network)
    ,m_pParent(parent)
{

}



void MyMeasureParameterManage::AddWidget(int nId, stMeasureParameterWidget par)
{
    auto itFind = m_mapWidget.find(nId);
    if(itFind == m_mapWidget.end())
    {
        m_mapWidget[nId] =  std::vector<stMeasureParameterWidget>();
        itFind = m_mapWidget.find(nId);
    }
    QString strName = typeid(*par.pWidget).name();
    itFind->second.push_back(par);
}

void MyMeasureParameterManage::ClearWidget()
{
    m_mapWidget.clear();
}

void MyMeasureParameterManage::AddMeasureParameters(const QString &tableName)
{
    //    if(!m_bInit)
    //    {
    //        return;
    //    }
    auto itFind = m_mapMeasureParameter.find(tableName);
    if(itFind == m_mapMeasureParameter.end())
    {
        MyMeasureParameter *par = new MyMeasureParameter(m_strUrl, m_pNetwork, m_pParent);
        par->getMeasureParameters(tableName, m_systemCode, m_hospitalCode, m_sex, m_strAge);
        m_mapMeasureParameter[tableName] = par;
    }

}

void MyMeasureParameterManage::ClearMeasureParameters()
{
    for(auto &itMap : m_mapMeasureParameter)
    {
        if(itMap.second != nullptr )
        {
            delete itMap.second;
            itMap.second = nullptr;
        }
    }
    m_mapMeasureParameter.clear();
    m_bInit = false;
}

void MyMeasureParameterManage::ClearAll()
{
    ClearMeasureParameters();
    ClearWidget();
}

void MyMeasureParameterManage::SetCheckPar(const QString &systemCode, const QString &hospitalCode, const QString &sex, const QString &strAge)
{
    m_systemCode = systemCode;
    m_hospitalCode = hospitalCode;
    m_sex = sex;
    m_strAge = strAge;
    m_bInit = true;
}

MyMeasureParameterManage *MyMeasureParameterManage::GetSingle(const QString &strUrl, CNetworkAccessManager *network, QObject *parent)
{
    if (nullptr == m_pSinglePtr)
    {
        m_pSinglePtr = new MyMeasureParameterManage(strUrl, network, parent);
    }
    return m_pSinglePtr;
}

MyMeasureParameterManage *MyMeasureParameterManage::GetSingle()
{
    return m_pSinglePtr;
}
bool IsDigit(const QString &strTitle)
{
    bool bDot = false;
    bool bOk = true;
    for(auto &itData : strTitle)
    {
        if(!bDot && itData == '.')
        {
            bDot = true;
            continue;
        }
        if(!isdigit(itData.toLatin1()))
        {
            bOk = false;
        }
    }
    return bOk;
}

QString MyMeasureParameterManage::CheckWidget(const QString &strId, const QString &strText)
{
    QString strDst;
    if(strText.isEmpty())
    {
        return strDst;
    }
    bool bDouble = false;
    double dData = strText.toDouble(&bDouble);
    if(!bDouble)
    {
        return strDst;
    }
    for(auto &itMapWidget : m_mapWidget)
    {
        for(auto &itVecWidget : itMapWidget.second)
        {
            if( itVecWidget.strId == strId )
            {
                for(auto &itMapTable : m_mapMeasureParameter)
                {
                    MY_MEASURE_PARAMETER_INFO info = itMapTable.second->GetMeasureParameter(itMapWidget.first);
                    if(!info.parameterNormalreminder.isEmpty())
                    {
                        if(dData <= info.normalMin)
                        {
                            strDst = info.parameterSmallreminder;
                        }
                        else if(dData >= info.normalMax)
                        {
                            strDst = info.parameterBigreminder;
                        }
                        break;
                    }
                }
                break;
            }
        }
        if(!strDst.isEmpty())
        {
            break;
        }
    }
    return strDst;
}
void SetWidgetText(QWidget *pWidget, const QString &strText)
{
    std::string strType = typeid(*pWidget).name();
    if( (int)strType.find("MyLineEdit") >= 0 && pWidget->isVisible())
    {
      //  pWidget->blockSignals(true);
        ((MyLineEdit*)(pWidget))->setText(strText);
      //  pWidget->blockSignals(false);
    }
}
QString GetWidgetText(QWidget *pWidget)
{
    // MyLineEdit* pW = ( MyLineEdit*)pWidget;
   // const std::type_info& info = typeid(pWidget);
    std::string strType = typeid(*pWidget).name();
    QString strText ;
    if((int)strType.find("MyCheckBox") >= 0)
    {
        strText = ((MyCheckBox*)(pWidget))->text();
    }
    else if( (int)strType.find("MyRadioButton") >= 0 )
    {

        strText = ((MyRadioButton*)(pWidget))->text();
    }
    else if( (int)strType.find("MyLineEdit") >= 0 )
    {
        strText = ((MyLineEdit*)(pWidget))->text();
    }
    return strText;
}
void MyMeasureParameterManage::ChangeWidgetValue(const  QString &strId)
{
    auto funcMath = [](const MY_MEASURE_PARAMETER_INFO &info, QJSValueList &args)
    {
        QJSEngine jsEngine;
        QJSValue fun = jsEngine.evaluate(QString("(function(%1) { return %2; })")
                                         .arg(info.dependentParameters.join(",")).arg(info.formula));
        QJSValue jsValue = fun.call(args);
        return jsValue.toString();
    };
    int nParameterId = -1;
    for(auto &itMap : m_mapWidget)
    {
        for(auto &itVec : itMap.second)
        {
            if(itVec.strId == strId)
            {
                nParameterId = itMap.first;
                break;
            }
        }
        if(nParameterId != -1)
        {
            break;
        }
    }
    if(nParameterId == -1)
    {
        return;
    }
   // auto itFind = m_mapMeasureParameter.find(tableName);
    for(auto &itFind : m_mapMeasureParameter)
    {
        //找到依赖里有fieldName的
        auto mapInfo = itFind.second->CheckNeedMath(nParameterId);
        if(!mapInfo.empty())
        {
            //遍历依赖里有fieldName的
            for(auto &itInfo : mapInfo)
            {
                //找到输出窗口
                auto itFindDstWiget = m_mapWidget.find(itInfo.second.parameterId);
                if(itFindDstWiget == m_mapWidget.end() || itFindDstWiget->second.empty())
                {
                    continue;
                }

                QJSValueList args;
                std::vector<int> vecId;
                //遍历所有依赖
                for(auto &itDepend : itInfo.second.dependentParameters)
                {
                    //将依赖名转成id
                   // int nId =
                    vecId.push_back(GetID(itDepend));
                }
                for(auto &itId : vecId)
                {
                    //if(itId == )
                    //找到依赖项窗口
                    auto itFindWiget = m_mapWidget.find(itId);
                    if(itFindWiget != m_mapWidget.end() && !itFindWiget->second.empty())
                    {
                        auto itWidget = itFindWiget->second.begin();
                     //   std::string strName = typeid(*(itWidget->pWidget)).name();
                        QString strText = GetWidgetText(itWidget->pWidget);
                        bool bIsOk = false;
                        double dValue = strText.toDouble(&bIsOk);
                        //获取依赖性数据
                        if(!bIsOk)
                        {
                            return;
                           // args.push_back(QJSValue(0.));
                        }
                        else
                        {
                            args.push_back(QJSValue(dValue));
                        }
                    }
                }
                //依靠依赖项参数计算输出结果
                QString str = funcMath(itInfo.second, args);
                auto itBeginWidget = itFindDstWiget->second.begin();
                SetWidgetText((itBeginWidget->pWidget), str);

            }
        }
    }
}

void MyMeasureParameterManage::updataToken(QString token)
{
    m_pNetwork->setRawHeader("Authorization", token.toLatin1());
}


MyMeasureParameterManage* MyMeasureParameterManage::getInstance()
{
    if (!m_pSinglePtr)
    {
        m_pSinglePtr = new MyMeasureParameterManage();
    }
    return m_pSinglePtr;
}

int MyMeasureParameterManage::GetID(const QString &fieldName)
{
    int nId = -1;
    for(auto &itMap : m_mapMeasureParameter)
    {
        nId = itMap.second->GetID(fieldName);
        if(nId != -1)
        {
            return nId;
        }
    }
    return nId;
}
