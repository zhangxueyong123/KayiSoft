#include "structuraldata.h"
#include "shared_mutex"
std::shared_mutex g_cs;
enum eMapDaTaType
{
    eMapDaTaType_UnKnow = -1,
    eMapDaTaType_Int = 0,
    eMapDaTaType_Long,
    eMapDaTaType_Double,
    eMapDaTaType_Char,
    eMapDaTaType_String,
    eMapDaTaType_Bool,
    eMapDaTaType_Context,
    eMapDaTaType_Nodes = eMapDaTaType_Context,
    eMapDaTaType_StateData,
    eMapDaTaType_ReportData,

};
void FindMapData(QVariantMap &mapData,const QString &strKey, void *par, eMapDaTaType type);
void TransNodesData(QVariantList &listData,  MapContext *mapNodes);
StructuralData::StructuralData()
{

}

StructuralData::StructuralData(const stStructuralData &data)
{
    m_StructuralData = data;
    m_StructuralDataOri = data;
}

StructuralData StructuralData::CreateNowStructuralData(const QString &strDepartment, const QString &strBodyPart)
{
    if(strDepartment.isEmpty())
    {
        return  StructuralData();
    }

    stStructuralData data;
    bool bCheckBodyPart =  !strBodyPart.isEmpty();
    auto itBeginContext = m_StructuralData.mapContext.begin();
    while(itBeginContext != m_StructuralData.mapContext.end())
    {
        auto &node = itBeginContext->second;
        bool bBotyPartOk = (bCheckBodyPart ? node.strBodyPart == strBodyPart : true) || node.strBodyPart.isEmpty();
        bool bAdd =(!node.strCode.isEmpty() ) ||    //如果strCode有值
                ((node.strDepartment == strDepartment)&& bBotyPartOk) ;//获取为所设参数
        if(bAdd)
        {
            data.mapContext[itBeginContext->first] = itBeginContext->second;
            if(!node.strBodyPart.isEmpty())
            {
                AddAllChild(data, itBeginContext->first);
            }
        }
        ++itBeginContext;
    }
    return StructuralData(data);
}

void StructuralData::Copy2Ori()
{
    m_StructuralDataOri = m_StructuralData;
}

void StructuralData::ResetData()
{
    m_StructuralData = m_StructuralDataOri;
}

//void StructuralData::TransWholeDatatoMapClass()
//{
//    m_mapStructuralData.clear();
//    QString strDepartment;
//    QString strBodyPart;
//    auto itBeginContext = m_StructuralData.mapContext.begin();
//    while(itBeginContext != m_StructuralData.mapContext.end())
//    {
//        auto &node = itBeginContext->second;
//        auto itFindDepartment = m_mapStructuralData.find(node.strDepartment);
//        if(itFindDepartment == m_mapStructuralData.end())
//        {
//            m_mapStructuralData[node.strDepartment] = stMapStructuralData(node.strDepartment);
//            itFindDepartment = m_mapStructuralData.find(node.strDepartment);
//        }
//        auto &mapData = itFindDepartment->second.mapStructuralData;
//        auto itFindBodyPart = mapData.find(node.strBodyPart);
//        if(itFindBodyPart == mapData.end())
//        {
//            mapData[node.strBodyPart] = stStructuralData();
//            itFindBodyPart = mapData.find(node.strBodyPart);
//        }
//        auto &mapNode = itFindBodyPart->second.mapContext;
//        mapNode[itBeginContext->first] = itBeginContext->second;
//        //        auto itFindNode = mapNode.find(itBeginContext->first);
//        //        if(itFindNode == mapNode.end())
//        //        {

//        //        }
//        ++itBeginContext;
//    }
//}



