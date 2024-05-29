#include "structuralwidget.h"
#include "ui_structuralwidget.h"
#include "structuraldata.h"
#include "qpainter.h"
#include <iostream>
#include <fstream>
#include "ctemplatemanage.h"

QString stTemplateWnd::m_currentPre = "";
QString beforeJson = "";
std::atomic_int g_NowTimes(0);
std::atomic_int g_EndTimes(0);
QScopedPointer<QEventLoop> g_eventLoop;
StructuralWidget::StructuralWidget(const QString &strUrl, CNetworkAccessManager *network, QWidget *parent,  eTemplateType eNowTemplateType, eVersionType eApiVersion ) :
    m_strUrl(strUrl),
    m_network(network),
    QWidget(parent),
    ui(new Ui::StructuralWidget),
    m_eTemplateType(eNowTemplateType),
    m_ApiVersion(eApiVersion)
{
    SetMeasureParamePar(strUrl, network);
    ui->setupUi(this);
    if (g_eventLoop.isNull()){
        g_eventLoop.reset(new QEventLoop);
    }

    auto funMeasureCall = [](QStringList *pList, StructuralWidget *pThis)
    {
         emit pThis->measureParameterCheck(*pList);
    };
    auto funExplainCall = [](QStringList *pList, StructuralWidget *pThis)
    {
        emit pThis->explainParameterCheck(*pList);
    };
    //接到模板数据，刷新界面
    auto funtemplateCall = [](QString * strFirst, QString* strSecond, QString* strTreeJson, StructuralWidget* pThis)
    { 
        if (*strTreeJson == "change")//一般切换
        {
            pThis->signReLoad(*strFirst, *strSecond,"");
            return;
        }
        if (*strTreeJson != "")//快照
        {
            pThis->signReLoad(*strFirst, *strSecond, *strTreeJson);
        }

        
    };
    //m_StructuralWnd.InitCallBaCk(std::bind(funMeasureCall, std::placeholders::_1, this),
    //                             std::bind(funExplainCall, std::placeholders::_1, this),
    //                             std::bind(funtemplateCall, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, this));
    m_StructuralWnd.InitCallBaCk(std::bind(funMeasureCall, std::placeholders::_1, this),
                                 std::bind(funExplainCall, std::placeholders::_1, this),
                                 std::bind(funtemplateCall, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, this));

    CTemplateManage::GetSingle(strUrl, m_network);

    connect(CTemplateManage::GetSingle(), &CTemplateManage::signSettingInfo, this, &StructuralWidget::slotSettingInfo);

    connect(ControlCenter::getInstance(), &ControlCenter::signSendTemplateData, this, &StructuralWidget::sendTemplateData);
    connect(ControlCenter::getInstance(), &ControlCenter::signDeleteTemplateData, this, &StructuralWidget::deleteTemplateData);   

    this->setStyleSheet(QString("MyGroup{border:1px solid rgb(183,198,221);border-radius:4px;background:rgb(220,231,244);}"
                                "QLabel{ color:rgba(0,0,0,0.7); }"
                                "QComboBox{ background-color: transparent;color:  rgb(0, 0, 0); border:1px solid #B7C6DD;}"
                                "QComboBox QAbstractItemView{ background-color: rgb(250,240,225);color:  rgb(0, 0, 0);}"
                                "MyPushButton{border:1px solid #B7C6DD; padding:1px;}"
                                "MyPushButton:hover{ background-color:rgb(255, 255, 255);}"));
}


void StructuralWidget::InitStructuralData(/*const QString &strDepartment, const QString &strBodyPart*/)
{
    //初始化数据 添加屏蔽参数
    if(!m_bIsInitStruct)
    {
        //加载接口树型结构
        LoadTreeData();
        m_bIsInitStruct = true;
    }
    UpdateUi(true);
    m_StructuralWnd.SetViewer(m_eViewerType);

}

void StructuralWidget::SetDepartment(const QString &strDepartment, const QString &strBodyPart/*, bool bUpdate*/)
{
    m_StructuralWnd.SetDepartment(m_eViewerType, strDepartment, strBodyPart);
    m_DepartmentAndBodyPart.strDepartment = strDepartment;
    m_DepartmentAndBodyPart.strBodyPart = strBodyPart;
  //  m_ShowStructuralData = m_StructuralData.CreateNowStructuralData(m_strDepartment, m_strBodyPart);
//    if(bUpdate)
//    {
//        m_StructuralWnd.Clear();
//        InitUiCtl();
//    }
}

