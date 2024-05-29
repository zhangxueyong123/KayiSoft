#ifndef STRUCTURALDATA_H
#define STRUCTURALDATA_H

#include <QtCore>
#include "structdef.h"
#include "qjsondocument.h"
#include "CNetWork.h"
#include "contrl_center.h"
#include <QtConcurrent>
#include <QApplication>
//Node
struct stStructuralNode;

//控件类型
enum eStructuralDataType
{
    eStructuralDataType_UnNow = 0,
    eStructuralDataType_Single = 1,  //单选
    eStructuralDataType_Multi = 2, //多选
    eStructuralDataType_Edit = 4, //编辑框
    eStructuralDataType_Combobox = 8, //下拉框
    eStructuralDataType_Illustrate = 16, //说明框
};
//控件选中状态
struct stTableState
{
    QString strState;  //选中文本
    bool bViewer = false; //是否可见
    QString strId;  //id
    bool bIsCheck = false; //是否是多选框
    bool bIsEdit = false; //是否是编辑框
    stTableState(const QString &state,const QString &id, bool bV, bool bEdit)
        :strState(state)
        ,strId(id)
        ,bViewer(bV)
        ,bIsEdit(bEdit)
    {

    }
    bool operator == (const QString &id) const
    {
        return strId == id;
    }
};



const QString SRRaType = "SRRaTemplate";
const QString SRUSType = "SRUSTemplate";
const QString SRESType = "SRESTemplate";
const QString AllType = "SRTemplateConfig";
static bool CheckTopType(const QString &strType)
{
    if(strType == SRRaType || strType == SRUSType || strType == SRESType)
    {
        return true;
    }
    return false;
}
static QString TransType2Str( eTemplateType type)
{
    QString str;
    switch(type)
    {
    case eTemplateType_SRRa:str = SRRaType;break;
    case eTemplateType_SRUS:str = SRUSType;break;
    case eTemplateType_SRES:str = SRESType;break;
    case eTemplateType_All:str = AllType;break;
defulat: break;
    }
    return str;
}
static eTemplateType TransStr2Type( const QString& strType)
{
    if(strType == SRRaType)
    {
        return eTemplateType_SRRa;
    }
    if(strType == SRUSType)
    {
        return eTemplateType_SRUS;
    }
    if(strType == SRESType)
    {
        return eTemplateType_SRES;
    }
    if(strType == AllType)
    {
        return eTemplateType_All;
    }

    return eTemplateType_Null;
}
enum eVersionType
{
    eVersionType_V1 = 0,
};

//节点信息，接口获取
typedef struct stStructuralNode
{
    std::map<QString,stStructuralNode> *pMapNodes = nullptr;
    std::map<QString,stStructuralNode> mapChild; //子节点
    QString strNodeId;
    int nChildrenNum = 0;
    int nCountChildrenNum = 0;
    QString strDataType;
    QString strStrictSort;
    QString strID;
    QString strParentId;
    QString strCode;
    QString strNumber;
    int nDepth = 0;
    QString strTitle;
    QString strUnionTitle;
    int nSort = 1;
    int nChildSort = 1;
    QString strValue1;
    QString strValue2;
    QString strValue3;
    QString strValue4;
    QString strHospitalCode;
    QString strHospitalId;
    QString strAllocate;
    QString strRemarks;
    QString strLoadRemarks;
    QString strOther;
    QString strSystemFlag;
    QString strHospitalName;
    QString strHospitalProvince;
    QString strHospitalProvinceCode;
    QString strHospitalCity;
    QString strHospitalCityCode;
    QString strHospitalDistrict;
    QString strHospitalDistrictCode;
    QString strHospitalTown;
    QString strHospitalTownCode;
    int nType = 0;
    int nLevelType = 0;
    int nHospitalNature = 0;
    int nAscriptionType = 0;
    QString strTypeSummary;
    QString strHospitalNatureSummary;
    QString strLevelTypeSummary;
    QString strAscriptionTypeSummary;
    QString strDistrictArea;
    int nLoadElementAttribute = 0;
    eStructuralDataType eElementAttribute = eStructuralDataType_Multi;
    bool bIsExplain = false;
    bool bHasElementAttribute = false;
    QString strAttributeSort;
    QString strSerialNumber;
    QString strLabel;
    QString strPrefix;
    QString strSuffix;
    QString strDefaultValue;
    // bool bChangeDir = false;
    bool bStatus = false;
    QString strDepartment;
    QString strBodyPart;
    QString strTextSort;
    QString strDataTypeSummary;

    bool bIsInit = false;

    bool bIsComboboxTop = false;

   // QString strOther; //提示表
    QString strTip; //提示字段
    QString strFormula;
    QString strFormulaSummary;

}stStructuralContext;