void TransNodeData(QVariantMap &mapData,  stStructuralNode *pNode)
{

    FindMapData(mapData, "NodeId", &pNode->strNodeId, eMapDaTaType_UnKnow);
    FindMapData(mapData, "ChildrenNum", &pNode->nChildrenNum, eMapDaTaType_UnKnow);
    FindMapData(mapData, "CountChildrenNum", &pNode->nCountChildrenNum, eMapDaTaType_UnKnow);
    FindMapData(mapData, "DataType", &pNode->strDataType, eMapDaTaType_UnKnow);
    FindMapData(mapData, "StrictSort", &pNode->strStrictSort, eMapDaTaType_UnKnow);
    FindMapData(mapData, "ID", &pNode->strID, eMapDaTaType_UnKnow);
    FindMapData(mapData, "ParentId", &pNode->strParentId, eMapDaTaType_UnKnow);

    FindMapData(mapData, "Code", &pNode->strCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Number", &pNode->strNumber, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Depth", &pNode->nDepth, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Title", &pNode->strTitle, eMapDaTaType_UnKnow);
    FindMapData(mapData, "UnionTitle", &pNode->strUnionTitle, eMapDaTaType_UnKnow);
    
    FindMapData(mapData, "Sort", &pNode->nSort, eMapDaTaType_UnKnow);
    FindMapData(mapData, "ChildSort", &pNode->nChildSort, eMapDaTaType_UnKnow);


    FindMapData(mapData, "Value1", &pNode->strValue1, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Value2", &pNode->strValue2, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Value3", &pNode->strValue3, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Value4", &pNode->strValue4, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalCode", &pNode->strHospitalCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalId", &pNode->strHospitalId, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Allocate", &pNode->strAllocate, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Remarks", &pNode->strLoadRemarks, eMapDaTaType_UnKnow);
    //    if(pNode->strLoadRemarks.isEmpty())
    //    {
    //        pNode->strRemarks = QStringLiteral("影像所见");
    //    }
    //    else
    {
        pNode->strRemarks = pNode->strLoadRemarks;
    }
    FindMapData(mapData, "Other", &pNode->strOther, eMapDaTaType_UnKnow);
    FindMapData(mapData, "SystemFlag", &pNode->strSystemFlag, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalName", &pNode->strHospitalName, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalProvince", &pNode->strHospitalProvince, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalProvinceCode", &pNode->strHospitalProvinceCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalCity", &pNode->strHospitalCity, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalCityCode", &pNode->strHospitalCityCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalDistrict", &pNode->strHospitalDistrict, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalDistrictCode", &pNode->strHospitalDistrictCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalTown", &pNode->strHospitalTown, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalTownCode", &pNode->strHospitalTownCode, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Type", &pNode->nType, eMapDaTaType_UnKnow);
    FindMapData(mapData, "LevelType", &pNode->nLevelType, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalNature", &pNode->nHospitalNature, eMapDaTaType_UnKnow);
    FindMapData(mapData, "AscriptionType", &pNode->nAscriptionType, eMapDaTaType_UnKnow);
    FindMapData(mapData, "TypeSummary", &pNode->strTypeSummary, eMapDaTaType_UnKnow);
    FindMapData(mapData, "HospitalNatureSummary", &pNode->strHospitalNatureSummary, eMapDaTaType_UnKnow);
    FindMapData(mapData, "LevelTypeSummary", &pNode->strLevelTypeSummary, eMapDaTaType_UnKnow);
    FindMapData(mapData, "AscriptionTypeSummary", &pNode->strAscriptionTypeSummary, eMapDaTaType_UnKnow);
    FindMapData(mapData, "DistrictArea", &pNode->strDistrictArea, eMapDaTaType_UnKnow);
    FindMapData(mapData, "AttributeSort", &pNode->strAttributeSort, eMapDaTaType_UnKnow);
    FindMapData(mapData, "SerialNumber", &pNode->strSerialNumber, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Label", &pNode->strLabel, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Prefix", &pNode->strPrefix, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Suffix", &pNode->strSuffix, eMapDaTaType_UnKnow);
    FindMapData(mapData, "DefaultValue", &pNode->strDefaultValue, eMapDaTaType_UnKnow);
    FindMapData(mapData, "TextSort", &pNode->strTextSort, eMapDaTaType_UnKnow);
    FindMapData(mapData, "DataTypeSummary", &pNode->strDataTypeSummary, eMapDaTaType_UnKnow);



 //   FindMapData(mapData, "Other", &pNode->strOther, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Tip", &pNode->strTip, eMapDaTaType_UnKnow);

    FindMapData(mapData, "Formula", &pNode->strFormula, eMapDaTaType_UnKnow);
    FindMapData(mapData, "FormulaSummary", &pNode->strFormulaSummary, eMapDaTaType_UnKnow);


    QString str;
    FindMapData(mapData, "ElementAttribute", &str, eMapDaTaType_UnKnow);
    if(!str.isEmpty())
    {
        pNode->nLoadElementAttribute = str.toInt();
        int nDataMath = eStructuralDataType_UnNow;
        switch(pNode->nLoadElementAttribute)
        {
        case 0:nDataMath = eStructuralDataType_Single;break;
        case 1:nDataMath = eStructuralDataType_Multi;break;
        case 2:nDataMath = eStructuralDataType_Edit;break;
        case 3:nDataMath = eStructuralDataType_Multi;  pNode->bIsExplain = true;break;
        }
        if(nDataMath != eStructuralDataType_UnNow)
        {
            pNode->eElementAttribute =(eStructuralDataType)(nDataMath) ;
        }
        pNode->bHasElementAttribute = true;
    }
    else
    {
        pNode->bHasElementAttribute = false;
    }
    if(pNode->strDefaultValue.isEmpty() &&  !pNode->strValue2.isEmpty())
    {
        pNode->strDefaultValue = pNode->strValue2 == "1" ? "true" : "false";
    }

    pNode->strDepartment = pNode->strValue3;
    pNode->strBodyPart = pNode->strValue4;
    pNode->bStatus = pNode->strValue1 == "1";
    if(pNode->bStatus)
    {
        FindMapData(mapData, "Nodes", &(pNode->mapChild), eMapDaTaType_Nodes);
    }
    else
    {
        FindMapData(mapData, "Nodes", (pNode->pMapNodes), eMapDaTaType_Nodes);
    }
}
void TransState(QVariantList &listData,
                std::vector<stTableState> *vecNodes)
{
    for(auto &itList : listData)
    {
        if(itList.type() == QVariant::Map)
        {
            auto result = itList.toMap();
            QString strFirst;
            QString strSecond;
            bool bViewer = false;
            bool bEdit = false;
            FindMapData(result, "id", &strFirst, eMapDaTaType_UnKnow);
            FindMapData(result, "state", &strSecond, eMapDaTaType_UnKnow);
            FindMapData(result, "viewer", &bViewer, eMapDaTaType_UnKnow);
            FindMapData(result, "edit", &bEdit, eMapDaTaType_UnKnow);

            if(!strFirst.isEmpty())
            {
                vecNodes->push_back( {strSecond,strFirst, bViewer,bEdit});
                //   mapNodes->insert({strFirst, {strSecond,bViewer}});
            }
        }

    }
}
void TransReportData(QVariantList &listData,  listReportData*pList)
{
    for(auto &itList : listData)
    {
        if(itList.type() == QVariant::Map)
        {
            auto result = itList.toMap();
            stReportData data;
            FindMapData(result, "id", &data.strId, eMapDaTaType_UnKnow);
            FindMapData(result, "preData", &data.drawData.strDrawPre, eMapDaTaType_UnKnow);
            FindMapData(result, "drawData", &data.drawData.strDrawData, eMapDaTaType_UnKnow);
            FindMapData(result, "suffData", &data.drawData.strDrawSuff, eMapDaTaType_UnKnow);
            FindMapData(result, "suffDataAdd", &data.drawData.strDrawSuffAdd, eMapDaTaType_UnKnow);


            QString strEdit;
            FindMapData(result, "edit", &strEdit, eMapDaTaType_UnKnow);
            data.drawData.bEdit = strEdit == "true";
            auto itFind = result.find("selectList");
            if(itFind != result.end()  && !itFind->isNull() && itFind->type() ==  QVariant::List)
            {
                auto selectList = itFind->toList();
                for(auto &itSelect : selectList)
                {
                    if(itSelect.type() == QVariant::Map)
                    {
                        auto selectObject = itSelect.toMap();
                        QString strId;
                        QString strDrawData;
                        FindMapData(selectObject, "drawData", &strDrawData, eMapDaTaType_UnKnow);
                        FindMapData(selectObject, "id", &strId, eMapDaTaType_UnKnow);
                        data.drawData.vecData.push_back(strDrawData);
                        data.drawData.vecId.push_back(strId);
                    }
                }
            }
            pList->push_back(data);
        }

    }
}
void TransNodesData(QVariantList &listData,  MapContext *mapNodes)
{
    for(auto &itList : listData)
    {
        if(itList.type() == QVariant::Map)
        {
            stStructuralNode node;
            node.pMapNodes = mapNodes;
            auto result = itList.toMap();
            TransNodeData(result, &node);
            // if( pNode->strValue1 == "1")
            mapNodes->insert({node.strID, node});
        }

    }
}

//为par赋值
void FindMapData(QVariantMap &mapData,const QString &strKey, void *par, eMapDaTaType type)
{
    auto itFind = mapData.find(strKey);
    if(itFind == mapData.end()  || itFind->isNull() || itFind->type() == QVariant::Invalid)
    {
        return;
    }
    if(type == eMapDaTaType_UnKnow)
    {
        auto findType = itFind->type() ;
        switch (findType) {
        case QVariant::Bool:type = eMapDaTaType_Bool;break;
        case QVariant::Int:
        case QVariant::UInt:type = eMapDaTaType_Int;break;
        case QVariant::LongLong:
        case QVariant::ULongLong:type = eMapDaTaType_Int;break;
        case QVariant::Double: type = eMapDaTaType_Int;break;
        case QVariant::Char: type = eMapDaTaType_Char;break;
        case QVariant::String:type = eMapDaTaType_String;break;
        default:
            return;
        }
    }
    switch (type)
    {

    case  eMapDaTaType_Int :
    {
        int *pPar = (int *)par;
        *pPar = itFind->toInt();
        break;
    }
    case  eMapDaTaType_Long :
    {
        long long *pPar = (long long *)par;
        *pPar = itFind->toLongLong();
        break;
    }

    case eMapDaTaType_Char:
    {
        QString *pPar = (QString *)par;
        auto charData = itFind->toChar();
        *pPar =QString(charData);
        break;
    }
    case eMapDaTaType_String :
    {
        QString *pPar = (QString *)par;
        //QString str = itFind->toString().toLocal8Bit();
        *pPar =itFind->toString();
        break;
    }
    case eMapDaTaType_Bool:
    {
        bool *pPar = (bool *)par;
        *pPar = itFind->toBool();
        break;
    }
    case eMapDaTaType_Nodes:
    {
        if(itFind->type() ==  QVariant::List)
        {
            auto result = itFind->toList();
        /*    QFuture<void > end = QtConcurrent::run([&result,par] {
                TransNodesData(result, (MapContext*)par); 
                });
            break;*/
            TransNodesData(result, (MapContext *)par);break;
        }
        break;
    }
    case eMapDaTaType_StateData:
    {
        if(itFind->type() ==  QVariant::List)
        {
            auto result = itFind->toList();
            TransState(result, (std::vector<stTableState>*)par);break;
        }
        break;
    }
    case eMapDaTaType_ReportData:
    {
        if(itFind->type() ==  QVariant::List)
        {
            auto result = itFind->toList();
            TransReportData(result, (listReportData *)par);break;
        }
        break;
    }
    default:
        break;
    }


}

std::vector<QString> TrasnStructuralData(QVariantMap &mapData, stStructuralData *pData)
{
    FindMapData(mapData, "StatusCode",&pData->nStatusCode,eMapDaTaType_UnKnow );
    FindMapData(mapData, "Success",&pData->bSuccess, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Msg",&pData->strMsg, eMapDaTaType_UnKnow);
    FindMapData(mapData, "Errors",&pData->strErrors, eMapDaTaType_UnKnow);
    if(!pData->bSuccess || (pData->nStatusCode != 200 && pData->nStatusCode != 0))
    {
        return  std::vector<QString>();
    }
    FindMapData(mapData, "Extras",&pData->strExtras,eMapDaTaType_UnKnow);
    FindMapData(mapData, "Timestamp",&pData->nTimestamp,eMapDaTaType_Long);
    FindMapData(mapData, "Context",&pData->mapContext,eMapDaTaType_Context);
    std::vector<QString> vecId;
    for(auto & itMap : pData->mapContext)
    {
        bool bIsTop = CheckTopType(itMap.second.strCode);
        if(!bIsTop )
        {
            vecId.push_back(itMap.first);
        }

    }
    return vecId;
}
void GetAllCode(const MapContext &mapNode,std::vector<QString> *pVecCode)
{
    for(auto &itMap : mapNode)
    {
        if(!itMap.second.strCode.isEmpty())
        {
            if(pVecCode->end() == std::find(pVecCode->begin(), pVecCode->end(),itMap.second.strCode ))
            {
                pVecCode->push_back(itMap.second.strCode );
            }
        }
        GetAllCode(itMap.second.mapChild,pVecCode );
    }
}
std::vector<QString> StructuralData::AddStructuralDataByJson(const QString &strJson, std::vector<QString> *pVecCode, const QString &strFirst , const QString &strSecond)
{
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            stStructuralData data;
            auto result = jsonDocument.toVariant().toMap();
            if(!result.empty())
            {
                bool bFirst = false;
                std::vector<QString> vecId;
                bool bEmpty = false;
                {
                    std::shared_lock<std::shared_mutex> lock(g_cs);
                    bEmpty = m_StructuralData.IsEmpty();
                }
                if(bEmpty)
                {
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    bFirst = true;
                    vecId = TrasnStructuralData(result, &m_StructuralData);
                    if(pVecCode != nullptr)
                    {
                        GetAllCode(m_StructuralData.mapContext, pVecCode);

                    }
                 //   pData = &m_StructuralData;
                }
                else
                {
                    vecId = TrasnStructuralData(result, &data);
                    if(pVecCode != nullptr)
                    {
                        GetAllCode(data.mapContext, pVecCode);

                    }
                  //  pData = &data;
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    m_StructuralData.MergeStructuralData(data,strSecond.isEmpty() ?  strFirst : strSecond);
                }

                {
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    if(!bFirst)
                    {
                        m_StructuralData.MergeStructuralData(data,strSecond.isEmpty() ?  strFirst : strSecond);

                    }
                    if(!strFirst.isEmpty())
                    {
                        m_StructuralData.SetTop(strFirst);
                    }
                    if(!strSecond.isEmpty())
                    {
                        m_StructuralData.SetTop(strSecond);
                    }
                }
                return vecId;
            }

        }

    }
    return std::vector<QString>();
}
QString StructuralData::screeningForNKJ( QString strJson,  QString strFirst,  QString strSecond)
{
    //strFirst = "edd58a5a-5037-4959-a647-6e3dd482cd22";
    // 解析JSON数据
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Failed to create JSON doc.";
        return "";
    }

    // 获取根节点
    QJsonObject root = doc.object();

    // 保存需要保留的节点数据
    QJsonArray newNodes;
    QJsonArray nodes = root["Context"].toArray()[0].toObject()["Nodes"].toArray();
    //先寻找默认节点
    if (strFirst == "")
    {
        for (const auto& node : nodes) {
            QJsonObject nodeObj = node.toObject();

            if (nodeObj["Value2"].toString() == "1") {
                strFirst = nodeObj["NodeId"].toString();
            }
        }

    }

    for (const auto& node : nodes) {
        QJsonObject nodeObj = node.toObject();
        ControlCenter::getInstance()->addFirstData(nodeObj["NodeId"].toString(), nodeObj["Title"].toString());
        if (nodeObj["NodeId"].toString() == strFirst) {  
            newNodes.append(nodeObj);
        }
    }
     // 创建新的JSON数据包
    QJsonObject newRoot;
    newRoot["Errors"] = root["Errors"];
    newRoot["Extras"] = root["Extras"];
    newRoot["Msg"] = root["Msg"];
    newRoot["StatusCode"] = root["StatusCode"];
    newRoot["Success"] = root["Success"];
    newRoot["Timestamp"] = root["Timestamp"];
    newRoot["Context"] = newNodes;
    QJsonDocument newDoc(newRoot);

    // 将新的JSON数据包转换为字符串并打印输出
    QString newJsonString = QString::fromUtf8(newDoc.toJson());
    return newJsonString;
}


QString StructuralData::screening(QString strJson, QString strFirst, QString strSecond, bool isFirst)
{
    // 解析JSON数据
    QJsonDocument doc = QJsonDocument::fromJson(strJson.toUtf8());
    if (doc.isNull()) {
        qDebug() << "Failed to create JSON doc.";
        return "";
    }

    // 获取根节点
    QJsonObject root = doc.object();

    // 保存需要保留的节点数据
    QJsonArray newNodes;
    QJsonArray nodes = root["Context"].toArray()[0].toObject()["Nodes"].toArray();
    if (strFirst == "" && strSecond == "")
    {
        if (ControlCenter::getInstance()->m_department != "" && ControlCenter::getInstance()->m_bodyPart != "")
        {
            for (const auto& node : nodes) {
                QJsonObject nodeObj = node.toObject();
                if (nodeObj["Value3"].toString() == ControlCenter::getInstance()->m_department)
                {
                    strFirst = nodeObj["NodeId"].toString();
                    ControlCenter::getInstance()->m_defaultFirstId = strFirst;
                    QJsonArray subNodes;
                    QJsonArray subNodesData = nodeObj["Nodes"].toArray();
                    for (const auto& subNode : subNodesData) {
                        QJsonObject subNodeObj = subNode.toObject();
                        auto list = ControlCenter::getInstance()->m_bodyPart.split(",");
                        if (list.contains(subNodeObj["Value4"].toString())) {
                            strSecond = subNodeObj["NodeId"].toString();
                            ControlCenter::getInstance()->m_defaultSecondId = strSecond;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            for (const auto& node : nodes) {
                QJsonObject nodeObj = node.toObject();
                if (nodeObj["Value2"].toString() == "1")
                {
                    strFirst = nodeObj["NodeId"].toString();
                    ControlCenter::getInstance()->m_defaultFirstId = strFirst;
                    QJsonArray subNodes;
                    QJsonArray subNodesData = nodeObj["Nodes"].toArray();
                    for (const auto& subNode : subNodesData) {
                        QJsonObject subNodeObj = subNode.toObject();

                        if (subNodeObj["Value2"].toString() == "1") {
                            strSecond = subNodeObj["NodeId"].toString();
                            ControlCenter::getInstance()->m_defaultSecondId = strSecond;
                        }
                    }
                }
            }
        }

    }

    for (const auto& node : nodes) {
        QJsonObject nodeObj = node.toObject();
        if (isFirst)
            ControlCenter::getInstance()->addFirstData(nodeObj["NodeId"].toString(), nodeObj["Title"].toString());
        if (nodeObj["NodeId"].toString() == strFirst) {
            QJsonArray subNodes;
            QJsonArray subNodesData = nodeObj["Nodes"].toArray();
            for (const auto& subNode : subNodesData) {
                QJsonObject subNodeObj = subNode.toObject();
                if (isFirst)
                    ControlCenter::getInstance()->addSecondData(subNodeObj["NodeId"].toString(), subNodeObj["Title"].toString(), nodeObj["NodeId"].toString());
                if (subNodeObj["NodeId"].toString() == strSecond) {
                    subNodes.append(subNodeObj);
                }
            }
            nodeObj["Nodes"] = subNodes;
            newNodes.append(nodeObj);
        }
        else
        {
            QJsonArray subNodes;
            QJsonArray subNodesData = nodeObj["Nodes"].toArray();
            for (const auto& subNode : subNodesData) {
                QJsonObject subNodeObj = subNode.toObject();
                if (isFirst)
                    ControlCenter::getInstance()->addSecondData(subNodeObj["NodeId"].toString(), subNodeObj["Title"].toString(), nodeObj["NodeId"].toString());
                if (subNodeObj["NodeId"].toString() == strSecond) {
                    subNodes.append(subNodeObj);
                }
            }
            nodeObj["Nodes"] = subNodes;
            //newNodes.append(nodeObj);
        }
    }
    /* ControlCenter::getInstance()->addFirstData("545454as","6666");*/
     // 创建新的JSON数据包
    QJsonObject newRoot;
    newRoot["Errors"] = root["Errors"];
    newRoot["Extras"] = root["Extras"];
    newRoot["Msg"] = root["Msg"];
    newRoot["StatusCode"] = root["StatusCode"];
    newRoot["Success"] = root["Success"];
    newRoot["Timestamp"] = root["Timestamp"];
    newRoot["Context"] = newNodes;
    QJsonDocument newDoc(newRoot);

    // 将新的JSON数据包转换为字符串并打印输出
    QString newJsonString = QString::fromUtf8(newDoc.toJson());

    return newJsonString;
}
static QString g_json = "";

std::vector<QString> StructuralData::AddStructuralDataByJson1(const QString& strJson, std::vector<QString>* pVecCode, const QString& strFirst, const QString& strSecond)
{
    QJsonParseError error;
    QJsonDocument jsonDocument; 
    //if (strJson != "")
    //{
    //    g_json = strJson;
        jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(), &error);
    //}      
    //else
    //{
    //    jsonDocument = QJsonDocument::fromJson(g_json.toUtf8(), &error);
    //}

    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            stStructuralData data;
            auto result = jsonDocument.toVariant().toMap();
            
            if (!result.empty())
            {
                bool bFirst = false;
                std::vector<QString> vecId;
                bool bEmpty = false;
                {
                    std::shared_lock<std::shared_mutex> lock(g_cs);
                    bEmpty = m_StructuralData.IsEmpty();
                }
                if (bEmpty)
                {
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    bFirst = true;
                    vecId = TrasnStructuralData(result, &m_StructuralData);
                    if (pVecCode != nullptr)
                    {
                        GetAllCode(m_StructuralData.mapContext, pVecCode);

                    }
                    //   pData = &m_StructuralData;
                }
                else
                {
                    QVariantMap result;
                    //把result处理一下       
                    QString str;
                    if (ControlCenter::getInstance()->m_type == eTemplateType_SRES)
                    {
                        str = screeningForNKJ(strJson, strFirst, "");
                        QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
                        if (doc.isNull()) {
                            qDebug() << "Failed to create JSON doc.";
                        }
                        result = doc.toVariant().toMap();
                    }                  
                    else
                    {
                        if (ControlCenter::getInstance()->needLoadAll)
                        {
                            str = screening(strJson, strFirst, strSecond, 1);
                            QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
                            if (doc.isNull()) {
                                qDebug() << "Failed to create JSON doc.";       
                            }
                            result = doc.toVariant().toMap();
                        }                  
                        else
                        {
                            BothId b(strFirst, strSecond);
                            auto it = ControlCenter::getInstance()->jsonMap.find(b);
                            if(it != ControlCenter::getInstance()->jsonMap.end())
                                result = it.value();
                        }                
                    }    
                    //保存所有json信息
                    if (/*ControlCenter::getInstance()->needLoadAll*/0)
                    {
                        auto bothMap = ControlCenter::getInstance()->getDataSecondMap();
                        QList<QFuture<void>> retList;
                        // 使用迭代器遍历嵌套的 QMap
                        for (auto it = bothMap.begin(); it != bothMap.end(); ++it)
                        {
                            QFuture<void> ret = QtConcurrent::run([it, this, strJson]
                            {
                                QString key = it.key();
                                QMap<QString, QString> innerMap = it.value();
                                for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
                                {
                                    QString innerKey = innerIt.key();
                                    QString json = screening(strJson, key, innerKey, 0);
                                    BothId b(key, innerKey);
                                    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
                                    if (doc.isNull()) {
                                        qDebug() << "Failed to create JSON doc.";
                                    }
                                    auto _result = doc.toVariant().toMap();
                                    ControlCenter::getInstance()->jsonMap.insert(b, _result);
                                }
                            });
                            retList.append(ret);
                        }
                        bool stop = 0;
                        while (!stop)
                        {
                            int _stop = 1;
                            for (int i = 0; i < bothMap.size(); i++)
                            {
                                _stop &= retList.at(i).isFinished();
                            }
                            if (_stop)
                            {
                                stop = 1;
                            }
                            else
                                QApplication::processEvents(QEventLoop::AllEvents, 3);
                        }
                        ControlCenter::getInstance()->needLoadAll = false;
                    }

                    //QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8());
                    //if (doc.isNull()) {
                    //    qDebug() << "Failed to create JSON doc.";       
                    //}
                    //result = doc.toVariant().toMap();


                    //------------------------
                    auto Context = result["Context"];
                    //auto Nodes = Context.toJsonArray();
                    vecId = TrasnStructuralData(result, &data);
                    if (pVecCode != nullptr)
                    {
                        GetAllCode(data.mapContext, pVecCode);
                    }
                    //  pData = &data;
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    m_StructuralData.MergeStructuralData(data, strSecond.isEmpty() ? strFirst : strSecond);
                }

                {
                    std::unique_lock<std::shared_mutex> lock(g_cs);
                    if (!bFirst)
                    {
                        //m_StructuralData.MergeStructuralData(data, strSecond.isEmpty() ? strFirst : strSecond);

                    }
                    if (!strFirst.isEmpty())
                    {
                        m_StructuralData.SetTop(strFirst);
                    }
                    if (!strSecond.isEmpty())
                    {
                        m_StructuralData.SetTop(strSecond);
                    }
                }
                return vecId;
            }

        }

    }
    return std::vector<QString>();
}


std::map<eTemplateType, stTopPar> StructuralData::GetTopPar()
{
    std::map<eTemplateType, stTopPar> mapData;
    for(auto &itNode : m_StructuralData.mapContext)
    {
//        if(!itNode.second.strCode.isEmpty())
//        {
//            int a = 0;
//        }
        auto dataType = TransStr2Type(itNode.second.strCode);

        if(dataType != eTemplateType_All && dataType  != eTemplateType_Null)
        {
            mapData[dataType] = stTopPar(itNode.second.strID, itNode.second.nDepth);
        }
    }
    return mapData;
}

stStructuralData *StructuralData::GetStructuralDataPtr()
{
    return &m_StructuralData;
}

stStructuralData StructuralData::GetStructuralData()
{
    return m_StructuralData;
}



void StructuralData::AddChildByJson(const QString &strJson, const QString &strId)
{

    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            stStructuralData data;

            auto result = jsonDocument.toVariant().toMap();
            if(!result.empty())
            {
                TrasnStructuralData(result, &data);
                auto itFind = m_StructuralData.mapContext.find(strId);
                if(itFind == m_StructuralData.mapContext.end())
                {
                    return;
                }
                itFind->second.mapChild = data.mapContext;

            }

        }

    }
}
stStructuralNode *StructuralData::FindNodePtrByTitle(const QString &strTitle)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strTitle == strTitle)
        {
            return &itMap.second;
        }
    }
    return nullptr;
}