void StructuralWidget::SetPatientPar(const QString &systemCode, const QString &hospitalCode, const QString &sex, const QString &strAge)
{
    auto itData = MyMeasureParameterManage::GetSingle();
    if(itData != nullptr)
    {
        itData->SetCheckPar(systemCode, hospitalCode, sex, strAge);
    }
}

bool StructuralWidget::SetViewerType(eTemplateType eViewerType, bool bReLoad, const QString &strFirstId, const QString &strSecondId, const QString &strJson, bool bSingle)
{
    ControlCenter::getInstance()->m_type = eViewerType;
    m_selectFirstId = strFirstId;
    m_selectSecondId = strSecondId;
    m_eViewerType = eViewerType;
    bool bReload =  ChangeViewer(bReLoad);
    if(m_bIsReset && !strJson.isEmpty())
    {
        ReloadTreeDataByJson(strFirstId, strSecondId, strJson, bSingle);
        m_bIsReset = false;
        bReload = true;
    }

    if(bReload)
    {
        InitUiCtl(true, false);
    }
    m_StructuralWnd.setFinish(true);
    //qDebug() << "defaultFirst = " << ControlCenter::getInstance()->m_defaultFirstId;
    //qDebug() << "defaultSecond = " << ControlCenter::getInstance()->m_defaultSecondId;

    //QString fileName = "./StructuralWidget/" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()) + ".png";
    //QPixmap pixmap = this->grab();
    //pixmap.save(fileName);
    
    return true;
}

StructuralWidget::~StructuralWidget()
{
    delete ui;
}

void StructuralWidget::ClearCallBack()
{
    m_StructuralWnd.ClearCallBack();
}

void StructuralWidget::ResetTreeData()
{
    m_bIsReset = true;
    m_StructuralData.ResetData();
    //m_ShowStructuralData = m_StructuralData.CreateNowStructuralData(m_strDepartment, m_strBodyPart);
    std::vector<stTableState> vecState =  m_StructuralData.TransWholeData2StatetoMap();//*m_ShowStructuralData.IsEmpty() ? : m_ShowStructuralData.TransWholeData2StatetoMap();
    SetReportStateByValue(vecState);
}

bool StructuralWidget::ChangeViewer(bool bReLoad)
{
    QString str = TransType2Str(m_eViewerType);

    auto itFind = m_StructuralData.FindNodePtrByCode(str);
    if (itFind == nullptr || !itFind->bIsInit)
    {
        bReLoad = true;
    }
    if (bReLoad)
    {
        m_StructuralData.ClearByCode(str);
        m_eTemplateType = m_eViewerType;
        LoadTreeData();
    }
    m_StructuralWnd.SetViewer(m_eViewerType);

    return bReLoad;
}

void StructuralWidget::InitUiCtl(bool sw ,bool bDeleteDelay)
{
    CheckCreate(sw,bDeleteDelay);
    MoveBtn(sw);
}

void StructuralWidget::ReloadTreeDataBySelf()
{
    m_StructuralWnd.CrateByStructuralData(this, /*m_ShowStructuralData.IsEmpty() ? */m_StructuralData/* : m_ShowStructuralData*/,true, false);
    m_StructuralWnd.SetViewer(m_eViewerType);
    m_StructuralWnd.SetDepartment(m_eViewerType, m_DepartmentAndBodyPart.strDepartment,m_DepartmentAndBodyPart.strBodyPart);
    MoveBtn(true);
}

void StructuralWidget::ReloadTreeDataByNet(eTemplateType viewerType)
{
    if (viewerType == eTemplateType_Null)
    {
        return;
    }
    m_eTemplateType = viewerType;

    ClearData();
    LoadTreeData();
    m_StructuralWnd.CrateByStructuralData(this, /*m_ShowStructuralData.IsEmpty() ?*/ m_StructuralData /*: m_ShowStructuralData*/,true, false);
    //m_StructuralDataOri = m_StructuralData;
    m_StructuralWnd.SetViewer(m_eViewerType);
     m_StructuralWnd.SetDepartment(m_eViewerType, m_DepartmentAndBodyPart.strDepartment, m_DepartmentAndBodyPart.strBodyPart);
    MoveBtn(true);
    m_bIsReset = true;
}

void StructuralWidget::ReloadTreeDataByNet()
{
    ReloadTreeDataByNet(m_eViewerType);
}