using MapNodes = std::map<QString,stStructuralNode>;
using MapContext = MapNodes;
//节点数据集
struct stStructuralData
{
    int nStatusCode = -1;
    MapContext mapContext;
    bool bSuccess = false;
    QString strMsg;
    QString strErrors;
    QString strExtras;
    long long nTimestamp = 0;
//    void MergeContext(const stStructuralContext &context)
//    {

//    }
//    void MergeContext(const  MapContext &context)
//    {

//    }
    void SetTop(const QString &strTop)
    {
       auto itFind = this->mapContext.find(strTop);
       if(itFind !=  this->mapContext.end())
       {
            itFind->second.bIsComboboxTop = true;
       }
    }
    void DeleteChild(const QString &strId)
    {
        auto itFind = this->mapContext.find(strId);
        if(itFind !=  this->mapContext.end())
        {
             this->mapContext.erase(itFind);
        }
        std::vector<QString> vecChild;
        for(auto &itNode :  this->mapContext)
        {
             if(itNode.second.strParentId == strId)
             {
                vecChild.push_back(itNode.second.strID);
             }
        }
        for(auto &itVecId : vecChild)
        {
            DeleteChild(itVecId);
        }
        //for(auto &itNode : )
    }
    //合并节点数据集
    void MergeStructuralData(const stStructuralData &data, const QString &strTop = "")
    {
        if(!data.bSuccess)
        {
            return;
        }
       // std::vector<QString> vecChild;
        if(!strTop.isEmpty())
        {
             DeleteChild(strTop);
        }
        for(auto &itVec : data.mapContext)
        {
            auto itFind = this->mapContext.find(itVec.first);
            if(itFind == this->mapContext.end())
            {
                mapContext[itVec.first] = itVec.second;
            }
            else
            {
                itFind->second = itVec.second;
            }
        }
    }
    //合并节点数据集
    void MergeStructuralData1(const stStructuralData& data, const QString& strTop = "")
    {
        if (!data.bSuccess)
        {
            return;
        }
        // std::vector<QString> vecChild;
        if (!strTop.isEmpty())
        {
            DeleteChild(strTop);
        }
        for (auto& itVec : data.mapContext)
        {
            if (itVec.first != strTop)
                continue;
            auto itFind = this->mapContext.find(itVec.first);
            if (itFind == this->mapContext.end())
            {
                mapContext[itVec.first] = itVec.second;
            }
            else
            {
                itFind->second = itVec.second;
            }
        }
    }
    bool IsEmpty()
    {
        return mapContext.empty();
    }
    void Clear()
    {
        mapContext.clear();
        nStatusCode = -1;
        bSuccess = false;
        strMsg.clear();
        strErrors.clear();
        strExtras.clear();
        nTimestamp = 0;
    }

    void InitOK()
    {
        nStatusCode = 200;
        bSuccess = true;
        strMsg.clear();
        strErrors.clear();
        strExtras.clear();
    }

};

using MapTable = std::map<stTableKey,std::map<stTableKey, stStructuralNode>>;
struct stTopPar
{
    QString strId;
    int nDepth = 1;
    stTopPar()
    {
    }
    stTopPar(const QString &id, int depth)
        :strId(id)
        ,nDepth(depth)
    {
    }

};

struct stTemplateData
{
    QString strId;
    QString strValue;
};

struct stSaveReportPar
{
    QString strAccessno; //访问号
    QString strHospitalCode; //医院编码
    QString strFirstId; //FirstId
    QString strSecondId; //SecondId
    QString strTreeJson; //TreeJson
    QString strListJson; //列表json
    QStringList listDictionaryId ;
    std::vector<stTemplateData> vecTemplateData;
    int nOperateType = 0;
};
//节点数据管理类
class StructuralData
{
public:
    StructuralData();