stStructuralNode StructuralData::FindNodeByTitle(const QString &strTitle, bool *pHas)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strTitle == strTitle)
        {
            *pHas = true;
            return itMap.second;
        }
    }
    *pHas = false;
    return stStructuralNode();
}

stStructuralNode *StructuralData::FindNodePtrById(const QString &strId)
{
    auto itFind =  m_StructuralData.mapContext.find(strId);
    if(itFind == m_StructuralData.mapContext.end())
    {
        return nullptr;
    }
    return &(itFind->second);
}

stStructuralNode StructuralData::FindNodeById(const QString &strId, bool *pHas)
{
    auto itFind =  m_StructuralData.mapContext.find(strId);
    if(itFind == m_StructuralData.mapContext.end())
    {
        *pHas = false;
        return stStructuralNode();
    }
    *pHas = true;
    return itFind->second;
}

bool StructuralData::SetInitByCode(const QString &strCode, bool bInit)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            itMap.second.bIsInit = bInit;
            return true;
        }
    }
    return false;
}

void StructuralData::SetAllInit(bool bInit)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(!itMap.second.strCode.isEmpty())
        {
            itMap.second.bIsInit = bInit;
        }
    }
}

stStructuralNode *StructuralData::FindNodePtrByCode(const QString &strCode)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            if(!itMap.second.bIsInit)
            {
                return nullptr;
            }
            return &itMap.second;
        }
    }
    return nullptr;
}

