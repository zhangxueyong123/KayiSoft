#include "explotwidget.h"
#include <qpointer.h>
#include "contrl_center.h"
#include "ctemplatemanage.h"
#include "mytextreportwidget.h"
#include "structuralwidget.h"
//int ExplotWidget::count = 0;
ExplotWidget::ExplotWidget(const QString& strUrl, QString token, QWidget* parent, eTemplateType eNowTemplateType/*, int eApiVersion*/) :
    QWidget(parent)
{
    m_strUrl = strUrl;
    m_token = token;
    //m_network = network;
    m_pStuctualWidget[0] = nullptr;
    m_pStuctualWidget[1] = nullptr;
    m_network = new CNetworkAccessManager;
    m_network->setRawHeader("Authorization", m_token.toLatin1());

    m_pStuctualWidget[m_clu] = new StructuralWidget(strUrl, m_network, this, eTemplateType_Null);
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::signReLoad, this, [=](const QString firstid, const QString secondid, QString json) {
        reloadStructuralWidget(firstid, secondid, json);
        ControlCenter::getInstance()->m_isUseDefault = false;
        });
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::measureParameterCheck, this, [=](const QStringList& measureParameterList) {
        measureParameterCheck(measureParameterList);
        });
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::explainParameterCheck, this, [=](const QStringList& explainParameterList) {
        explainParameterCheck(explainParameterList);
        });
    layout = new QHBoxLayout(this);
    layout->addWidget(m_pStuctualWidget[m_clu]);

    connect(&timer, &QTimer::timeout, this, [=]() {
        int delete_clu = m_clu == 0 ? 1 : 0;
        delete m_pStuctualWidget[delete_clu];
        m_pStuctualWidget[delete_clu] = nullptr;
        timer.stop();
        });

    connect(ControlCenter::getInstance(), &ControlCenter::signStructuralWidgetBeUsed, this, [=]() {
        m_updateDataCallBackEnable = true;
        setEditCallBack();
        });
}

ExplotWidget::~ExplotWidget()
{
    //ControlCenter::getInstance()->m_isUseDefault = false;
    ControlCenter::getInstance()->needLoadAll = true;
}

bool ExplotWidget::SetViewerType(eTemplateType eViewerType, bool bReLoad, const QString& strFirstId, const QString& strSecondId, const QString& strJson, bool bSingle)
{
    ControlCenter::getInstance()->firstLoadFinish = false;
    BothId b(strFirstId, strSecondId);
    ControlCenter::getInstance()->setOpenedReportJson(b, strJson);
    //ClearCallBack();
    if (isFirst)
    {
        if (m_pStuctualWidget[m_clu] == nullptr)
            return false;
        m_eViewerType = eViewerType;
        m_pStuctualWidget[m_clu]->SetPatientPar(m_systemCode, m_hospitalCode, m_sex, m_strAge);
        m_pStuctualWidget[m_clu]->InitStructuralData();
        m_pStuctualWidget[m_clu]->SetDepartment(ControlCenter::getInstance()->m_department, ControlCenter::getInstance()->m_bodyPart);
        auto result = m_pStuctualWidget[m_clu]->SetViewerType(eViewerType, bReLoad, strFirstId, strSecondId, strJson, bSingle);
        if (strJson != "")
        {
            m_updateDataCallBackEnable = true;
            setEditCallBack();
        }
        isFirst = false;

        return result;
    }
    else
    {
        reloadStructuralWidget(strFirstId, strSecondId, strJson);
        return 1;
    }
}

void ExplotWidget::initStructuralWidget(QString firstId, QString secondId, QString json)
{
    if (m_pStuctualWidget[m_clu] != nullptr)
    {
        m_pStuctualWidget[m_clu]->hide();
    }
    m_clu = m_clu == 0 ? 1 : 0;
    m_pStuctualWidget[m_clu] = new StructuralWidget(m_strUrl, m_network, this, eTemplateType_Null);
    m_pStuctualWidget[m_clu]->SetPatientPar(m_systemCode, m_hospitalCode, m_sex, m_strAge);
    m_pStuctualWidget[m_clu]->InitStructuralData();
    m_pStuctualWidget[m_clu]->SetDepartment(ControlCenter::getInstance()->m_department, ControlCenter::getInstance()->m_bodyPart);
    layout->addWidget(m_pStuctualWidget[m_clu], 1);
    m_pStuctualWidget[m_clu]->show();
    if (json != ""){
        m_updateDataCallBackEnable = true;
        setEditCallBack();
    }

    m_pStuctualWidget[m_clu]->SetViewerType(m_eViewerType, false, firstId, secondId, json, true);

    connectAll();
}

void ExplotWidget::reloadStructuralWidget(QString firstId, QString secondId, QString json)
{
    if (isFirst && m_eViewerType != eTemplateType_SRES)
    {
        isFirst = false;
        return;
    }
    ControlCenter::getInstance()->needLoadAll = true;
    initStructuralWidget(firstId, secondId, json);
    timer.start(10);
    return;
}

void ExplotWidget::setBtnStyleSheet(QString modifyBtn, QString deleteBtn)
{
    ControlCenter::getInstance()->setBtnStyleSheet(modifyBtn, deleteBtn);
}

void ExplotWidget::showAllDepartment(bool sw)
{
    if (sw)
    {
        m_beforeDepartment = ControlCenter::getInstance()->m_department;
        m_beforeBodyPart = ControlCenter::getInstance()->m_bodyPart;
        ControlCenter::getInstance()->m_department = "";
        ControlCenter::getInstance()->m_bodyPart = "";
        reloadStructuralWidget("", "", "");
    }
    else
    {
        ControlCenter::getInstance()->m_department = m_beforeDepartment;
        ControlCenter::getInstance()->m_bodyPart = m_beforeBodyPart;
        reloadStructuralWidget("", "", "");
    }
}