void StructuralWidget::ReloadTreeDataByJson(const QString& strFirstId, const QString& strSecondId, const QString& strJson, bool bSingle)
{
    std::vector<QString> vecCode;
    auto vecId = m_StructuralData.AddStructuralDataByJson(strJson, &vecCode, strFirstId, strSecondId);
    if (!bSingle)
    {
        LoadSubList(vecId);
    }
    if (!vecCode.empty())
    {
        for (auto& itVec : vecCode)
        {
            m_StructuralData.SetInitByCode(itVec, true);
        }
    }
    //m_ShowStructuralData = m_StructuralData.CreateNowStructuralData(m_strDepartment, m_strBodyPart);
//    m_StructuralWnd.CrateByStructuralData(this, m_ShowStructuralData.IsEmpty() ? m_StructuralData : m_ShowStructuralData,true);
//    m_StructuralWnd.SetViewer(m_eViewerType);
//    MoveBtn(true);
}

void StructuralWidget::UpdateTreeDataStateByJson(const QString& strJson)
{
    std::vector<stTableState> vecState = m_StructuralData.TransWholeJson2StatetoMap(strJson);
    SetReportStateByValue(vecState);
}

void StructuralWidget::SaveTemplate(const QString &strTemplateName)
{
    stSaveReportPar saveReportPar;
    GetSaveReportPar(saveReportPar,m_eViewerType, false);
    CTemplateManage::GetSingle()->AddTemplateData(saveReportPar.strFirstId, saveReportPar.strSecondId,strTemplateName, saveReportPar.strTreeJson);
//    QJsonObject dataObject;
//    dataObject["FirstId"] = saveReportPar.strFirstId;
//    dataObject["SecondId"] = saveReportPar.strSecondId;
//    dataObject["TemplateName"] = strTemplateName;
//    dataObject["TreeJson"] = saveReportPar.strTreeJson;
  //  m_StructuralWnd.SaveTemplate(m_eTemplateType, strTemplateName);
}

void StructuralWidget::DeleteTemplate(const QString &strTemplateName)
{
    stSaveReportPar saveReportPar;
    GetSaveReportPar(saveReportPar,m_eViewerType, false);
    CTemplateManage::GetSingle()->DeleteTemplateData(saveReportPar.strFirstId, saveReportPar.strSecondId, strTemplateName);
    //m_StructuralWnd.DeleteTemplate(m_eTemplateType, strTemplateName);
}

void StructuralWidget::DeleteTemplate(CheckCallBack checkCallBack)
{
   // m_StructuralWnd.DeleteTemplate(checkCallBack);
}

void StructuralWidget::SetMeasureParamePar(const QString &strUrl, CNetworkAccessManager *network)
{
    MyMeasureParameterManage::GetSingle(strUrl, network, nullptr);
}

void StructuralWidget::GetReportKeyValueState(std::vector<stTableState>* pVecState)
{
    m_StructuralWnd.GetReportKeyValueState(m_eViewerType, pVecState);
}

QString StructuralWidget::GetReportKeyValueJson()
{
    return m_StructuralWnd.GetReportKeyValueJson(m_eViewerType);
}

void StructuralWidget::GetSaveReportPar(stSaveReportPar &saveReportPar, eTemplateType type, bool bCheckStruct)
{
    eTemplateType nowType = (m_eViewerType == eTemplateType_Null || m_eViewerType == eTemplateType_All) ?type :  m_eViewerType;
    nowType = (nowType == eTemplateType_Null || nowType == eTemplateType_All) ? eTemplateType_SRRa :  nowType;
    m_StructuralWnd.GetSaveReportPar(nowType , saveReportPar,m_StructuralData,bCheckStruct);
}

QString StructuralWidget::GetSaveReportJson(eTemplateType type)
{
    QJsonDocument doc;
    QJsonObject dataObject = GetSaveReportPar(type);
    doc.setObject(dataObject);
    QByteArray arr = doc.toJson();
    QString str = doc.toJson();
    return doc.toJson();
}
              