stStructuralNode StructuralData::FindNodeByCode(const QString &strCode, bool *pHas)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            *pHas = true;
            return itMap.second;
        }
    }
    *pHas = false;
    return stStructuralNode();
}

std::vector<stStructuralNode > StructuralData::FindChildById(const QString &strId, bool bSort)
{
    std::vector<stStructuralNode > vecNode;
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strParentId == strId)
        {
            vecNode.push_back(itMap.second);
        }
    }
    if(bSort)
    {
        std::sort(vecNode.begin(), vecNode.end(), [](const stStructuralNode &node1, const stStructuralNode& node2)
        {
            return node1.nSort < node2.nSort;
        });
    }
    return vecNode;
}
void CheckChild(std::vector<stStructuralNode *> & vecFindNode,  std::vector<stStructuralNode > &vecChildNode)
{
    if(vecFindNode.empty())
    {
        return;
    }
    std::vector<stStructuralNode *> vecNewFindNode;
    for(auto &itChild : vecChildNode)
    {
        QString strParentId = itChild.strParentId;
        QString strId= itChild.strID;
        for(auto &itFind : vecFindNode)
        {
            for(auto&itFindNode : itFind->mapChild)
            {
               
                if(itFindNode.first == strParentId)
                {
                    itFindNode.second.mapChild[strId] = itChild;
                    vecNewFindNode.push_back(&(itFindNode.second));
                    break;
                }
            }
        }
    }
    CheckChild(vecNewFindNode,vecChildNode);
}
MapTable TransMapNodes2MapTable(const MapNodes & mapNodes)
{
    MapNodes firstNode;
    MapNodes childNode;

    std::vector<stStructuralNode> vecChildNode;
    //  std::vector<QString> vecParent;
    std::vector<QString> vecId;
    for(auto &itMap : mapNodes)
    {
        // vecParent.push_back(itMap.second.strParentId);
        vecId.push_back(itMap.second.strID);
    }
    for(auto &itMap : mapNodes)
    {
        if(vecId.end() == std::find(vecId.begin(), vecId.end(),itMap.second.strParentId))
        {
            firstNode[itMap.first] = itMap.second;
        }
        else
        {
            vecChildNode.push_back(itMap.second);
        }
    }
    MapTable table;
    struct stTableTmp
    {
        int nSort = 0;
        std::vector<QString> vecId;
    };
    std::map<QString, stTableTmp> mapTitle;
    for(auto &itMap : firstNode)
    {
        QString strKey = itMap.second.strTitle;

        auto itFind = mapTitle.find(strKey );
        if(itFind == mapTitle.end())
        {
            mapTitle[strKey] = stTableTmp();
            mapTitle[strKey].nSort = itMap.second.nSort;
            mapTitle[strKey].vecId.push_back(itMap.first);
        }
        else
        {
            mapTitle[strKey].nSort = mapTitle[strKey].nSort < itMap.second.nSort ? mapTitle[strKey].nSort :  itMap.second.nSort;

            mapTitle[strKey].vecId.push_back(itMap.first);
        }
    }
    int nMaxSort = (int)mapTitle.size();
    int nCul = 0;
    for(auto &itMapTitle : mapTitle)
    {
        QString strKey = itMapTitle.first;

        for(auto&itNow : table)
        {
            if(itNow.first.nSort ==  itMapTitle.second.nSort)
            {
                itMapTitle.second.nSort = nMaxSort + nCul++;
            }
        }
        table[stTableKey(itMapTitle.first, itMapTitle.second.nSort)] = std::map<stTableKey, stStructuralNode>();
    }

    for(auto &itMap : firstNode)
    {
        for(auto &itFind : table)
        {
            QString s1 = itMap.second.strTitle;
            QString s2 = itFind.first.strKey;
            if(itFind.first == itMap.second.strTitle)
            {
                stTableKey key(itMap.first, itMap.second.nChildSort);
                itFind.second[key] = itMap.second;

            }
        }
    }
    std::vector<stStructuralNode *> vecFindNode;
    for(auto &itChild : vecChildNode)
    {
        QString strParentId = itChild.strParentId;
        QString strId= itChild.strID;
        for(auto &itFind : table)
        {
            for(auto &itFindNode : itFind.second)
            {
                if(itFindNode.first.strKey == strParentId)
                {
                    itFindNode.second.mapChild[strId] = itChild;
                    vecFindNode.push_back(&(itFindNode.second));
                    break;
                }

            }
        }
    }
    CheckChild(vecFindNode,vecChildNode);
    return table;
}
MapTable StructuralData::FindChildTable(const QString &strId)
{
    auto itFind = m_StructuralData.mapContext.find(strId);
    return itFind == m_StructuralData.mapContext.end() ? MapTable() : TransMapNodes2MapTable(itFind->second.mapChild);
}

