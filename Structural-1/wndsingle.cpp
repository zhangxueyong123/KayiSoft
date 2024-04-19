#include "wndsingle.h"
#include "qlineedit.h"
#include "qsizepolicy.h"
#include <iostream>
#include <fstream>
const QString comboBoxQss = QStringLiteral(
    "QComboBox{ background-color: transparent;color:  rgb(0, 0, 0);}"
    "QComboBox QAbstractItemView{ background-color: rgb(250,240,225);color:  rgb(0, 0, 0);}"
);


bool WndSingle::m_bIsUpadteReport = false;
//初始化布局。为WndSingle对象设置一个水平布局，并初始化一些子窗口和控件。
void WndSingle::InitLayout(QWidget* parent)
{
    if (m_pLayout == nullptr)
    {
        m_pLayout = new QHBoxLayout(parent);
        m_pLayout->setContentsMargins(6, 6, 6, 6);
        m_pLayout->setSpacing(6);
        m_pLayerWnd = new MyLayerWnd();
        m_pLayerWidget = new QSingleLineWidget(parent);
        m_pLayerWidget->m_pWndSingle = this;
        m_pLayerWidget->setLayout(m_pLayout);

    }

}

<<<<<<< HEAD
WndSingle::WndSingle(StructuralData* pData, UpdateDataCallBack updateCall, QWidget* parent,QString parentId)
//    :QWidget(nullptr)
{
    if (pData != nullptr)
=======

WndSingle::WndSingle(StructuralData *pData, UpdateDataCallBack updateCall, QWidget *parent)
//    :QWidget(nullptr)
{
    if(pData != nullptr)
    {
        m_pData = pData;
    }
    if(parent != nullptr)
    {
        m_pParent = parent;
        InitLayout(parent);
    }
    if(updateCall != nullptr)
    {
        m_UpdateDataCallBack = updateCall;
    }
}

void WndSingle::ShowChild(const QString &strId ,bool bShow, bool bMulti)
{

    for(auto &itMap : m_childTable)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        m_pData = pData;
    }
    if (parent != nullptr)
    {
        m_pParent = parent;
        InitLayout(parent);
    }
    if (updateCall != nullptr)
    {
        m_UpdateDataCallBack = updateCall;
    }
    if(parentId != "")
        m_strParentId = parentId;
}

//显示或隐藏子控件。根据提供的ID和布尔标志，决定是否显示特定的子控件。如果bMulti为false，还会隐藏所有不匹配的子控件。
void WndSingle::ShowChild(const QString& strId, bool bShow, bool bMulti)
{
    for (auto& itMap : m_childTable)
    {
        if (itMap.first == strId)
        {
<<<<<<< HEAD
            for (auto& itMapData : itMap.second)
=======

            for(auto &itMapData : itMap.second)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                itMapData.second->m_BoxListCollection.InitChild();
                itMapData.second->m_pLayerWidget->setVisible(bShow);
                itMapData.second->m_BoxListCollection.SetListVisible(bShow);
            }
        }
        else if (!bMulti)
        {
            for (auto& itMapData : itMap.second)
            {
                itMapData.second->m_BoxListCollection.InitChild();
                itMapData.second->m_pLayerWidget->setVisible(false);
                itMapData.second->m_BoxListCollection.SetListVisible(false);
            }
        }
    }
}

bool WndSingle::SetLabel(QWidget* parent, const QString& strTitle)
{
    if (m_pLabel == nullptr)
    {
        m_pLabel = new QLabel(parent);
        m_pLabel->setWordWrap(true);
        m_pLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    else
    {
        return false;
    }
    m_pLabel->setText(strTitle);
    return true;

}
//创建一个空的布局。可能用于占位或作为动态添加控件的容器
void WndSingle::CreateEmpty(QWidget* parent, const QString& strTitle, MyLayerWnd* pLayerWnd, int nDataPostion)
{
    stBoxListCollection* pBoxPtr = &m_BoxListCollection;
    InitLayout(parent);
    pBoxPtr->dataType = (pBoxPtr->dataType | eStructuralDataType_UnNow);
    bool bIsInit = SetLabel(parent, strTitle);
    m_strTitle = strTitle;
    QLabel* pLabel = new QLabel("", parent);
    if (bIsInit)
    {
        m_pLayout->addWidget(m_pLabel);

        m_pLayout->addWidget(pLabel);
        m_pLayout->setStretch(0, 1);
        m_pLayout->setStretch(1, 2);
    }
    if (pLayerWnd == nullptr)
    {
        m_pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
    }
    else
    {
        pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
    }
}
//创建单选框
QString WndSingle::CreateSingle(QWidget* parent,
    bool bInit, const QString& strTitle, const std::vector<stStructuralNode>& vecNode, bool bCreateChild,
    const QString& strRemarks,
    MyLayerWnd* pLayerWnd, stBoxListCollection* pBox, bool bUseTitle, eDataPostion pos, int nDataPostion)
{
    stBoxListCollection* pBoxPtr = pBox == nullptr ? &m_BoxListCollection : pBox;
    if (pBox == nullptr && !strRemarks.isEmpty())
    {
        pBoxPtr->strRemarks = strRemarks;
    }
    InitLayout(parent);
    pBoxPtr->dataType = (pBoxPtr->dataType | eStructuralDataType_Single);
    bool bIsInit = SetLabel(parent, strTitle);

    QString strParent;
    m_strTitle = strTitle;
    int nCul = 0;
    for (auto& itVec : vecNode)
    {
        if (!itVec.bStatus)
        {
            //根据据节点信息创建单选框
<<<<<<< HEAD
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, this);

            if (itVec.bIsExplain)
            {
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent, call,
                    itVec.strID, itVec.strPrefix,/*bUseTitle ? itVec.strTitle : itVec.strLabel*///,
                    "", itVec.strSuffix,
                    itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                    pos, bInit, itVec.strTip, itVec.strDataType, itVec.strLabel);
=======
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,  this);

            if(itVec.bIsExplain)
            {
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent,call,
                                                                      itVec.strID, itVec.strPrefix,/*bUseTitle ? itVec.strTitle : itVec.strLabel*///,
                                                                      "",itVec.strSuffix,
                                                                      itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                                                                      pos, bInit,itVec.strTip,itVec.strDataType,itVec.strLabel);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                //pBoxPtr->strExplain = itVec.strLabel;
            }
            else
            {
<<<<<<< HEAD
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent, call,
                    itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                    itVec.strPrefix, itVec.strSuffix,
                    itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                    pos, bInit, itVec.strTip, itVec.strDataType, "");
=======
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent,call,
                                                                               itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                                                                               itVec.strPrefix,itVec.strSuffix,
                                                                               itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                                                                               pos, bInit,itVec.strTip,itVec.strDataType, "");
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            }


            // Add2MeasureParameterWidget(ptr, itVec.strTip);

            pBoxPtr->bChangeDir = itVec.strTextSort == "1";

            pBoxPtr->mapType[itVec.strID] = eStructuralDataType_Single;
            pBoxPtr->radioList.SetExclusive(true);
            strParent = itVec.strParentId;

            if (!itVec.mapChild.empty() && bCreateChild)
            {
                //创建子节点
                CreateTableChild(parent, strTitle, pLayerWnd, itVec, pos, pBoxPtr, nCul++, strRemarks);
            }
        }

    }
    if (!pBoxPtr->radioList.Empty())
    {
        pBoxPtr->AddLayout(pBoxPtr->radioList.list.pLayout);
    }
    if (bIsInit)
    {
        m_pLayout->addWidget(m_pLabel);
        if (!pBoxPtr->radioList.Empty())
        {
            m_pLayout->addWidget(pBoxPtr->pSingleWidget);
        }
        m_pLayout->setStretch(0, 1);
        m_pLayout->setStretch(1, 2);
    }
    for (auto& childBox : pBoxPtr->childBox)
    {
        for (auto& itBox : childBox.second)
        {
            pBoxPtr->pSingleLayout->addWidget(itBox.pSingleWidget);
        }
    }
    if (pBox == nullptr)
    {
        if (pLayerWnd == nullptr)
        {
            m_pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
        }
        else
        {
            pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
        }
    }
    return strParent;
}
//创建多选框
QString WndSingle::CreateMulti(QWidget* parent,
    bool bInit,
    const QString& strTitle,
    const std::vector<stStructuralNode>& vecNode,
    bool bCheckState,
    bool bCreateChild,
    const QString& strRemarks,
    MyLayerWnd* pLayerWnd,
    stBoxListCollection* pBox,
    bool bUseTitle,
    eDataPostion pos,
    int nDataPostion)
{

    stBoxListCollection* pBoxPtr = pBox == nullptr ? &m_BoxListCollection : pBox;
    if (pBox == nullptr && !strRemarks.isEmpty())
    {
        pBoxPtr->strRemarks = strRemarks;
    }
    InitLayout(parent);
    m_strTitle = strTitle;
    pBoxPtr->dataType = (pBoxPtr->dataType | eStructuralDataType_Multi);
    bool bIsInit = SetLabel(parent, strTitle);
    QString strParent;
    int nCul = 0;
    //int nWnd = 0;

    for (auto& itVec : vecNode)
    {
        //根据据节点信息创建多选框
        bool bCreate = true;
        if (bCheckState && itVec.bStatus)
        {
            bCreate = false;
        }
        if (bCreate)
        {
<<<<<<< HEAD
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, this);
            if (itVec.bIsExplain)
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, itVec.strPrefix,
                    "", itVec.strSuffix, itVec.strDefaultValue == "true",
                    pos, bInit, itVec.strTip, itVec.strDataType, itVec.strLabel);
                // pBoxPtr->strExplain = itVec.strLabel;
            }
            else
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                    itVec.strPrefix, itVec.strSuffix, itVec.strDefaultValue == "true",
                    pos, bInit, itVec.strTip, itVec.strDataType, "");