QJsonObject StructuralWidget::GetSaveReportPar(eTemplateType type)
{
    stSaveReportPar saveReportPar;
    GetSaveReportPar(saveReportPar, type);
    QJsonObject dataObject;
    dataObject["FirstId"] = saveReportPar.strFirstId;
    dataObject["SecondId"] = saveReportPar.strSecondId;
    dataObject["TreeJson"] = saveReportPar.strTreeJson;
    dataObject["ReportJson"] = saveReportPar.strListJson;
    QJsonArray arrayDictory;
    for (auto& itList : saveReportPar.listDictionaryId)
    {
        arrayDictory.push_back(itList);
    }
    dataObject["DictionaryId"] = arrayDictory;
    QJsonArray  arrayData;
    for (auto& itVec : saveReportPar.vecTemplateData)
    {
        QJsonObject dataObj;
        dataObj["ID"] = itVec.strId;
        dataObj["Value"] = itVec.strValue;
        arrayData.push_back(dataObj);
    }
    dataObject["TemplateData"] = arrayData;
    return dataObject;
}

void StructuralWidget::SetReportStateByJson(const QString& strJson)
{
    m_StructuralWnd.SetReportStateByJson(m_eViewerType, strJson);
}

void StructuralWidget::SetReportStateByValue(const std::vector<stTableState>& vecState)
{
    m_StructuralWnd.SetReportStateByMap(m_eViewerType, vecState);
}

bool StructuralWidget::IsInit(eTemplateType viewerType)
{
    return m_StructuralWnd.IsInit(viewerType);
}

void StructuralWidget::Clear(bool bWithData)
{
    m_StructuralWnd.Clear();
    m_DepartmentAndBodyPart.strDepartment.clear();
    m_DepartmentAndBodyPart.strBodyPart.clear();
    //m_vecDepartmentAndBodyPart.clear();
    if(bWithData)
    {
        ClearData();
        if( MyMeasureParameterManage::GetSingle() != nullptr)
        {
            MyMeasureParameterManage::GetSingle()->ClearAll();
        }
    }
}

std::vector<QString> StructuralWidget::GetDataTypeList()
{
    std::vector<QString> dataTypeList;
    m_StructuralData.GetRemarksList(&dataTypeList);
    return dataTypeList;
}

void StructuralWidget::ClearData()
{
    m_StructuralData.Clear();
//    QString m_strDepartment;
//    QString m_strBodyPart;
    //m_ShowStructuralData.Clear();
}
std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> end;
//调用了2次，第一次为了 获取超声、内窥镜、放射三者的combobox中的下拉框合集，根据选择设置最左上角的一个combobox， （新文件6）
//         第二次为了 获取 如放射下，所有的第二个combobox中的下拉框合集，再根据第一个下拉框的选择设置第二个下拉框（新文件7）,还有下拉框下的多选框区域
void StructuralWidget::LoadTreeData()
{
    //创建结构参数
    QString strUrl = m_strUrl;
    if(m_ApiVersion == eVersionType_V1)
    {
        //strUrl += "/api/v1/sr_classify/tree/code?code=";
        strUrl += "/api/v1/sr_classify/tree/code?code=";
    }

    m_StructuralData.m_network = m_network;
    strUrl +=   TransType2Str(m_eTemplateType);
    QString strDst;
    if(m_StructuralData.IsEmpty() && m_eTemplateType != eTemplateType_All)
    {
        //如果数据是空的 先把最外围加载一次
        QString strTmp = m_strUrl + "/api/v1/sr_classify/tree/code?code=";
        strTmp += TransType2Str(eTemplateType_All);
        auto ret = CNetWork::netWorkGet(m_network, strTmp, strDst);
        if(ret != 0 || strDst.isEmpty())
        {
            return;
        }

        //根据接口json创建结构化报告节点
        m_StructuralData.AddStructuralDataByJson(strDst);
    }
    if(m_eTemplateType == eTemplateType_Null)
    {
        return;
    }
    // auto s =  clock();
    auto ret = CNetWork::netWorkGet(m_network, strUrl, strDst);
    if (ret != 0 || strDst.isEmpty())
    {
        return;
    }
    auto mapTopId = m_StructuralData.GetTopPar();
    auto vecId = m_StructuralData.AddStructuralDataByJson(strDst);

    //根据接口json创建结构化报告节点（获取列表之上，两个combobox之下的那个多选框区域）（新文件8）
    LoadTreeList(mapTopId, m_eTemplateType);
    if (g_EndTimes != 0)
    {
        //1
        g_eventLoop->exec();
        g_EndTimes = 0;
        g_NowTimes = 0;
    }
    if(m_ApiVersion == eVersionType_V1)
    {
         m_StructuralData.m_strUrl = m_strUrl + "/api/v1/sr_dictionary/tree/dictionaryid?dictionaryid=";
    }
//    vecId = m_StructuralData.FindAllStateId();

//    //根据节点数据获取所有结构化列表数据
//    start = std::chrono::high_resolution_clock::now();
//    if(!vecId.empty())
//    {
//        LoadDictionaryData(vecId);
//        if(g_EndTimes != 0)
//        {
//            g_eventLoop->exec();
//            g_EndTimes = 0;
//            g_NowTimes = 0;
//        }
//    }

    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(  start - end);
    std::cout << duration.count() << std::endl;
    if(m_eTemplateType == eTemplateType_All)
    {
        m_StructuralData.SetAllInit(true);
    }
    else if (m_eTemplateType == eTemplateType_Null)
    {

    }
    else
    {
        m_StructuralData.SetInitByCode(TransType2Str(m_eTemplateType), true);
    }
    m_StructuralData.Copy2Ori();
    //m_ShowStructuralData = m_StructuralData.CreateNowStructuralData(m_strDepartment, m_strBodyPart);
}