std::vector<stStructuralNode> StructuralData::FindChildByCode(const QString &strCode, bool bSort)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            return FindChildById(itMap.first, bSort);
        }
    }
    return std::vector<stStructuralNode>();
}

std::vector<QString> StructuralData::FindChildIdById(const QString &strId)
{
    std::vector<QString > vecNode;
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strParentId == strId)
        {
            vecNode.push_back(itMap.first);
        }
    }
    return vecNode;
}

std::vector<QString> StructuralData::FindChildIdByCode(const QString &strCode)
{
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            return FindChildIdById(itMap.first);
        }
    }
    return std::vector<QString>();
}

std::vector<QString> StructuralData::FindAllStateId()
{
    std::vector<QString> vecId;
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.bStatus)
        {
            vecId.push_back(itMap.first);
        }
    }
    return vecId;
}

void StructuralData::FindAllChildIdById(const QString &strId, std::vector<QString> *pVecId, bool bSelf )
{
    if(pVecId == nullptr)
    {
        return;
    }
    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.first == strId && bSelf)
        {
            pVecId->push_back(itMap.first);
        }
        else if(itMap.second.strParentId == strId)
        {
            pVecId->push_back(itMap.first);
            FindAllChildIdById(itMap.first, pVecId,true);
        }
    }
}