=======
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,  this);
            if(itVec.bIsExplain)
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent,call,itVec.strID,  itVec.strPrefix,
                                                                               "",itVec.strSuffix, itVec.strDefaultValue ==  "true",
                                                                               pos, bInit,itVec.strTip, itVec.strDataType,itVec.strLabel);
               // pBoxPtr->strExplain = itVec.strLabel;
            }
            else
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent,call,itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                                                                               itVec.strPrefix,itVec.strSuffix, itVec.strDefaultValue ==  "true",
                                                                               pos, bInit,itVec.strTip, itVec.strDataType, "");
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            }


            //   Add2MeasureParameterWidget(ptr, itVec.strTip);
            pBoxPtr->bChangeDir = itVec.strTextSort == "1";

            pBoxPtr->mapType[itVec.strID] = eStructuralDataType_Multi;
            pBoxPtr->boxList.SetExclusive(false);
            strParent = itVec.strParentId;
            if (!itVec.mapChild.empty() && bCreateChild)
            {
                QString strRemarks;
                //创建子节点
<<<<<<< HEAD
                CreateTableChild(parent, strTitle, pLayerWnd, itVec, pos, pBoxPtr, nCul++, strRemarks);
=======
                CreateTableChild(parent, strTitle, pLayerWnd, itVec, pos, pBoxPtr, nCul++,strRemarks);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                //                auto itEnd = pBoxPtr->childBox.end();
                //                --itEnd;
                //                for(auto &itBox :itEnd->second )
                //                {
                //                    pBoxPtr->boxList.list.pLayout->addWidget(itBox.pSingleWidget);
                //                 //   pBoxPtr->pSingleLayout->addWidget(itBox.pSingleWidget);
                //                }
            }

        }

    }
    if (!pBoxPtr->boxList.Empty())
    {
        pBoxPtr->AddLayout(pBoxPtr->boxList.list.pLayout);
    }
    if (bIsInit)
    {
        m_pLayout->addWidget(m_pLabel);
        if (!pBoxPtr->boxList.Empty())
        {
            m_pLayout->addWidget(pBoxPtr->pSingleWidget);
        }
        m_pLayout->setStretch(0, 1);
        m_pLayout->setStretch(1, 2);
    }
    for (auto& childBox : pBoxPtr->childBox)
    {
        for (auto& itBox : childBox.second)
        {
            pBoxPtr->pSingleLayout->addWidget(itBox.pSingleWidget);
        }
    }
    if (pBox == nullptr)
    {
        if (pLayerWnd == nullptr)
        {

            m_pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);

            m_pLayerWnd->SetTitle(m_strTitle);
        }
        else
        {
            pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
        }
    }

    return strParent;
}
<<<<<<< HEAD
//创建下拉框（包括左上角的左侧和右侧）
MyComboBox* WndSingle::CreateCombobox(QWidget* parent, const QString& strId, const std::vector<QString>& vecCombobox,
    const std::vector<QString>& vecId,
    const std::vector<QString> vecRemarks,
    const std::vector<QString>& vecOther,
    const std::vector<QString>& vecShow,
    bool hasThrid,
    //                                       const QString &strOhter,
    //                                       const QString &strTip,
    int nDefulat, bool bAddLayer2Wnd
    , MapNameAndIdList* pVecNex, bool bNormal)
=======

MyComboBox * WndSingle::CreateCombobox(QWidget *parent, const QString &strId,const std::vector<QString> &vecCombobox,
                                       const std::vector<QString> &vecId,
                                       const std::vector<QString> vecRemarks,
                                       const std::vector<QString>& vecOther,
                                       const std::vector<QString>& vecShow,
                                       //                                       const QString &strOhter,
                                       //                                       const QString &strTip,
                                       int nDefulat,bool bAddLayer2Wnd
                                       ,MapNameAndIdList *pVecNex ,bool bNormal  )
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
{
    InitLayout(parent);
    m_BoxListCollection.dataType = eStructuralDataType_Combobox;


<<<<<<< HEAD
    //根据据节点信息创建下拉框，itNow：左侧的（CT结构化MR结构化等7项）combobox
    auto call = std::bind(&WndSingle::ComboboxChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, this);
    auto itNow = m_BoxListCollection.comboboxList.AddSingle/*<ComboboxChangeCallBack>*/(parent, call, strId, "",
        "", "", false, eDataPostion_Left, true, "", "", "");
    itNow->level = 1;
    for (auto& itVec : vecOther)
=======
    //根据据节点信息创建下拉框
    auto call = std::bind(&WndSingle::ComboboxChange, this, std::placeholders::_1, std::placeholders::_2, this);
    auto itNow = m_BoxListCollection.comboboxList.AddSingle/*<ComboboxChangeCallBack>*/(parent,call, strId, "",
                                                                                        "","", false, eDataPostion_Left, true, "", "", "");
    for(auto &itVec : vecOther)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        if (!itVec.isEmpty())
        {
            MyMeasureParameterManage::GetSingle()->AddMeasureParameters(itVec);
        }
    }
    itNow->setStyleSheet(comboBoxQss);
    itNow->setMinimumWidth(30);
    itNow->clear();
    itNow->m_mapComboboxId.clear();
    auto itVecId = vecId.begin();
    auto itVecShow = vecShow.begin();