void StructuralWidget::LoadSubList(const std::vector<QString>& vecId)
{
    QString strUrl = m_strUrl;
    if (m_ApiVersion == eVersionType_V1)
    {
        strUrl += "/api/v1/sr_classify/sublist?id=";
    }

    int nEndTimes = (int)vecId.size();
    g_EndTimes += nEndTimes;
    for(auto &itVec : vecId)
    {
        QString strNow = strUrl + itVec;

        QNetworkRequest request = QNetworkRequest(strNow);
        QNetworkReply* reply = m_network->get(request);
        connect(reply, &QNetworkReply::finished, [=]
            {

                if (reply->error() == QNetworkReply::NoError)
                {
                    auto vecNewId = m_StructuralData.AddStructuralDataByJson(reply->readAll());
                    LoadSubList(vecNewId);
                    // ++nNowTimes;
                    ++g_NowTimes;
                    if (g_NowTimes == g_EndTimes)
                    {
                        g_eventLoop->quit();
                    }
                }
            });
    }

}
//获取列表之上，两个combobox之下的那个多选框区域
void StructuralWidget::LoadTreeList(const std::map<eTemplateType, stTopPar>& mapTop, eTemplateType type)
{
    std::map<eTemplateType, stTopPar> mapParNow;
    if (type == eTemplateType_All)
    {
        mapParNow = mapTop;
    }
    else
    {
        auto itFind = mapTop.find(type);
        if (itFind == mapTop.end())
        {
            return;
        }
        mapParNow[itFind->first] = itFind->second;
    }
    int nEndTimes = (int)mapParNow.size();
    g_EndTimes += nEndTimes;
    for(auto &itMap : mapParNow)
    {

        QString strUrl = m_strUrl;
        if (m_ApiVersion == eVersionType_V1)
        {
            strUrl += "/api/v1/sr_classify/tree?id=" + itMap.second.strId;
            char chDepth[64];
            sprintf(chDepth, "&depth=%d", 10);
            strUrl += chDepth;
        }
        QNetworkRequest request = QNetworkRequest(strUrl);
        QNetworkReply* reply = m_network->get(request);
        connect(reply, &QNetworkReply::finished, [=]
            {

                if (reply->error() == QNetworkReply::NoError)
                {
                    /* auto vecNewId =*/ 
                    m_StructuralData.AddStructuralDataByJson1(reply->readAll(),nullptr,/*"4756628e-34c2-408f-b510-9f3f7785cb4f"*/m_selectFirstId,/*"2aa8215f-bbb1-422a-b591-b9c9e4897443"*/m_selectSecondId);
                    //LoadSubList(vecNewId);
                    // ++nNowTimes;
                    ++g_NowTimes;
                    if (g_NowTimes == g_EndTimes)
                    {
                        //2
                        g_eventLoop->quit();
                    }
                }
            });
    }
}