void StructuralData::ClearById(const QString &strId)
{
    std::vector<QString> vecId;
    FindAllChildIdById(strId, &vecId,false);

    if(vecId.empty())
    {
        for(auto &itVec :vecId)
        {
            auto itFind = m_StructuralData.mapContext.find(itVec);
            if(itFind != m_StructuralData.mapContext.end())
            {
                m_StructuralData.mapContext.erase(itFind);
            }
        }
    }
}

void StructuralData::ClearByCode(const QString &strCode)
{

    for(auto &itMap : m_StructuralData.mapContext)
    {
        if(itMap.second.strCode == strCode)
        {
            itMap.second.bIsInit = false;
            ClearById(itMap.first);
            return;
        }
    }
}

QString StructuralData::TransMapStatetoString(const std::vector<stTableState> &vecState)
{
    if(vecState.empty())
    {
        return "";
    }
    QJsonDocument doc;
    QJsonArray array;
    for(auto &itMap : vecState)
    {
        QJsonObject singleState;
        singleState["id"] = itMap.strId;
        singleState["state"] = itMap.strState;
        singleState["viewer"] = itMap.bViewer;
        singleState["edit"] = itMap.bIsEdit;
        array.append(singleState);
    }
    QJsonObject state;
    state["data"] = array;
    doc.setObject(state);
    return QString(doc.toJson());
}