<<<<<<< HEAD
    for (auto& itVec : vecCombobox)
    {
        itNow->addItem(itVec, *itVecId);
        itNow->m_mapComboboxId[*itVecId] = { *itVecId, itVec, "", *itVecShow, false };
=======
    for(auto &itVec: vecCombobox)
    {
        itNow->addItem(itVec, *itVecId);
        itNow->m_mapComboboxId[*itVecId] = {*itVecId, itVec, "", *itVecShow, false};
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
        ++itVecShow;
        ++itVecId;
    }
    if (!bNormal)
    {
        itNow->setCurrentIndex(-1);
    }
    if (!vecCombobox.empty() && bNormal)
    {
        if (nDefulat >= 0 && nDefulat < (int)vecCombobox.size())
        {
            itNow->setCurrentIndex(nDefulat);
        }
        else
        {
            itNow->setCurrentIndex(0);
        }

    }
    m_pLayout->addWidget(itNow);
    m_pLayout->setStretch(0, 1);
    m_pLayout->setStretch(1, 1);
    m_pLayout->setStretch(2, 1);
    //m_pLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);  
    //m_pLayout->setAlignment( Qt::AlignLeft | Qt::AlignVCenter);
    //m_pLayerWidget->setLayout(m_pLayout);
    if (bAddLayer2Wnd)
    {
        m_pLayerWnd->AddWnd(m_pLayerWidget);
    }

    if (pVecNex != nullptr)
    {
        itNow->m_mapNextNameAndId = *pVecNex;
        std::vector<QString > vecOtherNext;
        std::vector<QString > vecShowNext;


<<<<<<< HEAD
        for (auto& itMap : itNow->m_mapNextNameAndId)
=======
        for(auto &itMap : itNow->m_mapNextNameAndId)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
        {
            for (auto& itVec : itMap.second)
            {
                vecOtherNext.push_back(itVec.strOther);
                vecShowNext.push_back(itVec.strShow);
            }
        }
        if (!vecCombobox.empty())
        {
<<<<<<< HEAD
            //SetChild设置右侧的combobox，（如CT结构化下对应的左肺、急性胰腺炎....等）
            auto itChild = CreateCombobox(parent, "child", std::vector<QString>(), std::vector<QString>(), std::vector<QString>(), vecOtherNext, vecShowNext, !hasThrid, 0, true);
=======

            auto itChild = CreateCombobox(parent, "child", std::vector<QString>(), std::vector<QString>(),std::vector<QString>(),vecOtherNext,vecShowNext, 0, true);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            itNow->SetChild(itChild, *vecId.begin());
            itChild->level = 2;
        }

    }
    if (hasThrid)
    {
        auto itChild = CreateCombobox(parent, "thrid", std::vector<QString>(), std::vector<QString>(), std::vector<QString>(),
            std::vector<QString>(), std::vector<QString>(), false, 0, true);
        itChild->level = 3;
        //itNow->SetChild(itChild, *vecId.begin());
    }
    return itNow;
}
//创建文本框
QString WndSingle::CreateText(QWidget* parent,
    bool bInit, const QString& strTitle, const std::vector<stStructuralNode>& vecNode,
    const QString& strRemarks,
    MyLayerWnd* pLayerWnd, stBoxListCollection* pBox, bool bUseTitle, eDataPostion pos, int nDataPostion)
{
    stBoxListCollection* pBoxPtr = pBox == nullptr ? &m_BoxListCollection : pBox;
    if (vecNode.empty())
    {
        return "";
    }
    if (pBox == nullptr && !strRemarks.isEmpty())
    {
        pBoxPtr->strRemarks = strRemarks;
    }
    InitLayout(parent);
    m_strTitle = strTitle;
    pBoxPtr->dataType = (pBoxPtr->dataType | eStructuralDataType_Edit);
    bool bIsInit = SetLabel(parent, strTitle);
    QString strParent;
    for (auto& itVec : vecNode)
    {
        if (!itVec.bStatus)
        {
            //根据据节点信息创建文本框
<<<<<<< HEAD
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, this);
            auto itNow = pBoxPtr->lineEditList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, itVec.strDefaultValue,
                itVec.strPrefix, itVec.strSuffix, false, pos, bInit, itVec.strTip, itVec.strDataType, "");
=======
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,  this);
            auto itNow = pBoxPtr->lineEditList.AddSingle/*<StateChangeCallBack>*/(parent,call,itVec.strID, itVec.strDefaultValue,
                                                                                  itVec.strPrefix,itVec.strSuffix, false, pos, bInit,itVec.strTip, itVec.strDataType, "");
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            // Add2MeasureParameterWidget(itVec.strTip);
            strParent = itVec.strParentId;
            pBoxPtr->mapType[itVec.strID] = eStructuralDataType_Edit;
            if (itNow != nullptr)
            {
                itNow->setMaximumWidth(120);
            }
            pBoxPtr->bChangeDir = itVec.strTextSort == "1";
        }
    }
    pBoxPtr->lineEditList.SetExclusive(false);
    if (!pBoxPtr->lineEditList.Empty())
    {
        pBoxPtr->AddLayout(pBoxPtr->lineEditList.list.pLayout);
    }
    if (bIsInit)
    {
        m_pLayout->addWidget(m_pLabel);
        if (!pBoxPtr->lineEditList.Empty())
        {
            m_pLayout->addWidget(pBoxPtr->pSingleWidget);
        }
        m_pLayout->setStretch(0, 1);
        m_pLayout->setStretch(1, 2);
    }
    if (pLayerWnd == nullptr)
    {

        m_pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
    }
    else
    {
        pLayerWnd->AddWnd(m_pLayerWidget, nDataPostion);
    }
    return strParent;
}
<<<<<<< HEAD
//创建子列表
WndSingle* WndSingle::CreateChildTable(QWidget* parent, StructuralData& data, const std::vector<QString>& vecCombobox, const std::vector<QString>& vecId
    , const std::vector<QString> vecRemarks,
    const std::vector<QString>& vecOther,
    const std::vector<QString>& vecShow, UpdateDataCallBack callBack, bool bIsCombobox, int nDefulat)
{
    //创建子列表
    WndSingle* pWnd = new WndSingle(&data, callBack, parent);
=======

WndSingle * WndSingle::CreateChildTable(QWidget *parent, StructuralData &data, const std::vector<QString> &vecCombobox, const std::vector<QString> &vecId
                                        ,const std::vector<QString> vecRemarks,
                                        const std::vector<QString>& vecOther,
                                        const std::vector<QString>& vecShow, UpdateDataCallBack callBack, bool bIsCombobox,  int nDefulat)
{
    //创建子列表
    WndSingle *pWnd = new WndSingle(&data, callBack, parent);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c

    if (bIsCombobox)
    {
        //如果子节点是下拉框
<<<<<<< HEAD
        pWnd->CreateCombobox(parent, "", vecCombobox, vecId, vecRemarks, vecOther, vecShow, false, nDefulat/*, true, nullptr, false*/);
=======
        pWnd->CreateCombobox(parent,"", vecCombobox,vecId,vecRemarks,vecOther,vecShow,  nDefulat/*, true, nullptr, false*/);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
    else
    {
        //先添加文本
        pWnd->SetLabel(parent, *vecCombobox.begin());
        pWnd->m_pLayout->addWidget(pWnd->m_pLabel);
        pWnd->m_pLayerWnd->AddWnd(pWnd->m_pLayerWidget);

    }
    pWnd->m_pLayerWidget->m_bIsTable = true;
    if (pWnd->m_pLayerTableWidgt == nullptr)
    {
        pWnd->m_pLayerTableWidgt = new QSingleLineWidget(parent);
        pWnd->m_pLayerTableWidgt->m_pWndSingle = pWnd;
    }
    pWnd->m_pLayerTableWidgt->m_bIsTable = true;
    //创建子列表
    //pWnd->m_UpdateDataCallBack = callBack;
<<<<<<< HEAD
    pWnd->CreateChildTable(parent, data, vecId, vecRemarks, pWnd, callBack);
    pWnd->m_pLayerWnd->m_pLayout->setSpacing(0);
    pWnd->m_pLayerWnd->m_pLayout->setContentsMargins(0, 0, 0, 0);
=======
    pWnd->CreateChildTable(parent, data, vecId,vecRemarks,pWnd,callBack);
    pWnd->m_pLayerWnd->m_pLayout->setSpacing(0);
    pWnd->m_pLayerWnd->m_pLayout->setContentsMargins(0,0,0,0);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    pWnd->m_pLayerTableWidgt->setLayout(pWnd->m_pLayerWnd->m_pLayout);
    return pWnd;
}

<<<<<<< HEAD
bool WndSingle::LoadDictionaryData(const QString& strId, bool bCheck, bool bWait)
{
    if (m_pData == nullptr)
    {
        return false;
    }
    if (m_childTable.empty())
    {
        return false;
    }
    auto nodes = m_pData->FindChildById(strId, false);
    if (!nodes.empty())
    {
        return false;
    }
    QString strUrl = m_pData->m_strUrl + strId + "&depth=10";
    // QString strDst;
   //  auto ret = CNetWork::netWorkGet(m_pData->m_network, strUrl, strDst);

    QEventLoop* eventLoop = nullptr;
    if (bWait)
    {
        eventLoop = new QEventLoop();
    }
    QNetworkRequest request = QNetworkRequest(strUrl);
    QNetworkReply* reply = m_pData->m_network->get(request);
    QObject::connect(reply, &QNetworkReply::finished, [=]
        {
            if (reply->error() == QNetworkReply::NoError)
=======
bool WndSingle::LoadDictionaryData(const QString &strId, bool bCheck, bool bWait)
{
    if(m_pData == nullptr)
    {
        return false;
    }
    if(m_childTable.empty())
    {
        return false;
    }
    auto nodes = m_pData->FindChildById(strId, false);
    if(!nodes.empty())
    {
        return false;
    }
    QString strUrl = m_pData->m_strUrl + strId + "&depth=10";
   // QString strDst;
  //  auto ret = CNetWork::netWorkGet(m_pData->m_network, strUrl, strDst);

    QEventLoop *eventLoop = nullptr;
    if (bWait )
    {
        eventLoop = new QEventLoop();
    }
    QNetworkRequest request = QNetworkRequest(strUrl);
    QNetworkReply* reply = m_pData->m_network->get(request);
    QObject::connect(reply, &QNetworkReply::finished, [=]
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QString strDst = reply->readAll();
            if(/*ret != 0 ||*/ strDst.isEmpty())
            {
                return ;
            }
            m_pData->AddChildByJson(strDst, strId);
            std::vector<QString> vecName;
            std::vector<QString> vecId;
            std::vector<QString> vecRemarks;
            vecId.push_back(strId);
            vecName.push_back(m_strTitle);
            vecRemarks.push_back(m_BoxListCollection.strRemarks);
            this->CreateChildTable(m_pParent, *m_pData, vecId,vecRemarks,this,m_UpdateDataCallBack);
            for(auto &itTable : m_childTable)
            {
                for(auto &itData : itTable.second)
                {
                    itData.second->m_BoxListCollection.InitChild();
                    itData.second->m_BoxListCollection.SetListVisible(bCheck);
                }
            }
            if(bWait && eventLoop != nullptr)
            {
                eventLoop->quit();
            }
        }
    });
    if(bWait&& eventLoop != nullptr)
    {
        eventLoop->exec();
        delete eventLoop;
        eventLoop = nullptr;
    }

    // m_bCanCallBack = true;
    return true;
    //    auto pChild =  CreateChildTable(m_pParent,*m_pData, vecName,vecId,vecRemarks,  std::vector<QString> (),std::vector<QString> (),  m_UpdateDataCallBack, false, 0);
    //    pChild->m_BoxListCollection.InitChild();
    //    pChild->m_strTitle = m_strTitle;
    //    m_vecChildWnd.push_back(pChild);
    //    m_pLayerWnd->ShowWithTable(true);
    //    m_pLayerWnd->AddWnd( pChild->m_pLayerTableWidgt/*,1*/);
    //    m_vecChildWnd.push_back(pWnd);
    //    m_pLayerWnd->AddWnd(pWnd->m_pLayerWnd->m_pGroupBox);

}
void WndSingle::CreateChildNode(QWidget *parent, const std::vector<stStructuralNode> &vecNode, StructuralData &data,
                                WndSingle *pParentWnd, UpdateDataCallBack callBack, const QString & addTitle, eStructuralDataType tableParentType
                                , int nNowDeap, int nCheckDeap,const QString &strRemarks)
{
    //创建子节点
    //    std::vector<QString> vecCombobox;
    //    std::vector<QString> vecId;
    //    std::vector<QString> vecRemarks;
    //    std::vector<QString> vecOther;
    //    std::vector<QString> vecShow;
    int nDefulat = -1;
    int nCul = 0;

    std::vector<stChildNodeDraw> vecDraw;

    for(auto &itVec : vecNode)
    {
        stChildNodeDraw draw;
        draw.strId = itVec.strID;
        draw.strTitle = itVec.strTitle;
        draw.strRemarks  = itVec.strRemarks;
        if(!strRemarks.isEmpty())
        {
            draw.strRemarks = strRemarks;
        }
        else  if(nNowDeap == nCheckDeap)
        {
            draw.strRemarks = itVec.strRemarks;
        }
        auto vecChileNow  = data.FindChildById(draw.strId, true);
        draw.eElementAttribute = itVec.eElementAttribute;

        bool bCrateTable = false;
        if(itVec.bStatus/*vecChileNow.empty()*/)
        {
            bCrateTable = true;
            draw.eElementAttribute  = eStructuralDataType_Combobox;
        }

        draw.strShowTitle = addTitle.isEmpty() ? draw.strTitle : addTitle;
        //根据状态创建控件
        draw.strOther = itVec.strOther;
        draw.strShow  = itVec.strValue3;
        if(draw.eElementAttribute != eStructuralDataType_Combobox)
        {
            draw.vecChild = vecChileNow;
        }
        else
        {
            if(nDefulat == -1 && itVec.strDefaultValue == "true")
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                QString strDst = reply->readAll();
                if (/*ret != 0 ||*/ strDst.isEmpty())
                {
                    return;
                }
                m_pData->AddChildByJson(strDst, strId);
                std::vector<QString> vecName;
                std::vector<QString> vecId;
                std::vector<QString> vecRemarks;
                vecId.push_back(strId);
                vecName.push_back(m_strTitle);
                vecRemarks.push_back(m_BoxListCollection.strRemarks);
                this->CreateChildTable(m_pParent, *m_pData, vecId, vecRemarks, this, m_UpdateDataCallBack);
                for (auto& itTable : m_childTable)
                {
                    for (auto& itData : itTable.second)
                    {
                        itData.second->m_BoxListCollection.InitChild();
                        //添加设置子控件类型及 子控件状态生效  zdy 20240416
                        itData.second->m_BoxListCollection.SetAllChildRemarks();
                        itData.second->m_BoxListCollection.AllBoxWork(true);
                        itData.second->m_BoxListCollection.SetListVisible(bCheck);
                    }
                }
                if (bWait && eventLoop != nullptr)
                {
                    eventLoop->quit();
                }
            }
<<<<<<< HEAD
        });
    if (bWait && eventLoop != nullptr)
    {
        eventLoop->exec();
        delete eventLoop;
        eventLoop = nullptr;
=======
            if(itVec.bIsComboboxTop)
            {
                nDefulat = nCul;
            }
            ++nCul;
        }
        vecDraw.push_back(draw);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
    if(!vecDraw.empty())
    {

<<<<<<< HEAD
    // m_bCanCallBack = true;
    return true;
    //    auto pChild =  CreateChildTable(m_pParent,*m_pData, vecName,vecId,vecRemarks,  std::vector<QString> (),std::vector<QString> (),  m_UpdateDataCallBack, false, 0);
    //    pChild->m_BoxListCollection.InitChild();
    //    pChild->m_strTitle = m_strTitle;
    //    m_vecChildWnd.push_back(pChild);
    //    m_pLayerWnd->ShowWithTable(true);
    //    m_pLayerWnd->AddWnd( pChild->m_pLayerTableWidgt/*,1*/);
    //    m_vecChildWnd.push_back(pWnd);
    //    m_pLayerWnd->AddWnd(pWnd->m_pLayerWnd->m_pGroupBox);

=======
        if(nDefulat == -1)
        {
            nDefulat = 0;
        }
        std::vector<QString> vecCombobox;
        std::vector<QString> vecId;
        std::vector<QString> vecRemarks;
        std::vector<QString> vecOther;
        std::vector<QString> vecShow;
        for(auto & itVec : vecDraw)
        {
            if(itVec.eElementAttribute == eStructuralDataType_Combobox)
            {
                if(tableParentType == eStructuralDataType_Combobox)
                {
                    vecCombobox.push_back(itVec.strTitle);
                    vecId.push_back(itVec.strId);
                    vecRemarks.push_back(itVec.strRemarks);
                    vecOther.push_back(itVec.strOther);
                    vecShow.push_back(itVec.strShow);
                }
                else
                {
                    WndSingle *pWnd = new WndSingle(&data,callBack,  parent);
                    pWnd->m_bIsCreateBySelf = true;
                    pWnd->m_strId = itVec.strId;
                    pWnd->m_strTitle = itVec.strTitle;
                    std::vector<QString> vecNameTmp;
                    std::vector<QString> vecIdTmp;
                    std::vector<QString> vecRemarksTmp;
                    std::vector<QString> vecOtherTmp;
                    std::vector<QString> vecShowTmp;
                    vecNameTmp.push_back(itVec.strTitle);
                    vecIdTmp.push_back(itVec.strId);
                    vecRemarksTmp.push_back(itVec.strRemarks);
                    vecOtherTmp.push_back(itVec.strOther);
                    vecShowTmp.push_back(itVec.strShow);
                    WndSingle *pChild = CreateChildTable(parent,data, vecNameTmp,vecIdTmp,vecRemarksTmp, vecOtherTmp, vecShowTmp, callBack, false, 0);
                    pChild->m_strTitle = itVec.strTitle;
                    pWnd->m_vecChildWnd.push_back(pChild);
                    pWnd->m_pLayerWnd->ShowWithTable(tableParentType == eStructuralDataType_Combobox);
                    pWnd->m_pLayerWnd->AddWnd( pChild->m_pLayerTableWidgt/*,1*/);
                    m_vecChildWnd.push_back(pWnd);
                    m_pLayerWnd->AddWnd(pWnd->m_pLayerWnd->m_pGroupBox);
                }

            }
            else
            {
                WndSingle *pWnd = new WndSingle(&data, callBack, parent);
                switch (itVec.eElementAttribute )
                {
                case eStructuralDataType_Single:
                {
                    pWnd->CreateSingle(parent,true, itVec.strShowTitle, itVec.vecChild, false,itVec.strRemarks );break;
                }
                case eStructuralDataType_Multi:
                {
                    pWnd->CreateMulti(parent,true, itVec.strShowTitle, itVec.vecChild, eStructuralDataType_Combobox == tableParentType, false,itVec.strRemarks);
                    break;
                }
                case eStructuralDataType_Edit:
                {
                    pWnd->CreateText(parent,true, itVec.strShowTitle, itVec.vecChild,itVec.strRemarks);break;
                }
                case eStructuralDataType_Combobox:
                {
                    break;
                }
                default:
                    break;
                }
                if(!itVec.vecChild.empty())
                {
                    pWnd->CreateChildNode(parent, itVec.vecChild,data, this,callBack,"", tableParentType,nNowDeap + 1, nCheckDeap,
                                          itVec.strRemarks/*,strNowRemarks*/);
                }
                pWnd->m_strId = itVec.strId;
                pWnd->m_strTitle = itVec.strTitle;
                m_vecChildWnd.push_back(pWnd);
            }
        }
        if(m_bAddChild)
        {
            for(auto &itVecChild : m_vecChildWnd)
            {
                m_pLayerWnd->AddWnd(itVecChild->m_pLayerWnd->m_pGroupBox);
            }
        }
        if(!vecCombobox.empty())
        {
            auto pWnd =  CreateChildTable(parent,data, vecCombobox,vecId,vecRemarks, vecOther,vecShow,  callBack, true,nDefulat);
            m_pLayerWnd->ShowWithTable(tableParentType == eStructuralDataType_Combobox);
            m_pLayerWnd->AddWnd( pWnd->m_pLayerTableWidgt/*,1*/);
            m_vecChildWnd.push_back(pWnd);
        }
    }
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
}
//创建所有子节点（递归创建（如：左肺-报告描述-前段：是，否，实性，部分实性））
void WndSingle::CreateChildNode(QWidget* parent, const std::vector<stStructuralNode>& vecNode, StructuralData& data,
    WndSingle* pParentWnd, UpdateDataCallBack callBack, const QString& addTitle, eStructuralDataType tableParentType
    , int nNowDeap, int nCheckDeap, const QString& strRemarks)
{
    int nDefulat = -1;
    int nCul = 0;

<<<<<<< HEAD
    std::vector<stChildNodeDraw> vecDraw;

    for (auto& itVec : vecNode)
    {
        stChildNodeDraw draw;
        draw.strId = itVec.strID;
        draw.strTitle = itVec.strUnionTitle;
        //先摆烂  不能这么写
        if (draw.strTitle == "")
            draw.strTitle = itVec.strTitle;
        draw.strRemarks = itVec.strRemarks;
        if (!strRemarks.isEmpty())
        {
            draw.strRemarks = strRemarks;
        }
        else  if (nNowDeap == nCheckDeap)
        {
            draw.strRemarks = itVec.strRemarks;
        }
        //  WndSingle *pWnd = new WndSingle(parent);
        auto vecChileNow = data.FindChildById(draw.strId, true);
        draw.eElementAttribute = itVec.eElementAttribute;

        bool bCrateTable = false;
        if (itVec.bStatus/*vecChileNow.empty()*/)
        {
            bCrateTable = true;
            draw.eElementAttribute = eStructuralDataType_Combobox;
        }

        draw.strShowTitle = addTitle.isEmpty() ? draw.strTitle : addTitle;
        // vecDraw.push_back();
         //根据状态创建控件
        draw.strOther = itVec.strOther;
        draw.strShow = itVec.strValue3;
        if (draw.eElementAttribute != eStructuralDataType_Combobox)
        {
            draw.vecChild = vecChileNow;
        }
        else
        {
            if (nDefulat == -1 && itVec.strDefaultValue == "true")
            {
                nDefulat = nCul;
            }
            if (itVec.bIsComboboxTop)
            {
                nDefulat = nCul;
            }
            ++nCul;
        }
        vecDraw.push_back(draw);
    }
    if (!vecDraw.empty())
    {

        if (nDefulat == -1)
        {
            nDefulat = 0;
        }
        std::vector<QString> vecCombobox;
        std::vector<QString> vecId;
        std::vector<QString> vecRemarks;
        std::vector<QString> vecOther;
        std::vector<QString> vecShow;
        for (auto& itVec : vecDraw)
        {
            if (itVec.eElementAttribute == eStructuralDataType_Combobox)
            {
                if (tableParentType == eStructuralDataType_Combobox)
                {
                    vecCombobox.push_back(itVec.strTitle);
                    vecId.push_back(itVec.strId);
                    vecRemarks.push_back(itVec.strRemarks);
                    vecOther.push_back(itVec.strOther);
                    vecShow.push_back(itVec.strShow);
                }
                else
                {
                    WndSingle* pWnd = new WndSingle(&data, callBack, parent);
                    pWnd->m_bIsCreateBySelf = true;
                    pWnd->m_strId = itVec.strId;
                    pWnd->m_strTitle = itVec.strTitle;
                    std::vector<QString> vecNameTmp;
                    std::vector<QString> vecIdTmp;
                    std::vector<QString> vecRemarksTmp;
                    std::vector<QString> vecOtherTmp;
                    std::vector<QString> vecShowTmp;
                    vecNameTmp.push_back(itVec.strTitle);
                    vecIdTmp.push_back(itVec.strId);
                    vecRemarksTmp.push_back(itVec.strRemarks);
                    vecOtherTmp.push_back(itVec.strOther);
                    vecShowTmp.push_back(itVec.strShow);
                    WndSingle* pChild = CreateChildTable(parent, data, vecNameTmp, vecIdTmp, vecRemarksTmp, vecOtherTmp, vecShowTmp, callBack, false, 0);
                    pChild->m_strTitle = itVec.strTitle;
                    pWnd->m_vecChildWnd.push_back(pChild);
                    pWnd->m_pLayerWnd->ShowWithTable(tableParentType == eStructuralDataType_Combobox);
                    pWnd->m_pLayerWnd->AddWnd(pChild->m_pLayerTableWidgt/*,1*/);
                    m_vecChildWnd.push_back(pWnd);
                    m_pLayerWnd->AddWnd(pWnd->m_pLayerWnd->m_pGroupBox);
                }

            }
            else
            {
                WndSingle* pWnd = new WndSingle(&data, callBack, parent);
                switch (itVec.eElementAttribute)
                {
                case eStructuralDataType_Single:
                {
                    pWnd->CreateSingle(parent, true, itVec.strShowTitle, itVec.vecChild, false, itVec.strRemarks); break;
                }
                case eStructuralDataType_Multi:
                {
                    pWnd->CreateMulti(parent, true, itVec.strShowTitle, itVec.vecChild, eStructuralDataType_Combobox == tableParentType, false, itVec.strRemarks);
                    break;
                }
                case eStructuralDataType_Edit:
                {
                    pWnd->CreateText(parent, true, itVec.strShowTitle, itVec.vecChild, itVec.strRemarks); break;
                }
                case eStructuralDataType_Combobox:
                {
                    break;
                }
                default:
                    break;
                }
                if (!itVec.vecChild.empty())
                {
                    pWnd->CreateChildNode(parent, itVec.vecChild, data, this, callBack, "", tableParentType, nNowDeap + 1, nCheckDeap,
                        itVec.strRemarks/*,strNowRemarks*/);
                }
                pWnd->m_strId = itVec.strId;
                pWnd->m_strTitle = itVec.strTitle;
                m_vecChildWnd.push_back(pWnd);
            }
        }
        if (m_bAddChild)
        {
            for (auto& itVecChild : m_vecChildWnd)
            {
                m_pLayerWnd->AddWnd(itVecChild->m_pLayerWnd->m_pGroupBox);
            }
        }
        if (!vecCombobox.empty())
        {
            auto pWnd = CreateChildTable(parent, data, vecCombobox, vecId, vecRemarks, vecOther, vecShow, callBack, true, nDefulat);
            m_pLayerWnd->ShowWithTable(tableParentType == eStructuralDataType_Combobox);
            m_pLayerWnd->AddWnd(pWnd->m_pLayerTableWidgt/*,1*/);
            m_vecChildWnd.push_back(pWnd);
        }
    }
}

void WndSingle::CreateChildTable(QWidget* parent, StructuralData& data, const std::vector<QString>& vecId,
    const std::vector<QString> vecRemarks, WndSingle* pParentWnd, UpdateDataCallBack callBack
/* ,  const QString &strRemarks*/)
=======
void WndSingle::CreateChildTable(QWidget *parent, StructuralData &data, const std::vector<QString> & vecId,
                                 const std::vector<QString> vecRemarks, WndSingle *pParentWnd, UpdateDataCallBack callBack
                                 /* ,  const QString &strRemarks*/)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
{
    struct stMathNode
    {
        QString strTitle;
        QString strRemarks;
        std::vector<stStructuralNode> vecNode;
    };

    std::map<QString, std::map<stTableKey, stMathNode>> mapNode;
    auto itRemarks = vecRemarks.begin();
    for (auto& itId : vecId)
    {
        auto mapChildData = data.FindChildTable(itId);
        auto itMapNode = mapNode.find(itId);
        if (itMapNode == mapNode.end())
        {
            mapNode[itId] = std::map<stTableKey, stMathNode>();
            itMapNode = mapNode.find(itId);
        }

        for (auto& itMap : mapChildData)
        {
            auto key = itMap.first;
            auto itFind = itMapNode->second.find(key);
            if (itFind == itMapNode->second.end())
            {
                itMapNode->second[key] = stMathNode();
                itFind = itMapNode->second.find(key);
            }
            itFind->second.strTitle = key.strKey;
            itFind->second.strRemarks = *itRemarks;
            for (auto& itMapData : itMap.second)
            {
                if (!itMapData.second.bHasElementAttribute)
                {
                    itMapData.second.eElementAttribute = eStructuralDataType_UnNow;
                }
                itFind->second.vecNode.push_back(itMapData.second);
            }
        }
        ++itRemarks;
    }
    int nPos = 1;
    for (auto& itMapNode : mapNode)
    {
        QString strKey = itMapNode.first;
        auto itMapTable = m_childTable.find(strKey);
        if (itMapTable == m_childTable.end())
        {
            m_childTable[strKey] = std::map<stTableKey, WndSingle*>();
            itMapTable = m_childTable.find(strKey);
        }
        for (auto& itNode : itMapNode.second)
        {
            auto strRemarks = itNode.second.strRemarks;
            auto key = itNode.first;
            auto itFind = itMapTable->second.find(key);
            if (itFind == itMapTable->second.end())
            {
                QString strTitle = itNode.second.strTitle;
<<<<<<< HEAD
                itMapTable->second[key] = new WndSingle(&data, callBack, parent);
=======
                itMapTable->second[key] =  new WndSingle(&data,callBack,  parent);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                itMapTable->second[key]->m_pLayerWidget->m_bIsTable = true;
                // auto type = itNode.second.eEle;
                CreateControlByNodes(parent, strTitle, pParentWnd->m_pLayerWnd, itNode.second.vecNode, nPos, itMapTable->second[key], strRemarks);

                ++nPos;

            }
        }

    }

}

void WndSingle::CrateByStructuralData(QWidget* parent, StructuralData& data, eTemplateType type)
{
    m_pData = &data;

    InitLayout(parent);
<<<<<<< HEAD
    QString str = TransType2Str(type);
=======
    QString str  = TransType2Str(type);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    auto vecNode = data.FindChildByCode(str, true);
    if (vecNode.empty())
    {
        return;
    }
    m_bAddChild = false;
    if (type == eTemplateType_SRES)
    {
        m_bChildIsConnectCombobox = true;
        std::vector<QString> vecId;
        std::vector<QString> vecName;
        std::vector<QString> vecRemarks;
        std::vector<QString>  vecOther;
        std::vector<QString>  vecShow;
        int nDefulat = -1;
        int nCul = 0;
        for (auto& itVec : vecNode)
        {
            vecId.push_back(itVec.strID);
            vecName.push_back(itVec.strTitle);
            vecRemarks.push_back("");
            vecOther.push_back(itVec.strOther);
            vecShow.push_back(itVec.strValue3);
<<<<<<< HEAD
            if (nDefulat == -1 && itVec.strDefaultValue == "true")
=======
            if(nDefulat == -1 && itVec.strDefaultValue == "true")
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                nDefulat = nCul;
            }
            if (itVec.bIsComboboxTop)
            {
                nDefulat = nCul;
            }
            m_strId = itVec.strParentId;
            ++nCul;
        }
        if (nDefulat == -1)
        {
            nDefulat = 0;
        }
<<<<<<< HEAD
        CreateCombobox(parent, "", vecName, vecId, vecRemarks, vecOther, vecShow, false, nDefulat, true);
=======
        CreateCombobox(parent, "",vecName, vecId, vecRemarks,vecOther, vecShow, nDefulat, true);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c

        m_BoxListCollection.SetListVisible(true);
        CreateChildNode(parent, vecNode, data, this, m_UpdateDataCallBack, QString("观察范围"), eStructuralDataType_Combobox,
            1, 2, "");
        // m_strId = CreateMulti(parent, QString("观察范围"), vecNode, true, false);
        //    CrateChildNode(parent,vecNode, data,  this, m_UpdateDataCallBack, "",eStructuralDataType_Combobox);
    }
    else if (type == eTemplateType_SRRa || type == eTemplateType_SRUS)
    {
        m_bChildIsConnectCombobox = true;
        std::vector<QString> vecId;
        std::vector<QString> vecName;
        std::vector<QString> vecRemarks;
        std::vector<QString> vecOther;
        std::vector<QString> vecShow;
        int nDefulat = -1;
        int nCul = 0;
        for (auto& itVec : vecNode)
        {
            vecId.push_back(itVec.strID);
            vecName.push_back(itVec.strTitle);
            vecRemarks.push_back("");
            vecOther.push_back(itVec.strOther);
            vecShow.push_back(itVec.strValue3);
<<<<<<< HEAD
            if (nDefulat == -1 && itVec.strDefaultValue == "true")
=======
            if(nDefulat == -1 && itVec.strDefaultValue == "true")
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                nDefulat = nCul;
            }
            if (itVec.bIsComboboxTop)
            {
                nDefulat = nCul;
            }
            m_strId = itVec.strParentId;
            ++nCul;
        }
        if (nDefulat == -1)
        {
            nDefulat = 0;
        }
        MapNameAndIdList mapNextName;
        std::vector<stStructuralNode > childNode;
        for (auto& itVecId : vecId)
        {
            auto vecChildNode = data.FindChildById(itVecId, true);
            mapNextName[itVecId] = std::vector<stNameAndId>();
            auto itMapNow = mapNextName.find(itVecId);
            bool bFirst = false;
            int nCul = 0;
            int nTop = 0;
            for (auto& itVec : vecChildNode)
            {
                stNameAndId tmp;
                tmp.strId = itVec.strID;
                tmp.strName = itVec.strTitle;
                tmp.strOther = itVec.strOther;
                tmp.strShow = itVec.strValue4;
<<<<<<< HEAD
                if (!bFirst && itVec.strDefaultValue == "true")
=======
                if(!bFirst && itVec.strDefaultValue == "true")
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                {
                    tmp.bSelect = true;

                }
                if (itVec.bIsComboboxTop)
                {
                    nTop = nCul;
                }
                ++nCul;
                itMapNow->second.push_back(tmp);
            }
            nCul = 0;
            for (auto& itId : itMapNow->second)
            {
                if (nCul == nTop)
                {
                    itId.bSelect = true;
                }
                else
                {
                    itId.bSelect = false;
                }
                ++nCul;
            }
            childNode.insert(childNode.end(), vecChildNode.begin(), vecChildNode.end());
        }
<<<<<<< HEAD
        CreateCombobox(parent, "", vecName, vecId, vecRemarks, vecOther, vecShow, false, nDefulat, false, &mapNextName);

        m_BoxListCollection.SetListVisible(true);
        auto s = clock();
        CreateChildNode(parent, childNode, data, this, m_UpdateDataCallBack, QString("观察范围"), eStructuralDataType_Multi,
            2, 3, "");
        auto e = clock();
        std::cout << "CreateChildNode:" << e - s << std::endl;
=======
        CreateCombobox(parent, "",vecName, vecId, vecRemarks,vecOther, vecShow, nDefulat, false, &mapNextName);

        m_BoxListCollection.SetListVisible(true);
        auto s = clock();
        CreateChildNode(parent,childNode, data,  this, m_UpdateDataCallBack, QString("观察范围"),eStructuralDataType_Multi,
                        2,3,"");
        auto e = clock();
        std::cout << "CreateChildNode:" <<  e - s << std::endl;
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c

    }
    SetAllChildRemarks();
}