void ExplotWidget::updataToken(QString token)
{
    m_token = token;
    m_network->setRawHeader("Authorization", m_token.toLatin1());
    MyMeasureParameterManage::getInstance()->updataToken(m_token);
    CTemplateManage::GetSingle()->updataToken(m_token);
}

void ExplotWidget::ResetTreeData()
{
    if (m_pStuctualWidget[m_clu] == nullptr)
        return;
    m_pStuctualWidget[m_clu]->ResetTreeData();
}

QJsonObject ExplotWidget::GetSaveReportPar()
{
    QJsonObject empty;
    if (m_pStuctualWidget[m_clu] == nullptr)
        return empty;
    return m_pStuctualWidget[m_clu]->GetSaveReportPar();
}

void ExplotWidget::ClearCallBack()
{
    m_textEdit.clear();
    m_updateDataCallBackEnable = false;

    if (m_pStuctualWidget[0] == nullptr)
        return;
    m_pStuctualWidget[0]->ClearCallBack();
    if (m_pStuctualWidget[1] == nullptr)
        return;
    m_pStuctualWidget[1]->ClearCallBack();
}

void ExplotWidget::SetDepartment(const QString& strDepartment, const QString& strBodyPart)
{
    ControlCenter::getInstance()->m_department = strDepartment;
    ControlCenter::getInstance()->m_bodyPart = strBodyPart;
}
//存疑
QString ExplotWidget::GetReportKeyValueJson()
{
    if (m_pStuctualWidget[m_clu] == nullptr)
        return "";
    return m_pStuctualWidget[m_clu]->GetReportKeyValueJson();
}
//匹配
void ExplotWidget::ReloadTreeDataByNet()
{
    if (m_pStuctualWidget[m_clu] == nullptr)
        return;

    reloadStructuralWidget(ControlCenter::getInstance()->m_defaultFirstId, ControlCenter::getInstance()->m_defaultSecondId, "");
}

void ExplotWidget::SetReportStateByJson(const QString& strJson)
{
    if (m_pStuctualWidget[m_clu] == nullptr)
        return;
    m_pStuctualWidget[m_clu]->SetReportStateByJson(strJson);
}

void ExplotWidget::connectAll()
{
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::signReLoad, this, [=](const QString firstid, const QString secondid, QString json) {
        reloadStructuralWidget(firstid, secondid, json);
        });
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::measureParameterCheck, this, [=](const QStringList& measureParameterList) {
        measureParameterCheck(measureParameterList);
        });
    connect(m_pStuctualWidget[m_clu], &StructuralWidget::explainParameterCheck, this, [=](const QStringList& explainParameterList) {
        explainParameterCheck(explainParameterList);
        });
}

void ExplotWidget::setTextEdit(QPointer<MyTextReportWidget>vec)
{
    if (m_pStuctualWidget[m_clu] == nullptr)
        return;
    // //是否存在
    // bool isExit = false;
    // auto keys = m_textEdit.keys();
    // for (auto it : keys)
    // {
    //     if (vec->GetRemarks() == it)
    //         m_textEdit[it] = vec;
    // }
    //不存在则添加
    // if (!isExit)
    //     m_textEdit.insert(vec->GetRemarks(), vec);


    m_textEdit.insert(vec->GetRemarks(), vec);
    m_pStuctualWidget[m_clu]->SetCallBck(vec->GetRemarks(), stCallBack{ std::bind([vec,this](listReportData* pListData, const QString& strChangeId, eCopyState copy) {
                                                                          if (!vec.isNull() && this->m_updateDataCallBackEnable) {
                                                                              vec->SetReportData(pListData, strChangeId, copy);
                                                                          }
                                                                      }, std::placeholders::_1, std::placeholders::_2, eCopyState_CopyDrawById),
                                                                      std::bind(&MyTextReportWidget::GetReportKeyValueJson, vec) });

    vec->SetTransListDataCallBack(std::bind(&StructuralWidget::UpdateByReportData, m_pStuctualWidget[m_clu], std::placeholders::_1));
}

void ExplotWidget::setEditCallBack()
{
    auto values = m_textEdit.values();
    for (auto it : values)
    {
        if (it.isNull())
            continue;
        //设置回调 比之前多添加一个回调 并且设置remark
        m_pStuctualWidget[m_clu]->SetCallBck(it->GetRemarks(), stCallBack{ std::bind([it,this](listReportData* pListData, const QString& strChangeId, eCopyState copy) {
                                                                                if (!it.isNull() && this->m_updateDataCallBackEnable) {
                                                                                    it->SetReportData(pListData, strChangeId, copy);
                                                                                }
                                                                            }, std::placeholders::_1, std::placeholders::_2, eCopyState_CopyDrawById),
                                                        std::bind(&MyTextReportWidget::GetReportKeyValueJson, it) });

        it->SetTransListDataCallBack(std::bind(&StructuralWidget::UpdateByReportData, m_pStuctualWidget[m_clu], std::placeholders::_1));

    }
    //第一次进入 json为空值 但文本编辑框内已有内容 先不绑定 等结构化列表被点击后再连接
}

void ExplotWidget::SetPatientPar(const QString& systemCode, const QString& hospitalCode, const QString& sex, const QString& strAge)
{
    m_systemCode = systemCode;
    m_hospitalCode = hospitalCode;
    m_sex = sex;
    m_strAge = strAge;
}