QString StructuralData::TransReporttoString(const listReportData &list, const QString &strRemarks)
{
    if(list.empty())
    {
        return "";
    }
    QJsonDocument doc;
    QJsonArray array;
    for(auto &itList : list)
    {
        QJsonObject singleState;
        singleState["id"] = itList.strId;
        singleState["preData"] =  itList.drawData.strDrawPre;
        singleState["drawData"] =  itList.drawData.strDrawData;
        singleState["suffData"] =  itList.drawData.strDrawSuff;
        singleState["suffDataAdd"] =  itList.drawData.strDrawSuffAdd;
        singleState["edit"] =  (itList.drawData.bEdit ? QStringLiteral("true") : QStringLiteral("false"));
        QJsonArray selectList;
        if(!itList.drawData.vecData.empty())
        {
            //  auto itData = itList.drawData.vecData.begin();
            auto itId = itList.drawData.vecId.begin();
            for(auto &itData : itList.drawData.vecData)
            {
                QJsonObject selectSingle;

                if(itId != itList.drawData.vecId.end())
                {
                    selectSingle["id"] = *itId;
                    ++itId;
                }
                else
                {
                    selectSingle["id"] = "";
                }
                selectSingle["drawData"] = itData;
                selectList.append(selectSingle);
            }
        }
        singleState["selectList"] = selectList;

        array.append(singleState);
    }
    QJsonObject data;
    data["data"] = array;
    data["remarks"] = strRemarks;
    QJsonArray listArray;
    listArray.push_back(data);
    QJsonObject listObj;
    listObj["list"] = listArray;
    doc.setObject(listObj);
    return QString(doc.toJson());
}

listReportData StructuralData::TransStringtoReport(const QString &strJson, const QString &strRemarks, bool bAll)
{
    QJsonParseError error;
    listReportData list;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            auto result = jsonDocument.toVariant().toMap();

            if(!result.empty())
            {
                auto itFind = result.find("list");
                if(itFind != result.end()  && !itFind->isNull() && itFind->type() ==  QVariant::List)
                {
                    auto listData = itFind->toList();
                    for(auto &itList : listData)
                    {
                        auto listMap = itList.toMap();
                        QString strNowRemarks;
                        FindMapData(listMap, "remarks",&strNowRemarks, eMapDaTaType_UnKnow);
                        if(bAll || strNowRemarks == strRemarks)
                        {
                            //  auto itFind = result.find("selectList");
                            FindMapData(listMap, "data",&list, eMapDaTaType_ReportData);
                        }


                    }
                }


            }
        }
    }
    return list;
}

QString StructuralData::MergeStructuralDataReportJson(const std::vector<QString> &vecJson)
{
    QString str;
    if(vecJson.empty())
    {
        return str;
    }
    //  QJsonObject listObj;
    QJsonArray array;
    // QJsonDocument jsonDocument;
    for(auto &itVec : vecJson)
    {
        if(itVec.isEmpty())
        {
            break;
        }
        QJsonParseError error;

        QJsonDocument jsonDocument = QJsonDocument::fromJson(itVec.toUtf8(),&error);

        if (error.error != QJsonParseError::NoError || !jsonDocument.isObject())
        {

            break;
        }


        auto result = jsonDocument.toVariant().toMap();
        if(result.empty())
        {
            break;
        }
        auto itFind = result.find("list");
        if(itFind != result.end()  && !itFind->isNull() && itFind->type() ==  QVariant::List)
        {
            auto nowList = itFind->toList();
            for(auto &itList : nowList)
            {
                array.append(itList.toJsonObject());
            }
        }
        else
        {
            break;


        }
    }
    QJsonDocument doc;
    QJsonObject listObj;
    listObj["list"] = array;
    doc.setObject(listObj);

    return QString(doc.toJson());
}

//bool StructuralData::CheckIsStruct(const std::vector<listReportData> &vecList)
//{
//    bool bIsStruct = false;
//    for(auto &itVec : vecList)
//    {

//    }
//    return bIsStruct;
//}


bool StructuralData::IsEmpty()
{
    return m_StructuralData.IsEmpty();
}

void StructuralData::GetRemarksList(std::vector<QString> *pVec, stStructuralNode *pNode)
{
    if(pVec == nullptr)
    {
        return ;
    }
    if(pNode == nullptr)
    {
        for(auto &itMap : m_StructuralData.mapContext)
        {
            GetRemarksList(pVec, &itMap.second);
        }
    }
    else if(!pNode->strRemarks.isEmpty())
    {
        auto itFind = std::find(pVec->begin(), pVec->end(), pNode->strRemarks);
        if(itFind == pVec->end())
        {
            pVec->push_back(pNode->strRemarks);
        }
        for(auto &itChild : pNode->mapChild)
        {
            GetRemarksList(pVec, &itChild.second);
        }
    }
}
QJsonArray GetStructNodeArray(const MapNodes &mapNode)
{
    QJsonArray array;
    for(auto &itMap : mapNode)
    {
        QJsonObject object;
        object["NodeId"] = itMap.second.strNodeId;
        object["ChildrenNum"] = itMap.second.nChildrenNum;
        object["CountChildrenNum"] = itMap.second.nCountChildrenNum;
        object["DataType"] = itMap.second.strDataType;
        object["StrictSort"] = itMap.second.strStrictSort;
        object["ID"] = itMap.second.strID;
        object["ParentId"] = itMap.second.strParentId;
        object["Code"] = itMap.second.strCode;
        object["Number"] = itMap.second.strNumber;
        object["Depth"] = itMap.second.nDepth;
        object["Title"] = itMap.second.strTitle;
        object["UnionTitle"] = itMap.second.strUnionTitle;

      //  FindMapData(mapData, "UnionTitle", &pNode->strUnionTitle, eMapDaTaType_UnKnow);
        object["Sort"] = itMap.second.nSort;
        object["ChildSort"] = itMap.second.nChildSort;
        object["Value1"] = itMap.second.strValue1;
        object["Value2"] = itMap.second.strValue2;
        object["Value3"] = itMap.second.strValue3;
        object["Value4"] = itMap.second.strValue4;
        object["HospitalCode"] = itMap.second.strHospitalCode;
        object["HospitalId"] = itMap.second.strHospitalId;
        object["Allocate"] = itMap.second.strAllocate;
        object["Remarks"] = itMap.second.strLoadRemarks;
        object["Other"] = itMap.second.strOther;
        object["Tip"] = itMap.second.strTip;


        object["SystemFlag"] = itMap.second.strSystemFlag;
        object["HospitalName"] = itMap.second.strHospitalName;
        object["HospitalProvince"] = itMap.second.strHospitalProvince;
        object["HospitalProvinceCode"] = itMap.second.strHospitalProvinceCode;
        object["HospitalCity"] = itMap.second.strHospitalCity;


        object["HospitalCityCode"] = itMap.second.strHospitalCityCode;
        object["HospitalDistrict"] = itMap.second.strHospitalDistrict;
        object["HospitalDistrictCode"] = itMap.second.strHospitalDistrictCode;
        object["HospitalTown"] = itMap.second.strHospitalTown;
        object["HospitalTownCode"] = itMap.second.strHospitalTownCode;
        object["Type"] = itMap.second.nType;

        object["LevelType"] = itMap.second.nLevelType;
        object["HospitalNature"] = itMap.second.nHospitalNature;
        object["AscriptionType"] = itMap.second.nAscriptionType;
        object["TypeSummary"] = itMap.second.strTypeSummary;
        object["HospitalNatureSummary"] = itMap.second.strHospitalNatureSummary;
        object["LevelTypeSummary"] = itMap.second.strLevelTypeSummary;


        object["AscriptionTypeSummary"] = itMap.second.strAscriptionTypeSummary;

        object["DistrictArea"] = itMap.second.strDistrictArea;
        object["AttributeSort"] = itMap.second.strAttributeSort;
        object["SerialNumber"] = itMap.second.strSerialNumber;
        object["Label"] = itMap.second.strLabel;
        object["Prefix"] = itMap.second.strPrefix;
        object["Suffix"] = itMap.second.strSuffix;


        object["DefaultValue"] = itMap.second.strDefaultValue;
        object["TextSort"] = itMap.second.strTextSort;
        object["DataTypeSummary"] = itMap.second.strDataTypeSummary;
        //公式
        object["Formula"] = itMap.second.strFormula;
        object["FormulaSummary"] = itMap.second.strFormulaSummary;


        if(itMap.second.bHasElementAttribute)
        {
            object["ElementAttribute"] = QString("%1").arg(itMap.second.nLoadElementAttribute);
        }
        object["Nodes"] = GetStructNodeArray(itMap.second.mapChild);
        array.push_back(object);

    }
    return array;
}