    StructuralData( const stStructuralData &data);
    //将读到的完整数据进行分类
    StructuralData CreateNowStructuralData(const QString & strDepartment, const QString & strBodyPart);
    void Copy2Ori();
    void ResetData();
    //添加表格数据
    void AddChildByJson(const QString &strJson,const QString &strId);
    //添加树型结构数据
    std::vector<QString> AddStructuralDataByJson(const QString &strJson, std::vector<QString> *pVecCode = nullptr, const QString &strFirst = "", const QString &strSecond = "");
    std::vector<QString> AddStructuralDataByJson1(const QString &strJson, std::vector<QString> *pVecCode = nullptr, const QString &strFirst = "", const QString &strSecond = "");
    QString screening( QString strJson,  QString strFirst,  QString strSecond, bool isFirst);
    QString screeningForNKJ( QString strJson,  QString strFirst,  QString strSecond);
    std::map<eTemplateType, stTopPar> GetTopPar();
    //获取数据(完整数据)
    stStructuralData *GetStructuralDataPtr();
    stStructuralData  GetStructuralData();
    //根据title获取节点
    stStructuralNode *FindNodePtrByTitle(const QString &strTitle);
    stStructuralNode FindNodeByTitle(const QString &strTitle, bool *pHas);

    //根据id获取节点
    stStructuralNode *FindNodePtrById(const QString &strId);
    stStructuralNode FindNodeById(const QString &strId, bool *pHas);


    bool SetInitByCode(const QString &strCode, bool bInit);
    void SetAllInit(bool bInit);
    //根据Code的获取节点
    stStructuralNode *FindNodePtrByCode(const QString &strCode);
    stStructuralNode FindNodeByCode(const QString &strCode, bool *pHas);
    //根据id获取子节点
    std::vector<stStructuralNode > FindChildById(const QString &strId, bool bSort);

    //获取子表格数据
    MapTable FindChildTable(const QString &strId);
    //获取子节点id列表
    std::vector<stStructuralNode > FindChildByCode(const QString &strCode, bool bSort);
    std::vector<QString > FindChildIdById(const QString &strId);
    std::vector<QString > FindChildIdByCode(const QString &strCode);
    //获取状态id列表
    std::vector<QString> FindAllStateId();
    //获取子节点id列表
    void FindAllChildIdById(const QString &strId, std::vector<QString> *pVecId, bool bSelf );
    //清空
    void Clear(){m_StructuralData.Clear();}
    void ClearById(const QString &strId);
    void ClearByCode(const QString &strCode);
    //转换状态为json
    static QString TransMapStatetoString(const  std::vector<stTableState> &vecState);
    //转换json到状态
    static  std::vector<stTableState> TransStringStatetoMap(const  QString &strJson);
    //转换列表数据为json
    static QString TransReporttoString(const  listReportData &list, const QString &strRemarks);
    //转换json到列表数据
    static listReportData TransStringtoReport(const  QString &strJson, const QString &strRemarks, bool bAll = false);
    QString MergeStructuralDataReportJson(const std::vector<QString > &vecJson);
    //  bool CheckIsStruct(const std::vector<listReportData> &vecList);
    void SetStructuralData(const stStructuralData &data){m_StructuralData = data;}
    bool IsEmpty();
    void GetRemarksList(std::vector<QString> *pVec, stStructuralNode *pNode = nullptr);

    QString Trans2WholeJson(const stStructuralData &data);
    void ChangeState( std::vector<stTableState> &vecState);
    std::vector<stTableState> TransWholeJson2StatetoMap(const  QString &strJson);
    std::vector<stTableState> TransWholeData2StatetoMap();
    QString m_strUrl;
    CNetworkAccessManager* m_network = nullptr;
public:
    void AddAllChild( stStructuralData &data, const QString &strId);
    //void DealExternValue(stStructuralData  * pData, bool bIsChild);
    stStructuralData m_StructuralData;
    stStructuralData m_StructuralDataOri;
    // MapStructuralData m_mapStructuralData;

};

#endif // STRUCTURALDATA_H