//void WndSingle::SetCallBck(UpdateDataCallBack updateCall)
//{
//    m_UpdateDataCallBack = updateCall;
//    for(auto &itVec : m_vecChildWnd)
//    {
//        itVec->SetCallBck(updateCall);
//    }
//    for(auto &itMap : m_childTable)
//    {

//        for(auto &itMapData : itMap.second)
//        {
//            itMapData.second->SetCallBck(updateCall);
//        }

//    }
//}

<<<<<<< HEAD
void WndSingle::UpdateByReportData(listReportData* pList)
{
    if (!pList->empty())
=======
void WndSingle::UpdateByReportData(listReportData *pList)
{
    if(!pList->empty())
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        CloseAllMulti(pList->begin()->strRemarks);
    }
    listReportData list = *pList;
    m_bIsUpadteReport = true;
    ClearAllSelect(pList->begin()->strRemarks);
<<<<<<< HEAD
    for (auto& itList : list)
=======
    for(auto &itList : list)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        QString strId = itList.strId;
        QString strData = itList.drawData.strDrawData;
        if (!itList.drawData.bEdit)
        {
            SetCheckStateById(strId, stTableState("true", strId, true, false), false, true, false);
        }
        else
        {
            SetCheckStateById(strId, stTableState(strData, strId, true, true), false, false, false);
        }

    }
    m_bIsUpadteReport = false;
}
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
bool WndSingle::SetCheckStateById(const QString& strId, const stTableState& strData, bool bCheckSelf, bool bDealMulti, bool bUnknowView)
{
    if (bCheckSelf)
    {
        if (m_BoxListCollection.SetStateById(strId, strData, bDealMulti, bUnknowView))
        {
            return true;
        }
    }
    if (m_childTable.empty())
    {
        for (auto& itChild : m_vecChildWnd)
        {
            if (itChild->SetCheckStateById(strId, strData, bCheckSelf, bDealMulti, bUnknowView))
            {
                if (!itChild->m_strId.isEmpty() && !bUnknowView)
                {
                    SetCheckStateById(itChild->m_strId, stTableState("true", itChild->m_strId, strData.bViewer, false), true, true, bUnknowView);
                }
                return true;
            }
        }
        return false;
    }

    for (auto& itChild : m_childTable)
    {
        for (auto& itData : itChild.second)
        {
            if (itData.second->m_BoxListCollection.SetStateById(strId, strData, true, bUnknowView))
            {
                if (!bUnknowView)
                {
                    SetCheckStateById(itChild.first, stTableState("true", itChild.first, strData.bViewer, false), true, true, bUnknowView);
                }
                return true;
            }
        }
    }
    return false;
}
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
void WndSingle::ClearAll()
{
    for (auto& itVec : m_vecChildWnd)
    {
        itVec->ClearAll();
        delete itVec;
        itVec = nullptr;
    }
    m_vecChildWnd.clear();
    for (auto& itMap : m_childTable)
    {

        for (auto& itMapData : itMap.second)
        {
            itMapData.second->ClearAll();
            delete  itMapData.second;
            itMapData.second = nullptr;
        }

    }
    m_childTable.clear();
    m_BoxListCollection.Clear();
    m_bChildIsConnectCombobox = false;

    m_bAddChild = true;
    if (m_pLayerWnd != nullptr)
    {
        delete m_pLayerWnd;
        m_pLayerWnd = nullptr;
    }
}
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
bool WndSingle::GetReportKeyValueState(std::vector<stTableState>* pVecState, bool bOnlyTrue)
{
    bool bRet = m_BoxListCollection.GetReportKeyValueState(pVecState, bOnlyTrue);
    if (bRet)
    {
        for (auto& itVec : m_vecChildWnd)
        {
            itVec->GetReportKeyValueState(pVecState, bOnlyTrue);
        }
        for (auto& itMap : m_childTable)
        {

            for (auto& itMapData : itMap.second)
            {
                itMapData.second->GetReportKeyValueState(pVecState, bOnlyTrue);
            }

        }
    }
    return bRet;
}
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
bool WndSingle::GetReportKeyValueState(std::vector<stTableState>* pTableState, std::vector<stTableState>* pDictionaryState,
    std::vector<stTableState>* pFirstAndSecondState, bool bOnlyTrue)
{
    bool bRet = m_BoxListCollection.GetReportKeyValueState(pFirstAndSecondState, bOnlyTrue);
    if (bRet)
    {
        for (auto& itVec : m_vecChildWnd)
        {
<<<<<<< HEAD
            itVec->GetReportKeyValueState(pTableState, pDictionaryState, pDictionaryState, bOnlyTrue);
=======
            itVec->GetReportKeyValueState(pTableState,pDictionaryState,pDictionaryState, bOnlyTrue);

>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
        }
        for (auto& itMap : m_childTable)
        {
<<<<<<< HEAD

            for (auto& itMapData : itMap.second)
=======
            for(auto &itMapData : itMap.second)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                itMapData.second->GetReportKeyValueState(pTableState, bOnlyTrue);
            }

        }

    }
    return bRet;

}
<<<<<<< HEAD
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
void WndSingle::CloseAllMulti(const QString& strRemarks)
{

    m_bCanCallBack = false;
    if (strRemarks == m_BoxListCollection.strRemarks)
    {
        m_BoxListCollection.CloseAllMulti();
=======

void WndSingle::CloseAllMulti(const QString &strRemarks)
{

    m_bCanCallBack = false;
    if(strRemarks == m_BoxListCollection.strRemarks)
    {
        m_BoxListCollection.CloseAllMulti();
    }
    m_bCanCallBack = true;
    for(auto &itVec : m_vecChildWnd)
    {
        itVec->CloseAllMulti(strRemarks);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
    m_bCanCallBack = true;
    for (auto& itVec : m_vecChildWnd)
    {
        itVec->CloseAllMulti(strRemarks);
    }
    for (auto& itMap : m_childTable)
    {
        for (auto& itMapData : itMap.second)
        {
            itMapData.second->CloseAllMulti(strRemarks);
        }
    }
}

QString WndSingle::GetReportKeyValueJson()
{
    std::vector<stTableState> vecState;
    GetReportKeyValueState(&vecState);
    if (vecState.empty())
    {
        return "";
    }
    return StructuralData::TransMapStatetoString(vecState);


}

void fucFindChild(stStructuralNode& node, StructuralData& data)
{
    if (node.strID.isEmpty())
    {
        return;
    }
    std::vector<stStructuralNode > vecNode = data.FindChildById(node.strID, false);
    for (auto& itVec : vecNode)
    {
        fucFindChild(itVec, data);
    }
    for (auto& itVec : vecNode)
    {
        node.mapChild[itVec.strID] = itVec;
    }
    // for(auto &itMap)

};

<<<<<<< HEAD
void WndSingle::GetSaveReportPar(stSaveReportPar& saveReportPar, StructuralData data, eTemplateType type, std::map<QString, stCallBack>& mapReport, bool bCheckStruct)
=======
void WndSingle::GetSaveReportPar(stSaveReportPar &saveReportPar, StructuralData data,eTemplateType type, std::map<QString, stCallBack> &mapReport, bool bCheckStruct)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
{
    std::vector<stTableState> vecState;
    std::vector<stTableState> vecDictionaryState;
    std::vector<stTableState> vecFirstAndSecondState;

    std::vector<QString> vecJson;
    for (auto& itMap : mapReport)
    {
        if (itMap.second.getJsonCallBack != nullptr)
        {
            vecJson.push_back(itMap.second.getJsonCallBack());
        }
    }

    saveReportPar.strListJson = data.MergeStructuralDataReportJson(vecJson);


    bool bIsStruct = !bCheckStruct;

    GetReportKeyValueState(&vecState, &vecDictionaryState, &vecFirstAndSecondState, true);
    //first and second
    auto itBegin = vecFirstAndSecondState.begin();
    if (itBegin != vecFirstAndSecondState.end())
    {
        saveReportPar.strFirstId = itBegin->strId;
        ++itBegin;
    }
<<<<<<< HEAD
    //    else
    //    {
    //        bIsStruct = false;
    //        return;
    //    }
    if (itBegin != vecFirstAndSecondState.end())
=======
    if(itBegin != vecFirstAndSecondState.end())
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        saveReportPar.strSecondId = itBegin->strId;
    }
    for (auto& itVec : vecDictionaryState)
    {
        saveReportPar.listDictionaryId.push_back(itVec.strId);
    }
    for (auto& itVec : vecState)
    {
        saveReportPar.vecTemplateData.push_back({ itVec.strId, itVec.strState });
    }

    vecState.clear();



    GetReportKeyValueState(&vecState, false);
    if (!vecState.empty())
    {
        data.ChangeState(vecState);
    }

<<<<<<< HEAD
    if (bCheckStruct)
    {

        std::vector<listReportData> vecList;
        for (auto& itVec : vecJson)
        {
            vecList.push_back(data.TransStringtoReport(itVec, "", true));
        }
        for (auto& itList : vecList)
        {
            for (auto& itNode : itList)
            {
                if (itNode.drawData.strDrawData.isEmpty())
                {
                    continue;
                }
                auto itFind = std::find(vecState.begin(), vecState.end(), itNode.strId);
                if (itFind != vecState.end())
=======
    if(bCheckStruct)
    {

        std::vector<listReportData> vecList;
        for(auto &itVec : vecJson)
        {
            vecList.push_back(data.TransStringtoReport(itVec, "", true));
        }
        for(auto &itList : vecList)
        {
            for(auto &itNode : itList)
            {
                if(itNode.drawData.strDrawData.isEmpty())
                {
                    continue;
                }
                auto itFind =  std::find(vecState.begin(), vecState.end(), itNode.strId );
                if(itFind !=vecState.end()  )
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                {
                    bIsStruct = true;
                    break;
                }
            }
<<<<<<< HEAD
            if (bIsStruct)
=======
            if(bIsStruct)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
            {
                break;
            }
        }
    }

<<<<<<< HEAD
    if (bIsStruct)
=======
   // bIsStruct = true;
    if(bIsStruct)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        vecState.clear();
        GetReportKeyValueState(&vecState, true);
        stStructuralData dataNow;
        dataNow.InitOK();
        std::map<eTemplateType, stTopPar> topPar = data.GetTopPar();
        auto itFind = topPar.find(type);
        if (itFind == topPar.end())
        {
            return;
        }
        //找到最上级节点
        QString strFindId = saveReportPar.strFirstId;

        // saveReportPar.strFirstId = itBegin->strId;
        stStructuralNode* pNode = data.FindNodePtrById(itFind->second.strId);
        if (pNode == nullptr)
        {
            return;
        }
        dataNow.mapContext[itFind->second.strId] = *pNode;
        auto itNode = dataNow.mapContext.find(itFind->second.strId);

        pNode = data.FindNodePtrById(strFindId);
        if (pNode == nullptr)
        {
            return;
        }
        itNode->second.mapChild[strFindId] = *pNode;
        if (!saveReportPar.strSecondId.isEmpty())
        {
            pNode = data.FindNodePtrById(saveReportPar.strSecondId);
            if (pNode == nullptr)
            {
                // strFindId = saveReportPar.strFirstId;
            }
            else
            {
                itNode = itNode->second.mapChild.find(strFindId);
                strFindId = saveReportPar.strSecondId;
                itNode->second.mapChild[strFindId] = *pNode;
            }
        }
        fucFindChild(itNode->second.mapChild[strFindId]/*dataNow.mapContext[strFindId]*/, data);
        saveReportPar.strTreeJson = data.Trans2WholeJson(dataNow);
    }
    else
    {
        saveReportPar.strFirstId.clear();
        saveReportPar.strSecondId.clear();
    }

}

void WndSingle::SetReportStateByJson(const QString& strJson)
{
    auto mapState = StructuralData::TransStringStatetoMap(strJson);
<<<<<<< HEAD

    if (!mapState.empty())
=======
    if(!mapState.empty())
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        SetReportStateByMap(mapState);
    }
}

void WndSingle::SetReportStateByMap(const std::vector<stTableState>& vecState)
{
    for (auto& itMap : vecState)
    {
        SetCheckStateById(itMap.strId, itMap, true, !itMap.bIsEdit, true);
    }
}

//bool WndSingle::SetReportStateById(const QString &strId, const QString &strState)
//{
//    //m_BoxListCollection.SetCheckStateById()
//    return true;
//}

void WndSingle::AllBoxWork()
{
    m_BoxListCollection.AllBoxWork();
    for (auto& itVec : m_vecChildWnd)
    {
        itVec->AllBoxWork();
    }
    for (auto& itMap : m_childTable)
    {
<<<<<<< HEAD
        for (auto& itMapData : itMap.second)
=======
        for(auto &itMapData : itMap.second)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
        {
            itMapData.second->AllBoxWork();
        }
    }
}

<<<<<<< HEAD
void WndSingle::SetDepartment(const QString& strDepartment, const QString& strBodyPart)
=======
void WndSingle::SetDepartment(const QString &strDepartment, const QString &strBodyPart)
{
    m_BoxListCollection.SetDepartment(strDepartment, strBodyPart);
}

void WndSingle::ClearAllSelect(const QString &strRemarks)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
{
    m_BoxListCollection.SetDepartment(strDepartment, strBodyPart);
}

void WndSingle::ClearAllSelect(const QString& strRemarks)
{
    for (auto& itVec : m_vecChildWnd)
    {
        itVec->ClearAllSelect(strRemarks);
    }
    for (auto& itMap : m_childTable)
    {

        for (auto& itMapData : itMap.second)
        {
            itMapData.second->ClearAllSelect(strRemarks);
        }
    }
<<<<<<< HEAD
    if (strRemarks == m_BoxListCollection.strRemarks)
=======
    if(strRemarks == m_BoxListCollection.strRemarks)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        m_BoxListCollection.ClearAllSelect();
    }
}

//void WndSingle::Add2MeasureParameterWidget(QWidget *pWidget, const QString &strTip)
//{
//    if(strTip.isEmpty() || pWidget == nullptr )
//    {
//        return;
//    }
//    auto pData = MyMeasureParameterManage::GetSingle();
//    if(pData != nullptr)
//    {
//        pData->AddWidget(strTip, pWidget );
//    }
//}

void WndSingle::SetAllChildRemarks()
{
    m_BoxListCollection.SetAllChildRemarks();
    for (auto& itVec : m_vecChildWnd)
    {
        if (!m_BoxListCollection.strRemarks.isEmpty() && itVec->m_BoxListCollection.strRemarks.isEmpty())
        {
            itVec->m_BoxListCollection.strRemarks = m_BoxListCollection.strRemarks;
        }
        itVec->SetAllChildRemarks();
    }
    for (auto& itMap : m_childTable)
    {
        for (auto& itMapData : itMap.second)
        {
            if (!m_BoxListCollection.strRemarks.isEmpty() && itMapData.second->m_BoxListCollection.strRemarks.isEmpty())
            {
                itMapData.second->m_BoxListCollection.strRemarks = m_BoxListCollection.strRemarks;
            }
            itMapData.second->SetAllChildRemarks();
        }
    }


}
//创建子节点
void WndSingle::CreateTableChild(QWidget* parent, const QString& strTitle, MyLayerWnd* pLayerWnd, const stStructuralNode& itVec, int pos,
    stBoxListCollection* pBoxPtr, int nCul, const QString& strRemarks)
{
    QString strId = itVec.strID;
    if (pBoxPtr->childBox.empty())
    {
        nCul = 0;
    }
    else
    {
        auto itEnd = pBoxPtr->childBox.end();
        --itEnd;
        nCul = itEnd->first.nSort + 1;
    }
    stTableKey key(strId, nCul);
    pBoxPtr->childBox[key] = std::vector<stBoxListCollection>();
    auto& childBox = pBoxPtr->childBox[key];
    std::map<stTableKey, std::vector<stStructuralNode>> mapChildNode;
    eStructuralDataType childType;
    for (auto& itChild : itVec.mapChild)
    {
        QString strTitle = itChild.second.strTitle;
        int nLeft = strTitle.indexOf("[");
        int nRight = strTitle.indexOf("]");
        // int nLeft = strTitle.indexOf("【");
        //int nRight = strTitle.indexOf("】");
        QString strKey("");
        if (nLeft >= 0 && nRight > nLeft)
        {
            strKey = strTitle.mid(nLeft + 1, nRight - nLeft - 1);
        }
        stTableKey childKey(strKey, itChild.second.nSort);
        childKey.keyEx.strLabelEx = itChild.second.strTitle;
        auto itFind = mapChildNode.find(childKey);
        if (itFind == mapChildNode.end())
        {
            mapChildNode[childKey] = std::vector<stStructuralNode>();
            itFind = mapChildNode.find(childKey);
        }
        itFind->second.push_back(itChild.second);
    }
    for (auto& itMapChild : mapChildNode)
    {
        std::sort(itMapChild.second.begin(), itMapChild.second.end(), [](stStructuralNode& node1, stStructuralNode& node2) {
            return node1.nChildSort < node2.nChildSort;
            });
        childBox.push_back(stBoxListCollection());
        stBoxListCollection* pChild = &childBox[(int)childBox.size() - 1];
        pChild->AddChildLabel(itMapChild.first.keyEx.strLabelEx);
        pChild->strChildDrawTitle = itMapChild.first.strKey;
<<<<<<< HEAD
        CreateControlByNodes(parent, strTitle, pLayerWnd, itMapChild.second, pos, this, strRemarks, &childBox[(int)childBox.size() - 1]);

=======
        CreateControlByNodes(parent,strTitle,pLayerWnd, itMapChild.second,pos,this,strRemarks, &childBox[(int)childBox.size() - 1]);
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
    for (auto& itMapChild : pBoxPtr->childBox[key])
    {
        itMapChild.pSingleWidget->setVisible(false);
<<<<<<< HEAD
        //        if(pBoxPtr->boxList.list.pLayout != nullptr)
        //        {
        //            pBoxPtr->boxList.list.pLayout->addWidget(itMapChild.pSingleWidget);
        //        }
              //  pBoxPtr->pSingleLayout->addWidget(itMapChildpSingleWidget);
=======
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
}

void WndSingle::CreateControlByNodes(QWidget* parent, const QString& strTitle, MyLayerWnd* pLayerWnd, const std::vector<stStructuralNode>& vecNode, int nPos, WndSingle* pWnd
    , const QString& strRemarks
    , stBoxListCollection* pBox)
{
    //qDebug() <<"CreateControlByNodes:" << strTitle;
    std::map<eStructuralDataType, std::vector<stStructuralNode>> mapNode;
    mapNode[eStructuralDataType_UnNow] = std::vector<stStructuralNode>();
    mapNode[eStructuralDataType_Single] = std::vector<stStructuralNode>();
    mapNode[eStructuralDataType_Multi] = std::vector<stStructuralNode>();
    mapNode[eStructuralDataType_Edit] = std::vector<stStructuralNode>();
    std::vector<eStructuralDataType> vecType;
    for (auto& itVecNode : vecNode)
    {
        auto itFindNode = mapNode.find(itVecNode.eElementAttribute);
        if (itFindNode != mapNode.end())
        {
            itFindNode->second.push_back(itVecNode);
            if (std::find(vecType.begin(), vecType.end(), itVecNode.eElementAttribute) == vecType.end())
            {
                vecType.push_back(itVecNode.eElementAttribute);
            }
        }
    }
    for (auto& itVec : vecType)
    {
        auto itMapNode = mapNode.find(itVec);
        if (itMapNode != mapNode.end() && !itMapNode->second.empty())
        {
            switch (itMapNode->first)
            {
            case eStructuralDataType_UnNow: pWnd->CreateEmpty(parent, strTitle, pLayerWnd, nPos); break;
            case eStructuralDataType_Single:pWnd->CreateSingle(parent, false, strTitle, itMapNode->second, true
                , strRemarks
                , pLayerWnd, pBox, false, eDataPostion_Left, nPos); break;
            case eStructuralDataType_Multi:pWnd->CreateMulti(parent, false, strTitle, itMapNode->second, true, true, strRemarks,
                pLayerWnd, pBox, false, eDataPostion_Left, nPos); break;
            case eStructuralDataType_Edit:pWnd->CreateText(parent, false, strTitle, itMapNode->second, strRemarks,
                pLayerWnd, pBox, false, eDataPostion_Left, nPos); break;
            case eStructuralDataType_Combobox:break;
            default:
                break;
            }
        }
    }
    //    for(auto &itMapNode : mapNode)
    //    {
    //        if(itMapNode.second.empty())
    //        {
    //            continue;
    //        }

    //    }
}


<<<<<<< HEAD
//处理控件状态变化时的行为，如根据复选框或单选按钮的选择显示或隐藏相关控件，或者是响应下拉框的选项更改。
void WndSingle::StateChange(bool bCheck, const QString& strId, const QString& strTitle, bool bWait, void* pPar)
=======

void WndSingle::StateChange(bool bCheck,const QString &strId, const QString &strTitle, bool bWait, void *pPar)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
{
    if (strId.isEmpty())
    {
        return;
    }
    for (auto& itVecChild : m_vecChildWnd)
    {
        if (itVecChild->m_strId == strId)
        {
            itVecChild->m_pLayerWnd->ShowWnd(bCheck);
            itVecChild->m_BoxListCollection.SetListVisible(bCheck);
            if (itVecChild->m_bIsCreateBySelf)
            {
                auto itChild = itVecChild->m_vecChildWnd.begin();
                if (itChild != itVecChild->m_vecChildWnd.end())
                {
<<<<<<< HEAD
                    if (bCheck)
=======
                    if(bCheck)
                    {
                        (*itChild)->LoadDictionaryData(strId,bCheck, bWait);
                    }
                    for(auto &itTable : (*itChild)->m_childTable)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                    {
                        //加载列表
                        (*itChild)->LoadDictionaryData(strId, bCheck, bWait);
                    }
                    for (auto& itTable : (*itChild)->m_childTable)
                    {
                        for (auto& itData : itTable.second)
                        {
<<<<<<< HEAD
                            if (bCheck)
                            {

=======
                            if(bCheck)
                            {
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                                itData.second->m_BoxListCollection.InitChild();
                            }
                            itData.second->m_BoxListCollection.SetListVisible(bCheck);
                        }
                    }
                    (*itChild)->m_BoxListCollection.AllBoxWork();
                }
            }
            break;
        }
    }
<<<<<<< HEAD
    if (MyMeasureParameterManage::GetSingle() != nullptr)
=======
    if( MyMeasureParameterManage::GetSingle()  != nullptr)
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        MyMeasureParameterManage::GetSingle()->ChangeWidgetValue(strId);
    }
    m_BoxListCollection.ShowChild(strId, bCheck);

    if (m_bCanCallBack && !m_bIsUpadteReport)
    {
        if (m_UpdateDataCallBack != nullptr)
        {
            m_UpdateDataCallBack(nullptr, strId);
        }
    }
}
static QString q_secondId;
static QString q_firstId;
static QString g_templateTexe = "";
static QString g_LastTemplateTexe = "";
//处理控件状态变化时的行为，如根据复选框或单选按钮的选择显示或隐藏相关控件，或者是响应下拉框的选项更改。
void WndSingle::ComboboxChange(const QString& strChange, const QString& strId, const QString& parentId, const int& level, void* pPar)
{
    if (parentId != "" && level == 1)
    {
<<<<<<< HEAD
        //q_firstId = parentId;
        //m_currentFirst = q_firstId;
=======
        if(!m_BoxListCollection.comboboxList.Empty())
        {
            LoadDictionaryData(strId, true, true);
            ShowChild(strId, true, false);
        }
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    }
    if (strId != "" && level == 2)
    {
        q_secondId = strId;
        m_currentSecnd = q_secondId;
        for (auto& itVecChild : m_vecChildWnd)
        {
            if (itVecChild->m_strId == strId)
            {
                itVecChild->m_pLayerWnd->ShowWnd(true);
                itVecChild->m_BoxListCollection.SetListVisible(true);
                //更新第三级目录
                MyComboBox temp ("","","","","",nullptr,nullptr);
                q_firstId = temp.getFirstId(q_secondId);
                m_currentFirst = q_firstId;
                auto list = CTemplateManage::GetSingle()->GetTemplateList(q_firstId, q_secondId);
                WndSingle* wnd = static_cast<WndSingle*>(pPar);
                templeteList.clear();
                if (!wnd->m_BoxListCollection.comboboxList.Empty())
                {
                    auto lastPair = *wnd->m_BoxListCollection.comboboxList.list.mapControl.rbegin();
                    auto combo = lastPair.second;
                    combo->blockSignals(1);
                    combo->clear();
                    combo->addItem("--");
                    for (auto it : list)
                    {
                        templeteList.append(it.keyEx.strLabelEx);
                    }
                    combo->addItems(templeteList);
                    combo->blockSignals(0);
                    combo->setCurrentText(g_templateTexe);
                }
            }
            else
            {
                itVecChild->m_pLayerWnd->ShowWnd(false);
                itVecChild->m_BoxListCollection.SetListVisible(false);
            }
        }
    }
    if (templeteList.contains(strChange) && level == 3)
    {
        //去找保存的报告模板
        auto strJson = CTemplateManage::GetSingle()->GetTemplateDataByName(q_firstId, q_secondId, strChange);
        //std::ofstream file("C:\\code\\json1.txt");
        //file << strJson.toStdString() << std::endl;
        //file.close();
        //通知界面刷新
        g_LastTemplateTexe = g_templateTexe;
        g_templateTexe = strChange;
        m_tempVecChildWnd = m_vecChildWnd;
        if (g_LastTemplateTexe != g_templateTexe && strJson != "")
            callback(q_firstId, q_secondId, strJson);
    }
    
    //  auto itNow = m_BoxListCollection.comboboxList.Empty();
    if (!m_bChildIsConnectCombobox)
    {
        if (!m_BoxListCollection.comboboxList.Empty())
        {
            LoadDictionaryData(strId, true, true);
            ShowChild(strId, true, false);
        }
    }



    //{
    //    //     一级目录             三级目录
    //    if (strChange == "" || strChange == "--")
    //    {
    //        return;
    //    }
    //    //取消模板选择，回到原始状态
    //    if (/*strChange == "--"*/0)
    //    {
    //        //qDebug() << "  ";
    //        //q_childId = strId;
    //        //for (auto& itVecChild : m_tempVecChildWnd)
    //        //{
    //        //    if (itVecChild->m_strId == strId)
    //        //    {
    //        //        itVecChild->m_pLayerWnd->ShowWnd(true);
    //        //        itVecChild->m_BoxListCollection.SetListVisible(true);
    //        //        //更新第三级目录
    //        //        //auto list = CTemplateManage::GetSingle()->GetTemplateList(parentId, strId);
    //        //        WndSingle* wnd = static_cast<WndSingle*>(pPar);
    //        //        if (!wnd->m_BoxListCollection.comboboxList.Empty())
    //        //        {
    //        //            auto lastPair = *wnd->m_BoxListCollection.comboboxList.list.mapControl.rbegin();
    //        //            auto combo = lastPair.second;
    //        //            combo->clear();
    //        //            combo->addItem("--");
    //        //            combo->addItems(templeteList);
    //        //            combo->setCurrentText(g_templateTexe);
    //        //        }
    //        //    }
    //        //    else
    //        //    {
    //        //        itVecChild->m_pLayerWnd->ShowWnd(false);
    //        //        itVecChild->m_BoxListCollection.SetListVisible(false);
    //        //    }
    //        //}

    //        //if (m_bCanCallBack && !m_bIsUpadteReport)
    //        //{
    //        //    if (m_UpdateDataCallBack != nullptr)
    //        //    {
    //        //        m_UpdateDataCallBack(nullptr, strId);
    //        //    }
    //        //}

    //        return;

    //    }

    //    //第三级目录，模板目录
    //    if (strId == "" && templeteList.contains(strChange) && level == 3)
    //    {

    //    }
    //    else
    //    {
    //       
    //        

    //    }
    //}

    if (m_bCanCallBack && !m_bIsUpadteReport)
    {
        if (m_UpdateDataCallBack != nullptr)
        {
            m_UpdateDataCallBack(nullptr, strId);
        }
    }
}