QString StructuralData::Trans2WholeJson(const stStructuralData &data)
{
    QJsonDocument doc;
    QJsonObject dataObject;
    dataObject["StatusCode"] = data.nStatusCode;
    dataObject["Success"] = data.bSuccess;
    dataObject["Msg"] = data.strMsg;
    dataObject["Timestamp"] = data.nTimestamp;
    QJsonArray array = GetStructNodeArray(data.mapContext);
    dataObject["Context"] = array;
    doc.setObject(dataObject);
    return QString(doc.toJson());
}

void ChangeMapNodesByState(MapNodes &node,  stTableState &state)
{

    if(node.empty())
    {
        return;
    }
    for(auto &itFind : node)
    {
        if(itFind.second.strID == state.strId)
        {

          //  state.bIsCheck;
            itFind.second.strDefaultValue = state.strState;

            if(state.bIsEdit)
            {
                itFind.second.strValue2 = "";
            }
            else if((state.strState == "true"))
            {
                itFind.second.strValue2 = "1";
            }
            else
            {
                itFind.second.strValue2 = "";
            }
        }
        else
        {
            ChangeMapNodesByState(itFind.second.mapChild, state);
        }
    }
}

void StructuralData::ChangeState( std::vector<stTableState> &vecState)
{
    for(auto &itVecState : vecState)
    {
        ChangeMapNodesByState(m_StructuralData.mapContext, itVecState);
    }

}
void TransMapContext2State(const MapContext &node, std::vector<stTableState> &vec)
{
    if(node.empty())
    {
        return;
    }
    for(auto &itMap : node)
    {
        vec.push_back(stTableState(itMap.second.strDefaultValue,itMap.second.strID, false, itMap.second.eElementAttribute == eStructuralDataType_Edit));

    }
    for(auto &itMap : node)
    {
        TransMapContext2State(itMap.second.mapChild, vec);
    }
}
std::vector<stTableState> StructuralData::TransWholeJson2StatetoMap(const QString &strJson)
{
    QJsonParseError error;
    std::vector<stTableState> vec;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            stStructuralData data;
            auto result = jsonDocument.toVariant().toMap();
            if(!result.empty())
            {
                TrasnStructuralData(result, &data);
                TransMapContext2State(data.mapContext, vec);
            }
        }
    }
    return vec;
}

std::vector<stTableState> StructuralData::TransWholeData2StatetoMap()
{
    std::vector<stTableState> vec;
     TransMapContext2State(m_StructuralData.mapContext, vec);
     return vec;
}


//QJsonObject StructuralData::Trans2WholeJsonObject(const stStructuralData &data)
//{

//    QJsonObject dataObject;
//    dataObject["StatusCode"] = data.nStatusCode;
//    dataObject["Success"] = data.bSuccess;
//    dataObject["Msg"] = data.strMsg;
//    dataObject["Timestamp"] = data.nTimestamp;
//    QJsonArray array = GetStructNodeArray(data.mapContext);
//    dataObject["Context"] = array;
//    return dataObject;
//}

//bool StructuralData::GetSaveReportPar(stSaveReportPar &saveReportPar)
//{
//    return true;
//}


void StructuralData::AddAllChild(stStructuralData &data, const QString &strId)
{
    auto vecFind = FindChildById(strId, false);
    for(auto &itFind : vecFind)
    {
        data.mapContext[itFind.strID] = itFind;
        AddAllChild(data, itFind.strID);
    }
}

std::vector<stTableState> StructuralData::TransStringStatetoMap(const  QString &strJson)
{
    QJsonParseError error;
    std::vector<stTableState> vecState;
    //  std::map<QString, stTableState> mapState;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);
    if (error.error == QJsonParseError::NoError)
    {
        if (jsonDocument.isObject())
        {
            auto result = jsonDocument.toVariant().toMap();
            if(!result.empty())
            {
                FindMapData(result, "data",&vecState, eMapDaTaType_StateData);
            }
        }
    }
    return vecState;
}

//void StructuralData::DealExternValue(stStructuralData *pData, bool bIsChild)
//{
//    //    if(pNode->strDefaultSelect.isEmpty() &&  !pNode->strValue2.isEmpty())
//    //    {
//    //        pNode->strDefaultSelect = pNode->strValue2 == "1" ? "true" : "false";
//    //    }
//    //    pNode->bStatus = pNode->strValue1 == "1";
//    auto itBeginContext = pData->mapContext.begin();
//    while(itBeginContext != pData->mapContext.end())
//    {
//        auto &node = itBeginContext->second;
//        if(!bIsChild)
//        {
//            node.strDefaultValue = node.strValue2 == "1" ? "true" : "false";
//        }
//        else
//        {
//            node.bChangeDir = node.strValue2 == "1";
//        }
//        node.bStatus = node.strValue1 == "1";
//        node.strDepartment = node.strValue3;
//        node.strBodyPart = node.strValue4;
//        ++itBeginContext;
//    }
//}