void StructuralWidget::LoadDictionaryData(const std::vector<QString>& vecId)
{
    //根据节点数据获取所有结构化列表数据
    QString strUrl = m_strUrl;
    if (m_ApiVersion == eVersionType_V1)
    {
        strUrl += "/api/v1/sr_dictionary/tree/dictionaryid?dictionaryid=";
    }
    int nEndTimes = (int)vecId.size();
    g_EndTimes += nEndTimes;
    for(auto &itVec : vecId)
    {
        QString strNow = strUrl + itVec;
        strNow += "&depth=10";
        QNetworkRequest request = QNetworkRequest(strNow);
        QNetworkReply* reply = m_network->get(request);
        connect(reply, &QNetworkReply::finished, [=]
            {
                if (reply->error() == QNetworkReply::NoError)
                {
                    QString strDst = reply->readAll();
                    m_StructuralData.AddChildByJson(strDst, itVec);
                    ++g_NowTimes;
                    if (g_NowTimes == g_EndTimes)
                    {
                        //4
                        g_eventLoop->quit();
                    }
                }
            });
        //connect(reply, &QNetworkReply::finished,this,&StructuralWidget::handleReply);
    }
}

void StructuralWidget::CheckCreate(bool bReset, bool bDeleteDelay)
{
    //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << __FUNCDNAME__;
    m_StructuralWnd.CrateByStructuralData(this,  /*m_ShowStructuralData.IsEmpty() ?*/ m_StructuralData /*: m_ShowStructuralData*/,bReset, bDeleteDelay);
    m_StructuralWnd.SetDepartment(m_eViewerType, m_DepartmentAndBodyPart.strDepartment, m_DepartmentAndBodyPart.strBodyPart);
}

void StructuralWidget::MoveBtn(bool bSend)
{
    m_StructuralWnd.Move(this->rect(), bSend);
    //m_StructuralWnd.Move(QRect(this->rect().left(),this->rect().top(),this->rect().width() - 100,this->rect().height() - 100), bSend);
    //qDebug() << this->width() << this->height();
}

void StructuralWidget::UpdateUi(bool bSend)
{
    CheckCreate(false, false);
    MoveBtn(bSend);
}

void StructuralWidget::resizeEvent(QResizeEvent* event)
{
    MoveBtn(false);
}

void StructuralWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void StructuralWidget::sendTemplateData(QString templateName,eTemplateType type)
{
    auto json = GetSaveReportJson(type);
    QString first = ControlCenter::getInstance()->m_currentFirstId;
    QString second = ControlCenter::getInstance()->m_currentSecondId;
    CTemplateManage::GetSingle()->AddTemplateData(first, second, templateName,json);
}

void StructuralWidget::deleteTemplateData(QString templateName, eTemplateType type)
{
    //通知快照combobox 改变当前index
    ControlCenter::getInstance()->initCombobox();
    QString first = ControlCenter::getInstance()->m_currentFirstId;
    QString second = ControlCenter::getInstance()->m_currentSecondId;
    CTemplateManage::GetSingle()->DeleteTemplateData(first, second, templateName);
}

void StructuralWidget::slotSettingInfo(QString title, bool flag)
{
    qDebug() << "快照设置结果:" << title << flag;
    if (label != nullptr)
    {
        label->hide();
        label->setParent(nullptr);
        label = nullptr;
    }
        
    label = new QLabel(this);
    label->setText(title);
    label->hide();
    QString successStyle = "QLabel{color: rgb(255, 0, 0);font: 9pt ""微软雅黑"";}";
    QString failStyle = "QLabel{color: rgb(0, 0, 0);font: 9pt ""微软雅黑"";}";;
    label->setStyleSheet(!flag ? successStyle : failStyle);
    //透明度
    goe = new QGraphicsOpacityEffect();
    label->setGraphicsEffect(goe);
    //    gce = new QGraphicsColorizeEffect(label);
    //    label->setGraphicsEffect(gce);
        //移动动画
    animation = new QPropertyAnimation(label, "geometry");

    connect(animation, &QPropertyAnimation::finished, this, [=]() {
        label->hide();
        timer->stop();
        });
    timer = new QTimer();
    //设置高精度定时器
    //timer->setTimerType(Qt::PreciseTimer);
    connect(timer, &QTimer::timeout, this, [=]() {
        m_opacityVal -= 0.05;
        goe->setOpacity(m_opacityVal);
        });


    m_opacityVal = 1.0f;
    int t = 2000;//动画时间
    //移动动画
    animation->setDuration(t);//设置动画时间
    animation->setEndValue(QRect((this->width() - label->width()) / 2, 0, 100, 30));//开始位置
    animation->setStartValue(QRect((this->width() - label->width()) / 2, this->height() / 4, 100, 30));//结束位置
    animation->start();//动画开始
    label->raise();
    label->show();
    timer->start(t / 10);
}
