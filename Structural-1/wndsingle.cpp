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

WndSingle::WndSingle(StructuralData* pData, UpdateDataCallBack updateCall, QWidget* parent, QString parentId)
//    :QWidget(nullptr)
{
    if (pData != nullptr)
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
    if (parentId != "")
        m_strParentId = parentId;
}

//显示或隐藏子控件。根据提供的ID和布尔标志，决定是否显示特定的子控件。如果bMulti为false，还会隐藏所有不匹配的子控件。
void WndSingle::ShowChild(const QString& strId, bool bShow, bool bMulti)
{
    for (auto& itMap : m_childTable)
    {
        if (itMap.first == strId)
        {
            for (auto& itMapData : itMap.second)
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
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, this);
            if (itVec.bIsExplain)
            {
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent, call,
                    itVec.strID, itVec.strPrefix,/*bUseTitle ? itVec.strTitle : itVec.strLabel*///,
                    "", itVec.strSuffix,
                    itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                    pos, bInit, itVec.strTip, itVec.strDataType, itVec.strLabel, itVec.strFormula);
                //pBoxPtr->strExplain = itVec.strLabel;
            }
            else
            {
                pBoxPtr->radioList.AddSingle/*<StateChangeCallBack>*/(parent, call,
                    itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                    itVec.strPrefix, itVec.strSuffix,
                    itVec.strDefaultValue == (bUseTitle ? itVec.strTitle : itVec.strLabel),
                    pos, bInit, itVec.strTip, itVec.strDataType, "", itVec.strFormula);
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
            bool flag = false;
            if (itVec.nLoadElementAttribute == 3)
            {
                QString str = itVec.strID;
                int k = 0;
                flag = true;
            }
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, this);
            if (itVec.bIsExplain)
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, itVec.strPrefix,
                    "", itVec.strSuffix, itVec.strDefaultValue == "true",
                    pos, bInit, itVec.strTip, itVec.strDataType, itVec.strLabel, itVec.strFormula);
                // pBoxPtr->strExplain = itVec.strLabel;
                //pBoxPtr->
            }
            else
            {
                pBoxPtr->boxList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, bUseTitle ? itVec.strTitle : itVec.strLabel,
                    itVec.strPrefix, itVec.strSuffix, itVec.strDefaultValue == "true",
                    pos, bInit, itVec.strTip, itVec.strDataType, "", itVec.strFormula);
            }


            //   Add2MeasureParameterWidget(ptr, itVec.strTip);
            pBoxPtr->bChangeDir = itVec.strTextSort == "1";
            if(flag)
                pBoxPtr->mapType[itVec.strID] = eStructuralDataType_Illustrate;
            else
                pBoxPtr->mapType[itVec.strID] = eStructuralDataType_Multi;
            pBoxPtr->boxList.SetExclusive(false);
            strParent = itVec.strParentId;
            if (!itVec.mapChild.empty() && bCreateChild)
            {
                QString strRemarks;
                //创建子节点
                CreateTableChild(parent, strTitle, pLayerWnd, itVec, pos, pBoxPtr, nCul++, strRemarks);
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
{
    InitLayout(parent);
    m_BoxListCollection.dataType = eStructuralDataType_Combobox;


    //根据据节点信息创建下拉框，itNow：左侧的（CT结构化MR结构化等7项）combobox
    auto call = std::bind(&WndSingle::ComboboxChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, this);
    auto itNow = m_BoxListCollection.comboboxList.AddSingle/*<ComboboxChangeCallBack>*/(parent, call, strId, "",
        "", "", false, eDataPostion_Left, true, "", "", "", "");
    itNow->level = 1;
    for (auto& itVec : vecOther)
    {
        if (!itVec.isEmpty())
        {
            MyMeasureParameterManage::GetSingle()->AddMeasureParameters(itVec);
        }
    }
    //itNow->setStyleSheet(comboBoxQss);
    itNow->setMinimumWidth(30);
    itNow->clear();
    itNow->m_mapComboboxId.clear();
    auto itVecId = vecId.begin();
    auto itVecShow = vecShow.begin();
    for (auto& itVec : vecCombobox)
    {
        itNow->addItem(itVec, *itVecId);
        itNow->m_mapComboboxId[*itVecId] = { *itVecId, itVec, "", *itVecShow, false };
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
    itNow->setToolTip();
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


        for (auto& itMap : itNow->m_mapNextNameAndId)
        {
            for (auto& itVec : itMap.second)
            {
                vecOtherNext.push_back(itVec.strOther);
                vecShowNext.push_back(itVec.strShow);
            }
        }
        if (!vecCombobox.empty())
        {
            //SetChild设置右侧的combobox，（如CT结构化下对应的左肺、急性胰腺炎....等）
            auto itChild = CreateCombobox(parent, "child", std::vector<QString>(), std::vector<QString>(), std::vector<QString>(), vecOtherNext, vecShowNext, !hasThrid, 0, true);
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
        MyPushButton* sendBtn = new MyPushButton(BtnType::Modify);
        MyPushButton* deleteBtn = new MyPushButton(BtnType::Delete);
        m_pLayout->addWidget(sendBtn);
        m_pLayout->addWidget(deleteBtn);
        auto strList = ControlCenter::getInstance()->getBtnStyleSheet().split(";");
        QPixmap pixmap(strList[0]);
        //QPixmap fitpixmap = pixmap.scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        sendBtn->setToolTip("存为结构化词库模板快照（新增或修改）");
        sendBtn->setIcon(QIcon(pixmap));
        sendBtn->setIconSize(QSize(17, 17));
        QPixmap pixmap1(strList[1]);
        //QPixmap fitpixmap1 = pixmap1.scaled(18, 18, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        deleteBtn->setToolTip("删除结构化词库模板快照");
        deleteBtn->setIcon(QIcon(pixmap1));
        deleteBtn->setIconSize(QSize(17, 17));
     
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
            auto call = std::bind(&WndSingle::StateChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, this);
            auto itNow = pBoxPtr->lineEditList.AddSingle/*<StateChangeCallBack>*/(parent, call, itVec.strID, itVec.strDefaultValue,
                itVec.strPrefix, itVec.strSuffix, false, pos, bInit, itVec.strTip, itVec.strDataType, "", itVec.strFormula);
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
//创建子列表
WndSingle* WndSingle::CreateChildTable(QWidget* parent, StructuralData& data, const std::vector<QString>& vecCombobox, const std::vector<QString>& vecId
    , const std::vector<QString> vecRemarks,
    const std::vector<QString>& vecOther,
    const std::vector<QString>& vecShow, UpdateDataCallBack callBack, bool bIsCombobox, int nDefulat)
{
    //创建子列表
    WndSingle* pWnd = new WndSingle(&data, callBack, parent);

    if (bIsCombobox)
    {
        //如果子节点是下拉框
        pWnd->CreateCombobox(parent, "", vecCombobox, vecId, vecRemarks, vecOther, vecShow, false, nDefulat/*, true, nullptr, false*/);
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
    pWnd->CreateChildTable(parent, data, vecId, vecRemarks, pWnd, callBack);
    pWnd->m_pLayerWnd->m_pLayout->setSpacing(0);
    pWnd->m_pLayerWnd->m_pLayout->setContentsMargins(0, 0, 0, 0);
    pWnd->m_pLayerTableWidgt->setLayout(pWnd->m_pLayerWnd->m_pLayout);
    return pWnd;
}

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
            ControlCenter::getInstance()->firstLoadFinish = false;
            if (reply->error() == QNetworkReply::NoError)
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
                ControlCenter::getInstance()->firstLoadFinish = true;

            }
        });
    if (bWait && eventLoop != nullptr)
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
//创建所有子节点（递归创建（如：左肺-报告描述-前段：是，否，实性，部分实性））
void WndSingle::CreateChildNode(QWidget* parent, const std::vector<stStructuralNode>& vecNode, StructuralData& data,
    WndSingle* pParentWnd, UpdateDataCallBack callBack, const QString& addTitle, eStructuralDataType tableParentType
    , int nNowDeap, int nCheckDeap, const QString& strRemarks)
{
    int nDefulat = -1;
    int nCul = 0;

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
                itMapTable->second[key] = new WndSingle(&data, callBack, parent);
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
    QString str = TransType2Str(type);
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
            if (nDefulat == -1 && itVec.strDefaultValue == "true")
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
        CreateCombobox(parent, "", vecName, vecId, vecRemarks, vecOther, vecShow, false, nDefulat, true);

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
            if (nDefulat == -1 && itVec.strDefaultValue == "true")
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
                if (!bFirst && itVec.strDefaultValue == "true")
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
        CreateCombobox(parent, "", vecName, vecId, vecRemarks, vecOther, vecShow, false, nDefulat, false, &mapNextName);

        m_BoxListCollection.SetListVisible(true);
        auto s = clock();
        CreateChildNode(parent, childNode, data, this, m_UpdateDataCallBack, QString("观察范围"), eStructuralDataType_Multi,
            2, 3, "");
        auto e = clock();
        std::cout << "CreateChildNode:" << e - s << std::endl;

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

void WndSingle::UpdateByReportData(listReportData* pList)
{
    if (!pList->empty())
    {
        CloseAllMulti(pList->begin()->strRemarks);
    }
    listReportData list = *pList;
    m_bIsUpadteReport = true;
    ClearAllSelect(pList->begin()->strRemarks);
    for (auto& itList : list)
    {
        QString strId = itList.strId;
        QString strData = itList.drawData.strDrawData;
        if (!itList.drawData.bEdit)
        {
            //                                     选中文本，id,是否可见，是否多选
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
            itVec->GetReportKeyValueState(pTableState, pDictionaryState, pDictionaryState, bOnlyTrue);
        }
        for (auto& itMap : m_childTable)
        {

            for (auto& itMapData : itMap.second)
            {
                itMapData.second->GetReportKeyValueState(pTableState, bOnlyTrue);
            }

        }
    }
    return bRet;

}
//这些函数用于管理控件的选中状态、清除所有控件、获取当前控件状态为报告格式，以及关闭所有复选框组的扩展/显示状态。
void WndSingle::CloseAllMulti(const QString& strRemarks)
{

    m_bCanCallBack = false;
    if (strRemarks == m_BoxListCollection.strRemarks)
    {
        m_BoxListCollection.CloseAllMulti();
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

void WndSingle::GetSaveReportPar(stSaveReportPar& saveReportPar, StructuralData data, eTemplateType type, std::map<QString, stCallBack>& mapReport, bool bCheckStruct)
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
    //    else
    //    {
    //        bIsStruct = false;
    //        return;
    //    }
    if (itBegin != vecFirstAndSecondState.end())
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
                {
                    bIsStruct = true;
                    break;
                }
            }
            if (bIsStruct)
            {
                break;
            }
        }
    }

    if (bIsStruct)
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

    if (!mapState.empty())
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
        for (auto& itMapData : itMap.second)
        {
            itMapData.second->AllBoxWork();
        }
    }
}

void WndSingle::SetDepartment(const QString& strDepartment, const QString& strBodyPart)
{
    m_BoxListCollection.SetDepartment(strDepartment, strBodyPart);
}

void WndSingle::SetFinish(bool sw)
{
    m_isFinish = sw;
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
    if (strRemarks == m_BoxListCollection.strRemarks)
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
//    auto pData = MyMeasureParameterMenage::GetSingle();
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
        // vecChildNode.push_back(itChild.second);
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
        CreateControlByNodes(parent, strTitle, pLayerWnd, itMapChild.second, pos, this, strRemarks, &childBox[(int)childBox.size() - 1]);

    }
    for (auto& itMapChild : pBoxPtr->childBox[key])
    {
        itMapChild.pSingleWidget->setVisible(false);
        //        if(pBoxPtr->boxList.list.pLayout != nullptr)
        //        {
        //            pBoxPtr->boxList.list.pLayout->addWidget(itMapChild.pSingleWidget);
        //        }
              //  pBoxPtr->pSingleLayout->addWidget(itMapChildpSingleWidget);
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

static QString g_currentFirstTitle;
//处理控件状态变化时的行为，如根据复选框或单选按钮的选择显示或隐藏相关控件，或者是响应下拉框的选项更改。
void WndSingle::StateChange(bool bCheck, const QString& strId, const QString& strTitle, bool bWait, bool* isNewChild, void* pPar)
{
    if (strId.isEmpty())
    {
        return;
    }
    if (ControlCenter::getInstance()->firstLoadFinish)
    {
        ControlCenter::getInstance()->signStructuralWidgetBeUsed();
    }
    //if (m_vecChildWnd.size() > 0)
    //{
    //    for (auto it : m_vecChildWnd)
    //    {
    //        if (it->m_strTitle == strTitle)
    //            g_currentFirstTitle = strTitle;
    //    }
    //}
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
                    if (bCheck)
                    {
                        //加载列表
                        (*itChild)->LoadDictionaryData(strId, bCheck, bWait);
                    }
                    for (auto& itTable : (*itChild)->m_childTable)
                    {
                        for (auto& itData : itTable.second)
                        {
                            if (bCheck)
                            {

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
    if (MyMeasureParameterManage::GetSingle() != nullptr)
    {
        MyMeasureParameterManage::GetSingle()->ChangeWidgetValue(strId);
    }
    m_BoxListCollection.ShowChild(strId, bCheck);

    //if (isNewChild != nullptr && *isNewChild /*&& m_vecChildWnd.size() == 0*/)
    //{
    //    *isNewChild = false;
    //    return;
    //}

    //是文本框时，判断公式
    if (m_BoxListCollection.dataType == 4)
    {
        for (auto& itMap : m_BoxListCollection.lineEditList.list.mapControl)
        {
            //找到被修改的那个文本框
            if (itMap.first.strKey == strId)
            {
                for (auto& it : m_BoxListCollection.lineEditList.list.mapControl)
                {
                    //找到有公式的那个文本框
                    if (it.second->m_strFormula != "")
                    {
                        //处理所有map
                        std::map<std::string, std::string> tempMap;
                        for (auto& itt : m_BoxListCollection.lineEditList.list.mapControl)
                        {
                            if (itt.first.strKey != it.first.strKey)
                            {
                                QString str = itt.second->text();
                                tempMap.insert(std::pair<std::string, std::string>(itt.first.strKey.toStdString(), str.toStdString()));
                            }

                        }
                        Parser p(it.second->m_strFormula.toStdString(), tempMap);
                        double r = p.getResult();
                        int rr = (r * 100.0) / 100.0;
                        //double r = GetRegexResult(tempMap, it.second->m_strFormula.toStdString());
                        //std::cout << "result :" << " = " << r << std::endl;
                        it.second->setText(QString::number(rr));
                        int k;
                        break;
                    }
                }
                break;
            }
        }
    }

    //是单选框时，判断公式
    if (m_BoxListCollection.dataType == 1)
    {
        for (auto& itMap : m_BoxListCollection.radioList.list.mapControl)
        {
            if (itMap.second->m_strFormula == "()")
            {
                auto childMap = m_BoxListCollection.childBox;
                for (auto childIt : childMap)
                {
                    if (childIt.first.strKey == itMap.first.strKey)
                    {
                        auto Lists = childIt.second;
                        for (auto it : Lists)
                        {
                            auto radioLists = it.radioList.list.mapControl;
                            auto editLists = it.lineEditList.list.mapControl;
                            //先从radioLists中取值
                            std::map<std::string, std::string> tempMap;
                            for (auto radioIt : radioLists)
                            {
                                if (radioIt.second->isChecked())
                                {
                                    QString value = radioIt.second->m_strTitle;
                                    tempMap.insert(std::pair<std::string, std::string>(childIt.first.strKey.toStdString(), value.toStdString()));
                                    break;
                                }
                            }
                            //再根据公式把值输入到编辑框中
                            //qDebug() << "values = " << value;
                            for (auto editIt : editLists)
                            {
                                if (editIt.second->m_strFormula != "")
                                {
                                    Parser p(editIt.second->m_strFormula.toStdString(), tempMap);
                                    double r = p.getResult();
                                    int rr = (r * 100.0) / 100.0;
                                    //double r = GetRegexResult(tempMap, it.second->m_strFormula.toStdString());
                                    //std::cout << "result :" << " = " << r << std::endl;
                                    editIt.second->setText(QString::number(rr));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //是说明框时，不显示
    if (m_BoxListCollection.dataType == 2)
    {
        auto childBox = m_BoxListCollection.childBox;
        QString targetID = "";
        for (auto it : childBox)
        {
            auto childVec = it.second;
            for (auto itt : childVec)
            {
                auto childMapType = itt.mapType;
                for (auto ittt : childMapType)
                {
                    if (ittt.second == 16)
                    {
                        targetID = ittt.first;
                    }
                }
                if (targetID != "")
                {
                    auto map = itt.boxList.list.mapControl;
                    for (auto iitt : map)
                    {
                        iitt.second->setDisabled(1);
                        iitt.second->setCheckable(0);
                        iitt.second->setDisabled(1);
                        iitt.second->setStyleSheet("QRadioButton::indicator { width: 0; }");
                    }
                }
            }
        }
    }
    if (m_bCanCallBack && !m_bIsUpadteReport)
    {
        if (m_UpdateDataCallBack != nullptr)
        {
            /*QString str = strTitle;*/
            m_UpdateDataCallBack(nullptr, strId);
        }
    }
}

//处理控件状态变化时的行为，如根据复选框或单选按钮的选择显示或隐藏相关控件，或者是响应下拉框的选项更改。
void WndSingle::ComboboxChange(const QString& strChange, const QString& strId, const QString& parentId, const int& level, bool* pNewChild, void* pPar)
{
    if (ControlCenter::getInstance()->m_type == eTemplateType_SRES)
    {
        //  auto itNow = m_BoxListCollection.comboboxList.Empty();
        if (!m_bChildIsConnectCombobox)
        {
            if (!m_BoxListCollection.comboboxList.Empty())
            {
                LoadDictionaryData(strId, true, true);
                ShowChild(strId, true, false);
            }
        }
        else
        {
            for (auto& itVecChild : m_vecChildWnd)
            {
                if (itVecChild->m_strId == strId)
                {
                    itVecChild->m_pLayerWnd->ShowWnd(true);
                    itVecChild->m_BoxListCollection.SetListVisible(true);
                    if (m_isFinish && level == 1)
                    {
                        callback(strId, "", "change");
                        return;
                    }
                }
                else
                {
                    itVecChild->m_pLayerWnd->ShowWnd(false);
                    itVecChild->m_BoxListCollection.SetListVisible(false);
                }
            }
        }
        if (m_bCanCallBack && !m_bIsUpadteReport)
        {
            if (m_UpdateDataCallBack != nullptr)
            {
                m_UpdateDataCallBack(nullptr, strId);
            }
        }
        return;
    }

    if (strId != "" && level == 2)
    {
        for (auto& itVecChild : m_vecChildWnd)
        {
            if (itVecChild->m_strId == strId)
            {
                itVecChild->m_pLayerWnd->ShowWnd(true);
                itVecChild->m_BoxListCollection.SetListVisible(true);
                //更新第三级目录    
                auto list = CTemplateManage::GetSingle()->GetTemplateList(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId);
                WndSingle* wnd = static_cast<WndSingle*>(pPar);
                templeteList.clear();
                if (!wnd->m_BoxListCollection.comboboxList.Empty())
                {
                    auto lastPair = *wnd->m_BoxListCollection.comboboxList.list.mapControl.rbegin();
                    auto combo = lastPair.second;
                    combo->blockSignals(1);
                    combo->clear();
                    templeteList.append("--");
                    for (auto it : list)
                    {
                        if (!templeteList.contains(it.strKey))
                            templeteList.append(it.strKey);
                    }
                    combo->addItems(templeteList);
                    combo->blockSignals(0);
                    combo->setCurrentText(ControlCenter::getInstance()->m_currentTemplateName);
                    combo->Tdisconnect();
                    combo->Tconnect();
                    ControlCenter::getInstance()->setCombobox(combo);
                    //combo->setToolTip();
                }
                if (m_isFinish)
                {
                    BothId b(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId);
                    auto json = ControlCenter::getInstance()->getOpenedReportJson(b);
                    callback(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId, json == "" ? "change" : json);
                    return;
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
        auto strJson = CTemplateManage::GetSingle()->GetTemplateDataByName(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId, strChange);
        //通知界面刷新
        ControlCenter::getInstance()->m_currentTemplateName = strChange;
        if (strChange != "--" && strJson != "")
            callback(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId, strJson);
    }
    //切换回正常的编辑
    if (strChange == "--" && level == 3)
    {
        
        ControlCenter::getInstance()->m_currentTemplateName = "--";
        BothId b(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId);
        auto json = ControlCenter::getInstance()->getOpenedReportJson(b);
        callback(ControlCenter::getInstance()->m_currentFirstId, ControlCenter::getInstance()->m_currentSecondId, json == "" ? "change" : json);
    }

    if (!m_bChildIsConnectCombobox)
    {
        if (!m_BoxListCollection.comboboxList.Empty())
        {
            LoadDictionaryData(strId, true, true);
            ShowChild(strId, true, false);  
        }
    }
    
    if (m_bCanCallBack && !m_bIsUpadteReport)
    {
        if (m_UpdateDataCallBack != nullptr)
        {
            m_UpdateDataCallBack(nullptr, strId);
        }       
    }
}
/****
绫清竹终是站定，浩瀚元力席卷而出，直接是在其上空化为无数道元力所凝聚而成的青色光剑，旋即其玉手轻挥，天地间顿时响彻起尖锐的破风之声，只见得那无数道光剑，直接是铺天盖地的呼啸而出，最后与那潮水般的元力弟子，狠狠的相撞。
“啊！”
凄厉的惨叫声响彻而起，鲜血喷涌而出，直接是染红了地面，那元门潮水般的攻势，直接是在此时被绫清竹一人硬生生的阻拦而下。
天空上，那元门大长老漠然的望着死伤颇为惨重的元门弟子，那眼中黑芒涌动却是没有丝毫的心疼之色，如今的整个元门，显然都是与以往变得不同起来。
“程普，王宿，苏立你三人去将她阻拦下来，这小辈的攻击有些古怪，自己小心，莫要再像徐凌那般无用。”不过这样被绫清竹将攻势阻拦下来毕竟不是这位元门大长老想要看见的，当即略作沉默，淡漠的道。
“是！”
听得他的话，其身后三名面色阴翳的老者顿时应道这三人气息与先前被绫清竹所斩杀的徐凌一般无二，显然也是那元门之中的顶尖强者。
三人身形同时一动，下一霎便是出现在元门弟子前方，三人一声冷哼，同时出手，眼中黑芒涌动，滔滔元力弥漫而出，化为道道光幕将那铺天盖地射来的光剑尽数的抵御下来。
“血洗九天太清宫！”
三人冷喝出声，那被阻拦的元门弟子顿时眼神赤红的应喝，然后对着后方的那些九天太清宫弟子冲杀而去。
绫清竹见状，银牙微咬，然而其身形刚动，破风声便是传来那三位元门长老，已是出现在其周身，将其封锁而住。
“滚开！”清冷的声音，蕴含着薄怒，自那轻纱之下传出，绫清竹玉手紧握长剑，美目之中，掠过一抹冰冷杀意。
“哼，你九天太清宫诸多长老尽数败于我等之手你这小辈，还敢如何？”那三位元门长老一声冷笑，旋即也不给绫清竹丝毫反应的时间，浩瀚元力奔腾而动，而后一指凌空点出，元力直接是化为三根擎天巨指，当头便是对着绫清竹重重镇压而去。
嘭！
巨指尚未落地，那地面已是崩塌千丈，道道巨大裂缝，犹如深渊般的蔓延而开。
绫清竹安静的望着那等凌厉攻势，下一刻，她修长玉手，突然相合，璀璨的青光，犹如潮水般的自其体内涌出，一朵巨大的青莲，在其头顶上方迅速成形。
砰！
三根擎天巨指，重重的落在那青莲之上，狂暴无匹的元力爆发开来。
“破！”
清冷的喝声，陡然响起，那青莲却是在这一瞬间猛的旋转而起，那道道花瓣带起锋利无比的寒芒，空间都是在此时被切割而开，那三道擎天巨指，也是在青莲的旋转间，生生的被绞碎而去。
那三位长老见状，眼神也是一凛，这绫清竹的气息也不过只是转轮境层次，但为何她的攻击，却是这般的凌厉？
“唰！”
就在他们心中惊疑间，那道青莲，已是震碎三道擎天巨指，而后化为一道青光，直奔三人而来。
“哼！”
“大元天斩！”
三人见状，眼神一寒，单手做刀，猛的怒劈而下，浩瀚元力涌动间，直接是化为三道巨大刀芒，斩破空间，狠狠的落在那青莲之上。
嘭！
青莲经受不住三人这般摧残，最终还是爆裂而开，不过，就在青莲爆裂时，一道青光陡然自其中闪掠而出，眨眼间，便已如鬼魅般出现在那三位元门长老之前。
“不好！”
那三位长老见到眼中青光闪烁，一股极度危险的感觉，顿时涌上心头，而后他们喉咙间，竟是传出一道野兽般的低吼之声，眼中邪恶黑光猛的暴涌。
“咻！”
三人惊怒出手，那手掌都是尽数化为漆黑之色，其上魔气缭绕，快若闪电般的对着前方那道青光怒拍而去。
“嗡！”
青光闪烁，露出一道绝美的倩影，她那秋水般的眼瞳中，在此时掠过一抹混沌之色，而后其玉手一抖，三道剑光掠出。
嗤嗤！
青光倩影，其三位元门长老身旁搽身掠过，而后长剑缓缓入鞘，身后三人，脖子间，血痕浮现，鲜血顿时冲出，那人头也是伴随着飞舞起来。
咻！
人头飞舞间，三道元神之光仓惶的遁出，就欲逃窜，不过这一次，有了准备的绫清竹眼神一寒，又是凌厉无比的三剑洞穿而出，竟直接是将那三道元神都是震碎而去。
电光火石间，又是三位转轮境的超级强者败与绫清竹之手，而且此次她出手更为狠辣，竟是连对方元神都是毁去，丝毫不留情。
“哼。”
不过在绫清竹将这三人解决时，那薄纱下也是传出一道轻轻的闷哼声，在其香肩处有着三道黑色掌印，一抹鲜艳的猩红，将那薄纱染红了一些。
但对于嘴角的血迹，绫清竹却是丝毫不顾，她只是螓首轻抬，美目紧紧的望着那站在黑云上，眼神漠然的望着她的元门大长老这些元门强者中，唯有此人，给予她一种极深的危险感。
“能够以这般年龄做到这一步东玄域年轻一辈第一人，怕是非你莫属，与你比起来，我元门那些所谓的三小王，倒真是一个笑话。”元门大长老淡漠的道。
“年轻一辈第一人…”
听得这个称谓，绫清竹那素来清冷如幽潭的眸子终是微微恍惚了一下，脑海之中，掠过一副三年之前的画面。
在那异魔城，满身鲜血的青年，手持长枪，傲然相对元门三大掌教那一仗了震惊整个东玄域。
“你错了，这第一人，怕是轮不到我来，今日或许我会葬于此地，不过…”
绫清竹那薄纱之下，似是有着一个绝美的笑容浮现出来：“我相信，来日，同样会有人，将你元门血洗！”
“是么？”元门大长老讥讽一笑，旋即道：“老夫陆峰，现在的你，有资格知道这个名字。”
“虽然老夫知道在这东玄域，不知道多少年轻人为了你神魂颠倒，不过这在老夫眼中，却是毫无吸引力，或许，不要天真的以为，我会对你手下留情。”
陆峰终于是缓缓的踏出黑云，在其眼瞳深处，黑芒疯狂的凝聚而来，滔天的魔气，在其身后汇聚，遮天蔽日，甚至连天际的烈日，都是被掩盖而去。
显然，这位元门的大长老，终于是忍不住的要亲自出手了。
感受着那来自陆峰身上的惊人压迫，绫清竹那纤细的娇躯，微微颤抖了一下，旋即偏过头，望着那些且战且退的九天太清宫弟子，玉手缓缓紧握，手中长剑，遥遥指向那陆峰。
现在的她，必须将这元门的大长老拖住，不然的话，九天太清宫的弟子，恐怕一个都走不了。
“你放心，九天太清宫的弟子，老夫不会放过一人。
似是知道绫清竹心中所想，那陆峰淡淡一笑，笑容却是格外的狰狞与暴虐，这些元门平日还有些仙风道骨的长老，如今，个个都犹如那最为残暴的恶魔一般。
“唰！”
绫清竹并未再回答，只是身形化为青虹，凌厉无匹的剑芒，快若闪电般的对着那陆峰暴刺而去。
嗡！
剑锋掠出，然而就在即将接触到陆峰时，两根布满着黑色魔纹的干枯手指，却是将其阻拦而下，火花四射间，那等凌厉剑芒，竟是无法伤其分毫。
“嘿。”
陆峰嘴角一掀，露出狰狞笑容，另外一只手掌紧握成拳，拳头之上，魔纹重重弥漫，一种极端惊人的邪恶之气，扩散而来。
绫清竹眼神冰寒，她望着陆峰那狂暴而来的一拳，却是一咬银牙，那眸子深处，神妙－的混沌之光，猛的涌现而出。
唰！
魔拳洞穿空间而来，绫清竹那长剑之上，浩荡元力突然尽数的消散而去，只是在那剑刃边缘，却是掠过一抹奇特光芒，而后剑身一斜，暴刺而出。
嗤！
陆峰的双指，在顷刻间随着长剑的掠过而飞起，鲜血滚滚涌出，那剑身却是丝毫不停，直奔他心脏而去。
嘭！
不过就在长剑即将洞穿陆峰心脏时，那狂暴无比的魔拳，却已是先一步的落在了绫清竹娇躯之上，滔天魔气席卷，空间都是在此时被震得碎裂开来。
噗嗤。
一口鲜血，自绫清竹檀口之中喷出，她那洞穿了陆峰胸前皮肉的剑锋再也无法向前，而后她美眸中掠过一抹悲色，身形在后方那些九天太清宫惊骇欲绝的目光中倒飞而出，盛开的鲜血，凄艳而心酸。
“清竹师姐！”
本在撤退的苏柔等众多九天太清宫弟子，顿时嘶声大喊，而后他们犹如发疯一般，竟是不再撤退，反而是拼命的对着绫清竹这边的方向冲来。
“哼。”
陆峰瞥了一眼被削断的双指，再看了看胸前那道即将刺破心脏的剑痕，眼中不由得掠过一抹戾气，手掌一握，魔气便是滚滚而来，化为一道千丈庞大的魔刀，袖袍一挥，魔刀便是洞穿空间，快若闪电般的对着受伤的绫清竹暴掠而去。
绫清竹望着那暴掠而来的魔刀，贝齿紧紧的咬着红唇，那清眸中掠过一抹无力与凄然之色，此战，终归还是败了。
她们九天太清宫，便是要这般消失在元门的屠刀之下了么…
“师姐！”
苏柔眼睁睁的望着那在魔刀之下显得柔弱不堪，马上即将凋零的倩影，泪水顿时忍不住的从眼眶中滚滚而出，而后，她嘶声大喊，那声音中，再没有了以往的清脆，其中充斥着无比的绝望与凄厉。
“咻！”
在那嘶哑的叫声响彻天际时，一道急促到无比的破风声，顿时尖锐的在这片天地间响彻而起，那一道道惊愕目光抬头，只见得那遥远处，空间扭曲，一道身影犹如闪烁般的暴掠而来。
轰！
魔刀也是在此时，轰然的斩落而下，顿时大地崩裂，一道道巨大的裂缝，犹如蜘蛛网般蔓延开来。
无数道骇然的目光，望向那尘灰扩散之处，众多九天太清宫的弟子，犹如失去了最后的失望，眼神瞬间黯淡下来。
“师姐。”
苏柔缓缓的跪倒在地，目光呆呆的望着远处那片地域，然而就在她眼神恍惚间，却是见到，那里的尘灰开始散去，破裂的大地上，有着一片完好无损的地面，而在那地面上，绫清竹竟是安然无恙！
“那是…”
突然有着惊呼声响起，苏柔急忙抬头，只见得在绫清竹的前方不远处，不知何时，有着一道削瘦的身影静静矗立，那道身影并不壮硕，但却将那所有的风浪，尽数的抵挡了下来。
“那是…林，林动大哥？！”
苏柔呆呆的望着那道依稀有着一些熟悉的身影，下一刻，她猛的站起身来，那俏丽的脸颊上，有着难以置信以及难以言明的狂喜涌出来。
后方传出的道道惊呼声，也是让得那紧闭着美目的绫清竹缓缓的睁开了双眼，而后她望着前方那道不知何时出现的身影，贝齿紧咬着红唇，一丝鲜血从嘴角渗透出来，但她却是强忍着心中那股波动，强行将自己的目光从那道背影上缓缓的转移而开。
前方，那道身影沉默了半晌，终于是微微侧过头，他目光复杂的望着后方那道一言不发的倩影，旋即漆黑眸子中，有着柔和之色悄悄涌动起来。
“放心吧，有我在，没人能伤你。”
听得这句略带着沙哑的轻声，这一刻，饶是以绫清竹的清冷，一股无法严明的心酸，涌上挺翘鼻尖，那眼眶之中，都是有着水气凝聚了起来。


第1198章 小心点
漫天魔气涌动，大地崩裂，而那道削瘦的身影，则是安静的矗在前方，任由那滔天巨浪，都是无法席卷而来。
当他说出那略显沙哑的话语时，他也是转过身来，然后行到那微偏着头的绫清竹身旁，他望着后者薄纱上沾染的血迹，后者柳眉间，依稀还能见到那熟悉的清冷，与应欢欢如今那种冰冷不同，她的冰冷，却总是有着一种与生俱来般的清傲之意，而这或许也是她为之优秀的原因。
绫清竹能够感觉到那来到身旁的青年，那对即便是在生死间都未曾有过太大波动的秋水眸子中，却是在此时紊乱了一些，玉手紧握着，控制着情绪，不让她去看此时那光芒突然变得耀眼得如鹰如隼的男子。
这番模样，与三年之前那般的惨烈，截然不同。
而在她心境微微紊乱间，面前的青年似是弯下了身子，然后直接将她拦腰抱起，霎那间的离地，让得绫清竹脑子空白了一瞬，但随即清醒过来后，便是一阵凶猛的挣扎。
“别动。”
略微有点低沉的声音传来，绫清竹娇躯微僵，终于是抬起螓首，一张比起三年前成熟坚毅了许多的年轻脸庞，印入了眼帘之中。
此时的后者，正微微皱着眉，那股气质与凌厉，即便如今的绫清竹，都为之一怔，三年的时间，似乎洗尽了他那一身铅华，那原本就隐藏在骨子深处的东西，正在逐渐的显露出来。
绫清竹银牙轻咬了咬，那挣扎总归还是因为如今身体的受伤而微弱了下来，唯有那紧握的玉手，显示着她内心的一些波动。
“林…林动大哥？”
苏柔等众多九天太清宫的弟子怔怔的望着那抱着绫清竹缓步走过来的青年，好半晌后，她方才猛的回过神来，那俏脸之上，顿时有着无法掩饰的狂喜之色涌了出来。
“林动大哥，真的是你！你回东玄域了？！我就知道，你一定会回来的！”
林动停在了苏柔前方，他望着少女那惊喜的俏脸，也是忍不住的微微一笑，当年那个怯怯弱弱的少女，如今，也终于是变得成熟了起来。
“做得不错。”
林动将怀中的绫清竹轻轻放下，然后伸出手掌笑着摸了摸苏柔的小脑袋，后者脸颊顿时一红，但前者这种熟悉的举动，也是令得她心中暖洋洋的，那霎那，仿佛浑身的疲惫都是尽数的消失而去。
“咳。”一旁的绫清竹突然轻咳出声，薄纱下，似是又有着一抹殷红浮现出来，显然先前那陆峰的一掌并不轻。
“清竹师姐，你没事吧？！”
苏柔大惊，周围的那些九天太清宫弟子也是急忙的围上来，眼中满是担忧。
绫清竹微微摇头，她望着周围那些师兄弟身上沾满的鲜血，轻声道：“都打起精神来吧，如今强敌未退…”
听得此话，众人也是迅速从先前的惊喜中回复了清醒，面色皆是沉重下来，在那周围，还有着元门大军虎视眈眈，现在连绫清竹都是败在了那元门大长老手中，他们算是失去了最后的力量。
“接下来的事，就交给我们道宗吧，你们护着她。”
林动看了绫清竹一眼，后者的眼神已是恢复了常见的清冷，只不过却始终不与他对视，每当目光有所碰触，她便是会立即转移而开。
“林动大哥…你，你小心点，那元门大长老实力很强的。”苏柔咬着红唇，虽然理智让得她认为这种时候，林动应该避让才对，但那心中对林动的盲目信任，却是让得她到嘴的话变了模样。
“道宗之友今日相助，清竹代九天太清宫先行谢过了，不过若是无法退敌，还望能先行离去，我九天太清宫即便是宗毁人亡，也断然不会与元门有丝毫妥协。”
绫清竹美目微垂，声音轻缓的道。
一旁的苏柔听得这话，顿时吐了吐舌头，不敢再说话，她与绫清竹关系颇为的亲密，隐约的知道一点两人间似乎有着一些莫名的关系，在林动离开的这三年，她曾见过绫清竹偶尔修炼间会有些发呆，那显然是在思念一个人时方才会出现的模样。
而这则是令得苏柔大感神奇，绫清竹性子清清冷冷，心境修为极其的厉害，这些年来东玄域不知道有着多少各路天才妖孽仰慕于她，但却始终未能在那关系上有更一步的进展，有时候甚至都连苏柔都认为，或许这天地间，真是没一个男子能够真的让自己这清傲如谪仙般的师姐如寻常小女儿一般，因此，当她在见到绫清竹那般模样时，心中方才会有着如此之大的震动。
不过让得她微微有些疑惑的是，既然清竹师姐对林动大哥有着关系，那为何眼下，又是这般清清淡淡甚至还显得有点陌生？
疑惑归疑惑，但绫清竹在九天太清宫显然有着极高的威严，因此连她也不敢偷偷询问。
“什么道宗之友，你难道不知道我的名字不成？”林动同样是因为绫清竹这语气而将眉毛微微扬了起来，黑色眸子中掠过一抹怒意，这女人是故意来气他的不成？
绫清竹眼神清淡，却是未曾抬头，也没有回答。
见到她这般，林动一声怒哼，转身就走，绫清竹那玉手也是在此时忍不住的紧握起来，薄纱下的贝齿，轻咬着红唇，但她却倔强得一言不发。
不过，就在她暗自倔强间，那走出两步的林动突然转身，掌风直接是在周围那些九天太清宫震惊的目光中对着绫清竹脸颊而去，那模样，竟是如同含怒出手一般。
而察觉到那掠来的掌风，绫清竹娇躯微颤，竟是闭上了美目，动也不动。
“嗤。”
携带着掌风的指尖掠过绫清竹的脸颊，然后她便是猛的感觉到脸颊一凉，那面上的薄纱，竟是被强行的扯去。
薄纱扯去，隐藏在其下的那张近乎完美般的美丽脸颊，也是曝光在了空气中，那一霎那，仿佛连空气都是明亮了起来。
“你！”
脸颊上传来的冰凉让得绫清竹一惊，急忙睁开双眼小手摸着如玉般的脸颊，然后她怔怔的望着那落在林动手中的面纱，那素来清冷的白皙脸颊上，一抹羞怒之色终于是涌了起来，那霎那间出现的风情，直接是让得周围那些九天太清宫的弟子不论男女皆是有些失神，在宫门内绫清竹素来都是清冷淡然，极少会出现类似眼下这种仙女落凡尘般的动人一幕。
“这比先前好看多了，在我面前扮仙子，我可不喜欢吃这套。”林动也是因为那美丽得令人目眩的美丽脸颊恍惚了一瞬，然后他看着绫清竹脸颊上涌上来的羞怒，戏谑的笑道。
“谁管你喜不喜欢。”
绫清竹恼怒的道然而以她的容颜，即便是恼怒中，那般风情，也是动人心魄。
“哈哈。”
林动大笑了一声，然后那漆黑双眸却是逐渐的冷厉下来，他缓缓偏过头，望着远处那些冷眼旁观的元门强者淡淡的道：“接下来的事就由我来了。”
绫清竹望着那道削瘦的背影，面纱被这家伙霸道的撕去，仿佛也是令得她的心绪紊乱了许多，她咬了咬银牙压下心中对这家伙之前举动而产生的羞怒，不管怎么样，在这最关键时候林动的出现，的的确确是令得她那古井不波般的心境中泛起了一些涟漪。
她与林动之间的关系复杂得连她都是略感头疼两人的性子，都注定他们不可能真的将当年所发生的那件荒唐事轻易的遗忘不论如何，他们彼此在对方的心中，总归是有着一些特殊，这一点，即便是清傲的绫清竹也不得不承认，若是换作其他男子来扯她面纱，恐怕她现在早便是一剑刺了过去，但换成林动后，那原本足以激起她杀意的举动，却仅仅只是令得她恼怒他的霸道与莽撞。
绫清竹的性格，无疑是清傲的，这一点，在那很多年前的第一次相遇时，便是显露了出来，只不过当年初见，她是九天太清宫之中的天之娇子，而林动，却不过只是一个小小的低级王朝之中的宗族分家，两人之间不论身份地位还是实力，都是有着天壤之别。
在当年那山峰上，绫清竹本欲一剑将这辱了自己清白的人杀了，但后者那明亮而炙热的目光却是令得她最终未能下手，那时候的她便是知道，这个看似稚弱的少年，骨子里面却是有着一种谁也比不上的韧性与执着。
她也知道，她成了他追逐奋斗的目标。
而且她同样还知道，他为了达到这一步，将要付出多么庞大的努力。
于是，为了那一天，尚是幼狼般的少年，在那条布满荆刺的道路上，开始奋力奔跑，即便遍体鳞伤，却依旧未曾有丝毫的后悔。
不过，或许那时候的她也并未曾想到，在许多年后的一天，那个曾经稚弱的少年，却是会站在自己的面前，用那并不宽阔的背影，为她将那连她都承受不住的狂风暴雨，尽数的一揽而过。
心中情绪翻涌，那久远的回忆也是回荡在绫清竹的脑海中，那清澈的双眸中，最终有着一抹柔和浮现，而后那轻声，缓缓的传进前方林动耳中。
“小心点。”


第1199章 讨债开始
在那后方无数道九天太清宫弟子紧张的目光以及前方那些元门强者冷漠的视线中，林动缓步走出，最后在那片崩裂的大地处缓缓站定，他抬头望着那陆峰，咧嘴一笑，只是那笑容，却是弥漫着无尽的杀意。
“你是何人？我元门的事，你也敢插手？”陆峰面色阴厉的盯着林动，淡漠的道。
“打的就是你们这些元门狗。”林动笑着摇了摇头，道。
陆峰眼中寒芒一动，刚欲说话，却是有着数名元门长老掠至他身旁，道：“大长老，此人似乎便是那道宗的林动！”
“哦？”
陆峰微讶，旋即打量了一番林动，讥笑道：“我道是谁，原来是被我元门撵出东玄域的丧家之犬啊，怎么？在外面厮混了三年，又偷偷的跑回来了啊？”
“我当年就说过，要灭你元门的啊。”林动笑道。
“就凭你？”陆峰冷笑道。
“那三条老狗还活着吧？”林动脸庞上的笑容逐渐的淡下来，黑眸冷冽的盯着陆峰，淡淡的道。
“小子，你找死？！”
一名元门长老勃然大怒，眼中杀意暴闪，下一刻，他身形已是暴掠而出，浩瀚元力席卷间，声势惊人，蕴含着全力的凶狠一拳，直接是撕裂长空，狠狠的对着林动爆轰而去。
一拳轰出，连地面都是被生生的撕裂出一道巨大的裂缝。
然而面对他的凶猛攻势，林动那盯着陆峰的眼睛，却是连动都未曾动过一下，待得那元门长老攻势已临身前时，他方才抬起手掌，轻轻探出。
嘭！
足以摧毁山岳般的一拳，重重的轰在林动手掌之上，可就在众人等待着惊天冲击波扩散而出时，却是惊愕的发现，林动的身影，却是连颤都未曾颤动一下，他的手掌轻轻的包裹着那元门长老的拳头，看似修长的手掌，却犹如一个无底黑洞般，直接是将他那狂暴无比的攻击，生生的吞噬而去。
“啊？”
那元门长老见到这一幕，面色也是一变，一股不安终于是涌上心头，可就在他急忙要撤退间，林动那漠然的目光，终于是转移到了他的身上。
唰。
林动脚步踏出半步，一只修长手掌犹如穿透空间般，直接落在那元门长老脑袋之上，而后黑眸中凶光一闪，掌心劲力喷吐，猛然一按。
砰！
那元门长老的身体，在此时骤然膨胀起来，而后砰的一声，便是直接爆成一团血雾，一道闪烁着黑芒的光芒掠过，然后一声凄厉惨叫，刚欲逃窜，那修长手掌已是穿过黑雾，将其一把抓住，没有丝毫留情的打算，手掌一握，直接是生生的将那道元神捏爆而去。
短短数个呼吸的时间，一名转轮境的超级强者，便是被彻底的抹杀，那般狠辣以及干脆利落的手段，比起之前绫清竹，无疑是要胜了数个档次。
“这…”
后方那些一直关注着前方交手的九天太清宫的弟子们，皆是在此时瞪大了眼睛，那脸庞上满是惊骇之色，盯着那道削瘦身影的目光，犹如见鬼一般，绫清竹能够达到那种程度，是因为接受了宫主的传承，但这林动，失踪三年，回来之时，居然已经恐怖至此？
“林动大哥好强。”苏柔拉住一旁绫清竹的娇嫩玉臂，那俏丽的脸颊上，满是激动之色。
绫清竹眸子略显复杂的望着那道谈笑间抹杀一名转轮境超级强者的背影，心中轻轻叹了一声，这么多年了，他终于走到她前面去了…
不过，想要在短短三年时间中达到这种地步，这个家伙，这三年，究竟经历了多么艰难的苦修以及磨练？
从你走那天，我也一直相信着，你会再回来，而那时，这东玄域的天，将再也遮不住你的眼。
绫清竹似是微微笑了笑，那霎那间的笑容将一旁的苏柔都是看呆了去。
“还有人要来么？”
林动笑着拍了拍手，然后望着天空上那些元门长老，道：“那三条老狗没来？”
陆峰眼神阴沉的盯着林动，那眼中的冷笑终是逐渐的消散而去，能够如此狠辣的解决一名晋入转轮境的超级强者，就连他都是颇难的做到，显然，眼前的这人，再也不是当年那个被他元门逼得走投无路的道宗弟子。
“看来这三年你倒的确是变强了不少，不过，光凭这个，想要灭我元门，只怕你还是天真了一些。”陆峰面色阴翳的道。
“我知道，你们这些畜生跟异魔搭上了些关系，不过没关系，不论是元门还是异魔，我都会好好的清理一下的。”林动淡笑道。
“你知道异魔？！”陆峰瞳孔一缩，道。
“你元门实力暴涨，无非是异魔暗中相助，不然你们想要荡平东玄域，做梦呢？”林动讥讽的摇了摇头，他盯着陆峰，道：“就连你，体内也是魔气旺盛，看来受魔气侵蚀也不是短时间的事了，你们元门，倒还真是一群杂碎，竟然会与异魔搅到一块去…”
“小子，留你不得！”
“大元天斩！”
陆峰眼中黑光闪烁，一股惊人的杀意弥漫开来，滔天魔气汇聚着元力在其身后犹如大海般奔腾，旋即他的手掌劈出，滔天魔气化为一道千丈刀芒，疯狂的怒劈而下。
这陆峰一出手，便是显露出远超其他长老的惊人实力，后方的那些九天太清宫弟子也是看得大惊，心情都是高悬了起来。
刀芒急落而下，空间都是被劈砍而开，然而林动的面色依旧淡漠，待得那刀芒要落下来时，他的头顶上空，突然有着巨大的黑洞凭空浮现。
嗤。
刀芒劈进黑洞之中，然后传出一道细微的闷响，之后便是再无了声息，黑洞旋转间，又是凭空的散去，一切都是显得那般的诡异。
“这？陆峰也是因此微微一惊，旋即他目光紧紧的盯在林动身上，声音阴沉的道：“那是吞噬之力？你竟然身怀吞噬祖符？”
林动淡漠一笑，他只是缓缓的伸出手掌，遥遥的对准了陆峰，然后猛然一握。
咻！
就在林动掌心握下的霎那，陆峰面色也是一变，身形猛的化为道道残影暴退而去，不过，其身形刚退，周遭空间便是扭曲起来，四道黑洞漩涡在其周身凭空成形，四顾吞噬之力爆发而出，可怕的撕扯力，直接是将那片被包裹的空间，扯得支离破碎！
“嘭！”
黑洞狠狠的旋转，那其中也是猛的有着一道闷哼声传出，不过旋即便是有着一股极端惊人的元力爆发而开，那黑洞漩涡形成的囚牢也是被蛮横的冲开，那陆峰略显踉跄的窜了出来，虽说林动这一手从吞噬之主那里学来的黑洞扭曲威力不弱，但这陆峰也不是省油的灯，虽说略有些吃亏，但却并未如同被林动一举抹杀。
“你果然拥有吞噬祖符！”这个时候，这陆峰终于是确定，那股独有的吞噬之力，这世间别无他物能够模仿。
林动笑笑，倒也并未反驳，现在的他，已不用再如同当年那般，小心翼翼的隐藏着身怀祖符的秘密，强横的实力，足以让得他拥有着自保之力。
“不过你有吞噬祖符又能如何？老夫便惧你不成！”
陆峰此时的面庞，也是显得格外的扭曲，眼中黑芒愈发的浓郁，满脸的暴戾邪恶，旋即他猛的一步跨出，魔气的滚滚，竟是在其身后，化为一道数千丈庞大的魔影。
“天元魔拳！”
低吼之声，自陆峰喉咙间传荡而出，此时的他，显然也是将力量催动到了极致，眼前的林动，让得他感觉到了浓浓的危险。
陆峰身形后退，直接是掠进那巨大魔影之影，而后那魔影模糊的脸庞上，便是射出两道猩红光芒，巨拳挥动，携带着一股毁天灭地般的恐怖力量，一拳对着下方的林动怒轰而去。
砰砰。
攻势尚未落地，那可怕的拳风，已是将空间震得扭曲下来，这片浮空岛屿，更是被生生的震得塌陷而去，急速的坠落，后方众人看得皆是面色大惊。
林动抬头，眼瞳之中倒映着那急速而来巨大魔拳，旋即他右臂缓缓的伸出，只见得黑芒以及雷光闪烁起来，接着，他的一条右臂竟是逐渐的呈现液体化，黑芒以及雷弧，不断的跳跃着，隐约间，能够见到两道古老的符文在那液体中沉浮不定。
以往的林动，仅仅只能使用两道祖符之力液化手指，然而如今伴随着实力的提升，整条手臂，都是变化成这般模样，那杀伤力，自然是更上一层楼。
“与元门的恩怨，就先从你这老狗身上开始讨回来吧…”
“砰！”
林动脚掌猛的一踏地面，身形直接是化为一道流光，他竟是正面对着那魔拳冲去，没有丝毫闪避迹象，这番模样，显然是要与那陆峰正面相对！
那些九天太清宫的弟子见到这般对碰，顿时惊得紧紧捂着嘴，就连苏柔脸颊上都是掠过一抹担忧之色，唯有着一旁的绫清竹还算平静，只是那玉手也是忍不住的轻握了起来。
咻！
两道惊天光芒掠过天际，下一刻，便是在那无数道目光的注视下，轰然相撞！
这一刻，仿佛天地都是颤抖了起来。


第1200章 显威
咚！
无法形容的能量冲击，犹如层层涟漪，飞快的自天空上扩散开来，而在那种冲击下，这片空间都是呈现了一种强烈的扭曲之感。
无数道目光，紧紧的盯着那能量冲击扩散的源头，那些九天太清宫的弟子，皆是紧握着手掌，手中满是汗水。
绫清竹眸子同样是凝聚在那天空，片刻后，其眼神突然一凝，只见得在那能量扩散源头，一道虹光，猛然暴冲而上，而那道巨大的魔拳，则是在那道虹光的冲击下，尽数蹦碎！
咻！
那道虹光过处，魔拳崩裂，空间扭曲，而后直奔天空上那巨大魔影冲击而去。
“吼！”
那魔影显然也是察觉到那道掠来虹光的厉害，当即有着怒吼之声响彻，只见得那滔滔魔气在其前方凝聚，而后竟是化为一道巨大无比的魔气光阵，光阵之中，弥漫着滔天般的邪恶魔气。
然而，面对着这般强力防御，那道虹光速度却依旧是丝毫不减，下一霎，他已是出现在光阵前方，那化为黑雷液体般的手臂，犹如一柄凌厉得足以撕裂天地的利剑，猛的轰出，重重的轰击在那魔气光阵之咚！
两者相撞，低沉的声音在那天空上传荡开来，只见得无数道黑雷光芒，自林动手臂之上倾泻而出，疯狂的轰击在那魔气光阵之上。
“林动，你就只有这些本事么？这点能耐还想找我元门三位掌教报仇？不自量力！”两者僵持，那魔影仰天咆哮，陆峰那嘲讽的大笑声滚滚的传开。
“你高兴得未免早了点。”
林动眼神淡漠的望着那咆哮中的魔影，那液体般的手掌，猛然紧握在其表面，两道古老的符文缓缓的浮现，而后悄然的交织，顿时，璀璨的黑雷光芒，猛的自林动手臂之上爆发开来。
“嘭！”
林动手臂缓缓收回，下一霎再度猛烈的轰出，最后重重的轰击在那魔气光阵之上。
咔嚓！
这一次，魔气光阵那可怕的防御却是彻彻底底的失效林动那液体般的手臂，竟是径直的将那光阵所穿透，而后手掌将其撕裂而开。
“你！”
防御被摧毁，那陆峰终于是失声惊道，那声音中，有着一些震惊他这般防御，乃是倾力而为，但竟然依旧无法阻拦下林动。
“唰！”
此时的林动却并没有再给予他丝毫的喘息机会，身形一动，直接出现在魔影之前，一拳轰出一道液体般的黑雷光束，猛的自其拳头之上，暴射而出。
黑雷光束，犹如一道流星，掠过天空，重重的轰击在那庞大的魔影之上，而后凄厉之声猛的响彻而起，那道黑雷光束，竟直接是生生的将那道魔影洞穿而去。
铺天盖地的魔气疯狂的从那魔影之中散逸开来，而后魔影迅速的淡化，那陆峰的身影再度出现在了所有人视线之中，只不过此时的他，胸膛之上，已是出现了一个深深的血洞，那原本阴翳的面色也是变得格外的苍白，阴冷的眼中，布满着浓浓的惊色。
“你…”
陆峰面色苍白的盯着不远处天空上露出身形的林动，他怎么都没想到，他全力之下，竟然也是如此迅速的便是败在了林动手中。
“怎么可能！”
陆峰喃喃道，他乃是触及轮回的强者，只要不是遇见那种轮回境的巅峰强者，想要胜他极其困难，而眼下，战斗却并非展开多久，他便是尽数的落败，这如何能让他接受得了。
“林动大哥赢了！”
后方的苏柔等人见到这一幕，却是忍不住惊喜得叫出声来，他们的眼中同样是有着浓浓的惊色，先前那陆峰的实力他们已是亲眼见识过，甚至连他们九天太清宫的宫主都是败于其中，没想到，即便是连这等强者，都不是林动的对手，看来今日他们九天太清宫的危机，终于是可以被解开了。
一旁绫清竹紧握的玉手，也是在此时悄悄的松开，掌心之间，略显湿滑。
“大长老，没事吧？”那元门的阵营中，三道光影迅速掠至陆峰身旁，他们望着后者的伤势，眼中也满是震惊之色，急忙问道。
陆峰咬着牙摇了摇头，旋即大手一挥，暴喝道：“所有元门弟子听令，给我将九天太清宫血洗了！”
“你三人，随我拦住这小子！”
陆峰又是看向身旁那仅剩的三位元门长老，吃了先前的亏，他显然也是不太敢一人对战林动。
“是！”
那元门无数弟子闻言，也是齐齐暴喝，他们眼中邪恶黑光涌动，而后再度爆发出浓浓杀气，竟是再度如潮水般的涌向那些九天太清宫的弟子。
绫清竹见到元门攻势再度对着她们而来，脸颊上也是掠过一抹冰冷寒意，而就在她要出手时，那远处天空上的林动却是淡淡一笑，旋即其手掌轻挥而下，淡漠的声音在天空上缓缓的传开。
“元门之人，已被魔气侵蚀，尽数的斩杀吧…”
咻咻咻！
就在林动声音落下时，那远处，猛的有着急促破风之声响彻，而后数道光影陡然出现在九天太清宫众多弟子上空。
“一群败类，竟然与异魔勾结，真是死有余辜！”
那数道身影一出现，便是有着滔天般的气息席卷而开，袖袍挥动间，浩瀚的元力化为道道光幕，将九天太清宫的弟子尽数的护在其中。
“好强大的气息…这些人，都是道宗的强者吗？”苏柔震惊的望着天空上突然出现的那些身影，喃喃道她能够感觉到，这些气息，每一道，都不比他们宫主弱。
“其中一些人很陌生，应该不是东玄域土的强者或许…是林动从外面带回来的强者。”绫清竹柳眉微蹙了一下，旋即微微摇头，道。
“林动大哥带回来的强者？”苏柔一惊，这些顶尖强者，不论放在哪里都是大人物，林动大哥竟然能够将他们也请来？
“唰唰！”
这出现的数人，自然便是跟随着林动而来的小貂祝犁大长老等人，他们一现身，目光便是瞟了那些元门弟子一眼而后浩瀚元力奔涌，一掌拍出，磅礴的攻势便是将那些元门弟子掀得人仰马翻，再也进不得半步。
“这…”
那陆峰等人见到这一幕，面色也终于是剧变起来，他们目光望着林动后方那些气息惊人的强者眼中掠过一抹浓浓的骇色。
“大长老，这林动竟然带来了这么多强者…”一名元门长老骇然道，从那些人身上，他感到了浓浓的危险味道。
“我们现在怎么办？”另外一位长老也是连忙问道，眼下这模样，随着林动等人的出现他们已是失去了所有的优势。
陆峰眼神急速的变幻，旋即猛的一咬牙：“撤！”
“林动，你们等着，待得我元门三位掌教出关，你们道宗，必死无疑！”陆峰怒吼一声，然后他直接转身而退，那模样，竟是连那些元门弟子都是懒得再理会。
“既然来了又何必再走？”林动望着准备撤退的陆峰等人，却是淡笑道。
陆峰却是对此不闻不顾，转身就走，异常的果断。
不过，就在他们刚刚掠出不到千丈距离时，却是突然感觉到天地间陡然变得冰冷下来，天空上雪花飘飘落下，弥漫了天地。
“不好！”
陆峰见状，面色却是一变，目光急忙抬起，然后他们便是见到，在那不远处的一座山峰上，一道纤细身影正慵懒的坐在一颗青松之上，一头冰蓝的长发垂落下来，一对冰蓝色的美眸，却是蕴含着比那雪花更为冷冽的寒气。
“是道宗的应欢欢！”
陆峰等人毕竟是元门的顶尖强者，在这一年中与道宗的交锋中，也是听闻过道宗这横空出世的强悍人物，因此也是一眼将其辨认了出来。
青松上，应欢欢美目瞟了他们一眼，旋即缓缓站起，凌空踏出，一朵朵的冰莲在其足下成形，而后其修长玉指凌空一点。
“冰封之墙！”
雪花在此时疯狂的凝聚而来，然后便是在那陆峰四人周围化为四道通天冰墙，将他们的退路尽数的封锁。
砰砰！
那陆峰四人见状也是急忙爆发出道道攻势，然而却依旧未将那冰墙轰破而去。
林动的身影一闪，便是出现在那冰墙上方，他望着被困在其中的四人，面色淡漠，手掌一握，一道迷你的石碑便是出现在其手中，赫然便是被其炼化的大荒芜碑。
“放心吧，很快那三条老狗，也会去陪你们了。”
林动淡淡的道，旋即手掌一挥，大荒芜碑呼啸而出，迎风暴涨，化为数千丈庞大，在那碑底之下，有着无尽古老的波动弥漫而出，而在这种波动的压迫下，那陆峰四人体内的魔气，竟是在以一种惊人的速度躲避散逸着。
“轰！”
巨大的石碑轰然落下，整片空间都是不堪重负般的爆发出嘎吱之声。
“掌教，救我们！”
石碑镇压而来，那陆峰眼中终于是有着恐惧之色涌出来，而后他急忙嘶声咆哮道。
“砰！”
就在他咆哮声刚落的霎那，这片天空，突然急速的扭曲起来，一道银色巨手，直接是自那空间中伸出，然后一拳轰破那层层冰墙，就欲对着陆峰四人抓去。
“终于忍不住出手了么？元门的老狗！”
当那银色巨手出现的霎那，林动眼瞳之中，也是有着凌厉寒芒涌现，冰冷之声，响彻天际。


第1201章 第八道祖符
银色大手，洞穿空间，轰破那层层冰墙，然后径直的对着陆峰等人抓去，那番模样，显然是要在林动下杀手之前，将他们给救走。
林动脚踏虚空，黑眸冰冷的望着那银色大手，眼中却是没有太多的意外之色，旋即其心神一动，大荒芜碑再度迎风暴涨，然后方向一转，狠狠的对着那银色大手镇压而去。
“哼。”
似是察觉到林动的举动，那破碎空间中，似是有着一道冷哼声传出，旋即那银色大手竟是反身迎上，重重的与那镇压而来的大荒芜碑碰撞在一起。
嘭！
那般强大对碰，顿时将这片空间震碎而去，狂暴无匹的能量冲击席卷而开，那靠得最近的陆峰四人，更是一口鲜血忍不住的喷了出来。
嗡嗡！
大荒芜碑激烈的颤抖着，一圈圈古老的波动不断的散发出来，不过那银色大手也是极端的恐怖，竟是生生的将大荒芜碑的镇压给承受了下来，虽然银色大手也是在不断的颤抖着，但始终未让得大荒芜碑镇压而下。
“冰凤镇魔锁！”
而就在林动与那银色大手纠缠间，那不远处，蕴含着寒气的冰冷喝声也是响彻而起，紧接着那漫天冰雪竟是凝聚成十头巨大冰凤，而后冰凤呼啸而出，围绕着那银色大手一阵旋转，那凤尾竟直接是化为巨大的寒冰锁链，紧紧的缠绕在那银色大手之上。
惊人的寒气自锁链上弥漫出来，那银色大手上，顿时冒腾起浓浓的白雾，阵阵冰层，自银色大手上蔓延而开。
这些寒气，也是令得银色大手威力大降，那大荒芜碑顿时坐落下来，要将其镇压。
“唰！”
不过，就在这银色大手被林动与应欢欢二人联手逼入落入下风时，其上银光突然大作，空间扭曲间，那银色大手，竟是凭空的消失而去。
轰。
大荒芜碑镇压而下，但却是落了一个空，下方的大地，直接是被震得崩塌而下。
林动见状，眉头却是一皱，那银色大手消失得太过诡异，而且诡异的是，他竟完全没感觉到那银色大手是如何消失的。
那远处的祝犁大长老等人也是在紧紧的注视着这边，在那银色大手消失时，他们眉头同样是紧皱了起来，眼中掠过一抹凝重。
“小心！”
而就在林动皱眉间，应欢欢喝声突然传进耳中，旋即他便是感觉到周遭寒气猛的席卷而来，竟是在其身后陡然化为一片巨大的冰墙。
砰！
而就在冰墙成形时，那空间突然裂开，一道银色大手狠狠拍来，一拳便是将那冰墙轰破，不过趁着这短暂霎那，林动也是迅速回神，身形暴退而出，将那银色大手的诡异攻击躲避而去。
银色大手攻击落空，但却并未再度缠斗，而是银光闪烁间，再度诡异的消失。
林动见到这有些诡异的一幕，眼瞳也是微微一缩，在先前的那一霎，他能够感觉到一种奇特的力量散发出来…
“怎么回事？”应欢欢娇躯一动，出现在林动身旁，她眸子中也是有些疑惑，这银色大手太过神出鬼没了。
林动微微摇头，目光微闪，旋即一声冷笑：“我看你能鬼祟多久，欢欢，我动手解决那几个家伙，你注意四周。”
眼下既然这银色大手异常诡异，那林动就将其逼得现出身来。
声音一落，林动身形直接是化为一道虹光对着那陆峰四人暴掠而出，那惊人的杀气，冲天而起。
陆峰四人见到林动满脸煞气而来，也是面色大骇，急急后退，不过林动却是并没有给予他们太多反抗的机会，掌心一曲，便是有着温和的白芒铺天盖地的涌出来，而后将四人团团包裹。
在这种温和白芒的照耀下，陆峰四人便是惊骇的感觉到，体内的魔气，竟是在此时逐渐的消融，特别是陆峰，他本就身有伤势，这样一来更是雪上加霜，那面色愈发的惨白。
“哼。”
林动却是没与他们有丝毫的客气，袖袍一挥，大荒芜碑再度席卷而出，碑底之下，有着古老的光芒凝聚，浩瀚而雄厚。
大荒芜碑带着阴影，当头对着陆峰四人落去，不过就在大荒芜碑即将落下时，四人不远处的空间，再度出现了一些扭曲。
“又来了么？”
那空间刚刚出现扭曲，林动眼中便是寒芒暴涌，而后其身形如电般的掠出，直接出现在那大荒芜碑之上，璀璨的紫金光芒，铺天盖地的自其体内席卷而出，顿时无数道紫金龙纹，盘旋在林动周身，那种纯正的龙威，弥漫天地。
“吼！”
龙吟响彻，足足三千道紫金龙纹铺天盖地的呼啸而起，凝聚之间，竟是化为一道数千丈庞大的紫金龙爪，龙爪挥动，空间都是被蛮横的撕裂，而后林动眼中紫金光芒一闪，那龙爪便是狠狠的轰在那诡异出现的银色大手之上。
嘭！
狂暴无比的对碰，在天空上爆发开来，惊雷般的声音，轰隆隆的传开，即便是百里之外，都是能够清晰可闻。
那道银色大手这一次，终于是被震退而去，那之上闪烁的银色光芒都是稍稍黯淡了一些，显然面对着林动此次的有备攻击，这银色大手也是吃了亏。
“元门三条老狗，不知道你是哪一条？”林动咧嘴一笑，目光盯着那银色大手，而后缓缓的道：“你还真当我是三年之前么？本体都不出现就想在我手下救走人？”
“倒的确是个难缠的小子，当年本座就认为你会是个麻烦，没想到还真是被我一语成谶了。”银色大手之上光芒闪烁，一道淡淡的声音，传了出来。
“既然你不敢本体现身，那就让我来毁了你这玩意吧！”林动眼神微寒，身形竟是主动暴掠而出，紫金龙爪，狠狠的对着那银色大手怒轰而去。
而在林动对那银色大手出手间，应欢欢娇躯一动，直接出现在那陆峰四人前方，只见得其玉手一抖，漫天冰雪瞬间化为道道寒冰锁链，锁链之尖犹如毒蛇，闪电般的掠过虚空，竟直接是自那陆峰四人身体之上洞穿而出。
咔嚓咔嚓！
寒冰锁链洞穿四人身体，只见得层层寒冰急速的从四人身体表面涌起，还不待他们反抗挣扎，那冰层已是将他们尽数的笼罩，寒冰表面，还有着一道古老的符文微微闪烁，将四人封印在其中。
应欢欢倒是与林动颇有些心意相通，如今后者全力出手对付那不知道哪一位隐藏在暗处的元门三巨头之一，因此她也是果断出手，将这陆峰四人擒拿，也是免了那神出鬼没的银色大手趁机将人救走。
咻咻。
远处，祝犁大长老，小貂等人也是迅速掠来，他们望着天空上林动与那银色大手的交手，眼中也是掠过一抹凝重之色，能够凭借着遥远的传递力量与林动斗成这样，看来那暗中之人，实力也是相当恐怖，说不定，已是达到了轮回境的境界。
“砰砰！”
紫金龙爪与银色大手在天空上凶狠的对碰着，每一次的对碰，都将会掀起阵阵劲风风暴，刺耳的气爆之声，远远的传开。
“我说过，元门老狗，想要对付我，就降临真身，玩这种手段，太过可笑了点。”再度交手数次，林动也是摸清了这银色大手的一些规律，当即眼中寒芒一动，冷笑道。
然而对于他的冷笑，却是无人回答，那银色大手的攻击倒是愈发的凌厉。
“冥顽不灵的老狗。”
林动眼中寒芒涌动，终于是不再留手，只见得他身形一动，竟直接是出现在那银色大手之上，而后双掌一曲，猛的拍下。
嗤嗤。
黑芒以及雷光，犹如蜘蛛网一般，陡然自其掌心扩散而出，短短瞬间，便是将这银色大手包裹而进，吞噬之力以及狂暴的雷霆之力奔涌间，也是将这银色大手之中所隐藏的那种奇特力量，尽数生生的摧毁而去。
“破！”
林动脚掌猛然一跺，那银色大手之上，顿时有着裂纹蹦碎而开，旋即光芒溅射间，银色大手便是化为无数光斑，凭空消散而去。
而在银色大手被摧毁时，那前方的空间也是有所扭曲，隐约间，林动似乎是见到一道模糊的身影盘坐其中。
虽然林动未曾看清那道身影，但他却是知道，此人必定是元门三巨头之一！
“林动，今日算你有些本事，当年的丧家之犬，如今倒的确是有些令人刮目相看，你是想要找我三人复仇吧，哈哈，那就来元门吧，我元门三巨头，等你前来，想要灭我元门，就看看你有这等资格吧！”
空间扭曲着，旋即迅速的散去，而那道身影，也是诡异的消散而去，没有带起丝毫的能量波动。
林动眼神冰冷的望着那道自扭曲空间中消失的身影，旋即双手缓缓紧握，漆黑眸子中，掠过浓浓的杀意。
“你感觉到了吗？”
应欢欢出现在林动身旁，她看了一眼那道身影消失的扭曲空间，突然道。
林动微微点头，淡淡的道：“感觉到了…那家伙，先前动用的，是祖符的力量，元门手中，应该也是掌握着一道祖符。”
应欢欢柳眉微蹙，轻声道：“若是我没有猜错的话，元门手中的那道祖符，应该便是八大祖符之中，最令人难以捉摸的…空间祖符。”
“空间祖符么…”
林动喃喃自语，看来元门这些年，隐藏得真的极深啊，没想到，连这一道外人所知最少的祖符，都有落进了他们的手中…


第1202章 空间祖符
天空上，林动凌空而立，脸庞上有着一抹沉思之色，这突然出现的空间祖符，显然是大大的出乎了他的意料。
“那元门与异魔之间的勾结显然是不用再怀疑了，既然如此，那元门三巨头又如何能够掌控空间祖符？”林动喃喃道，一般说来，祖符对于异魔皆是有着相当强烈的克制性，只要身体中拥有着魔气的存在，那祖符必然会对其竭力反抗，更遑论将其掌控了。
但先前那种波动，林动却是感知得极为的清楚，那的的确确是租符的力量，而且看那银色大手能够随意的穿透空间的能力，也只有八大祖符之中的空间祖符方才能够办到，不然的话，即便是踏入轮回境的巅峰强者，都很难做到在穿过空间时，如此的不着痕迹。
应欢欢也是微蹙着眉，缓缓的摇了摇头，显然她也并不太清楚，元门究竟是使用了什么手段，来操控了空间祖符。
“算了，不管他们用的什么手段，不过祖符这般天地神物，可不能落在他们的手中。”林动淡淡一笑，那漆黑眸子中，寒芒涌动：“待得将这三条老狗解决了，将空间祖符抢过来便是！”
应欢欢点点头，旋即她那冰蓝色的眸子突然瞥了一眼远处，纤细玉指挽起垂落耳畔的冰蓝发丝，美丽而冰寒的脸颊上，神色莫名。
在那视线所及处，同样是有着一道仿佛集天地灵气而生的绝色女子。
林动也是察觉到了她的目光当即刚刚在面对着那元门巨头都未曾有丝毫动容的脸庞，顿时变得有些不自在起来。
眼下三人间的关系，想想的确是略微有些尴尬，林动与绫清竹之间有着肌肤之亲，很多年以前他之所以会从那小小的大炎王朝中走出来，有很大一部分的原因，是想要追逐这个曾经在他生命中昙花一现的人儿，绫清竹是骄傲的，而林动也是偏执的，当年的我需要仰望你，那我就将你超越！
对于绫清竹他不得不承认，他内心深处对她有着不小的征服欲望，这个欲望在当年那一座孤峰上便是埋下了种子。
而在那种征服欲之下，也是有着一抹属于男人的霸道，他无法想象，若是绫清竹依偎在别的男人怀中时，他的心境是否能够保持属于他的平静。
当年的荒唐事，都是在两人的心中留下了极深的烙印，再也无法抹除，这一点，或许两人谁都没办法否认。
而应欢欢则是在之后遇见，面对这个女孩对自己的诸多付出以及等待，林动就算再铁石心肠也是无法将其无视异魔城中的那一幕幕，即便是如今想来，都是在心中泛着微酸的味道。
这个曾经娇俏活泼的少女，从某种角度来说，也是因为他，方才变成了如今这般的冰山美人，那种性格之间的转变，其中究竟经历了多少的心酸与疼痛，或许就连应欢欢自己都是无法数清。
这两个女子的优秀，毋庸置疑，一个是九天太清宫这么多年以来最为优秀的弟子，一个如今更是拥有着冰主轮回者这个恐怖身份，而这也造就了她们心中的骄傲，两个同样光芒耀眼的女孩碰在一起，想要她们谁服个软，显然是一件不可能的事。
而这，也同样是林动倍感头疼的地方。
“杀！”
而在林动因为这般事情苦恼得微微失神时，下方也是有着惊天般的厮杀声响起，只见得那些九天太清宫的弟子，此时已是在对着那些逃窜的元门弟子冲杀而去，而失去了诸多长老的而元门弟子，显然士气全无，因此面对着九天太清宫弟子的追杀，一时间，倒是漫天惨叫声，不绝于耳。
这种时候，已没有了什么残忍的说法，元门发动了战争，这便是需要所付出的代价，更何况，这些元门的弟子，也已被魔气侵蚀，长久下次，怕也只会理智淡薄，沦为只知杀戮的人形野兽。
厮杀声持续了许久，漫山遍野都是元门弟子的尸体，一些侥幸活下来的，也是狼狈的四处逃窜，再没了之前的凶狠。
“林动大哥！”那远处，苏柔对着林动所在的方向扬了扬小手，小脸上因为兴奋而显得一片涨红。
林动见状，终还是迈开脚步对着那里落去，在其后方，应欢欢微微犹豫了一下，然后轻咬了咬红唇，不快不慢的跟在他后面。
“林动大哥，你没受伤吧？”苏柔望着落到身前的林动，急急的将其看了一圈，这才笑道。
此时周围也还有着无数九天太清宫的弟子，他们也是目光好奇的盯着林动，那眼中弥漫着感激，今日若非林动等人出手相助，或许九天太清宫，也将会成为东玄域上的一个历史。
林动笑着摇了摇头，然后将目光看向了苏柔身后那一身白裙，气质清冷得犹如谪仙般的绫清竹，后者察觉到他的目光，则是微微侧头，却是将视线投向了林动身后的应欢欢。
两女目光交织，空气仿佛在此时悄悄的凝固了一瞬。
两人的气质，都是有些冰冷，只不过应欢欢因为体质的缘故，周身的寒气异常的冻人，而绫清竹的那种清冷，则是来源于她的气质。
两女不论放在那里，都是那种近乎万众瞩目的焦点般的存在，然而如今放在一起，彼此的光芒以及冰冷，倒是有种互扎的感觉。
这片区域，仿佛都是在此时陷入了两女的那种奇特气场之中，这令得周围那些九天太清宫的弟子也是紧闭了嘴巴，目光不断的在两人身上扫来扫去，然后又是瞥瞥处于两女中间的林动，一些敏感之人眉头微微挑挑，最后将一种又是艳羡又是同情的目光投向了林动。
小貂，祝犁大长老等人也是掠来，他们见到这古怪的气氛，倒是忍不住的一笑，望向林动的目光有些戏谑。
“咳。”
古怪的气氛持续着，而应欢欢与绫清竹皆是没有开口说话，最后林动只能一声轻咳，将气氛打破，然后他看了一眼这残破的九天太清宫，略作沉默，道：“你们宫主？”
“师傅坐化了。”
绫清竹秋水般的眼瞳中掠过一抹黯然，道：“长老们也皆是重伤，如今我们九天太清宫，算是元气大伤。”
林动轻叹了一声，这毕竟也是没办法的事，与元门比起来，如今的九天太清宫的确难以形成多少阻碍，若他们再来得晚些，恐怕九天太清宫的损失将更为惨重。
“这次…我代九天太清宫所有弟子，谢过了，还有，其他诸位道宗的朋友以及外域的前辈。”绫清竹轻声道。
“呵呵，都是应该的。”祝犁大长老笑了一声，旋即他摇了摇头，道：“算了，你们年轻人多聊聊吧，这气氛老夫我有些受不了。”
说着，他便是转身而去，小貂等人笑了笑，也是跟着散开，周围那些九天太清宫的弟子，也是三三两两的退开，只是那好奇的目光，还是不断的在对这边投射而来。
应欢欢见到周围散去的众人，冰蓝的美目看了林动一眼，然后冲着绫清竹微微一笑，仲出纤细玉手，道：“道宗应欢欢，我想我们应该已经见过了。”
“绫清竹，欢欢姑娘的名字如今在这东玄域，怕已是无人不知。”绫清竹那绝美的脸颊上，同样是有着一抹极为罕见的笑容浮现出来，她仲手与应欢欢轻轻一握，骄傲的两人，谁也不愿意在对方面前露出丝毫的下风。
“那你们先聊。”应欢欢并没有再多说什么，只是眸子看着林动，声音变得柔和了许多：“处理完事我们就先回道宗吧，我等你。”
说完，她不再停留，也是转身而去。
“欢欢姑娘对你很好。”绫清竹望着应欢欢离去的倩影，沉默了一下，突然道。
“你去过大炎王朝？我娘说她见过你。”林动笑了笑，目光盯着绫清竹，道。
绫清竹貌似很平静的点了点头，只是那白皙的脸颊上，依旧是在此时渗透出了一丝极淡的红润，道：“正好有事去大炎王朝，然后，顺便看了一下你的爹娘。”
她似是平静的在说着，但那素来没有太多情绪波动的眸子，却是在此时有点细微的涩意，因为这借口，实在是太难以让人信服了点。
林动笑着点了点头，道：“我娘说啥时候把你们带回去就好了。”
绫清竹白了他一眼，那股子风情让得人骨头都酥软了一下，然后她柳眉扬了一下，敏感的感觉到一些词汇：“我们？”
林动顿时头大起来。
绫清竹红唇微微撇了一下，然后偏过头去：“三年不见，你倒是变了很多。”
林动笑了一下，他抬起头，望着那斜落的残阳，道：“因为我想再回来啊。”
绫清竹沉默，短短的一句话，让得她明白这三年时间，眼前的青年究竟付出了多么巨大的艰辛，外人只是见到归来后他的光芒万丈，却是又怎会知道为了获得这些力量，他多少次在生死间徘徊…
“其实，三年前，你也很厉害的。”
绫清竹轻声道，她的脑海中，掠过三年前的画面，那道满身鲜血的单薄身影，手持长枪，即便是面对着元门三大巨头，却依旧是没有丝毫的惧意，那般气魄，令无数人为之动容，包括了她。
林动笑了笑，然后他缓缓的伸了一个懒腰，深深的吐出一口气：“既然回来了，那就也该将所有的事情都做一个了结了啊。”
“你打算怎么做？”绫清竹察觉到他话语中突然涌出来的血腥，道。
林动抬头，目光盯着那如血般的残阳，淡淡的声音，缓缓的传开。
“三天后，与元门决一死战！”


第1203章 诛元盟
元门试图抹杀九天太清宫的计划，因为林动等人的赶到，算是就此夭折，不过虽说元门溃败，但九天太清宫也是化为了一片废墟，其宫主更是重伤坐化，其余长老也皆是重伤，如今的九天太清宫，显然是极为虚弱的时候。
“你带着九天太清宫的弟子先随我们去道宗吧，这里已经不能留了，万一元门再来偷袭，九天太清宫的弟子必然会死伤惨重。”想到如今的九天太清宫已是无多少自保之力，林动也是以此来征询着绫清竹的意见。
“这…”
而听得此话，绫清竹显然是微微犹豫了一下，这样去道宗的话，难免会有点寄人篱下的感觉，虽然她知道林动所说是最好的办法，但心中难免还是有点小小的抗拒。
“你现在可已经是九天太清宫的宫主。”林动看了她一眼，缓缓的道。
绫清竹玉手紧握了一下，旋即螓首轻点，现在的她，的确要时刻为这些九天太清宫的弟子着想，即便因此受一些委屈。
“你先吩咐一下吧。”
林动望着眼前的废墟，轻叹了一口气，短短数日时间，那曾经屹立在东玄域之上的超级宗派，便是差点烟消云散，想来此时的绫清竹心中，也是颇为的不好受吧。
说完，他也是转身走开，而后坐在山头上，望着那开始将消息传递下去，而导致九天太清宫的弟子们有些低落的情绪。
“你是让她们去我们道宗吗？”身旁有着寒气靠近过来，应欢欢也是在他身旁优雅的坐下来，玉手托着香腮，美目望着那些收拾着残局的九天太清宫弟子。
“嗯。”
林动点了点头，笑道：“不会介意吧？”
“我才没那么小气。”应欢欢白了他一眼，然后她盯着眼前的废墟，喃喃道：“其实我挺理解她的，因为在你未回来之前，整个道宗都是我在担负着，元门强者层出不穷，三巨头更是从头到尾未曾出过手，我担心哪一天我失败，我们道宗那么多的弟子，也将会如眼前这般…”
“这三年，我很想你，但同时又必须倾尽全力的保护着道宗…”应欢欢修长玉腿轻轻蜷起，有着俏美尖弧的雪白下巴抵着膝盖，那一直有着寒气缭绕的俏脸上，却是在此时涌上一些令人心疼的柔弱。
“我害怕等不到你回来，或者…等你回来时，道宗已毁，而我，也被逼成了另外的一个人。”
林动怔怔的望着身旁的女孩，这三年他固然过得不容易，但留在这里的人，又何曾简单过？三年前的她，活泼可爱，她是所有道宗弟子捧在手心的小公主，每天快快乐乐什么也不用多想，然而之后，那整个道宗无数弟子的存亡，皆是落在她那娇嫩的肩上，那种重担，想象都是让人心疼。
“辛苦你了，不过我回来了，这些担子就让我来吧，我这人皮糙肉厚的，对扛这些东西很擅长的。”
林动眼神温柔，伸出手掌，轻轻的揉了揉应欢欢脑袋，柔顺的发丝透着点点冰凉，极为的舒服。
“嗯。”
不过他的这番动作，却是令得应欢欢那俏美的脸颊上掠过一抹淡淡的绯红，这位如今在道宗的声望并不弱于林动的女孩，以一种极为罕见的乖巧，轻轻的嗯了一声，而后她用仅有自己方才能够听见的声音，喃喃道：“你放心吧，不管你做什么，我都会帮你的。”
远处，正在吩咐着九天太清宫的弟子收拾着残局准备撤退的绫清竹突然微偏过头，然后美目便是看见了山头上的两人，贝齿轻咬了下红唇，那素来古井无波般的心境，荡起了一些紊乱的涟漪。
“啧啧，清竹师姐，看来你这次遇见了一个很厉害的对手啊。”一旁，突然有着低低的声音响起，只见得苏柔不知何时凑了过来，她视线也是不断的看着那山头上，笑嘻嘻的道。
绫清竹淡淡的看了她一眼，道：“应欢欢为了他能够在那异魔城挺身而出，更是以命逼应玄子现身相救，这般敢爱敢恨心性，我自是不及她。”
“哪有，你们毕竟是两种性子，表达方式也是不同，哼哼，我其实也是知道的，你喜欢林动大哥，不然的话，以你的性子，怎么可能会每年都去大炎王朝一趟。”苏柔道。
“清竹师姐你的心虽然包裹着一层冰，不过那冰后面可是火热热的哦，只不过这种火热，恐怕这辈子，也就林动大哥能够有幸享受到了…”
面对着苏柔这般打趣，绕是以绫清竹这静如莲花的钉子，那绝美的脸颊上都是略过一抹红润，作势恼道：“你这丫头，谁教你的这些讨厌门道，快去帮忙，然后撤退。”
“是。”
苏柔笑着应了一声，挥挥小手：“不过清竹师姐可别错过了哦，不然世界上可没后悔药可吃。”
绫清竹望着笑着跑开的苏柔，红唇微抿，刚刚方才逐渐平静下来的心境，却又是波动起来，那般涟漪，愈发的浓郁。
花费了半日的时间，残局收拾完毕，林动等人便是再未有所停留，目光看了一眼这片在数日前还恢弘大气的地方，最终一声轻叹，手掌一挥，带着众多人马，转身而去。
大部队回到道宗，自然也是引发不小的骚动，道宗弟子望着那些神色黯淡低落的九天太清宫弟子，皆是暗叹，这场战争波及整个东玄域，若不是他们道宗出了一个应欢欢，再加上林动及时归来，恐怕下场还比九天太清宫更为的悲惨。
道宗宗殿之内，应玄子听得九天太清宫所发生事，也是一声长叹，神色略显暗淡。
“如今东玄域，除了元门，便仅剩下我道宗还尚存，我想元门，应该也不会放任我们继续固守的。”应玄子道。
“掌教也不用过于担心，此次九天太清宫虽然损失不小，但元门同样付出了极大的代价，诸多长老尽数被抹杀，想来就算是元门有着异魔暗中协助，但也不可能视这种损失于无物。”
林动摇了摇头，道：“如今的元门，或许已无再主动进攻之力，那元门三巨头狡猾异常，这般时候，恐怕不会轻易动手。”
“那你的意思？”应玄子目光一闪。
“主动进攻，灭掉元门，结束东玄域的混乱。”林动眼中凶芒涌动，声音却是平淡之极。
应玄子等道宗长老闻言，皆是惊了一下，如今元门势强，大都是选择避其锋芒固守宗门，但林动却是比他们更狠，竟要主动杀上元门。
“爹，元门势强，如今我道宗也并不弱，真要拼起来，鹿死谁手还犹未可知。”
应欢欢也是在此时开口道：“而且异魔诡计多端，若是任由元门拖下去，谁也保不准会不会有其他变故。”
林动微微点头，在这元门后面，或许便是站着那所谓的“魔狱”，这些家伙，显然是有着重大图谋，他们不敢明面现身，所以便是侵蚀元门来代为动手，若他们不尽早解决掉元门，或许祸端难平。
“应玄子前辈，元门经此损失，或许在东玄域，已并不算是势强…”那一直静静听着众人说话的绫清竹，突然轻声道。
“哦？”应玄子一怔，看向绫清竹。
“元门发动的战争，波及了整个东玄域，诸多超级宗派被其毁灭，不过这些超级宗派虽然被毁，但却并不是彻底的抹杀，其中依旧有着不少强者逃脱元门杀手，若此时道宗主动出击，以诛灭元门为口号，我想，或许会有无数隐藏在暗处的强者蜂拥而来，到时候，局势逆转，落入下风的，便该是元门了。”绫清竹略显清冷的声音，在大殿中回荡着，却是令得不少人眼睛都是猛的亮了起来。
林动也是诧异的看了绫清竹一眼，这手倒是来得狠，直接釜底抽薪，借助着元门所制造的仇恨，将那些隐藏在暗处的各方强者尽数的与他们绑在一起，一同对元门宣战，这样一来的话，元门便是真正的要与整个东玄域一决生死了。
“林动，你认为如何？”应玄子眼中亮光掠过，脸庞上有着一道难掩的兴奋之色。
林动笑着点了点头。
“好！”
应玄子猛的起身，面庞在此时有些激动，这一年来，道宗都快被元门压得喘不过气，而现在，这种局面，真的能够开始逆转了吗？
“将消息传出去，广邀东玄域众多强者，共建诛元盟，三日之后，与元门，一决生死！”
诛元盟的消息，很快的便是在东玄域之上流传而开，虽然在刚开始的时候，并没有太大的动静，但随着消息真实程度被证实后，这片地域，瞬间沸腾起来，无数道隐藏在东玄域各个地方的强者，皆是暴冲而起，然后急速的对着道宗所在的方向而去，在这些强者的面庞，有着相同的情绪，那是对元门的仇恨…
东玄域的天空，仿佛都是在这短短两三天内变得热闹非凡，无数道身影破空而去，源源不断的对着道宗而去，一种复仇的气息，悄然的笼罩了整个东玄域。
元门。
如今的这座宗派，已是没有了以往的那种大气磅礴，黑色的魔气，缭绕在宗内，阴寒之风不断的吹拂着，那些穿梭在其中的元门弟子，吸纳着那魔气，面色陶醉而狰狞。
这个宗派，已是犹如一片魔域。
而在元门最深处，一座深涧上，三道身影负手而立，隐约间，有着极端恐怖的气息，缓缓的自他们体内荡漾而出。
“听说道宗要组建“诛元盟”，与我们决一生死了啊…”三道人影，其中一人突然轻笑了一声，道。
“那就等他们来吧，将这联盟解决掉，东玄域，应该就不会再有任何的反抗了，我们元门，将会是这里唯一的主宰。”三人居中，是一名身着白袍的男子，他双目之中，呈现黑白之色，犹如阴阳沉浮，玄奥无比，此人正是元门三巨头之中的天元子。
“我与那林动交过手了，倒是没想到三年时间，他竟然强到了这种地步，真是有趣。”天元子身旁，一头银发的人元子淡漠一笑，道。
“三年前我们能随手抹杀他，三年后，同样如此，结果不会有丝毫的改变。”天元子淡淡的道。
人元子笑着点点头，袖袍一挥，一道银光自其手中掠出，冲进下方深涧：“这空间祖符我果然还是无法彻底动用，毕竟不是自己的东西啊，真是可惜…”
银光冲进深涧，那极深处，黑雾波动，似是有着一道模糊的人影犹如磐石般的盘坐着，银光一闪，便是钻进了他的身体。
天元子望着那深渊最深处的模糊人影，脸庞上却是有着一抹诡异之色浮现出来，喃喃道：“诛元盟，呵呵，来吧，来吧…道宗，我会给你们一个你们永远都想不到的惊喜的…”


第1204章 汇聚，决战来临
短短三日，东玄域的风气，不知道何时开始，竟已是出现了一些奇特的变化，那些原本四处躲藏着元门追杀的强者，陆陆续续的从藏身之所走出，然后走向了如今东玄域之上除了元门外唯一还尚存的超级宗派，道宗。
一则所谓的“诛元盟”，所引发的震动以及效果，远远的超越了应玄子等人的预料，因此当他们在第一眼看见那道宗之外铺天盖地涌来的强者时，那面色还略微的有些滞然。
说到底，还是他们小看了元门在这东玄域之上所制造的仇恨。
道宗之外，漫山遍野都是人影，喧哗的声音汇聚在一起，冲上云霄，直接是将这片辽阔的山脉变得吵杂如闹市。
在天空上，还时不时的有着道道光影掠至，其中不少气息强横之人，不过他们皆是有着一个相同点，那便是眼中都是涌动着浓浓的恨意。
宗门被毁，师兄弟被残杀，亡命天涯，这任何一个，都足以让得他们与那元门至死不休。
嗡嗡。
伴随着越来越多的强者赶来，那道宗的护宗阵法也是颤动起来，而后光罩缓缓的散去，再接着，浩瀚般的元力波动犹如潮水般的扩散出来。
咻咻！
无数道身影，铺天盖地的自其中掠出，最后悬浮天际，那凝聚在一起的气息浩瀚强大，每一位道宗弟子脸庞上，都是有着一种自信，即便他们明白现在所要面对的对手究竟有着多么的强大。
那些来自其他超级宗派的强者，他们望着道宗弟子这般气质也是一声暗叹，与这道宗比起来，他们宗内的弟子，的确是要弱上一筹，难怪道宗能够在元门的屡次进犯下，成为东玄域上仅存其二的超级宗派。
“轰！”
而在他们感叹间，突然间有着滔天般的煞气冲天而起这股煞气之浓，令得无数强者为之一惊，急忙抬头只见得一片黑色云彩，自那道宗之内掠出，最后纹丝不动的悬浮在天际上，他们的呼吸，宛如一体，浓浓的煞气仿佛在他们上空化为一头万丈巨虎，凶气滔天。
“这…”
那些各方强者见到这般凶悍的人马，面色顿时大变，眼中掠过一抹骇色，显然都是察觉到了这支人马的厉害。
“这支人马是道宗的？为何以前从未听说过？”
“据说是道宗的林动从外域带回来的，极其的厉害。”
“林动？难道是三年前那被元门三巨头逼出东玄域的道宗弟子林动？”
“嘿不是那位还能有谁？不过现在的他，可不再是当年那无名小子了，三日之前，元门大长老陆峰率人马攻打九天太清宫，他赶去救援，结果那些元门的老杂碎，一个都没跑掉。”
“这么厉害？道宗真是有福了啊，当年出了一个周通，现在又是出了一个林动真是…”
“道宗敢正面对元门开战，也是因为这林动归来，而且他回来时，还带了一批实力极端恐怖的帮手…这一次，元门那三个老杂毛，总算是遇见对手了。”
随着那批凶悍人马的出现，这片天地也是陡然间爆发出众多窃窃私语声，不少人的目光中都是带着惊讶与好奇，显然这短短数天的时间，林动这个曾经在东玄域传扬过一段时间的名字，再度被挖掘了出来。
“咻。”
随着虎噬军出现后，后方又是有着道道流光掠出，然后悬浮在天空上，那般身影，正是应玄子，林动，祝犁大长老等人。
而当应玄子现身后，这片喧哗的天地顿时逐渐的安静了下来。
“诸位，想来我们都不算陌生了，对于这一年来大家的遭遇，我道宗也是深表同情，不过以往道宗也是堪堪自保，无法伸出援手，所以还希望诸位不要心生怨恨。”应玄子目光一扫，双手一拱，沉喝声，在这山岳之中回荡起来。
“应掌教言重了，这般劫难，是我们之前太过短见，若是能够及时联手，又怎会落得这般下场。”在那不远处，有着大批的人影凌空而立，最前方是一名枯瘦的青衣老者，老者背负着一柄青色大剑，在其周身，有着极端凌厉的剑气散发出来，而此时，这名老者的面色，分外的苦涩。
“唉，墨掌教。”应玄子望着那青衣老者，也是叹了一声。
“这人是谁？”林动看了那青衣老者一眼，后者气息极强，竟也是踏入了转轮境的超级强者，而且此人周身的剑气，相当的强横，显然不会是无名之辈。
“那是剑宗的宗主，墨鉴，不过如今剑宗早已烟消云散，恐怕也就眼前这些规模了。”应欢欢在身旁轻声道。
“在这里还有着一些曾经八大超级宗派的宗主以及长老，他们从元门的手中逃了出来，不过之前一直都不敢现身。”在林动左侧，绫清竹美目扫过那茫茫人海，而后有着略显空灵的悦耳声音传出。
“多亏了你这办法，若是能够将这般力量整合，解决掉元门的把握又将大上不少。”林动由衷的赞道，若光依靠他你道宗去与元门决一死战，即便最后能够胜利，恐怕也必然是一场惨胜。
“这也是因为你三天前那场大胜才能有这番效果，不然的话，这些人也必然是不敢现身的，到时候，他们或许宁愿选择离开东玄域，都不会与元门为敌。”绫清竹螓首微摇，道。
“要依靠这些人来组建一个“诛元盟”，我想若是不好好敲打敲打，恐怕会成事不足败事有余。”应欢欢淡淡的道。
“欢欢姑娘说得不错，这些人的确是不小的战力，但有时候，这种力量无法控制，反而会是累赘。”绫清竹看了应欢欢一眼，也是点了点头。
林动微微点头，然后他与应玄子对视了一眼，也是缓步走出，而随着他走出来，那漫天的目光，则是迅速的投射到了他的身上，隐约的有着一些骚动传出，想来是将其认了出来。
“诸位东玄域的朋友，今日将大家请来，想来都知道是何原因，元门发动战争，波及整个东玄域，众多超级宗派宗毁人亡，想来对于元门，诸位应该是我道宗一般的心态。”
林动目光环顾，眼中凌厉之色闪烁，那般气势，竟是连墨鉴这等剑宗宗主都是暗感心凛，他那沉声，也是在天地间回荡着。
“不过对于元门的实力，诸位也同样清楚，想要解决这个祸害，唯有一途可走，那便是将我们的力量，尽数的汇聚在一起。”
“我道宗与元门之仇不共戴天，今日宗门倾巢而出，誓要与元门决一死战，若是诸位与我道宗是一条心，那我道宗必然视其为兄弟盟友，不过，若是有谁暗中使绊子，动我道宗灭元之心，那我林动，也唯有将其视为元门之人，一同对待！”
“轰隆！”
喝音一落，林动眼中凶光陡然掠过，滔天般的紫金光芒席卷而出，数千道紫金龙纹盘旋，竟直接是化为一道数千丈庞大的紫金龙爪，而后一爪挥出，只见得大地崩裂，一道万丈之长的深深沟壑，便是将这片山岳，生生的一分为二，那般声势，骇人之极。
巨声，滚滚不休的在天地间回荡着，下方大地尘雾弥漫，那无数强者望着那突然间变得煞气惊人，犹如一尊杀神般的林动，眼中皆是有着浓浓的惊愕，额头上，也是有着冷汗浮现着。
与应玄子的温和不同，眼前的这年轻人的手段，似乎更为的凌厉。
“吼！”
滔天般的虎啸声也是随之响彻，虎噬军仰天嘶吼，滚滚煞气，遮天蔽日，更是将此时的林动，衬托得杀气凛然。
整片天空，都是逐渐的安静下来，就连那些曾经是超级宗派的掌教或者长老的超级强者，也是因为林动这般煞气而心生余悸，不敢触其霉绫清竹望着天空上那林动一言将这东玄域无数强者震慑的凛然模样，唇角也是有着淡淡的弧度掀起来，谁能想到，多年之前那个稚弱的少年，如今却是成长到了这种地步，现在的他，恐怕即便是要说成东玄域的真正第一人，都未曾不可。
“林动小哥说的是，我们与那元门皆是灭门之仇，我们的目的相同，使绊子的事是绝不会做出来的，而若是有人要这般做，我想在场的人，应该都不会放过他。”那剑宗的墨宗主逐渐的回过神来，眼中余悸渐退，而后抱拳笑道。
“墨宗主说得对，只要能灭了元门，林动小哥，你有任何差遣，我洪荒宗莫敢不从！”那不远处，一名身材极为干瘦的老者也是沉声说道，这人乃是洪荒宗宗主，也是东玄域上赫赫有名的超级强者。
有了他们两人领头，那顿时有着众多应喝声响起，一时间，无数人情绪激昂，那对元门共同的仇恨，也是在此时会汇聚在了一起。
应玄子望着眼前这气势与之前截然不同的庞大人海，也是悄悄的松了一口气，望向林动的目光中，有着欣慰之色掠过。
“多谢两位掌教。”
林动也是冲着那墨掌教二人一拱手，那滔天般的煞气，终于是缓缓的收敛：“既然我们共同一心，那诛元盟便就此成立，事后诸位是留是走，全凭各自之愿。”
“而现在…”
林动声音一顿，他目光远眺，望着眼前那铺天盖地的人海，那些目光中，皆是充斥着对元门的仇恨。
林动的手掌，缓缓的抬起，下一霎，一股凛然杀气，陡然涌上眼眸。
“动身！今日，必灭元门！”
“吼！”
震天般的嘶吼声，响彻天地，旋即这片山岳竟是颤抖起来，元力沸腾间，无数道身影呼啸而出，然后犹如滔滔洪流，自天际奔腾而过。
与元门的决战，终是来到。


第1205章 元门之外
元门，百里之外。
一座山峰上，上百名元门弟子汇聚在此处，这是元门宗门之前的哨点，专门用来监视宗门周围的一举一动，在以往时候，这种哨点自然是未曾存在，因为那时候只有元门去打别人的份，哪还有人来元门这里撒野。
当然，也说了，那是以前。
如今东玄域的局势，显然是在这短短三天时间内，出现了一个惊人的转变，这种转变，甚至是连那些依附在元门之下的各方王朝都是有所感觉，因此其中一些聪明的王朝，皆是连忙收回手脚，固守在各自的王朝中，再也不敢胡乱去攻打其他的王朝，他们很清楚，一旦元门落败，虽然不可能将他们这些依附元门的所有王朝抹杀，但想来一些跳得最狠的，必然会遭到清洗。
所以，在这局势未明下，还是老实的收敛一点最好。
山峰上，这些哨子锐利的目光不断的扫视着周围，时不时的会有着人掠出，向着其他地方查探而去。
“那是什么？”
时间流逝间，突然有有着一人惊声道，在其身旁，也是有着一些人抬目望去，只见得在那遥远的天际，光芒突然变得明亮了许多，那滔滔的光芒中，仿佛有着无数道身影。
“糟了，发信号！”
那些哨子，面色猛的大变，眼中有着一抹恐惧之色涌出来，尖叫声，陡然自他们嘴中传出。
咻！
火红的光芒，自他们手中喷射而出，最后在天际之上绽放开来，即便是百里之外，也是能够清晰可见。
而就在信号刚刚放出时，那天际之边的光芒瞬间席卷而来，他们这才看见，那光芒之中，竟是铺天盖地的人影，这些人影，目光凶狠的将他们给盯着，那眼中，有着刻骨般的仇恨。
唰唰！
没有丝毫的废话，数不尽的攻击瞬间自那天空上呼啸而至，整座山峰，都是在那般狂暴的攻势下，尽数的夷平。
山峰夷平，天空上那铺天盖地的身影没有丝毫的停留，直奔那远处的元门宗门而去，而也就是在这边天空信号扩散的同时，那另外的数个方向，也是有着灿烂的信号冲天而起，不过很快的，那些火光，便是被后方蜂拥而来的光影，尽数的淹没。
复仇的火焰，从四面八方而来，将元门包围得水泄不通。
元门宗门。
魔气缭绕在其中，仿佛有着凄厉的鬼啸之声从中传出，而此时，这元门内也是混乱起来，无数道身影闪掠，而后他们抬头，便是见到了那远处席卷而来，近乎看不见尽头的光影人海。
那般数目，根本不知道究竟来了多少人。
如此阵仗，就算是元门的这些弟子眼中有着魔气缠绕，但脸庞上，依旧是掠过一抹浓浓的惊骇之色。
“杀！”
天空上，无数道身影出现在元门宗门之外，其中不少人目光一看见元门的人，眼睛便是血红起来，一声咆哮，成千上万道身影，直接是红着眼冲向元门。
“哼，找死的东西。”
而就在他们冲击元门宗门时，那元门之内，一道冰冷之声响彻而起，旋即一道千丈庞大的黑色匹练猛的暴射而出，匹练横扫而过，只要是被沾染上者，身体瞬间被腐蚀成森森白骨，连惨叫声都未曾发出一声，便是从天空上坠落了下去。
遭遇了这么凶狠的阻拦，那些人眼中的猩红方才被惊吓而退，恢复理智后，他们急忙向后狼狈的退去，那望向元门宗门之内的目光中，充斥着恐惧之色。
有了先前那些莽撞家伙的前车之鉴，后面那铺天盖地的光影也是冷静下来，再不敢乱闯，一个个立于天空上，用仇恨的目光将那防卫森严的元门给盯着。
漫天光影，在此时出现了一些骚动，而后裂开一道巨大的裂缝，道宗那庞大的人马，涌入而进，领头当先的，正是林动，应玄子等人。
而随着他们的出现，周围那些强者士气也是振奋了一些，那望向元门的眼中，也是多出了一些怒火。
林动脚踏虚空，目光看向那魔气缭绕的元门之内，忍不住的摇了摇头，没想到这曾经东玄域的最强大宗派，如今，却是变成这般魔域之“元门三狗，都这般时候了，还要缩着不成？”
林动淡漠的视线望向元门深处，声音却是在雄浑元力的包裹下，犹如雷鸣般，在这天地间轰隆隆的响彻起来。
“大胆，你们竟敢闯我元门宗门，当真是找死！”在那元门宗门上空，一名元门长老厉声喝道。
“这里也有你说话的地方？”柳青眼睛一瞪，猛的张嘴，顿时一道龙吟声波席卷而出，声波蕴含着滚滚龙威，震破空间，狠狠的对着那名元门长老冲击而去。
不过，就在那龙吟声波刚刚冲进元门宗门时，一道黑光陡然自那宗门深处掠出，直接是生生的将那道龙吟声波震碎而去。
“呵呵，你倒是有些的能耐，竟然连龙族的强者都请到了东玄域来。”
声波震碎，那深处三道黑光掠来，最后便是径直的出现在了天空上，三道人影，居右之人，皮肤白皙如婴儿，一头银发，居左者，一身黑袍，皮肤洁白如玉，一只手掌，显得格苍白。
而在两人之中，便是一位白袍男子，他那对眼瞳，黑白相融，犹如阴阳沉浮，深邃得令人仿佛要陷入其中。
这般模样，这般与三年之前如出一辙的气势，除了那元门三巨头之外，还能有何人？
林动目光停留在三人的身上，那眼眸中，有着一种浓郁到极致的杀意在涌动着，这一天，自从三年前被逼出东玄域时，他便无时无刻不是在期盼着…
这三年，为了这一天，他已经不知道他究竟在生死间徘徊过多少次。
呼。
林动深深的吐出一口气，而他的气息，也是在这口气吐出时，逐渐的改变，若说之前他是一柄带鞘长剑时，那么此时，这把剑，已是出鞘。
凌厉之气，仿若洞穿了云霄。
小貂，小炎手掌也是缓缓紧握起来，那眼中，有着无尽的凶芒在闪烁。
元门之内，天元子三人也是感应到了林动变化的气息，当即眼神微微一凝，脸庞上的神色略凝重了一些，不管他们承不承认，但无可否认的是，现在的林动，的的确确不再是三年之前的那个小小的道宗弟子了…
“当年，真该不惜一切手段杀了你的。”天元子似是叹息了一声，道。
“当年你们还留手了不成？”林动笑了笑，语气中不无讥讽，当年为了对付他一个道宗的弟子，元门三巨头齐齐出手，最后若不是青雉穿破空间相救，他们能否逃离还真是两说的事。
天元子双目微眯，眼中寒芒闪过，淡淡的道：“你也用不着得意，当年让你跑了，这次再解决掉也是一样。”
“这一次的话，或许该是你们这三条老狗倒霉了啊。”林动轻声道。
“狂妄的东西！你以为这三年就你变强了不成？本座现在就让你看看，我们之间，究竟有着多么庞大的差距！”
人元子面色一沉，身形一动，空间波动，竟直接是出现在了林动前方，大手探出，竟是有着异常浓郁的轮回波动弥漫而出。
“轮回境？！”周围的祝犁大长老等人感应到这般波动，皆是微惊，刚欲出手，却是被林动阻拦下来，他袖袍一挥，一道黑光自袖中暴掠而出，而后在面前化为一道黑影，黑影手持一柄黑色长刀，漠然的目光望着那大手带着轮回波动抓来的人元子，也没有丝毫的犹豫，手中黑色长刀，一刀劈出！
唰！
黑色长刀掠出，没有任何惊人的波动，但那人元子的瞳孔，却是在此时猛的一缩，一种极度危险的感觉涌上心头。
嗤！
在其心中不安涌动时，黑色长刀，已是掠过，重重的劈砍在其手掌之上，那玄奥无比的轮回波动，竟直接是被那黑色长刀生生劈开，而后长刀入肉，鲜血溅射而出。
啊！
人元子惨叫声爆发而出，旋即他身形猛的暴退，空间扭曲间，便退到了元门之内，而后他目光惊悸的望着林动面前的那道黑影。
“小心点，那东西有古怪。”天元子瞳孔盯着林动面前的那道黑影，缓缓的道。
人元子咬了咬牙，面色青白交替，没想到这刚刚出手，便是被林动下了一个如此厉害的下马威。
“这小子，是有备而来的。”那一直未曾说话的地元子，声音阴寒的道。
“有备而来么？”
天元子淡淡一笑，道：“既然如此，那我们也先将惊喜奉送而上吧。”
话音一落，他袖袍轻挥，而后他周身的空间便是急速的扭曲起来，黑雾涌动间，仿佛是有着一道模糊的身影，缓缓的自那空间中走出来。
这般的动静，自然也是被林动他们所察觉，当即眉头都是微微一皱，从那扭曲空间中，他们察觉到一股分外古怪的波动。
沙沙。
一只脚掌，踏出了扭曲空间，他身体上的黑雾，逐渐的淡化而去，显露出一道单薄的身躯，他身着黑袍，全身都是被笼罩在阴影中。
“我说，道宗这么多人来了这里，你也见见老朋友吧。”天元子脸庞上，诡异之色愈发的浓郁，他对着那道人影笑道。
苍白的手掌，从袖中探出来，然后他在那无数道目光的注视下，缓缓的将黑色斗篷掀下，而随着斗篷的落下，一张英俊中透着惨白的年轻面容，便是出现在了所有人的注视之中。
“这是？”
林动望着这陌生的人，却是微微一怔，不知为何，这人明明陌生得紧，但却给予他一种熟悉的味道。
一阵骚乱，突然从林动后面传出，他偏过头来，却是见到应玄子的一张脸庞，突然变得惨白下来，他手指颤抖的指着那道身影，眼中满是不可思议之色。
“你…你…周通，你还活着？！怎么可能！”
周通？
林动心头猛的一震，目光有些震动的望着那道身影，这人，竟然便是那位百年之前道宗最为优秀的弟子，甚至连大荒芜经都是被他参悟的…周通？！
他，他不是死在元门手中了么？！


第1206章 周通
周通？！
一股无法遏制的骚动，在这一刻猛的自道宗弟子之中爆发开来，他们面色苍白的望着远处的那道身影，眼中满是难以置信之色。
“怎么可能？！”
应笑笑也是惊呼出声，玉手紧握，喃喃道：“周通师兄明明百年前就死在了元门手中，怎么可能…还活着啊？”
一旁，应玄子的身体不断的颤抖着，那眼中神色急速的变幻着，显然此时其心中的波动太过的激烈，眼前的事，给予了他极大的冲击。
林动望着哗然起来的道宗的弟子，眉头也是微微一皱，眼神略显凝重，百年之前，在那时候的道宗弟子眼中，周通的地位，或许就如同现在的他在道宗弟子心中一般，甚至，就连林动在刚刚进入道宗时，内心深处都是将这位周通师兄视为一个目标。
他超越着周通留下的一个个记录，最后逐渐的在道宗之内声名鹊起，他只是不希望，在他的修炼道路上，会有着一个无法超越的东西。
每个时代，都会有着属于那个时代的传奇，而显然，百年之前，周通便是道宗之内的传奇，他同样领悟了那深奥异常的大荒芜经，带领着道宗弟子在那宗派大会上，与元门怒而争雄，那最后，更是一人独自杀上元门，虽然最后陨落，但却是在东玄域上，留下了显赫的一笔，这个名字永久的铭记在所有的道宗弟子心中。
而现在，这个曾经道宗的传奇，却是再度活生生的出现，而且，还出现在了他们道宗的对立面…这一刻曾经信仰倒塌的感觉，令得道宗不少弟子原本高昂的士气，都是出现了低落。
“他的确是周通师兄。”一旁的应欢欢，也是缓缓的说道，她那冰蓝美目，紧紧的盯在远处的那道身影上，眼神略显复杂。
曾经的她也是这位周通师兄的崇拜者。
“周…周通？你还活着？”应玄子死死的盯着那道熟悉的身影，声音中，有着一丝细微的颤抖。
“呵呵真是感人的一幕，应玄子，你这毕生最得意的弟子，道宗无数弟子心目曾经的传奇，如今却是站在了我们元门的阵营，你觉得可笑吗？”天元子淡淡一笑道。
“为什么？！”应玄子浑身颤抖，厉声暴喝。
听得他的暴喝，那周通嘴唇动了动，竟是一句话都未能说出来。
“你，你这个叛徒！”应玄子的神态仿佛都是在这一霎那变得苍老了许多，一旁的应笑笑连忙将其扶住后面的那些原本气势如虹的道宗弟子，也是呆呆的望着这一幕。
林动见到这般变故，眉头不由得愈发的紧皱起来，他锐利的目光，紧紧的盯着周通，半晌后，缓缓的道：“周通师兄，我想，现在的你恐怕已经不是你了吧？”
周通的身体似是在这一瞬凝了凝，而后他双目转向林动，依旧未曾说话，只是那眼神深处，仿佛是波动了一下。
“不管你是如何变成这般模样，不过我们道宗弟子，永远都会记着当年那个敢于为了道宗，只身单剑杀上元门的周通师兄，我想，我们也相信，如今的这一幕，不是你自己的意愿。”林动略显沙哑的声音，缓缓的传开，也是令得道宗弟子之中的骚乱逐渐的平息下来，不少弟子开始恢复冷静，以周通师兄的性子，怎么可能会站到元门的那一边去？这之中，必定是有着隐情！
周通目光就这样的看着林动，许久后，轻风拂来，长发飘起，他那英俊的脸庞上，竟是有着泪水滚落下来，他的身体剧烈的颤抖着，仿佛是在挣扎着什么，那嘴中竟是有着极为嘶哑的声音模糊的传出来。
“师…傅！”
那嘶哑的声音中，透着无尽的绝望。
应玄子身体一僵，他望着那身体挣扎的周通，下一刻仿佛是明白了什么，狰狞的目光陡然投向天元子，咆哮道：“你们究竟对他做了什么？！”
天元子眉头微皱的看了一眼周通，冷哼了一声，袖袍一抖，那周通挣扎的身体便是缓缓的凝固，脸庞上刚刚浮现的情绪，也是逐渐的淡漠而去，犹如一具被控制了神智的行尸。
“你这弟子，心智之坚韧倒的确罕见，受尽百年侵蚀，竟然还能保持一点清明，呵呵，其实本座也没做什么，只是舍不得这么好的一颗苗子，所以帮他把忠于道宗的心，换成了忠于我们元门而已。”天元子淡笑道。
应玄子暴怒，脸庞都是扭曲了起来，他怎么都没想到，自己那视为亲子般的得意弟子，竟是受了这元门百年折磨！
“我杀了你！”
应玄子猛的一声嘶吼，浩瀚元力瞬间爆发而出，旋即他竟直接是暴掠而出，凌厉无匹的攻势，闪电般的对着天元子爆轰而去。
天元子见状，却是诡异一笑，道：“周通，你这师傅忍不住了，你这做徒弟的，去教教他什么叫做平心静气吧。”
唰！
他的声音一落，一旁的周通便是暴掠而出，竟直接是如同鬼魅般的出现在应玄子前方，手掌一抓，那空间便是剧烈的扭曲起来，无形的空间，仿佛是在他掌下凝成实质。
嘭！
应玄子那凌厉攻势重重的撞在那扭曲成实质般的空间上，竟直接是被震飞而去，旋即那周通一步跨出，身形再度诡异的出现在应玄子前方，一指点出，指尖仿佛是穿透了空间，直奔应玄子额头而去，出手狠辣之极。
“爹！”
那后方的应笑笑等人见到短短两回合间，应玄子便是落入险境皆是面色大变。
嗤！
泛着银色光芒的指尖，犹如锋利的利剑，不过，就在这般凌厉攻势即将落到应玄子额头时，一股浩瀚的精神力陡然在其额头处凝聚成一片光幕。
虽说那精神力光幕仅仅只是抵御了瞬息，但待得那指尖将其突破时，一道泛着黑芒的修长指光，也是猛的自应玄子身后掠来，然后重重的与那周通攻击硬憾在了一起。
吱吱！
两道指光相撞，却出奇的是没有太过狂暴的能量扩散出来，只是双指周遭的空间似是在此时崩塌了下去。
嗤。
两道身影，皆是一震，而后身形皆是倒退十数步。
“这是…原来空间祖符是在你的手上。”
林动救回应玄子目光奇特的看了周通一眼，在先前交手的那一霎那，后者动用的竟是空间祖符之力，而且这股力量，比起数日之前那人元子施展的更为的强横，显然那空间祖符，并不是人元子所有，而是眼前的周通。
周通依旧没有回答，只是他的目光看着苍老了许多的应玄子，眼神最深处，似是有着难言的悲痛以及苦涩。
“掌教现在的周通师兄已经被控制了。”林动轻叹了一声，道。
应玄子双手紧握，许久后，他缓缓的点了点头，神态苍老的道：“是我莽撞了。”
林动望着他这般模样，也是有点不太好受，旋即他抬头，将那冷厉的目光投向了天元子三人，道：“掌教放心吧我会让他们付出代价的。”
“一切就交给你了。”应玄子点了点头，他也明白，这般场面，以他的能力，已是无法掌控了。
“呵呵，你们若是不将周通解决掉，恐怕他不会让你们对元门出手的。”天元子淡漠的笑道：“今日东玄域这么多强者在场，看来是要好好欣赏一场道宗内部的龙争虎斗了。”
“老狗，你放心，待我擒住你，同样会让你生不如死的。”林动冲着天元子露出一道狰狞的笑容，眼神冷厉。
“是么？”
天元子眼中也是杀意涌过，旋即他袖袍一挥，道：“周通，这位便是继你之后在道宗崛起的林动，他比起你当年可是有过之而无不及，你便好好领教一下他的本事吧，当然，若是能够杀了的话，那自然是最好。”
然而，面对着天元子此次的话，周通却是纹丝不动，淡淡的银芒在其眼瞳中疯狂的闪烁着。
“哼，现在还想抵抗？痴人说梦。”
天元子见状，顿时一声冷哼，手掌猛的一握，只见得那周通眼中，猛的有着灰气弥漫开来，将那些银光迅速的侵蚀而去。
周通的身体，剧烈的颤抖着，很快的便是满头大汗，他牙关紧咬着，鲜血从嘴中渗透出来，令得他此时看上去格外的狰狞。
“你这老狗！”
见到这天元子将周通逼成这样，林动眼中怒芒闪过，身形一动，就欲出手，但旋即面前空间扭曲，周通的身影便是挡在了他的面前。
灰气疯狂的侵蚀着周通眼中的银光，林动能够感觉到，后者残存的心智，在迅速被抹除着。
“林…林动师弟…”
而在银光最后挣扎间，周通张开了嘴，嘶哑而模糊的声音，带着一种哀求之意，传了出来。
“请你答应我…一定，一定要杀了我！”
林动望着眼前这曾经是道宗骄傲的周通师兄被折磨成这番模样，心中怒火滔天，那黑眸之中，杀意疯狂的涌动着，片刻后，他深深的吸了一口气，双掌缓缓紧握，那低沉而充斥着杀意的声音，在这天空上传开。
“周通师兄，放心吧，我会让那三条老狗把加诸在你身上的痛苦，尽数的还回来！”
“谢…谢谢。”
周通的脸庞上，一抹欣慰的笑容扩散出来，而其眼中的银光，似乎也是在此时，彻彻底底的消散而去。


第1207章 林动vs周通
当周通眼中最后一丝银光被灰芒侵蚀时，他脸庞上的神色，也是在此时缓缓的凝固，最后一点点的淡化下去，直到最后的彻底面无表情，此时的他，基本与一具傀儡无疑。
林动望着变幻成这样的周通，双掌也是忍不住的紧握，眼中有着惊人的杀意在堆积着。
后方应玄子的身体也是在不断的颤抖着，片刻后，他挥了挥手，闭目道：“林动，送他一程吧。”
林动微微点头，目光凝聚向面前那面无表情的周通，他能够感觉到后者那惊人的实力，那种程度，丝毫不比祝犁大长老他们这种触及轮回的顶尖强者弱，而且，在这周通的体内，他还能够察觉到一股熟悉的波动，那是空间祖符。
拥有着空间祖符的周通，恐怕就算是面对着轮回境的巅峰强者，都是拥有着正面相战的能力，这就如同摩罗，后者也是这般实力，只不过在对于火焰祖符的掌控上，摩罗近乎已达到出神入化的境界，就是不知道，如今的周通，对这空间祖符的运用，又是达到了何种的程度。
不过，如今的周通的确实力恐怖，但林动，同样并非是省油的灯，他身怀两大祖符，本身实力，同样是触及轮回，更加上小符宗境的精神力，真要说起来，如今的他若是全力而为，就算是摩罗都难以胜他，这天元子想要用周通来打败他，或许也是想得过于天真了一些。
虽然吞噬天尸此时或许也是极其的厉害，但林动一是需要它来震慑元门三巨头，二便是他想要亲自出手，唯有在交手中，他方才能够知道眼前的周通究竟是怎么回事，他想要尝试着，看是否能够有着最后的希望，将周通从这种被控制的状态中解救出来。
林动目光瞥了一眼那元门内，心神一动，只见得那手持黑色长刀的吞噬天尸便是掠出，刚好是横隔在他与元门之间，有着这具吞噬天尸镇守此处，除非天元子三人同时出手，不然的话，是绝对无法摆脱它的阻拦，但他们一动手，他们这“诛元盟”无数强者也是会顷刻爆发，到时候，想来这天元子三人也是相当的忌惮。
天元子三人见到林动这般举动，目光也是微闪，但却并没有其他的动作，只是那人元子盯着吞噬天尸的目光有些阴冷，显然还是没忘记先前被吞噬天尸所伤的事。
“放心，我们还等着看一场精彩的师兄弟之争呢，自然不会出手干预。”天元子一笑，他的眼中，有着一缕深邃黑芒掠过。
林动淡漠的瞥了他一眼，便是不再理会，此时多说无益，待得他将事情解决后，这三个老狗，一个都跑不掉！
“周通师兄，得罪了。”林动对着周通一抱拳，而后气息陡然暴涨，那漆黑眸中，目光也是逐渐的凌厉起来。
此时的周通，显然已是失去了所有的心智，因此对于林动的话语，他没有任何的反应，但在其身体表面，也是有着淡淡的银光散发出来，周遭空间，悄然的扭曲。
“唰！”
空间扭曲，周通步伐踏出，一跨之下，便是犹如穿透了空间，以一种极为诡异的速度出现在林动身后，只见得其手掌之上，银光闪烁，犹如一柄凌厉到极点的空间之刃，狠狠的对着林动暴刺而去。
轰。
雷光陡然掠出，一柄闪烁着雷芒的权杖倒射而出，重重的与周通手掌相触，清脆的金铁之声伴随着火花，扩散开来。
“嗡！”
攻击被阻截，周通身形一动，空间扭曲间，直接是如同瞬移一般出现在林动右上方，袖中十指连弹，空间扭曲间，只见得银光铺天盖地暴掠而下，那全部都是由空间之力凝聚而成的空间光刃，看似绚丽，但却凌厉得足以将一名转轮境强者的身体撕裂而开。
林动身形闪退，双掌一合，一圈黑洞便是在面前迅速的扩张开来，然后将那些暴掠而来的空间光刃，尽数的吞噬而去。
待得空间光刃尽数被吞噬的那一霎，林动身形也是猛的暴掠而出，手中雷帝权杖带着轰隆雷光，道道杖影，蕴含着极端狂暴的劲风，对着那周通周身要害笼罩而去。
嗤嗤。
而面对着林动迅猛反击，周通身躯扭动，空间扭曲间，竟是将那些锁定他周身要害的杖影，尽数的闪避而去，那番入微躲避，即便是林动都是略感不及，这空间祖符，果然也是玄奥异常。
天空之上，两道身影在短短数十息时间内，却是交手十数次，彼此攻击皆是异常凶狠，但却皆是被对方尽数的化解，那般激烈程度，看得不少人面色凝重，这两人的实力，放眼这玄域，恐怕都寻不出几人能够与他们相媲美。
“没想到周通师兄的实力，竟然也是强到了这种地步。”应笑笑望着天空上两人的交手，脸颊也是一片凝重，道。
“这才只是开始呢。”
应欢欢轻声道，这两人，一个身怀吞噬，雷霆两大祖符，另外一个也是拥有着空间祖符，这要动起手来，可真不是寻常触及轮回的顶尖强者能够相比的。
“希望林动能够送周通师兄一程吧，免得他落在元门的掌控中，反而生不如死。”应笑笑叹道，黯然的神色中有着怒火在涌动。
“这一次，会让他们付出代价的。”应欢欢那冰蓝美目中，也是寒光流动，元门此举，同样是激起了她心中的杀意。
吼！
天空上，猛的有着龙吟声响彻，林动身形暴退，数千道紫金龙纹在其周身奔腾，直接是化为一道数千丈庞大的紫金龙爪，一掌便是对着周通怒轰而去。
唰！
周通身形一动，出现在另外一片天空，一对带着黑色尖锐拳套的手掌伸出袖子，双手一握，只见得面前空间便是急速的扭曲，眨眼间，那片空间，竟然是化为一柄千丈庞大的空间大刀。
“轰！”
空间大刀被周通紧紧相握，而后怒劈而下，直接是硬生生的劈在那呼啸而来的紫金龙掌之上。
铛！
异常刺耳的声音传荡开来，只见得那空间大刀之上，银光绽放，一股无法形容的锋利散发出来，那等锋利，即便是空间，都能够将其撕裂。
空间大刀自紫金龙掌之上划过，然后众人便是惊愕的见到，那巨大的龙掌，居然直接一分为二，那断裂处，光滑如镜。
“好厉害的空间祖符。”
柳青望着这一幕，满脸的凝重，他同样是修炼了青天化龙诀，自然是知道由龙纹凝聚出来的龙形有着多么的强横，再加上林动还身怀洪荒龙骨，更是令得他所修炼的青天化龙诀如虎添翼，但没想到眼下，依旧是被那空间大刀生生劈开，这如何能不让得感到惊异。
“这周通，的确不简单，不过你也别小看了林动小哥，这小子，现在连老夫都看不透了。”祝犁大长老目光炯炯的盯着天空，缓缓的道。
柳青也是微微点头，虽然如今的他距那轮回境仅有一步之遥，但他依旧能够从林动身上感觉到那种危险的味道。
空间大刀生生的劈开紫金龙掌，那周通刚欲进一步展开攻势，一道光影，却猛的自那紫金龙掌之后暴掠而来，一闪之下，便是出现在其面前，闪烁着紫金龙鳞的拳头，渗透着丝丝轮回波动，狠狠的对着他怒轰而至。
唰！
这般攻势太过的突兀，即便是周通身体都是微微一滞，不过，就在林动那紫金拳头即将落到后者身体上时，他的身影，突然模糊而去。
拳头洞穿那道残影，却是扑了一个空，然而林动眼神却是没有丝毫的变化，就在那道身影模糊的霎那，他反手一旋，只见得一道巨大的黑洞便是蔓延而开，吞噬之力猛然爆发，竟直接是生生的将那后方的空间扯碎而去。
砰。
而在空间被黑洞扯碎间，一道身影，则是自那破碎空间中露了出来。
轰隆！
天际之上，雷云瞬间凝聚，一道巨大无比的雷龙张牙舞爪的冲击而下，林动手掌一握，那雷龙便是在其手掌之上，化为一片流淌的雷浆，包裹着他的手臂。
唰！
这一切，都是发生在电光火石间，雷霆落下，林动握住的瞬间，他的身形已是出现在了那被黑洞的吞噬之力扯出的周通面前。
没有丝毫的犹豫，林动那流淌着雷浆的手臂，犹如贯穿了天地的雷霆，撕裂长空，轰了出去。
雷光在周通暗灰色的眼瞳中急速的放大，这般时候，即便他拥有着空间祖符，也是再也躲避不及。
噗！
雷浆手臂，最终落在了周通身体之上，而后一声闷响，林动整条手臂，都是自周通胸膛洞穿而出，雷光暴溢而开，后者身体之上的黑袍，瞬间化为灰烬消散而去。
而在他身上黑袍化去时，林动的瞳孔，却是骤然一缩，那后方也是在此时爆发出阵阵骇然惊呼。
“这是…”


第1208章 魔皇锁
“这是…”林动目光落在那周通身体上，瞳孔却是陡然紧缩，眼中有着浓浓的惊色涌出来。
黑袍之下，并非是想象之中的血肉之躯，相反，在那里，竟是没有丝毫血肉的存在，而是一具黑色骨骼，在那骨骼之上，林动能够见到密密麻麻的诡异黑色纹路，这些魔纹，一层层的缠绕在周通浑身的骨骼上，犹如跗骨之蛆，无法抹除。
而在那黑色骨骼内，则是能够看见还在跳动的内脏，只不过这些内脏，也完全是呈现漆黑之色，显然是被魔气侵蚀到了极点的表现。
最令得林动震惊的是，在周通胸腔内，一颗黑色心脏，以一种极为缓慢的速度轻轻的跳动着，那颗黑色心脏，透着无尽的邪恶。
谁都未能想到，在那一颗完整的脑袋之下，竟然会是这般惊悚的一幕。
在那诡异的身体之中，能够看见错综复杂的经脉，在那经脉之内，有着银色光芒涌动，那是空间祖符的力量。
而放眼周通这具身体，也唯有那些经脉，尚还未被魔气侵蚀，而这恐怕也是为什么他还能够动用空间祖符力量的主要缘故。
黑色的魔纹，交织在周通的身体之内，隐隐间，仿佛是形成了一副邪恶到极点的魔锁，而正是这具魔锁，将周通害成这般模样。
“这是…魔皇锁？”岩的惊声，在此时突兀的自林动心中响起。
“魔皇锁？这是什么？”林动眉头一皱，急忙问道。
“远古时期，那位与主人斗得不相上下的异魔皇创造出来的残酷手段，这东西只要加注在身体上，那此人便是会逐渐的被侵蚀心智，而且他的元神也会被封锁在这具身体之内，永世无法脱身，甚至连轮回都无法做到，当年那些异魔害怕一些轮回境强者轮回，便是将被抓住的轮回强者尽数安上这魔皇锁，绝了他们的所有生机。”
“异魔皇创造的手段？还能绝了轮回强者的生机？”林动瞳孔微缩，心中暗感骇然。
“这般手段极为残忍，要将其人血肉剥离，五脏六腑用魔气侵蚀数十年，而后以魔体为锁，断绝生机。”
岩凝重的道：“眼下这魔皇锁虽然没远古时期的完整，但也绝不是寻常异魔王能够弄出来的，看来这元门，果然与那魔狱有勾结。”
“这些畜生！”林动听得眼中怒火暴涌，难怪先前周通拼着最后的心智只求一死，想来这种折磨，他已是承受不住。
“岩，现在周通师兄还有救么？”
“凡是被中了魔皇锁的人，必死无疑，甚至连轮回都无法进入，不过…”岩顿了一下，接着道：“不过这周通却是有些不同，他身怀空间祖符，元神应该是处于祖符的庇护之中，不至于被魔气彻底的侵蚀。”
“若是能够将这魔体给毁了，说不得还能救出他的元神…”
将这魔体给毁了？
林动目光一闪。
“咻！”
林动与岩的谈话，看似长久，但在心中却是转瞬即过，而此时后方众多哗然声方才刚刚鹊起，不过那面无表情的周通，一只带着黑色拳套的手掌上，已是有着银色光芒闪烁起来，而后空间扭曲，那股锋利到极致的劲风，直奔林动咽喉而去。
轰！
璀璨雷光，自林动手掌上爆发而起，而后反手一掌与那银光硬憾在一起，雷弧跳跃，也是将那空间之力尽数的抵御下来。
“唰！”
林动身形一扭，出现在周通身后，元力暴涌，狂暴的拳风带着道道龙吟，狠狠的落在那周通身体之上。
铛铛铛！
然而面对着林动这般狂暴攻击，周通那黑色骨骼却是爆发出道道火花，如此攻击，丝毫未能对那黑色骨骼造成什么破坏。
咚咚咚！
周通倒是极快转身，银光涌动，同样是锋利无比的掌风呼啸而出，与林动正面相撼，道道狂暴无比的拳风掌影掠出，两人周身的空间，都是被震得不断的破裂而开。
砰！
又是一记狠狠的交锋，狂暴元力席卷间，两人的身影皆是急急后退，不过这种硬碰，显然是林动占据着上风。
不论如何，两道祖符之力，总归是比周通手中的空间祖符要强横一些。
“哼。”
那远处的天元子见到周通落入下风，却是一声冷哼，眼中黑光一闪，那周通的身体便是一阵剧颤，旋即他猛的一掌隔空对着大地按下，一股奇特的波动，迅速的传进大地，而后，众人便是见到，下方的大地，竟然是开始以一种惊人的速度变得荒芜起来。
“大荒芜经？！”
见到这熟悉的一幕，应笑笑等人顿时惊呼出声，旋即眼神复杂，没想到，周通即便是被控制了，依旧还能施展出他们道宗荒殿最厉害的武学。
“既然如此，周通师兄，那我们便来比试一下，究竟谁的大荒芜经更强！”
林动眼中光芒闪烁，双手也是猛的结印，而后对着下方大地的虚按而下，低喝之声，自其嘴中传出：“大荒芜经！”
下方大地，荒芜的速度愈发的迅猛，所有人都是能够看见，那芜的气息，顺着大地飞快的蔓延出去，短短数息间，这数千之内，竟都是生生的化为一片荒芜之地，这般霸道的武学，倒是看得不少人有些心惊。
天空上的两人，遥遥而对，彼此皆是保持着双掌虚触地面的姿势，看似平静，但任谁都是能够自那流动的空气中，感受到一股极端狂暴的波动。
而这种狂暴，在地底之下尤为的疯狂，所有人都是能够感觉到大地的震动，方圆数千里之内的大地力量尽数被汇聚在这里，然后由两股意念在争抢着。
两人之间的下方地面，最中间的位置有着茂密的青草生长出来，充斥着勃勃生机，这在荒芜的大地中极为的显眼。
应玄子，应笑笑，应欢欢等道宗的人皆是紧紧的望着这一幕，心情颇为的复杂，因为他们知道这是道宗这数百年之内，两位最为优秀杰出弟子之间的比试。
在道宗弟子的心中，他们一个是曾经的传奇一个是现在的传奇，然而今日，这两个传奇，却是必须分出个胜负来。
嗡嗡！
荒芜的大地，在此时犹如地震一般，不断的颤抖着在那地面上，隐约能够看见光芒从地底深处渗透出来，那下面狂暴到极点的能量，已是要忍不住的喷发出来。
“呼。”
林动在此时深深的吸了一口气，他目光盯着远处的周通，脸庞上却是有着一抹笑容浮现出来：“周通师兄这一次的比拼，或许是我胜了。”
他的声音一落，只见得其眼瞳中，黑洞陡然成形，掌心之下，也是有着黑光涌出，最后竟是化为两道巨大黑洞，吞噬之力，喷涌而出。
轰！
大地终于是在此时蹦碎数千丈庞大的能量光柱，犹如火山一般喷发而出，而后直冲天际，连那云层都是被生生的撕裂而去。
轰！
而在林动这般能量被强行扯出时，那周通下方大地，也是有着光柱冲出，只不过，他这道能量光柱，比起林动那道，却是纤细了一倍之多。
显然，在比拼吸收吞纳能量这一项上，他远远不是林动的对手，大荒芜经加上吞噬祖符，那等霸道的吸扯，远远超过了他。
两者这般对比，也是引起那无数的哗然声，应笑笑等人更是紧握着手掌，脸颊上有着喜悦涌出来。
“轰！”
数千丈庞大的能量光柱，自天空一阵盘旋，而后没有丝毫停顿，一声呼啸，便是洞穿虚空，直奔周通而去。
而在那可怕的能量光柱冲向周通时，他附近空间，突然有着黑洞浮现出来，那般狂暴的吞噬之力，将空间撕裂而去，让得此时的周通再无法使用空间祖符躲避。
而既然无法躲避，那周通眼中也是有着灰芒掠过，旋即其脚掌一跺，那盘旋在其周身的巨大能量光柱，也是陡然呼啸而出，犹如一道横跨天际的匹练，最后与那道体积超过它数倍的超级能量光柱，狠狠的撞击在了一起。
轰！
无法形容的巨声，在此时这那天际之上滚滚的传开，千里之外，也是能够清晰可闻，那可怕的能量冲击波扩散开来，竟是将双方那庞大的人马都是掀得人仰马翻。
双方能够抵御这般冲击的强者，则皆是紧紧的盯着那两道光柱对冲之处，那里的空间，不断的破碎着。
唰！
而就在他们凝神于此处时，眼瞳突然一缩，只见得一道光影，猛的自那光柱内掠出，旋即其一声低吼，一道巨大无比的黑洞自其掌下蔓延而出，旋即双手生生一扭，竟然是直接将那两道蕴含着无比狂暴能量的光柱，尽数的扭在了一起。
林动双手虚抱着那将近万丈庞大的能量光柱，手臂之上，青筋如同虬龙般的耸动着，而后他用尽全力，将其生生抡动而起，然后狠狠的对着那周通怒砸而下！
咚！
能量光柱呼啸而下，那下方的万丈大地，都是在此时轰然崩塌。
天元子见到林动这般骇人声势，眼神也是微微一变，手印一动，刚要催动周通全力相迎，却是猛的发现，那周通的身体，竟然是纹丝不动。
“该死的！”
天元子一惊，只见得那周通原本灰芒满布的眼中，居然又是有着一点点银光闪现，而他的身体剧烈的颤抖着，显然是在此时竭力的反抗着天元子的控制。
虽然这种挣扎极为的短暂，但在这个时候，已是足够，但周通眼中的银光再度被强行侵占时，他已是看见那可怕的能量光柱，重重的抡至身体之上，当即那面无表情的脸庞，在此时露出了一抹解脱以及欣慰之色。
林动师弟…你赢了，谢谢你。
在他心中低喃间，可怕的光柱，在那无数人的注视下，将其彻彻底底的淹没而去。


第1209章 解救
耀眼的光芒，犹如烈日升腾，陡然自天空上扩散开来，令得无数人都是忍不住的虚眯着眼睛，不过即便是眼睛刺痛，但他们却依旧是强忍着眼中的酸痛，死死的盯着天空上。
所有人都能想知道，这两位道宗传奇的交锋，最后究竟是谁能够更胜一筹。
应玄子目光紧紧的盯着天空，以他的实力，自然是能够清楚的看见那狂暴能量之中的景象，因此他亲眼瞧见，那狂暴得无法形容的能量光柱犹如擎天之柱一般的落下，然后重重的落在了周通的身体上。
那般恐怖能量，就算是一名触及轮回的强者，也绝对无法抵挡！
咔嚓！
狂暴的能量，犹如潮水一般疯狂的冲击在周通那黑色骨骼之上，其周身空间尽数的破碎，紧接着，只见得那黑色骨骼上，一道细微的裂纹悄然的浮现出来。
咔嚓咔嚓。
那一道裂纹虽然细小，但却是在此时引起了连锁反应，那先前连林动都是奈何不得的坚硬魔体，迅速的崩裂出道道裂纹，短短数息的时间，便是蔓延到了周通全身。
邪恶得尽数粘稠的魔气，自那黑色骨骼中散发出来，但却是被那等恐怖能量，尽数的蒸发。
魔气不断的消失，周通的脸庞上，则是缓缓的露出一抹笑容，而后狂暴能量降临而至，他的身体直接是在此时，轰然爆碎。
光柱呼啸而过，最后落入远处的大地，顿时大地轰隆隆的颤抖着，一道万丈庞大的深渊被蛮横的撕裂开来，一整片山脉，都是被生生的夷平而去。
无数道目光投射向天空，然后他们便是刚好见到了周通身体彻彻底底爆碎的那一幕，当即心头皆是一震，这胜负，终于分出来了么？
淡淡的黑雾缭绕在周通身体爆碎的地方，其中再没有任何的生机波动传出，此时的周通似乎是彻彻底底的抹杀了。
无数道宗弟子望着这一幕，皆是苦涩的叹息一声，心晴颇为的复杂，此战虽然是林动胜了，但他们却无法提起太多的喜悦，心中的元门的仇恨与愤怒反而更为的浓郁，如果不是元门这些杂碎，林动又怎么会对周通出手，导致同门相残？！
远处，天元子盯着天空，那脸庞略显阴翳眼瞳内，黑芒闪烁。
天空上，林动同样是盯着那团黑雾，手掌紧握，心中有着一些紧张，按照岩所说，只要将魔体给毁了，周通的元神应该便是能够逃出来，但眼下…莫非是他先前下手太重将周通的元神也给抹杀了不成？
“别急，先等等…他有空间祖符保护，应该不至于这么脆弱。”岩的声音在其心中响起，不过林动却是从他的话语中听出了一些不确定，显然，就连岩都没绝对的把握周通能够存活下来。
虽然心中有些焦躁，但林动也是明白这等情绪没对事情没有任何的帮助，只能逐渐的平息下心中的躁动，目光紧紧的望着那团飘荡的黑雾。
那团黑雾飘在空中，一直都没有什么其他的动静，不过就在林动有些忍耐不住时，他眼神猛的一凝，只见得在那黑雾之中，一道银光猛的闪现出来。
在那银光之中，隐约可见一道微弱到极致的金光，而后那道银光便是包裹着那道微弱金光，猛的对着黑雾之外窜去。
“哼，想跑？”
然而就在那银光刚刚出现的霎那，那天元子眼中寒芒陡然一闪，单手结印，冷喝道：“魔皇锁！”
嗡！
原本飘荡的黑雾，突然在此时爆发出惊天魔气，接着竟是化为无数道黑色光线掠出，光线交织间，隐隐的，仿佛是化为一道邪恶到极点的魔锁，魔锁缠绕间，试图将那道银光再度封锁。
“林动，快出手，若是空间祖符再被魔皇锁锁住，那周通残存的元神也保不住了！”岩急促的声音，猛的在林动心中响起。
他的话音刚刚落下，林动已是暴起出手，虽然他不清楚眼下这变故确切为何，但却是能够感觉到那必然是对周通极为不利的事情。
“嗤嗤！”
几乎是在林动出手的霎那，黑芒雷光陡然自其手臂上的渗透出来，短短瞬息间，他的一条手臂便是化为黑雷般的液体般，而后一手抓出，竟是生生的穿过那邪恶的魔锁，一把将那道银光抓进了手中。
天元子见到林动出手，面色陡然阴沉，手印一变，只见得那魔皇锁便是转移目标，疯狂的对着林动手臂侵蚀而去。
“滚！”
林动眼神一寒，两大祖符之力，陡然自手臂之上喷薄而出，那般磅礴之力，直接是生生的将那魔皇锁冲散而去。
当年元门能够在周通身上种下魔皇锁，那是因为已将后者擒住，但现在的林动却处于战力爆棚间，而且他体内不仅身怀两大祖符，更是还有着祖石等诸多专门克制异魔的强大神物，这天元子想要以此来对付林动，无疑是有些痴人说梦。
林动那呈现液体般的手瞬间自魔皇锁中收回，而后袖袍一挥，只见得温和白芒暴涌而′出，直接是将那魔皇锁包裹而进。
嗤嗤！
而在白芒的照耀下，那魔皇锁则是爆发出嗤嗤声响，然后飞快的被净化而去，短短数分钟时间，便是彻彻底底的化为一片虚无。
见到那魔皇锁被毁，林动这才在心中松了一口气，然后将那森然的目光投向远处面色阴沉的天元子，眼中杀意毕露。
林动手臂上光芒消退，而后逐渐的恢复正常，他这才缓缓的张开手掌，只见得一团银光闪烁着，一股古老的波动散发出来，那是空间祖符。
此时的银光，微微的蠕动着，而后光芒猛的绽放开来，在那银光最深处，一道极为微弱的金光缓缓的升腾起来，那是周通残存的元神。
周通这道元神虽然有着空间祖符保护，不过显然也是遭受到了严重的创伤，金光极其的黯淡，甚至是连丝毫的波动都未曾传出。
“他的元神受创太重，我将他收入祖石，为其温养吧，修为到了这一步，只要元神丧存，要恢复肉体到不是难事。”岩说道。
“嗯。”
林动点点头，而后一道温和白芒自其体内射出，将那空间祖符连带着那道微弱的金光包裹着，不过就在岩准备将其带走时，那金光突然震动起来，而在那震动间，金光竟是从那空间祖符中脱离而出，独自的钻进了白芒内。
“这？”
林动见状，却是微微一怔。
“林动师弟，我这些年虽然有着空间祖符的保护，但却已受到魔气侵蚀，祖符乃是天地神物，对魔气极为的排斥，现在的我，已是没资格再拥有它，我见你天赋异禀，身怀多道祖符，这空间祖符若是在你手中，应当会比我发挥出更大的作用…”
在林动微愕间，突然一道虚弱到近乎要消散的低微声音，自白芒中传出，而后传进了他的耳中。
“周通师兄。”
林动闻言，急忙出言，但那道金光已是尽数的黯淡下去，然后被岩收进祖石之中，他则是眉头微皱着的望着他手中的空间祖符。
“林动，他说得没错，现在的他有着被魔气侵蚀的迹象，虽说祖符有灵一直在保护着他，但祖符毕竟有着它们的规则，强行掌控，伤及双方。”岩出声道。
林动略有点犹豫，虽说祖符的确不凡，但他毕竟已身怀两道，那种诱惑对他而言少了不少，再加上这是周通之物，这样拿到手，似乎是有些不妥。
不过此时，优柔寡断反而是矫情了一些，因此他接着点点头，道：“空间祖符我暂时拿着，不过等周通师兄被你净化出来后，我再将它物归原主。”
“随你吧，另外小心一些元门那些家伙，不知为何，我总感觉到一点不对劲。”岩点了点头，旋即道。
林动眼神一凝，旋即微微点头，反手将空间祖符也是收进体内，然后身形一动，出现在应玄子他们面前，道：“放心吧，周通师兄还有救。”
听得此话，应玄子等人脸庞上这才有着喜色涌出来。
“接下来怎么办？”小貂靠近过来，目光闪烁着寒芒的盯着元门三巨头，眼中的杀意，浓郁得近乎实质。
林动并未答话，同样是将目光望向那魔气缭绕的元门之内，眼中寒气涌动。
“呵呵。”
似是察觉到他们的目光，元门上空，天元子却是淡淡一笑，道：“真是场精彩的龙争虎斗，还真是江山代有才人出，林动，看来你已经超越周通了啊。”
“老狗，接下来就该你了。”林动盯着天元子，面无表情。
天元子笑了笑，那眼中的诡色却是愈发的浓郁，而后他看了看那数量庞大无比的诛元盟强者，点点头，说道：“虽然让你救走了周通，不过也无所谓了，他的作用已经达到，而接下来…便请你们来尝一场魔道盛宴吧。”
天元子话音一落，三巨头眼中黑芒猛然掠过，而后三人齐齐结印，三道魔光，猛的自他们体内暴射而出，魔光上触天空，下接大地。
咻！
黑色的魔光，自天空与大地上飞快的蔓延而开，而后天地黯淡，寒风阵阵，紧接着，无数道凄厉惨叫声猛的自元门之内传出，众人望去，只见得那些元门弟子的身体，竟然是在此时凭空的爆炸开来，而后血肉汇聚着浓浓的魔气，涌荡在这片天地间。
“轰隆！”
而在这天地间魔气荡漾时，下方的大地，也是开始崩裂开来，粘稠的魔气弥漫出来，在那魔气中，似乎是有着什么恐怖之物，缓缓破地而出。
整片天地，都是在此时陷入了魔气的海洋！


第1210章 盛大魔宴
整片天地，都是在此时急速的黯淡下来，魔气滔天，邪恶的阴寒之意，弥漫而开，令得那无数强者面色皆是有些变化。
林动也是眼神凝重的望着眼前这等变故，他视线射向那元门之内，此时那里还不断有着低沉的爆炸声传出，那元门弟子，一个接一个的爆成血雾，而魔气，也是在疯狂的涌出来。
这元门三巨头，手段竟是如此的狠辣，为了解决掉他们，竟然不惜将大半个元门尽数的葬送！
“小心点，这下面…”应欢欢来到林动身旁，她那冰蓝色的美目紧紧的盯着下方翻滚裂开的地面，在那里，她感受到了极端磅礴的魔气。
林动眼瞳之中，深邃的黑芒缓缓流转，犹如化为黑洞，接着他的目光便是看透那重重魔物，面色瞬间剧变起来，他看见无数道魔影，猛的撕裂大地，然后犹如野兽般的窜了出来。
“那是什么？”
小貂惊声道，下方魔气升腾，无数道身影，悬浮而起。
众人的视线皆是望去，只见得那一道道身影，竟是残破不堪，而起它们的身体格外的壮硕，隐隐可见人的形态，只不过那种邪恶到极点的气息，却是让得人明白它们是什么东西。
“是异魔！”祝犁大长老沉声道，这元门地底之下，竟然隐藏着这么多的异魔？
“不对，这些都是异魔骨骸应该是死去的异魔，不过现在似乎被元门三巨头用什么手段给操控了。”林动摇了摇头，这些异魔眼眶之中并没有太多的灵智，反而充斥着嗜血的暴虐，那番模样犹如被人操控的杀戮之尸。
“当年天地大战，被斩杀的异魔大多都是被净化而去，连尸骸都是未能留下，没想到在这元门地底，还隐藏着这么多，看来元门这些年，没少去收集异魔之尸。”应欢欢俏美的脸颊上寒气涌动道。
“小心一些，这些魔尸虽然不复当年之力，但也不可小觑。”
林动点点头眼下这一幕，倒真是有点像一场缩小型的天地大战，或许在那远古时期，那些远古强者们，便是在面对着数量以及实力都远远超过眼前这般规模的异魔军队吧。
“吼！”
那些异魔，猛的仰天嘶啸那深陷的眼眶望着天空上那无数的强者，旋即犹如蝗虫般掠出，铺天盖地的对着天空上那无数强者冲去。
“诸位，一起出手，斩杀这些异魔，灭除元门！”
那剑宗的宗主墨鉴倒是一声厉喝旋即凌厉无匹的剑气呼啸而出，直接是将上百道魔尸生生斩断，气势惊人。
“杀
墨鉴此招，倒是令得那诛元盟无数强者士气一振，而后迅速的压下心中升腾起来的惧意，元力涌动间，那攻势便是铺天盖地的落下，那些魔尸顿时犹如断翅的飞鸟一般，噼里啪啦的坠落而下不过紧接着，却是有着更多的魔尸从地底冲了出来。
诛元盟的强者数量，毕竟相当的庞大，因此那魔尸的第一次冲击，也并未取到丝毫的好处，不过这种情况伴随着那魔气的喷涌，却是开始转变。
吼！
数团巨大魔气冲天而起，只见得那其中，突然冲出数十具干瘦的魔影，他们的体积虽然不大，但那种魔气的浓郁程度，却是远远的超越了之前的魔尸，显然，这些魔尸生前，必然都是异魔将的实力，真要说来，那便是堪比转轮境的强者。
虽然如今他们的实力有所减弱，但气势依旧是相当的可怕，竟是顶着那磅礴如雨的攻势，冲进了那诛元盟的大部队之中，而后魔气喷涌间，瞬间便是有着不少强者被凌厉斩杀。
有了这些强横魔尸做冲击，后方那魔尸也是铺天盖地的涌来，而后两股洪流冲击在一起，魔气以及元力，疯狂的涌开，那般声势，骇人之极。
“掌教，你们也去帮忙，将那些厉害的魔尸阻拦下来。”林动看了一眼混乱的天空，那些实力堪比异魔将的魔尸，横冲直闯，将那防线撕裂出道道口子，这若是不加控制的话，极容易被后方魔尸大军所冲散。
“嗯。”
应玄子面色凝重的点点头，旋即一挥手，便是带着道宗众多强者冲出，将一些难缠的魔尸尽数的阻拦下来。
“祝犁大长老，柳青大哥，欢欢，你们多注意点，万一有极为厉害的魔尸出现，便出手对付。”林动目光盯着那魔气翻涌的大地，他能够感觉到，在那下面，还有着更为强横的魔尸未曾出现，元门为了今天，显然是做了极大的准备，若是一个不慎，恐怕这东玄域诸多强者，皆是会葬生此地。
“嗯。”
祝犁大长老等人也是知道局势的严峻，皆是凝重的点点头。
“那元门三条老狗消失了，他们才是最麻烦的。”小貂眼中凶芒闪烁，低声道。
林动目光一扫，果然是发现那元门三巨头不知何时消失，这令得他眉头微微皱了一下，这三条老狗，的确很麻烦。
“轰！”
而就在林动想要探寻那元门三巨头方位时，只见得下方地面，猛的有着一股滔天魔气席卷出来，而在魔气中，似乎是有着头－道魔影缓缓的走出。
“该死的，这么强大的魔气，这些魔尸生前莫非是异魔王？”天龙妖帅有些震动的望着那滔天魔气中走出来的数道魔影，道。
魔气逐渐的散去，接着便是有着六道魔影浮现出来，他们的身体与其他魔尸的残破不同，反而是保存得异常的完整，那眼眶之中，邪恶的魔气涌动着，隐约还能够感觉到一股淡淡的威压，显然，这些魔影生前，极有可能都是异魔王。
这六道魔影一出现，便是将邪恶的目光投向了林动他们一群人，看来他们也是察觉到后者等人才是最为厉害的。
“动手吧。”
祝犁大长老沉声道，这些异魔王虽然因为死亡而实力大降，但也不是寻常转轮境强者能够抗衡，唯有他们出手，才能将其阻拦。
咻！
然而还不待他们先出手，那六道魔影已是仰天嘶啸，而后魔气滚滚，直接是对着他们暴掠而来。
“你们小心点。”林动看了一眼身旁的应欢欢与绫清竹，轻声说道。
“那元门三巨头，就由你们三人去寻找对付吧，只有解决掉他们，才能结束这场战争。”
绫清竹点点头，声音依旧是清清冷冷，只是话音落了，才顿了顿，补充道：“多注意点，若是有问题，我来帮你。”
话音落下，绫清竹也不待林动回话，娇躯已是掠出，娇喝间，凌厉剑芒涌动，便是将一道魔影阻拦而下。
林动望着绫清竹的倩影，微微笑了笑，而后他便是感觉到手掌中传来一阵冰凉，只见得一只犹如羊脂玉般的芊芊素手轻轻握着他，转头一看，然后应欢欢那冰蓝色美目，便是落进了他眼中。
应欢欢倒是没多说什么，只是握着林动的手掌轻轻捏了一下，接着便是松手掠了出去，滔天寒气涌动，连那邪恶的魔气都是被冻成片片冰屑。
随着身旁众人的掠出，林动身旁便唯有小貂与小炎尚在，他们对视一眼，皆是从对方眼中看出一抹浓郁到极点的凶芒。
“接下来，就让我们三兄弟，来会会那三条老狗吧。”林动微笑道。
“三年前这三条老狗趁貂爷伤势未复占了点便宜，这次，可就要给貂爷全部的还回来！”小貂俊美的脸庞上，杀气弥漫，三年前的事，一直被他引以为耻，当初的三兄弟，他实力最强，按照他那性子，自然认为他必须保护着林动与小炎，但谁料到，最后反倒是实力远不如他的林动在那里拼命的护着他。
小炎憨笑了一下，笑容一如三年之前，只不过如今那笑容中，却是有着滔天般的凶戾在涌动着。
三兄弟脚踏虚空，那等弥漫的凶气，竟是连那弥漫天地的魔气都是无法靠近。
轰隆隆！
下方的大地，也是在此时疯狂的抖动起来，而后林动三人便是见到，下方粘稠的魔气中，一道千丈庞大的魔影，缓缓的自那地底最深处爬了出来。
那等魔气，滔天肆虐。
魔气缭绕着，而那道庞大的魔影也是逐渐的现出身来，那仿佛是一道巨型魔怪，魔怪通体漆黑，生有四只巨大的魔臂，它并无眼目，在那脸庞处，唯有着一张布满着黑色利齿的狰狞巨嘴，魔气犹如洪水般，顺着那巨嘴喷吐着。
“这是什么？”
林动三人望着这魔怪，眼神皆是一凝。
“林动小心，这是大天邪魔，乃是异魔之中极为强悍的种类，当年那异魔皇的坐骑，便是一只大天邪魔，那邪魔的力量，足以媲美实力达到两重轮回劫的真王层次！”岩凝重的声音在此时响起。
“堪比两重轮回劫？”林动瞳孔微微一缩，心中略感骇然。
“眼下这大天邪魔，显然也只是尸骸，不过却是被那元门三巨头操控，那等实力，恐怕寻常轮回境强者也是无法匹敌。”
林动深吸一口气，眼中满是凝重，没想到，这元门三巨头，居然还隐藏着如此强大的手段。
“桀桀。”
在林动心中震动间，只见得那大天邪魔巨脸上，魔气缓缓的蠕动着，接着便是有着三张人脸浮现出来，他们诡异的盯着天空上的林动三人，变了声的尖锐大笑传出：“林动，本座早便是说过，即便是三年后，你们三人，在我眼中，依旧只是蝼蚁！”
“现在，你们还有动手的勇气么？”
林动眼神冰寒，他盯着那大天邪魔，然后嘴角却是有着一抹弧度掀起来，漆黑眼中，一股灼热缓缓的涌了出来。
“的确很强大，不过这样，打败你们才更有雪耻意义，不是么？”
说着，林动偏头，望着小貂与小炎，后者两人，眼神也是逐渐的疯狂起来。
“准备好拼命了吗？”
小貂二人咧嘴一笑，眼神狰狞，重重点头，这一天，他们等三年了“既然如此…”
林动双掌缓缓紧握，凌厉气息冲天而起。
“那就战吧！”


第1211章 龙，虎，貂
那就战吧。
当林动此话一出时，三股凌厉气息，犹如出鞘利剑，下山猛虎般陡然冲上天际，那等波动，直接是将那些弥漫而来的魔气生生的震散而去。
三人脚踏虚空，目光冷若寒冰，锁定着那前方庞大的大天邪魔。
“哈哈，真是不自量力的东西，本座三人即便是分散对战，也足你灭你三兄弟十数回，更遑论我三人如今更是融入的这大天邪魔之中，你三人竟然还想一雪前耻？真是可笑，哈哈！”
感受着三人那磅礴战意，大天邪魔上面那三张诡异脸庞也是仰天大笑，三道合在一起的尖锐笑声传荡开来，其中充斥着讥讽以及不屑。
他们三人如今已是真正的晋入轮回境，如今再催动自身血肉融入这大天邪魔，虽然依旧无法令得其恢复巅峰战力，但也绝对不是寻常轮回境的强者能够阻拦，更何况，林动三人，仅仅只是转轮境而已，这般实力在他们看来，实在是不堪一击。
“待得将你们三兄弟解决，这里的人，一个都逃不掉，哈哈，林动啊林动，你还妄想血洗我元门，却是不知我三人早便在此等候多时，你将这些反抗我元门的强者带来，本座也正好尽数的解决，到时候，看这东玄域，还有谁敢与我元门争锋？！”
林动面色漠然的望着那大天邪魔上面三张大笑的诡异脸庞，双掌缓缓的紧握，璀璨的紫金光芒，从其体内弥漫出来，道道龙吟，震天响彻。
“吼！”
小炎也是在此时爆发出惊天虎啸，而在其虎啸声落下间，那后方不远处，虎噬军猛的盘坐而下，接着便是有着滔天黑光自他们体内席卷而出，一道巨大黑色光罩将他们尽数的笼罩，可怕的煞气弥漫开来。
滔天黑光奔腾，最后直接被小炎尽数的吸入体内，顿时他那本就如同铁塔般的身体立即在此时继续的膨胀着，黑白的坚硬虎毛，自他身体表面唰唰的生长出来，一股惊天般的凶戾之气，席卷而开。
小炎的实力，仅仅只是转轮境，但这虎噬军却是与息息相关，数千人的力量被他汇聚于一身，再配合着他如今体内那转化成黑暗圣虎的虎族顶尖血脉，那所爆发出来的力量，恐怕就算是祝犁大长老这种触及轮回的强者都唯有暂避锋芒。
在小炎实力尽数爆发间，小貂也是腾空而起，一道尖啸直冲云霄，紫黑色的浩瀚能量蔓延出来，在其背后，一对千丈庞大的蝠翼，遮天蔽日的仲展开来，蝠翼之上，弥漫着紫金色的纹路，渗透着一丝高贵与古老。
小貂如今本就是触及轮回的强者，再加上他那天妖貂族最为精纯的血脉，那等战力，同样是非同小可。
“战吧！”
小貂弥漫着凶气的低沉暴喝，猛的响彻天际，三人的气息，都是在此时强横到极点。
“哈哈。
元门三巨头大笑，旋即只见得那庞大的大天邪魔竟是奔掠而出，大地颤抖间，滔滔魔气席卷开来，犹如形成道道风暴。
“吼！”
小炎虎目赤红，大手一握，巨大的神锤闪现出来，锤身之上，九道鼎纹闪烁着夺目光华，而后他率先掠出，手中神锤化为千丈锤影，直接是轰爆空间，狠狠的对着那大天邪魔轰了过去。
“咻！”
小貂掌心紫黑光芒奔涌，一道犹如河流般的紫黑匹练掠出，匹练中，弥漫着浩浩荡荡的能量。
紫金光芒也是在此时自林动体内爆发，三千道紫金龙纹奔腾，化为一道数千丈庞大的紫金龙掌，悍然挥出。
三人一出手，便是动用真正力量，那霎那间爆发出波动，几欲将这片天地都是给掀翻过去。
“邪魔之盾！”
不过面对着三人的联手，那元门三巨头却是一声冷笑，而后魔气翻涌，竟直接是在大天邪魔前方化为一道千丈巨大的黑盾，盾牌之上，布满着狰狞的魔脸，魔气森然。
砰砰砰！
三道强悍攻击，几乎是同时掠来，而后狠狠的轰在那黑色巨盾之上，惊天之声响彻，下方的大地，则是在此时不断的崩裂开来。
能量余波逐渐的散去，那黑色巨盾上，裂纹浮现，而后缓缓的蹦碎而去，不过林动三人的攻势，却是被尽数的阻拦了下来。
“三年时间，你们就成长了这么一些吗？”
大天邪魔之上的三张脸庞诡异一笑，而后那大天邪魔巨脚猛的一踏，那庞大的身体却是展现出极端惊人的速度，一闪之下便是出现在了小炎后方，布满着魔纹的魔臂紧握，那蕴含着毁灭般力量的一拳便是毫不留情的对着小炎轰了下去。
吼！
身后可怕劲风袭来，小炎也是仰天虎啸，凶煞之气冲天而起，黑白光华席卷而开，竟是在其周身化为一道巨大黑白光虎之影，而后巨虎咆哮，虎掌紧握神锤，毫不退缩的与那大天邪魔力撼在一起。
嘭！
两者冲撞，空间蹦碎间，小炎的身体也是陡然倒飞出去，那握着神锤的虎掌有着鲜血渗透出来，以他的实力要去正面相撼这汇聚了元门三巨头实力的大天邪魔，显然还是勉强了一些，如果不是因为如今的他获得黑暗圣虎的精血传承而再度变强，恐怕光是这一击就有些接不下来。
元门三巨头似乎也是看出了小炎是三人中相对而言较弱的一环，因此在将小炎击退后，竟并未转移目标，庞大身躯一动，就欲再度下杀手。
“紫月斩！”
不过他打算，林动与小貂二人自然不会让其达到，小貂率先出手，单手一握，只见得一轮巨大无比的紫色弯月便是在其上空成形，而后屈指移下那轮紫月便是洞穿虚空，重重的对着大天邪魔呼啸而去。
“滚！”
尖锐的喝声，自那大天邪魔上面三张脸庞中传出，而后大天邪魔转身一拳轰出，滚滚魔气席卷，直接是一拳生生的轰在那道凌厉无匹的紫月之上，魔气奔涌间，一拳便是将那紫月震碎而去。
嗤！
一拳轰爆紫月，那大天邪魔狰狞巨嘴猛的张开一道浓郁得尽数粘稠的魔气光束快若闪电般的掠出，直奔小貂而去。
不过就在那蕴含着惊人杀伤力的魔气光束冲出时，一圈巨大的黑洞顿时在前方蔓延开来，一口便是将那道魔气光束吞噬而进。
砰！
不过那道光束之中所蕴含的魔气显然是非同小可，这圈黑洞在将其吞噬后，竟是飞速的膨胀开来，最后一声巨声直接是爆炸开来接着一道比起之前弱小了不少的魔气光束洞穿而出，但却是被小貂阻拦了下来。
林动见到竟然连吞噬之力所形成的黑洞都是被这大天邪魔的攻击洞穿，眼神也是微微一凝，这元门三巨头汇聚在一起后的实力，果然相当的恐怖。
“一起出手！”
耀眼的紫金光芒弥漫着，阵阵古老的龙吟自林动体内传荡出来他那漆黑眸子中，也是有着无尽的战意涌出来，他今日倒是要看看，他们三兄弟联手，究竟能否斗过这元门三巨头！
轰轰轰！
他喝声一落，小炎与小貂皆是一声长啸，而后体内力量运转到极致三道弥漫着凶气的光影暴掠而出狂暴无比的攻势，犹如倾盆暴雨一般，将那大天邪魔笼罩着。
而面对着三人的联手，那大天邪魔也是爆发出阵阵刺耳尖啸声魔气犹如大海般奔涌而动，而后将三人的攻势尽数的接下来。
这般战斗，无疑是有些惊天动地的问题，战场所过之处大地崩裂，空间扭曲每一次的元力与魔气的对轰，都犹如是在天地间刮起了龙卷风暴，一些被波及的魔尸以及强者，皆是急急而退。
咚咚！
那大天邪魔面对着林动三人的疯狂围攻，却并没有展露出丝毫的疲态，魔气滚滚，浩瀚无尽。
“哈哈哈哈。”
大天邪魔巨拳一拳将飞扑而来的小炎轰飞而去，变了声调的尖锐声音不断的传出，那三张诡异的脸庞之上，满是讥讽之色。
“林动，你们就只有这点能耐么？这魔宴世界中，魔气永久不息，魔尸即便被打败也会重新站起，而你们这诛元盟的强者却是会越来越少，哈哈，此消彼长，你们这里的所有人，今日怕一个都是逃不掉！”
林动听得此话，眼神也是一凝，目光微扫，果然是见到下方那些被打败的魔尸，在经过魔气的侵蚀后，竟然又是悍不畏死的冲出，而反观诛元盟那边，却是损失开始加大，如果不是祝犁大长老，应欢欢等顶尖强者维持的话，或许那死伤还要更为的惨重。
“林动，的确得赶快解决掉这家伙，不然那些魔尸会越来越强。”岩凝重的声音也是响起。
林动面色阴沉，旋即他缓缓的点了点头，目光看向不远处的小貂与小炎，彼此眼中都是掠过一抹猩红之光。
“吼！”
小炎的身体，突然在此时以一种惊人的速度膨胀起来，黑白光芒弥漫，一种无法形容的狂暴波动，弥漫开来。
惊天虎啸传出，黑白光芒间，一头体积丝毫不比那大天邪魔小的黑白巨虎，便是踏着天空的浮现出来，那种古老的波动，弥漫着天地。
显然，此时的小炎，已是变幻出了黑暗圣虎的本体，这已算是身为妖兽的最强战斗形态。
紫黑光芒，同样也是在此时自小貂体内弥漫出来，而后他那千丈蝠翼，开始迅速的生长，然后交叉在前方将其身体遮掩，隐约间，仿佛是有着光芒渗透出来。
蝠翼最终缓缓的张开，只见得那其中的小貂，身体之上，出现了一层紫黑色的战甲，在其俊美的脸庞上，一道道有些诡异的紫黑色纹路，悄然的蔓延着，一头同样被渲染成紫黑的长发落下来，在月光的衬托下，犹如暗夜之王。
“吼！”
龙吟响彻起来，紫金光华大作，一头万丈庞大的紫金巨龙，蜿蜒盘踞，一股无法形容的古老以及纯正的龙威，蔓延开来，弥漫天地。
大天邪魔足踏大地，周身魔气滔天，在其上空，紫金巨龙，黑暗圣虎，天貂盘踞。
这片战圈，已近疯狂。


第1212章 斗魔
元力奔腾，魔气呼啸。
紫金巨龙，黑暗圣虎，天妖貂盘踞三方，惊人的凶戾之气自那庞大的身躯中散发出来，那眼中皆是有着灼热得足以将人融化的战意在涌动着。
那元门三巨头目光望着天空上化为三头凶兽般的三人，目光也是略微有些变幻，以他们的眼力，自然是看得出来此时三人那暴涨的气息。
一般说来，妖兽变幻成本体后，战斗力比起同级的人类强者都是要强上一些，更何况如今眼前的三人所变幻出来的龙虎貂，显然都不是寻常妖兽。
“冥顽不灵，本座今日倒是要看看，你们究竟能翻出什么浪来！”
不过三巨头为林动三人这般气势惊了一下，但旋即便是冷笑出声，他们实力强了林动三人一个档次，再加上这大天邪魔之身，林动三人想要翻身，谈何容易？
吼！
林动三人倒是没了与三巨头废话的想法，小炎所化的黑暗圣虎仰天长啸，虎啸之声几乎是化为了实质般的声波，滚滚的传荡开来。
唰！
虎啸一落，小炎已是暴掠而出，庞大的身躯犹如洞穿了空间，庞大的阴影笼罩向那大天邪魔，巨嘴咆哮，一道呈现黑白色泽的能量光波，砰的一声，直奔大天邪魔而去。
光波刚刚冲进大天邪魔周身百丈，魔气便是滚滚而来，化为一道光幕，直接是将那光波尽数的抵御下来。
吼！
不过就在光波刚至时，小炎已是带着滔天凶气而来，那布满着森森利齿的虎嘴仿佛连空间都是被其生生的撕裂，而后狠狠的对着大天邪魔噬咬而去。
“哼！”
大天邪魔身体表面魔气暴涌，魔拳之上，道道魔气风暴席卷，然后便是一拳对着小炎重重的轰了过去。
咻！
魔拳挥出，但不带它落至小炎身体上，前方猛的有着铺天盖地紫黑光芒席卷而来，只见得一轮巨大的紫黑色弯月呼啸而来，重重的与其拳风相撞。
砰！
元门三巨头眼中寒芒一闪，那魔拳之上，魔气猛的澎湃起来，而后化拳为刀，五指犹如锋利无比的魔枪，魔气涌动竟直接是生生的将那轮紫黑弯月洞穿而去，接着去势不减的直奔小炎而去。
吼！
古老的龙吟声，突然在此时响彻而起，一道覆盖着紫金龙鳞的巨大龙爪，猛的撕裂了前方的空间，然后狠狠的与那魔掌所化的锋利长刀生生的硬憾在一起。
吱吱。
刺耳的金铁之声在此时传荡开来，火花伴随着狂暴的能量疯狂的扩散开来，那紫金龙爪却是被逼得不断后退，紫金光芒闪烁间，抵御着那魔气的侵蚀。
唰！
龙爪抵挡着魔掌，小炎庞大的身躯却是借此虎跃而出，那无比森寒的虎嘴，毫不留情的狠狠噬咬在那大天邪魔魔臂之上，尖锐的嘎吱声传出，只见得道道黑色鲜血，迅速的从那魔臂之上流了下来，小炎这一口，显然是咬得极狠。
“你找死！”
魔臂上传来的剧痛，也是令得那元门三巨头怒喝出声，另外一只魔掌猛的抓住小炎那庞大的身躯，然后一把便是将其狠狠的甩了出去。
砰砰！
小炎的身体在那地面上搽出一道数千丈的深深痕迹，沿途更是将一座山峰撞成了粉末，而后刚刚稳下身形，那虎目之中狰狞闪烁，虎蹄一踏，又是携带着漫天凶气冲了出去。
“唰！”
小貂背后数千丈庞大的蝠翼一扇，手中紫黑光芒涌动，化为一柄紫黑长枪，身形也是犹如一抹闪电般的掠出，可怕的凌厉攻势，狠狠的对着那大天邪魔席卷而去。
吼！
林动所化的紫金巨龙，也是仰天长啸，紫金光芒犹如耀日，将那数千丈之内的魔气都是生生的蒸发而下，覆盖着紫金龙鳞的锋利龙爪，直接是生生的撕裂空间，疯狂的对着大天邪魔攻去。
嗷！
而面对着三人如此凶悍的攻势，大天邪魔也是爆发出犹如野兽般的嘶啸声，而后他魔掌一抓，滚滚魔气在其手中化为一根千丈庞大的魔气图腾柱，柱子挥动间，下方大地直接是崩裂出一道道千丈庞大的深渊裂缝。
四道庞大的身影，便是这般狠狠的冲击在一起，巨虎撕咬，天妖貂蝠翼扇动，紫黑长枪洞穿空间，铺天盖地的掠出，巨龙长啸，那拥有着瞬间摧毁山岳之力的龙爪，也是狂暴的挥出。
而在三人如此凶残的围攻下，那大天邪魔则是丝毫不让，手中魔气图腾柱每一次挥动，都将会其中一人生生震退。
若只是论实力的话，显然是元门三巨头要强上一筹，但林动三人毕竟都非寻常强者，他们都是拥有着各自奇特的能力，而且厮杀间，那是一种如出一辙的以命搏命的凶狠姿态，因此在这混战中，虽说林动三人身体上不断的出现伤势，但那大天邪魔也是在三兄弟红眼的疯狂攻击下出现了不小的创伤。
战圈所过之处，大地崩裂，空间扭曲，那般战斗，当真是有些天崩地裂的感觉。
眼下的这片地域，显然是极其的混乱，天地都是被笼罩着魔气之中，那远处厮杀声震天，无数诛元盟的强者与那些魔尸狠狠的厮杀在一起，这般时候了，他显然也是杀红了眼，强大元力涌动间，毫不留情的对着些魔尸席卷而去。
天空上，时不时的会有着人影坠落而下，其中有魔尸，但同样也有着不少诛元盟的强者，那两个可怕的绞肉机，每一刻都将会碾碎无数道身影。
不远处应欢欢玉手一握，数百道冰凌自其手中暴掠而出，直接是生生的将周遭上百道魔尸洞穿而去，寒气弥漫间，一道道冰冻的魔尸便是坠落下去最后摔成漫天冰屑。
不过这里的魔尸数量实在是太多庞大，再加上那无尽魔尸的滋润，只要不是类似应欢欢这种将魔尸毁成碎片，一般说来都会再度的爬起来，因此应欢欢刚刚解决掉周身的魔尸，又是有着更多的数量汇聚而来。
唰唰！
凌厉的剑气，突然铺天盖地的席卷而来剑芒过处，那些对着应欢欢冲去的魔尸顿时被切割得支离破碎。
应欢欢见状，微微偏头然后便是见到那一身白衣的绝色女子站在后方，而后者那清冷的双眸，也是与其对视了一眼，然后两人皆是不约而同的偏开头。
周围的魔尸再度如同潮水般的涌来，虽然这些魔尸大多都是无法伤及两人，但为了减轻其他人的复杂她们也只能尽可能的多清除一些，因此两人也是逐渐的靠在了一起。
应欢欢周身寒气涌动，玉手挥动间，再度数百具魔尸化为冰屑，而后她目光便是忍不住的看向遥远处，那里有着极端狂暴的波动扩散出来那几乎算是这片地域中最为凶狠的战圈。
应欢欢望着那边，旋即柳眉便是微微一簇，林动三人联手间，几乎是在那与元门三巨头搏命，彼此硬憾间，鲜血飞洒，极为的惨烈。
“我去帮他。”
应欢欢见到林动所化的紫金巨龙被那大天邪魔图腾柱轰飞，美目中寒气一闪，刚欲掠出却是突然被一只略显温凉的手给拉住了。
“你干什么？”应欢欢不悦的道。
“这种战斗，你就别去了，那是他们三兄弟的事，你去了，他反而不高兴。”绫清竹松开应欢欢，声音轻缓的道。
应欢欢闻言，身形也是微缓下来，动了动嘴想要反驳，但在想起林动那性子后，这才安静下来，她知道林动平时看似温和，但骨子里的傲气却是比谁都还高，三年前，他被元门三巨头逼得退出道宗，逃出东玄域，想必其心中必定引以为耻，而如今归来，以他的性子，这曾经的耻辱，一定是要亲手讨回来的，也正是因为如此，除了他那两个兄弟，他却是连那具极为强横的傀儡都未曾动用。
的确如同绫清竹所说，这种战斗，林动不会喜欢她去帮忙的。
“真是死要脸活受罪的家伙！”
应欢欢望着远处那鲜血飞洒的惨烈战圈，银牙轻咬了咬，忍不住恼怒的叱了一声，旋即其周身寒气暴涌，竟直接是冲进了那魔尸大军中，寒气席卷间，所过处，魔尸尽数的化为冰屑，那下手狠辣程度比起之前尤要胜之，显然她是想心中的火气，发泄在了这些可怜的魔尸身上。
嘭！
黑色的图腾柱带着可怕的魔气席卷而过，然后狠狠的抡在那冲来的黑暗圣虎身躯之上，虎嘴之中发出一道痛嚎，但那虎目中却是凶光涌动，那狰狞虎嘴狠狠的噬咬在大天邪魔魔臂之上，虎头一甩，生生的将那魔臂之上撕开一道巨大的血口子。
“唰！”
紫黑光芒铺天盖地的席卷而来，狂风而至，一道约莫千丈庞大的枪影竟直接是洞穿而来，最后快若闪电般的点在那大天邪魔胸膛之上。
嗤！
凌厉无比的枪影，即便是一名触及轮回境的顶尖强者都足以洞穿，但落在那大天邪魔身体上，却仅仅只是刺入半丈。
吼！
龙吟紧随而至，数千丈庞大的紫金龙爪紧握成拳，竟是一拳狠狠的轰在那千丈庞大的枪柄之上，一股肉眼可见的力量波纹，猛的扩散而出。
受到这般强力一击，那千丈长枪，终于是洞穿了那大天邪魔胸膛，而后自那后方贯穿而出。
嗷！
蕴含着痛苦之声的凄厉咆哮，猛的自那大天邪魔巨嘴中传出，其上的三张诡异脸庞，也是在此时涌上狰狞之色。
“你们这三个混蛋，竟敢伤了这大天邪魔，你们知道为了修复它，我们付出了多大的代价吗？！”
尖锐的咆哮声响彻起来，一股极端恐怖的魔气，猛的自那大天邪魔体内席卷而出，那元门三巨头厉声暴喝：“玩也玩够了，你们三个小杂碎，都给本座去死吧！”
大天邪魔那无比狰狞的巨嘴，突然在此时尽数的睁开，可怕的漩涡在其中成形，一股无法形容的魔气波动，自那嘴中席卷而出，死亡的味道，散发出来。
“大邪魔渊！”


第1213章 雪耻之战
滔天般的魔气，在此时自那大天邪魔狰狞巨嘴之中爆发而出，在那巨嘴深处，有着浓浓的死亡波动散发出来，令人头皮发麻。
这片大地，都是在此时剧烈的颤抖起来，一股强大的吸力，自那大天邪魔嘴中传出，那股吸力之强，即便是林动三人如今所化的庞大体型，身体都是有种被吸走的迹象。
吸力涌动，突然有着一道黑色光圈自大天邪魔嘴中暴射而出，然后将小炎所化的黑暗圣虎笼罩在了其中。
咻。
光圈笼罩，那种吸力猛然间暴涨，小炎那庞大的身躯居然直接是腾空飞起，然后便是对着大天邪魔那犹如深渊般的巨嘴投射而去。
吼！
小炎虎啸阵阵，剧烈的挣扎着，黑白光华不断的从其体内涌出，试图抵抗着那股吸力，但却是毫无作用，只能眼睁睁的看着自己距那巨嘴越来越近，在那巨嘴深处，犹如是死亡深渊，一股无法形容的危险感觉，自心中弥漫出来。
小貂见到小炎落险，面色也是一变，那巨大蝠翼猛的一振，身形便是暴掠而出，试图将小炎给救回来。
不过他身形刚动，那大天邪魔手中魔气图腾柱便是狠狠的挥出，浩瀚魔气涌动间，直接是将小貂给逼退了回去。
“哈哈，落入这大邪魔渊的人，可从没人能活下来！”元门三巨头怪笑道，笑声中充满着邪恶狠毒之意。
小炎激烈的挣扎着，不过待得他发现的确无法挣脱时，那虎目中猛的有着凶光闪现，庞大的身躯之上，竟是有着鲜血渗透出来，一股极端狂暴的能量从其体内散发出来，那竟是要自爆的迹象。
砰！
然而，就在小炎准备拼命给元门三巨头来一下狠的时候，紫金光芒突然笼罩而来，一道巨大的紫金龙尾狠狠的甩在他身体上，那强横的力道，竟直接是生生的将小炎给抽飞了出去。
小炎虎目一愣，然后急忙一看，顿时骇得亡魂皆冒，只见得林动所化的紫金巨龙取代了先前他的位置，然后便是被那散发着无尽魔气的巨嘴，一口给吞了进去。
“大哥！”
暴戾的猩红，几乎是霎那间便是自小炎虎目之中涌了出来，而后他掉头便是对着那大天邪魔冲了过去，不过旋即便是被出现在其前方的小貂给拦了下来。
“二哥？！你干什么？”小炎见状，顿时怒道。
“慌个什么劲？你当他是那么鲁莽的人么？想要吞了他，哪有那么容易！”小貂俊美脸庞一沉，喝斥道。
“可…”小炎也是犹豫了一下，稍稍恢复了点冷静，但虎目中猩红依旧浓郁，他先前可是清楚的感觉到了那大天邪魔巨嘴之中恐怖的魔气，若是落入其中，恐怕就算是轮回境的强者都是会被侵蚀。
“哈哈，你们两人还在痴心妄想，落入大邪魔渊的人，就算是轮回境强者都得被魔气化为灰烬，那林动自大的以为能躲过一劫，真是可笑！”元门三巨头仰天大笑，大笑之中，充斥着讥讽。
小貂面无表情的看着那三张大笑的脸庞，只是对着小炎淡淡的道：“待会一有机会，就全力出手。”
小炎见到小貂脸庞上真没多少担忧之色，这才微微放心，然后重重的点了点头。
那元门三巨头将最为棘手的林动解决，显然是极为的兴奋，那阴冷的目光盯着小貂二人，旋即一声狞笑，就欲出手。
嗡。
而就在他们将要出手时，一道奇特的嗡鸣声，突然自那大天邪魔体内传出，听得这道声音，元门三巨头眼中顿时有着惊疑之色涌现出来。
“三条老狗，小爷可是块硬骨头，吃下去，就怕你们消化不了！”嗡鸣声响起后，一道冷笑声，也是随之响起，并且传了出来。
“倒的确是块硬骨头，不过本座就不信你落进了那大邪魔渊，还解决不掉你！”三巨头眼中寒芒一闪，只见得那大天邪魔身体之上颜色突然变得暗沉下来，惊人的魔气，疯狂的对着体内涌去，试图将那其中的林动给抹杀而去。
“哈哈，你来多少魔气，今日我就给你净化多少！”
不过就在魔气疯狂的涌动间，林动的大笑声再度的传出，而后那大天邪魔的身体，突然剧烈的颤抖起来，他那庞大的身体上，竟然是有着温和的白芒渗透出来。
啊！
白芒浮现，那三巨头面色顿时剧变起来，竟是凄厉惨叫出声，那大天邪魔庞大的身躯都是在此时颤抖了起来。
“轰隆！”
白光闪烁间，又是有着低沉的雷鸣声响起，似乎是有着极端狂暴的雷霆在大天邪魔体内爆炸，那等雷霆之力，也是将那些浩瀚的魔气轰得尽数的消散。
“你这混账！”
元门三巨头厉声咆哮，旋即眼中凶芒一闪，只见得那大天邪魔体内便是响起闷雷声，那巨大的肚腹不断的蠕动着，魔气狠狠的对着体内的林动冲击而去。
“动手，缠住他！”
小貂见状，顿时厉声道，而后他身形率先冲出，手掌一握，十轮紫黑弯月在其周身闪现出来，而后带起浓浓的轮回波动，狠狠的对着那大天邪魔呼啸而去。
吼！
小炎也是虎啸出声，早已忍耐不住的他速度比小貂还快上一分，黑白光芒绽放开来，锋利的虎爪撕裂空间，直接是将那大天邪魔犹如黑铁般的身体上，撕出深深的血痕。
大天邪魔暴怒，手中图腾柱直接是狠狠的砸在小炎身躯之上，低沉之声响起，也是将后者砸得皮开肉绽，鲜血直流，但小炎却没有丝毫要退缩的迹象，虎爪之上黑白光芒疯狂的涌动着，然后也是疯狂的落在大天邪魔身躯上，那番模样，仿佛是要将其撕成碎片一般。
砰砰砰！
凌厉的紫黑弯月也是紧随而至，狠狠的轰至那大天邪魔身体上，带出一道道深深的血痕，黑色鲜血滚滚的落下来。
被小貂两人这番干扰，那三巨头也无法再全力对付体内的林动，只能将攻势转向小貂二人，而虽说失去了林动，小貂二人立即便是被逼入下风，但两人却是将那凶悍性格发挥到极致，即便是拼得重伤，也是要在那大天邪魔身体上留下一些伤痕。
而在小貂二人纠缠间，那大天邪魔的力量却是逐渐的开始削弱，那是因为体内的魔气正在被林动飞快的净化着，这元门三巨头将林动给吞进去，显然不是什么一件明智的事，后者全身上下全是专门对付异魔的神物，这般施展出来，即便那大天邪魔体内魔气再如何的浩瀚，面对着林动怕都是难以有太多的效果。
咚！
而趁着大天邪魔力量削弱间，小貂二人却是抓住机会，狂暴攻势，狠狠的落在了那大天邪魔身躯上，可怕的力量倾泻开来，即便是那大天邪魔，胸膛上都是被撕裂出一道巨大的深深血痕。
“吼！”
大天邪魔遭此重击，庞大的身躯顿时倒飞了出去，而后仰天嘶啸，一道黑光，终是被他喷了出去，而后黑光在半空中逐渐的化为一道人影，正是被其吞进去的林动。
“老狗，这下总该知道小爷有多硬了吧？”
林动此时身体上同样是布满着伤痕，嘴角还挂着一丝血迹，虽然他在这大天邪魔体内闹得天翻地覆，但显然同样也是受了不轻的伤。
“混账东西，本座要杀了你！”
元门三巨头愤怒咆哮，他们能够察觉到大天邪魔体内的那种伤势，那种伤势比起之前林动三人拼了命的攻击所造成的伤势都要重，而且最关键的是，那大天邪魔体内的魔气，被林动那般一搅合，却是紊乱之极，这无疑是令得他们力量有些削弱。
若是早知这样，他们定然是不会将这么一个祸害给吞进体内去！
林动望着暴怒中的元门三巨头，眼中却是掠过一抹厉色，而后缓缓搽去嘴角的血迹，接着凌空盘坐而下。
“小貂，小炎，护我。”
声音落下，林动竟是逐渐的闭目而去，全身浩瀚的元力，也是在此时逐渐的收敛而去，这番模样，看上去异常的平静。
那元门三巨头也是因林动这般平静一愣，旋即心中有些不安涌上来，在被林动之前摆这么那么一大道之后，他们再也不敢小觑这个三年之前被他们逼得犹如丧家之犬一般逃出东玄域的青年。
他们眼中狰狞闪过，那大天邪魔直接咆哮起来，而后手中魔气图腾柱竟是化为一道黑光，洞穿虚空，直奔林动而去。
咻！
不过在那图腾柱掠进林动百丈范围时，紫黑光芒闪掠而来，小貂那数千丈庞大的紫黑双翼护拢在前，犹如一面翼盾，硬生生的与那图腾柱撞击在一起。
惊人的能量冲击波扩散开来，小貂喉咙顿时一甜，一口鲜血喷出，但他却是生生的将身体给稳了下来，也是拦截下了大天邪魔这般凶狠攻击。
吼！
小炎也是在此时疯狂低吼，滔天般的黑白光芒从其庞大身躯中涌出来，顿时他那本就巨大的身躯，竟然再度膨胀了一圈，那虎目之中，无尽的猩红涌动着，凌厉无比的虎爪，撕裂空间，疯狂的对着大天邪魔攻去。
砰砰！
面对着小炎的疯狂攻击，那大天邪魔也是厉吼出声，魔拳舞动，每一拳落在小炎身躯上，都是有着地动山摇般的低沉声传出，空间抖动间，小炎那黑白色的身躯上，也是逐渐被鲜血染红。
小貂同样是眼神赤红，手握两轮巨大弯月，双翼振动间，鬼魅般的在大天邪魔周身穿梭，弯月划起惊人的弧度，狠狠的劈砍在那大天邪魔身体上。
咚咚！
三方混战在一起，小貂与小炎虽然不断的被轰飞而去，但却转眼红着眼又冲了出来，那番近乎搏命般的姿态，竟然也是将大天邪魔拖得动弹不得。
“给本座滚开！”
大天邪魔厉声大吼，只见得其魔臂探出，一手穿透空间抓住了小貂一只蝠翼，另外一手则是抓住小炎虎头，然后狰狞的将两者狠狠的互撞在一起，鲜血溅射间，两人都是倒射了出去，将那地面撕裂出万丈裂缝。
“本座今天就要杀了你们！”
元门三巨头望着遍体鳞伤的大天邪魔，也是暴怒不已，满心杀意涌动，刚欲追杀，三人的面庞猛的一变，豁然抬头，只见得那天空上进步双目的林动，竟是在此时缓缓的睁开了双目，而随着其双目的睁开，其眉心间，一道黑色光线缓缓裂开，然后…一枚深邃的黑暗中仿佛有着雷霆闪烁的神秘之眼，便是出现在了林动眉心之间。
当这颗黑芒与雷霆同时闪烁的神秘之眼出现时，即便是元门三巨头，心中都是猛的涌上一股骇然之意，无法言语的危险之感，遍布全身。


第1214章 祖符之眼
林动凌空盘坐，眉心之间，漆黑深邃犹如黑洞般眼睛缓缓的睁开，在那枚神秘之眼中，仿若黑洞流转，而其内又是包含着雷霆之光，玄奥无比。
黑洞深邃，雷霆狂暴。
两股同样古老但却雄浑无比的力量，在那枚眼中，衍变得淋漓尽致。
那是由两道祖符之力所形成的祖符之眼！
“林动，如今的你虽然掌控着两大祖符，但这两种力量凝聚而成的祖符之眼威力太大，极难控制，现在的你恐怕施展起来颇为的勉强！”岩也是察觉到林动体内的波动，急忙出声道。
祖符之眼一般说来，只有在对祖符的掌控达到某个极为高深的层次后方才能够凝聚出来，类似在那破碎空间中，吞噬之主便是施展出吞噬之眼，差点将那实力恐怖的十王殿都是抹杀而去，当然，林动也很清楚，如今的他即便施展出了祖符之眼，那威力必然也是比不上吞噬之主，但他却是有着一个吞噬之主不曾具备的优势，那便是他体内不仅仅只有吞噬祖符，还有着雷霆祖符！
吞噬之主的吞噬之眼，是纯粹到极点的吞噬之力，而林动这祖符之眼，却是融贯了吞噬祖符以及雷霆祖符的力量，虽说这样一来必然极其难以掌控，但那威力想来也会相当的惊人。
而对于这些，林动自然也是明白，但眼下这种场景已是容不得林动再做多想，元门三巨头并不是寻常角色，如今三人更是借助着这诡异手段融入那大天邪魔之中，那般实力，寻常轮回境强者根本不是对手他们三兄弟能够坚持到现在，也是各自凭借着诸多手段，但即便如此，小貂与小炎也是受伤极重，若再无法祭出杀手锏，恐怕此战将会极其的凶险。
这场战斗，与三年之前一般的惨烈只不过如今的他们，不再是只能逃跑，现在他们同样是拥有着扭转局面的底牌以及能力！
呼。
林动轻吐出一团白气，目光看了一眼遍体鳞伤的小貂与小炎，此时的二人虽然依旧凶气滔天，但那眼中也是有着一丝疲色，为了能够与三巨头血拼，他们也是拼尽了全力。
但即便是如此他们却依旧没有丝毫放弃的迹象，元门三巨头的确很强，但不管怎样，三年前的那一幕，他们绝对不会允许再出现一次“我知道。”
听得林动那平静得没有丝毫波澜的回答，岩也就不再出声他知道前者的性子，若真决定要做一件事，那恐怕还真没人能够让他改变。
“我们三兄弟，我是大哥，三年前已让他们受了委屈，这次，若是还无法打败三巨头的话，这大哥，可就真没脸继续做了啊…”
林动笑了一下然后他那逐渐冷冽的目光，缓缓的转向了那大天邪魔，此时那上面的三张诡异脸庞，正有些变色的将他给盯着。
“三条老狗，三年了，该还债了。”林动望着三巨头，眉心之间那道祖符之眼愈发的深邃，而其面色，却是在以一种惊人的速度变得苍白起来。
“痴人说梦！”
三巨头刺耳的尖笑声传出，不过他们的脸庞此时都是变得凝重了起来，他们能够感觉到那自林动眉心眼中散发出来的惊人危险气息，而且此时的他们也不再是巅峰状态，经历了之前林动体内的一番大闹，再加上小貂小炎二人的拼死攻击，同样是令得他们出现了不小的消耗。
当然虽说状态不是最佳，但三巨头依旧有着极大的自信，他们倒是不相信，以他们这般极强的战斗形态，竟然会输给林动三人！
“轰！”
滚滚魔气，在此时疯狂的自那大天邪魔体内席卷而出，只听得那大天邪魔体内竟是传出道道犹如野兽般的咆哮后声，粘稠的魔气在那大天邪魔魔掌之中疯狂的凝聚而来。
而在那种粘稠魔气疯狂凝聚间，只见得那大天邪魔庞大的身躯居然是逐渐的缩小，不过伴随着那大天邪魔身躯的缩小，一种极端恐怖的波动，也是开始自其魔掌中散发出来。
小貂二人感受到这大天邪魔的举动，眼神皆是一变，他们能够感觉得出来，这三巨头似乎也是在动用杀手锏般的手段。
滔天魔气滚动，而与三巨头这边的疯狂相比，林动那处，却是显得极端的平静，在其周身，甚至是没有丝毫元力波动传出。
极动与极静，却是让得看得心惊肉跳。
犹如这边动静实在是太大，即便是那远处混乱无比的战场，也是有着不少目光忍不住的投射而去，这里的人都很清楚，这场战争，最为关键的点就在林动三兄弟与三巨’间，不论双方哪一方失败，对于那一方，都将会是致命般的打击。
“好惊人的魔气。”
祝犁大长老，柳青等人也是眼神微凛，面庞上满是凝重之色，他们能够感觉到那大天邪魔这般攻势的恐怖，若是换作他们的话，恐怕根本无法抵挡。
“也不知道林动能否接下来…”他们望着那在滔天魔气对面，却是安静凌空盘坐的林动，心中掠过一抹担忧。
轰。
当大天邪魔庞大的身躯在缩小到仅有百丈时，终于是停了下来，而此时，在其魔掌处，粘稠般的魔气，犹如一条河流，缓缓的流淌，那空间也是在这粘稠魔河流淌间，逐渐的蹦碎而开。
“哈哈。”
三巨头仰天大笑，笑声中满是暴虐之意，而后他们眼中杀气凝聚，暴喝之声，陡然响彻而开。
“万魔冥河，魔蚀万物！”
轰隆！
粘稠魔河猛的爆发出轰隆巨响，而后那魔河直接呼啸而出，而后携带着惊天般的魔气，以一种席卷天地之势，疯狂的对着林动冲击而去。
魔河过处，天地仿佛都是被撕裂了开来。
那种声势，不仅小貂二人色变，就连远处的应欢欢等人，身体都是忍不住的绷紧起来。
无数道目光汇聚而来，这一刻，其他的战场都是显得无足轻重，若是林动败在了三巨头手中，接下来的局势恐怕将会对他们极为的不利。
魔河奔涌而来，短短数息之间，已是笼罩向林动，而此时后者的身体，突然在此时细微的颤抖起来，脸庞上的苍白，以一种惊人的速度扩散出来，甚至连那数修长手掌，都是开始泛白。
而与其满身苍白相比，其眉心间那道祖符之眼，却是逐渐的变得明亮深邃起来。
“祖符之眼，净魔之光！”
虚弱的声音，在此时自林动的嘴中轻轻的传出，而后他的双目逐渐的闭上，而眉心那祖符之眼陡然睁大，一道不过手掌大小的黑色光线，悄然的掠出。
这道光线与那浩瀚的魔河相比，犹如萤火与皓月之别，但它却是并没有丝毫的犹豫，光线掠过，一闪下，便是洞穿了空间，冲进了那魔河之中。
轰！
即将冲击到林动身体之上的庞大魔河，在这一瞬间陡然凝固，一抹魔水，距林动脸庞仅有半尺距离，但却是再也落下不得。
嗡嗡。
嗡鸣声，自魔河之中传出，然后众人便是感觉到一股奇特的波动，自魔河内荡漾出来，那道波动过处，浩瀚的魔河，竟是以一种惊人的速度消融，那邪恶无比的魔气，也是铺天盖地的消散而去。
短短不过数息的时间，那连一名轮回境强者都无法抵挡的魔河，便是这般诡异的消失得干干净净。
“怎么可能？！”那三巨头原本暴戾的脸色也是在此时凝固，他们望着这一幕，眼中有着骇然以及难以置信涌上来。
咻！
而在他们目瞪口呆间，那消散的魔河中，一道细小的光线，再度掠出，直奔这大天邪魔而来。
三巨头见到那光线掠来，眼中顿时有着惊骇涌出来，在见识了先前这东西的恐怖后，他们显然再不敢将其无视。
吼！
大天邪魔仰天咆哮，手中魔气图腾柱狠狠的对着那道细小光线怒砸而去。
嗤！
光线射中魔气图腾，顿时有着一道道裂缝自上面蔓延开来，不过霎那间，那蕴含着磅礴魔气的图腾柱，便是爆裂而开。
而那道光线，则是势如破竹般的冲进，最后在三巨头那惊恐的目光中，射进了大天邪魔那庞大的身躯之中。
天空上，面色惨白的林动，也是在此时缓缓的睁开双目，其眉心处的祖符之眼也是飞快的散去，他望着那面色骇然惊恐的三巨头，脸庞上有着一抹淡淡的笑容浮现出来。
“你们，输了。”
就在林动此言刚刚落下时，那大天邪魔庞大的身躯上，突然有着一道道光芒裂缝浮现出来，而后光芒自那裂缝中暴射而出。
三巨头面色骇然，他们望着林动缓缓抬起手掌，然后对着他们轻轻握下。
嘭！
就在林动手掌握下的霎那，那大天邪魔庞大的身躯，也是在此时，轰然爆炸！
魔气滔天，三巨头那凄厉的惨叫声，也是在这片魔气世界中，响彻而起。


第1215章 斩杀三巨头
耀眼的光芒，宛如烈日，自那大天邪魔爆炸之处升腾起来，周遭滔天般的魔气遇见那种光芒，却是犹如遇见岩浆的残雪一般，立即以一种惊人的速度消融而去。
那等光芒在持续了足足十数分钟后，方才逐渐的散去，顿时那无数道紧张的目光，也是急忙的投射而去。
光芒散去，一具仅仅只剩下半个身体的巨大魔尸躺在地面上，黑色魔血流淌出来，散发着一种淡淡的腥臭之意。
原本强悍无匹的大天邪魔，此时却是尽数的没了生机。
“赢了？”那远处的诛元盟等无数强者望着这一幕，先是一愣，旋即猛的爆发出惊天般的欢呼声。
那近处，小貂二人也是面带惊异之色，而后浑身光芒闪烁，开始化为正常形态，只不过那盯着大天邪魔尸体的目光中，依旧还充斥着警惕之色。
噗嗤。
天空中，林动突然一口鲜血喷了出来，那本就惨白的面色更是浓了数分，浑身气息极为的萎靡。
“大哥？”小貂二人见状急忙掠上，小炎一把扶住林动，连连叫道，声音中满是担忧。
林动微微摇了摇头，漆黑双眸盯着那毫无动静的魔尸，然后那双目便是微微一眯。
他的动作虽然细微，但依旧还是被小貂收入眼中，他对于林动的性子极为的了解，当即其瞳孔微微一缩，身体之上，再度有着浓郁的能量波动散发出来，将林动护于身后。
“二哥？”小炎微惊。
“这三个老狗可还没死透。”林动异常虚弱，连带着声音都是变得嘶哑了许多，自从获得吞噬之主传承后，他还是第一次拼到这种地步。
小炎闻言，面色顿时一变，全身也是猛的紧绷了起来。
“不过也离那一步不远了。”林动拍了拍小炎肩膀，然后目带冷笑的望着下方大天邪魔的尸体，道：“怎么？之前不是还很威风么？现在竟然还玩这种手段？也不怕丢了你们元门三巨头的脸么？”
他声音落下，但那魔尸之中却毫无反应，甚至连丝毫波动都未曾传出。
林动见状，眼中冷笑更甚，手掌轻轻一挥。
“嗡。”
小貂也是冷笑一声，双手一握，紫黑光芒便是在其前方凝聚成一道巨大的紫黑弯月，而后屈指一弹，那弯月便是洞穿虚空，狠狠的对着那魔尸冲去。
咻！
而就在弯月即将轰中那残存的魔尸时，一抹黑光终于是忍不住的冲了出来，然后将那弯月生生的震飞而去，不过弯月倒飞，那黑光也是化为三道人影，正是那元门三巨头，只不过此时的他们，面色比林动还要惨白，眼神灰暗，那气息也是萎靡到了极致。
三巨头面色铁青的望着天空上的林动三人，目光急速的闪烁。
“三条老狗，怎么堂堂元门三巨头，也学会跟死狗一样的装死了？”小貂望着三人，讥讽的道，语气刻薄无比。
三巨头脸庞抽搐，眼神狠毒的盯着三人，下一刻，三人身形一动，竟是化为黑光要对着地面钻去，看这模样，显然是想要逃了。
“想逃？”
然而对于他们的举动，小貂却是一声冷笑，脚掌一跺，只见得那地面之上便是有着紫黑光芒席卷而出，直接是生生的将三人给弹了回去。
此时的三巨头，实力显然是骤降，即便是面对着小貂都无法再抗衡。
嗡。
三巨头被弹回来，还不待他们再度有所举动，只见得林动手掌一扬，一座古老的石碑便是呼啸而出，然后迎风暴涨，在那天空上化为一座数千丈庞大的石碑。
石碑悬浮天空，而后爆发出清澈的嗡鸣之声，在那碑底，光芒凝聚而来，似是化为一道古老的阵法，接着狠狠的对着三巨头镇压而去。
“啊！”
在大荒芜碑那种光芒的照耀下，那三巨头身体犹如针扎，剧痛传来，令得他们惨叫出声，他们望着那镇压而来的大荒芜碑，眼中也是有着惊恐之色浮现，他们知道，若是被那大荒芜碑镇压，恐怕就真是他们彻底灭绝之日。
“林动，你真以为本座三人没有最后的手段么？你一定要拼得你死我活么？！”天元子面色阴晴不定，厉声喝道。
林动眼神漠然，丝毫不为所动，那大荒芜碑镇压的速度愈发的加快。
“好，混账东西，想要灭本座三人，那你们这里所有人就随我们陪葬吧！”见到林动没有半点留情的迹象，那天元子三人眼神也是愈发的狠毒下来，旋即他们对视一眼，皆是狠狠一咬牙。
“既然如此，那就都死在这里吧！”
天元子三人猛的掌心对碰，一口黑血自他们嘴中喷出，而后他们的眼神急速的灰暗，身体则是以一种惊人的速度膨胀起来。
“快退。”
林动见到这一幕，顿时一惊，喝道。桀桀，晚了，都去死吧！”
天元子三人脸庞上有着狰狞涌现出来，旋即身体表面有着黑色的液体流出来，最后砰的一声，竟是生生的自爆开来。
轰！
无法形容的魔气，疯狂的自三人体内席卷出来，方圆数千丈之内瞬间夷为平地，小貂与小炎二人当即一口鲜血喷出，身形被震飞而去。
林动在那关键时刻，袖袍一抖，玄天殿浮现出来，将其身体包裹，不过那等魔气冲击实在是太过的恐怖，玄天殿刚刚出现，没坚持多久便是被冲击得黯淡下去，最后钻进林动体内。
而玄天殿一消失，林动顿时暴露在那魔气冲击中，以他此时的状态，显然是没办法再来抵御这种冲击，当即只能一咬牙，强行催动体内元力抵抗。
唰！
不过就在他准备强行抵抗时，突然感觉到一股寒气笼罩而来，一只纤细而冰凉的玉手，轻轻的环住他的腰，而后身形暴退。
风声在耳边呼啸，林动睁开眼，然后便是见到冰蓝色的长发飘过眼前，应欢欢一只手环住他，另外一只玉手飞速的变幻着道道印法，惊人的寒气自其掌心席卷而出，在那前方化为道道巨大的冰盾。
砰砰！
不过冰盾刚刚形成，就会被那席卷而来的魔气冲击化为粉末，但却是有着更多的冰盾形成，这样一来，整片天空都是冰盾爆裂的声音，而应欢欢带着林动飞速倒退，躲避着那魔气的冲击。
但显然，三名轮回强者自爆的冲击，远远超出了林动的意料，即便是有着应欢欢出手，但那魔气冲击，依旧很快的就追了上来。
“让开！”
而就在那魔气冲击即将追上两人时，一道清冷喝声，突然自后方响起，应欢欢闻言，微微犹豫，旋即猛的抓着林动侧开了身子。
在他们让开时，一道白衣绝色身影自后方掠来，在她后方，祝犁大长老，柳青等众多强者也是紧随而来，旋即恐怖元力奔涌而出，竟是在那前方化为一道千丈庞大的元力光幕。
轰！
魔气犹如涛浪般，重重的撞击在那元力光幕上，顿时令得后者剧烈的一颤，那众多强者面色皆是一白，旋即暴喝出声，浩瀚元力尽数涌出。
砰砰砰！
魔气不断的冲击在那元力光幕上，虽然光幕不断的颤抖着，但由于有着众多强者齐齐出手，最后竟是真的将那种冲击给阻拦了下来。
随着魔气冲击逐渐的变缓，众人方才微微松了一口气，那元力光幕缓缓的散去。
“你没事吧？”
见到危机解除，应欢欢急忙看向林动，后者煞白的面色令得她声音中多了一些焦急。
绫清竹以及众人也是围了上来，眼中都是有着担忧之色。
“没事，只是消耗太大。”林动笑着摇了摇头，只是那煞白的面色以及极端虚弱的身体显然没有太大说服力，众人平常都是见他如同打不死的小强一般，何时见过他这般虚弱的时候。
远处，满身鲜血的小貂二人掠来，他们见到林动被救下来也是松了一口气，小炎抹了一把脸庞上的鲜血，然后看了看这片天地，道：“这下那三条老狗应该彻底死了吧？”
林动也是抹去嘴角的血迹，微皱着眉头望着这片魔气世界，刚欲说话，一道狰狞的大笑声，却是在这天地间回荡起来。
“哈哈，你们也别高兴得太早了，我三人虽说自爆，但魔气已让得这魔宴世界彻底的构成，哈哈，从此以后，这里就是魔气的世界，你们一个都别想出去，就老老实实等着被魔气一个个的侵蚀吧！”
那道狰狞笑声响彻起来，最后逐渐的消散而去，而随着声音的消散，那元门三巨头的气息，也终于是彻彻底底的消失。
而林动等人听得这家伙临死前的狠话，面色也是微微一变，急忙转头，然后便是见到那天地间的魔尸也是开始接二连三的坠落下去，一股股魔气飞快的从它们体内涌出来，最后散发而开。
“你看上面。”
应欢欢脸颊上浮现一抹凝重，轻声道。
林动闻言，急忙抬头，顿时见到那天空上，一道厚重得根本无法看透的魔气光罩蔓延开来，然后将这片天地尽数的笼罩着。
在那魔气光罩之上，还能够看见无数诡异的魔纹蠕动着，这道光罩，犹如隔绝了这里与外界的任何联系。
众人望着这一幕，面色也是逐渐的苍白起来。


第1216章 两女联手
厚重的魔气光罩，遮掩着天空，然后在那极为遥远的地方垂落下来，犹如倒扣的大碗，将其中的所有人都是笼罩在了其中。
众人望着那光罩内蕴含的魔气波动，面色皆是略有些发白，他们能够感觉到，这片魔气世界仿佛因为天元子三人的自爆，再加上那无数的魔尸分解，已是自成一片魔气小世界。
这种魔气世界，仿佛无缝可寻。
原本火暴无比的战斗，在这一刻却是变得极为的安静，那些先前还嗜血无比的魔尸，已是尽数的消散，天空上，那些诛元盟的强者面面相觑，旋即眼带忧色的望着天空上的魔气光罩，一种骚乱，逐渐的传开。
“现在怎么办？”应笑笑见到那些传荡而开的骚动，皱眉轻声道。
“这里魔气太重，以我们的实力倒还无碍，但一些寻常强者若是呆的久了，恐怕会被魔气侵蚀。”应玄子道。
“这元门三巨头倒还真是有些手段，以前我天妖貂族倒是与他们交过手，不过那时候的元门想要与我们交锋还得将整个东玄域不少超级宗派拉上，没想到现在竟是强到这种地步。”祝犁大长老有些感叹的道。
“他们是借助了异魔的力量，不过这种方法，最终只能毁灭自己。”林动面色苍白的道，他的目光也是顶着那片厚重无比的魔气光罩，道：“得想办法将其破开才是。”
“我来！我倒是要试试这什么狗屁魔宴世界能有多厉害。”
柳青咧嘴一笑，旋即其眼神便是凌厉起来，周身青光大作，龙吟响彻间，上千道青龙光纹奔腾，而后便是在其一声低喝间，化为一道巨大青龙呼啸而出，最后狠狠的轰击在那魔气光罩之上。
嘭！
一圈青色光芒蔓延开来，那道光幕泛起一道涟漪，然后又是逐渐的平息而下，柳青这强横的攻势，竟是没带来丝毫的效果。
众人见状，眼神皆是一凝，柳青脸庞上也是浮现一抹凝重之色，旋即摇了摇头，沉声道：“的确很厉害。”
连狂傲如他，都是不得不承认这魔宴世界的厉害，由此可见眼下这局面究竟是有着多么的棘手。
林动眉头紧锁，现在的他正处于虚弱期间，显然是无法出手，再加上这周围弥漫的魔气，即便他拥有着吞噬祖符也不可能将这些魔气吞噬吸收，这样一来，恢复速度自然是大减。
“应掌教，林动小哥，我们现在如何是好？”那剑宗的宗主墨鉴以及其他的一些强者也是迅速的掠来，他们冲着林动一抱拳，在见识了林动之前那恐怖的实力后，他们显然不敢对后者丝毫的不敬。
“要不我们招集所有人一起出手攻破这魔宴世界？”应笑笑提议道。
“不行，这片地域，恐怕是元门早就布置好的，再加上元门三巨头自爆，这片地域的魔气已成循环之态，我们人数虽多，但却是杂乱不堪，强行而来的话，元力反而会被魔气同化，令得那魔气愈发的浓郁。”应欢欢微微摇头，道：“想要破这魔宴世界，人越少越好。”
“但人数少的话…又如何能破得了这魔宴世界？而且眼下林动显然是不能再动手了。”应玄子叹了一口气，道。
众人一阵沉默，连柳青这等实力的攻击都没多大作用，这里能够超过柳青的，除了林动外，还能有何人？
“我来吧。”
沉默间，突然有着两道女子声音同时的想起，众人目光一看，只见得那说话二人竟是绫清竹以及应欢欢。
应欢欢见到绫清竹说话，也是微微怔了一下，旋即淡淡的道：“虽然如今你也是晋入转轮境，不过这般实力对这魔宴世界可是没多大的作用。”
“任何东西都不能只看表面，你说是吗？”绫清竹看着应欢欢那同样俏美的冰冷容颜，道。
应欢欢微蹙着柳眉，那冰蓝美目也是盯着绫清竹，显然她并不相信后者。
众人见到两女这般模样，皆是微愕，旋即目光古怪的看了林动一眼，都未曾说话。
“咳。”
被众人古怪目光盯着，林动也只能轻咳了一声，道：“你们一起出手吧，或许联手机会也会大一些。”
见到林动开口，应欢欢这才将美目移开，而后道：“我先出手吧，你看情况再接手。”
绫清竹性子清冷，不喜与人相争，若非如今事关重大她也不会出面，所以在见到应欢欢相让后，她也是不再多说，只是螓首轻点。
应欢欢莲步轻移，缓步上前，紧接着便是有着惊人的寒气自其体内散发出来，周遭的空气都是在此时咔嚓的凝结成冰花，飘然而落。
寒气在她周身凝聚，应欢欢双目却是缓缓的闭上，而在其双目闭上间，众人皆是能够见到，她那一头冰蓝色的长发，竟然是在此时逐渐生长起来，而且那种冰蓝之色，愈发的浓郁。
冰蓝色的长发，飘荡在其身后，远远看去，犹如一道冰蓝色瀑布，美丽中透着一种刺骨的冰冷。
后方的林动望着应欢欢那生长起来的冰蓝色长发，手掌却是忍不住的紧握了一下。
“永恒冰封戟。”
弥漫着无尽寒气的清澈娇声，猛的响彻而起，应欢欢美目陡然睁开，纤细指尖在面前划出一道玄奥冰纹，而后冰蓝色光芒大作，一道异常华丽而冰寒的冰戟，迅速在其前方成形，冰戟之上，布满着玄奥的符文。
咻！
冰戟一出现，这天地温度都是骤降下来，而应欢欢轻咬舌尖，一道精血喷射而出，落在那冰戟之上，而后冰戟颜色迅速由冰蓝色化为暗红色泽，一种凌厉的冰寒扩散开来。
“去！”
应欢欢美目一凝，一声冷喝，那暗红冰戟陡然暴射而出，狂暴的劲风四散而开，将其那长长的冰蓝色长发震得飘舞起来，那一刻，这一道纤细身影，犹如一朵凛然绽放的绯色之花，耀眼无比。
唰！
暗红冰戟势如破竹般的撕裂重重魔气，最后狠狠的射在那魔气光幕之上，顿时惊天般的寒气弥漫开来，霸道的寒气，疯狂的冰冻着那涌来的魔气。
咔嚓。
一道细微的裂缝，突然在那魔气光罩上浮现出来，不过当这道裂缝出现时，那暗红色的巨大冰戟上，也是有着密密麻麻的裂纹浮现。
两道裂纹不断的蔓延着，不过显然，冰戟之上的裂纹蔓延速度，要更为的迅猛。
短短十数息的时间，那魔气光罩裂缝已达十数丈，不过眼看那厚重魔气光罩将要被洞穿撕裂时，那暗红冰戟，也是砰的一声，爆裂开来。
应欢欢嘴中传出一道细微的闷哼，红唇处有着一丝血迹渗透出来，娇躯一震，竟是倒飞了出去，后方林动见状急忙掠出，一把揽住那纤细柔软的腰肢，将其接了下来。
“唰！”
而在林动掠出时，一旁的绫清竹也是在同一时间疾掠而出，旋即其玉足轻踏虚空，玉手一握，一柄青峰长剑便是闪现而出。
她玉手紧握长剑，浑身上下波动内敛，只是在其头顶上空，突然有着一道极端朦胧的光芒散发出来，一股无法言明的波动，悄悄扩散。
“这是…”在那股波动传出的霎那，林动分明的感觉到体内的岩猛的震惊出声。
“什么？”林动急忙在心中反问，但岩却是突然陷入了沉默，只有那尚还在微微震动的符石显露着他内心那震荡的波动。
虽然心中有些疑惑，但林动也明白此时不是多问的时候，目光急忙投射向绫清竹，只见得她手持长剑，突然轻轻一剑刺出。
剑身之上，并没有太过浩瀚的元力波动，但却是有着一道薄薄的奇特光芒所覆盖，那种光芒，即便是林动，都是感觉到皮肤阵阵发寒。
一剑刺出，一道并不起眼的剑芒掠出，然后洞穿虚空，轻轻的落在先前应欢欢撕裂而出的裂纹之上。
剑芒与滔天魔气对恃，下一霎，其上光芒一闪，众人只听得噗的一声，剑芒顿时洞穿魔气，那道魔气光罩之上的裂纹，终是在此时被刺穿了过去。
一缕阳光，透过裂纹射将进来，外界的天地元力涌入，立即便是将这片魔宴世界的循环所打破，顿时那光罩上，密密麻麻的裂纹蔓延开来，最后砰的一声，彻彻底底的爆炸开来。
温暖的阳光再度铺天盖地的降临而下，众人望着那出现在眼中的光亮以及远处的葱郁，那种劫后余生之感，令得他们忍不住狂呼出声来。
这一战，他们终于是获胜了。
林动深吸了一口气，体内吞噬之力运转，那天地元力便是滚滚的涌入体内，感受着那种畅快之感，他脸庞上也是有着一抹如释重负般的笑容浮现出来。
“总算是彻底解决了…”应玄子也是忍不住的露出笑容，元门已毁，这东玄域的战争，也终于是可以停歇下来了。
其他的那些东玄域强者脸庞上也满是激动，元门被灭，大仇得报，他们总算能够继续的留在这里了。
绫清竹绝美的脸颊泛起一抹淡淡的苍白，然后她看了一眼被林动揽在怀中的应欢欢，低下头将手中长剑缓缓入鞘。
“你们都没事吧？”林动倒是眼尖，见到她脸颊上的苍白，有些担忧的问道。
绫清竹与应欢欢皆是摇了摇头，刚欲说话，后者美目猛的一凝，而林动的面色也是在同时间有些变化，缓缓的转过头，目光望向不远处的一座山峰，那里，一道黑影双手负于身后，他立在那里犹如一座石雕般，没有任何的气息，根本无法感应，但其本身又是真实的存在着，这种诡异的感官，令得林动等人面色皆是凝重了起来。
“呵呵，设计这么多年的棋子，竟然便是这般的被毁了…真是有些可惜。”
那道黑影笑望着林动等人，而后他的目光似是停留在了应欢欢的身上，微微一笑，道：“冰主，万千载不见，别来无恙啊。”


第1217章 四王殿
黑影淡淡的笑声，在这天空中扩散开来，闻似轻微的声音，却是在此时盖过了那无数强者的欢呼。
因此众多欢呼声噶然而止。
一道道惊疑的目光转移，然后望向那山峰上的黑影，不少强者体内已是有着元力运转起来，眼神之中，充满着警惕。
“阁下是何人？”应玄子眼神凝聚在那山峰上，沉声道。
林动目光紧紧的盯着那道黑影，手掌却是忍不住的紧握起来，听他先前那句话的意思，此人显然也是异魔，而且身份恐怕还不低。
应欢欢眼神冰冷，冷声道：“藏头露尾的，想来也不会是什么好东西。”
周围的祝犁大长老等人已是簇拥了过来，眼中充满着戒备的盯着这神秘黑影。
“呵呵。”
那道黑影笑了笑，然后他缓缓的抬起头来，阳光照耀下，那似乎是一张格外苍白的年轻脸庞，只是那脸庞显得格外的阴柔，狭长的双目内，一片漆黑，犹如那阴煞的深渊之底。
在这张年轻脸庞眉心处，有着一道褐色的魔纹，魔纹延伸到他的眼角处，看上去透着一丝鬼魅之意。
“倒是忘了，现在的你似乎并不认识我，呵呵，那便介绍一下我现在的身份吧，魔狱四王殿…”那人微微一笑，道。
“四王殿？”
林动瞳孔一缩，面色陡然凝重起来，然后他一把拉住应欢欢，将其拉在身后，目光紧紧的盯着那道黑影，在那破碎空间中，他已经遇见过魔狱的十王殿，后者那恐怖的实力给他留下了极深印象，眼前之人，在那魔狱之中排名比十王殿还要靠前，想来实力也是极为的恐怖。
“呵呵，原本以为天元子他们能解决掉你们的，没想到…唉，废物始终都是废物，亏我魔狱在他们身上花费了这么大的心血。”四王殿笑吟吟的道。
林动面无表情，体内吞噬之力急速的运转着，然后将周围天地元力吞进体内飞速的炼化。
“你便是那个林动吧？据说你获得了吞噬之主的传承…这样说来的话，天元子他们倒是败得不冤枉，吞噬之主那家伙，就算是我全盛时期也打不过，他的传承者应该也不简单的。”
四王殿的目光，又是转向了林动，旋即轻轻一笑，道：“你是想要快点恢复实力与我动手吗？”
林动淡淡一笑，道：“阁下来这里就是来说这些废话的么？”
“呵呵，倒的确是想要来抹除一些麻烦的，特别是在这里竟然还能够遇见冰主的轮回转世，真是好运啊，若是将你在这里杀了的话，想来这第二场天地大战也就没必要开始了吧？”四王殿看着应欢欢，笑着道。
“那你就来试试。”应欢欢俏脸冷若寒冰，冰蓝色的美目中，有着许些杀意掠过。
“如果你真是冰主的话，那现在我恐怕还真就跑了，可惜…”四王殿摇了摇头，似是有些惋惜的道。
林动盯着这四王殿，眼瞳之中，突然有着黑芒涌动起来，而后眼前的世界略微有些改变，那四王殿的身体，仿佛看上去也是有点模糊。
“咦？”
在林动察觉到这一情况时，那四王殿眼中也是掠过一抹讶异，他盯着前者，笑道：“竟然被发现了啊，难怪天元子他们会败于你手。”
“原来不是本体。”
林动看了四王殿一眼，心中微微松了一口气，眼前的这四王殿，并非是真身，不过这家伙能够在隔着不知道多远的距离凝聚出这么一具同样强大的分身，这实力，倒也的确有些恐怖。
“虽然只是一具魔体分身，不过对付你们应该也已经足够的。”四王殿似是知道林动心中所想，当即摇了摇头，道。
林动冷笑的望着四王殿，道：“你也不用虚张声势，你们异魔在这世间躲躲藏藏，显然也是有所忌惮，不然以你们的性子，恐怕早便是杀了出来将这片天地所侵占。”
“你眼下不敢现真身，想来也是在惧怕着什么吧？我猜，你是在害怕着其他几位远古之主吧？”
四王殿脸庞上的笑容似乎是在这一霎微微的凝了凝，旋即他盯着林动，轻声道：“真是个聪明的人啊…”
“不过…就算你知道这些，又能怎么样呢？我现在要杀你们，也并不难的，那几个家伙沉睡的沉睡，谁又能知道这里的事？”
四王殿戏谑的看着林动，眼中却是有着冰冷的杀意在缓缓的涌动着。
“你们这种强大的魔气在这天地间可太容易感应了…你们若真敢大张旗鼓行动的话，又何必花这么大的心思来栽培元门？”林动淡笑道。
“只不过是想要他们自己人杀自己人，然后看看乐子而已。”
四王殿双手缓缓的垂下，淡漠的看了林动一眼，道：“好了，废话就到此结束吧，既然你们这么相信自己有自保的能力，那便让你们亲自来试一试吧。”
“大荒芜经！”
他的声音刚刚落下，只见得林动脚掌猛的一跺地面，一道玄奥波动扩散而开，这片大地顿时颤抖起来，浩瀚的能量奔涌而来，最后源源不断的灌注进林动体内。
“真是个狡猾的小子。”
四王殿见到林动这果断异常的举动，忍不住的一笑，只是那笑容中弥漫着杀意，原来之前林动与其说是在故意暗中准备着施展手段加速恢复实力。
“不过这样的话，我更想杀了你了！”
当四王殿声音一落时，只见得他人影已是出现在了那前方不远处，身形飘忽如一缕黑烟。
“杀了他！”
他一靠近，那诛元盟无数强者也是爆发出怒吼之声，下一刻，铺天盖地的元力攻势呼啸而出，对着那四王殿笼罩而去。
咻。
不过那四王殿身形如鬼魅，直接是自那漫天攻击中飘掠而过，却是没有半道攻击落到他的身体之上。
就在四王殿穿过那漫天攻势时，又是有着滔天寒气弥漫而来，应欢欢纤细双手猛的结印，只见得寒气瞬间凝聚成漫天冰雪，闪电般的对着那道身影汇聚而去，短短数息间，便是将那道身影，化为一座冰雕。
“小心！”
就在那道冰雕成形时，一道清冷喝声突然在应欢欢身后响起，而后一道剑芒暴掠而出，狠狠的刺在了前方虚无处。
叮！
剑芒刺在虚无处，却是爆发出金铁之声，火花溅射间，一道人影便是鬼魅般的浮现出来，正是那四王殿，而此时的他，双指轻夹着剑尖，冲着应欢欢与绫清竹淡淡一笑。
轰隆！
天空上，有着光芒笼罩而下，四王殿眉头微皱，抬起头来，只见得一道极端庞大复杂的阵法，不知何时自那天空上凝聚出来。
“这是…乾坤古阵？”四王殿望着那有些眼熟的古老阵法，那眼神终是一凝。
浩瀚元力以及精神力在此时猛的自林动体内席卷而出，最后尽数的灌注进那乾坤古阵之中，而后阵法运转，一道光柱陡然呼啸而下，直奔那四王殿而去。
光柱瞬间撕裂了天地，那四王殿见到这般攻击，也是微微犹豫，旋即身形闪掠而退。
咻！
光柱落空，但却并未消失，一个转弯，再度对着那四王殿追击而去。
见到这般攻击没完没了，四王殿也是一声冷哼，双手猛的轻合，然后陡然拉开，一道巨大无比的黑色光盾，便是瞬间凝成嘭！
光柱狠狠的轰击在那黑色光盾之上，可怕的冲击波席卷而来，将下方地面都是震得塌陷了下去。
四王殿的身形也是急退了数步，然后稳下来，眼神漠然的望着那正盯着他的林动，淡淡一笑，掌心之间，开始有着粘稠得近乎实质般的魔气缓缓的涌动着。
“即便是经历这般惨战，还能有着隐藏的手段，你这家伙，倒还真是有点不简单。”
“不过…即便如此，今日恐怕依旧改变不了结局了。”
四王殿的身形，在随着他最后一个字的落下时，已是再度冲了出去，这一次，他直奔林动而去，那眼中的杀意，令人心寒。
林动盯着那直奔他而来的四王殿，袖袍一挥，一道黑影同样是冲出，而后黑色长刀怒劈而下，刀芒犹如撕裂了虚空。
那一道刀芒极端的厉害，就连那四王殿都是有所察觉，但古怪的是，他身体却是避也不避，任由那黑刀斩落，然后，刀身从他的身体上，穿透了过去。
林动见到这一幕，瞳孔突然一缩，猛的厉声道：“欢欢小心！”
喝声中，他已是转过头，然后便是见到那鬼魅般出现应欢欢身前的四王殿，这个家伙，从一开始的目标，就是应欢欢。
“呵呵，跟解决冰主比起来，你显然是没那么重要的。”
四王殿似是冲着林动露出一抹嘲讽笑容，然后他手掌便是对着应欢欢迅猛的抓了过去。
林动面色阴寒，心神一动，那乾坤古阵上，一道凌厉光柱便是奔涌而下，直射四王殿而去。
“轰！”
不过就在林动出手的那一霎，那下方的大地突然崩裂，一道岩浆火柱也是破地而出，而去先那光柱一步，抵挡在了应欢欢的面前。
四王殿掌风落在那岩浆火柱上，手掌瞬间通红，而后他的面色突然在此时剧变起来，身形一动，就欲暴退。
嘭！
而就在他暴退间，一只弥漫着岩浆的手掌已是自火柱中探出，最后狠狠的印在其胸膛之上。
四王殿身形倒飞而出，脚步踉跄的在虚空急点，这才堪堪的稳住身形，目光极端阴沉的望着那岩浆火柱。
“这？”
突如其来的岩浆火柱，也是令得林动等人一惊，他们皆是盯着火柱，然后火柱逐渐的平息下来，接着，一道欣长的身影，也是缓缓的出现在了众人眼前。
那是一名挺拔的男子，他身着金色的袍服，一头头发，呈现火红之色，犹如岩浆在流淌，他仅仅只是站在这里，这片天地的温度仿佛都是逐渐的升高起来。
林动望着这道身影，眼神却是陡然一凝，这模样，似乎是有些熟悉…
在那众多目光中，那红发男子也是缓缓的转过头来，露出一张英俊中透着一些威严的年轻脸庞，而此时，他并没有看向其他人，只是将目光停留在身后应欢欢的身上，那对深邃的眼目中，有着一抹温柔之色浮现出来，接着他微微一笑，轻声道：“小师妹。”


第1218章 炎主
红发男子站在应欢欢面前，那金色的袍服透着一些尊贵，只不过此时那张英俊脸庞却是颇为的柔和，他的目光紧紧的盯着应欢欢，眼中有着温柔的波动在泛起。
应欢欢倒是被他那目光看得有些不自在，微微退后了两步，柳眉微蹙的扫了他一眼，道：“你是谁？”
红发男子一愣，旋即明白过来，笑道：“原来小师妹还未恢复啊。”
说到此处，他话音顿一下，道：“我是炎主。”
这话一出，周围所有人除了林动外，都是变了脸色，那一道道望向红发男子的目光中，充斥着震惊以及惊骇。
炎主？那位传说之中的远古八主之一？
对于这里的很多人而言，炎主这个名字都仅仅只是存在于传闻之中，他们只能从远古的一些古籍中知道这位曾经天地间巅峰强者的强远古的那场天地大战，这位炎主，便是曾经让得那无数异魔闻风丧胆，然而现在，那传闻之中的巅峰强者却是活生生的出现在了他们的面前，这如何能让得他们心境平静？
“炎主？”应欢欢听到这个名字，也是怔了怔，旋即美目看了他一眼，再度退后了两步，摇着头道：“我是应欢欢，不是冰主。”
“迟早都会是的。”炎主不在意的一笑，道。
应欢欢闻言，那俏美脸颊上顿时有着寒气涌上来，不过不待她说话，一道人影已是从后方跨过，然后挡在了她身前。
“原来是炎主大人驾到，真是好久不见。”
林动望着面前这有些熟悉的人影，脸庞上有着一抹笑容浮现出来，对于这位炎主他可不陌生，当年他刚刚进入乱魔海时，便是闯进了那神秘的岩浆空间，在那里见到了沉睡之中的后者，而且当时的他还顺手从他身上拿走了一道炎神牌。
“是你？”
炎主看着眼前的林动，脸庞上也是有着一抹讶色掠过，当初虽说他是沉睡，但还是感应到了林动在那岩浆空间中的所作所为，不过那时候的林动在他眼中，或许就与寻常蝼蚁没什么区别，只是没想到短短数年不见，竟是强到了这种程度。
炎主目露神光的扫视着林动，那眼中的惊色愈发的浓郁，许久后，他有些复杂的叹了一口气，道：“你获得了吞噬之主的传承？”
林动点点头，以炎主的能耐，要看出这些的确不难。
炎主沉默，眼神复杂，当年吞噬之主燃烧轮回护了他们几人，但自己却是付出了陨落的代价，而既然如今他的传承交给了林动′想来他也是彻彻底底的消散在了世间，这令得炎主心中忍不住的有些黯然。
在两人说话间，应欢欢倒是看着那道挡在她身前的身影微微一怔，旋即脸颊上的寒气逐渐的淡去，唇角掀起一抹细微的弧度，这家伙，是在吃醋吗？真是难得呢。
心中微微愉悦间，应欢欢则是伸出那冰凉的玉手，然后轻轻的握在林动手掌上。
手上突然传来的冰凉触感让得林动也是愣了一下，但很快的便是回过神来，脸庞上一抹笑容浮现，反手轻握住那冰凉玉手，缓缓用力。
两人的举动，自然也是被炎主看在眼中，然后他眉头微微一皱，迟疑了一下，开口道：“小师妹。”
“你还是叫我应欢欢吧，我喜欢现在这个名字。”应欢欢螓首轻摇，道。
这话听得林动心头微畅，这小妮子如今也终是变得这般善解人意了，旋即他看着炎主那神情，忍不住的在心中别扭的道：“岩，这炎主是怎么回事？你最好别告诉我，他和冰主是情侣。”
“咳…”岩干咳了一声，道：“冰主素来都是独自一人，哪来的什么情人，不过似她那般人儿，魅力自然是不会小，这炎主，便一直喜欢她，只不过是落花有意流水无情罢了。”
“原来是单相思。”林动这才恍然，没想到这位威名自远古流传至今的大人物，竟然也有情场失意的时候，看来那位冰主倒的确是心高气傲的人。
炎主听得应欢欢的话，也不怎么在意，只是深深的看了林动一眼，道：“既然你这样说，那我也就不勉强，等你变回冰主的那一天，我再这样称呼你。”
林动眉头皱了一下，那炎主看他的目光，略微的有些古怪，似是有点同情，这让得他心中微恼，这家伙，就认定了应欢欢会变成他所认识的那什么鬼冰主么？
不过恼怒归恼怒，但林动不知为何心情却是微微有些发沉，一种莫名的情绪让得他心中发堵，那握着应欢欢冰凉玉手的手掌忍不住的用力了许多，而后者仿佛也是察觉到他的动作，美目微垂，修长玉指微微蜷缩，然后也是反握住林动的手掌。
“呵呵，真是没想到…竟然连炎主都是露了面了过我看你这模样，似乎也不是真身降临啊？”在林动心中情绪翻滚时，那不远处，一道淡漠的笑声传来，只见得那四王殿正一脸淡笑的望着现身的炎主。
听得这话，林动也是看了一眼面前的炎主，这才发现后者的身体散发着淡淡的光芒，那种情况，的确不像是真身降临，看这模样，后者也是如同那四王殿一般，以大能力凝成分身，出现在距那本体极为遥远的地方。
“若是真身来了，现在你不早便是逃之夭夭了？”炎主看了看四王殿，旋即挥了挥手，道：“滚吧。”
虽然之前在应欢欢面前这位炎主柔和异常，但此时在面对着这四王殿时，举手投足间，那身为远古八主之一的威严与霸气便是显露了出来。
“嘿，同样是分身降临，你就真当我怕了你不成？”四王殿那阴柔的脸庞上掠过一抹难看之色，冷笑道。
炎主眼神平淡的盯着那四王殿，旋即其眼中，一抹赤红之光掠过，整片天地都是在此时陡然灼热起来，天空上云彩也是转变成赤红之色。
轰隆！
天空上，突然有着巨声响彻，众人抬头，然后便是骇然的见到，一颗火焰陨石竟然不知何时在那云层之中成形，然后直接划过天地，狠狠的对着那四王殿轰了过去。
这炎主一出手，便是展现出了极端恐怖的实力，远古八主之一，名不虚传。
那四王殿见到炎主出手，脸庞上也是掠过一抹凶光，但其眼中，却是有着深深的忌惮，显然，对于炎主，他不敢有丝毫的怠慢。
磅礴粘稠般的魔气，猛的自四王殿体内席卷而出，旋即他大手一握，那魔气便是化为一片魔海，悬浮在天空之上。
轰！
火焰陨石狠狠的冲进那片魔海，溅起滔天魔水，赤红之色，陡然蔓延开来，那魔海竟是无法将其浇灭，反而是在火海的蔓延下，以一种惊人的速度被融化而去。
四王殿的脸庞上，掠过一抹苍白，旋即其身形一动，再度出现时，已在应欢欢身后，弥漫着魔气的一拳，毫不留情的对着后者轰了过去。
“哼。”
林动早已时刻警惕着这家伙，在其身形刚动之时，他袖袍便是一挥，那手持黑色长刀的吞噬天尸立即出现在应欢欢身后，一刀劈出。
铛！
那四王殿泛着魔气的手掌探出，与那吞噬天尸黑色长剑硬憾在一起，火花溅射间，虽然能够取得上风，但这般攻势，毕竟也被吞噬天尸给阻拦了下来。
炽热的火焰，瞬间自地面涌出，化为一道火拳，狠狠的轰在那那四王殿身体之上，火焰奔涌间，那四王殿身形顿时倒飞出数千丈。
四王殿面色有点苍白的稳下身来，眼中满是阴沉之色，他望着那将应欢欢护在身后的炎主以及林动二人，心中已是明白，他再无得手的机会。
“炎主，你们也不用得意，上一场天地大战你们因为符祖燃烧轮回而侥幸获胜，可这一次，没有了符祖，你们难道还想赢第二场天地大战么？”四王殿冷声道。
“我知道你们隐藏这么多年，应该也是在谋划着什么，如今的我的确还暂时腾不出手来，不过你们还是得小心了，因为…”
炎主淡淡一笑，道：“雷主他已经苏醒了，现在正满世界寻你们，所以，你们可得先躲好了，被他寻着了，恐怕就不容易逃了。”
四王殿瞳孔微微一缩，冷笑道：“雷主么？苏醒了又能怎样？光凭一位远古之主，恐怕还奈何不了我们！”
“是么？”炎主不置可否。
四王殿冷哼一声，事已至此，多说无益，当即他一挥手，身形便是逐渐的变得虚幻起来，最后彻彻底底的消失而去。
“炎主，你们就等着吧，这一次的天地大战，我异魔族必要灭了你们这片位面！”
随着四王殿的消失，他那阴狠的声音，却依旧是滚滚不休的回荡在这片天地。
炎主面色没有丝毫的变化，然后他转过身来，对着林动，应欢欢道：“走吧。”
“走？”
林动一愣，目光古怪的看着他，道：“你难道要跟我们一起走？”
炎主淡淡一笑，目光却是停留在应欢欢的身上，道：“如今她的身份已是暴露，在她彻底觉醒之前，我都会留在她身旁。”
说完，他就不再多言，双目微闭，闭目养神。
林动一听，面色顿时黑了下来，他娘的，这货还赖上了？


第1219章 树静风止
对于这突然粘上来的牛皮糖，林动最终也没能将他甩开，得他颇感无奈，不过事已至此，这炎主显然是不会离开应欢欢身边，既然改变不了，那就暂时接受一下吧，如今这天地间的局势已是有些动乱的迹象，那些蛰伏万千载的异魔也是开始露头，再加上应欢欢冰主轮回的身份暴露，若是再有强大异魔前来，也是极大的麻烦，有了炎主这免费打手，想来也是能够安全许多。
心中这般想着，林动也就不再多虑，眼下元门已被灭，继续停留已是没了必要，因此众人在略作交流后，便是大军开动，赶回道宗。
而至于这临时组建的诛元盟，此时显然也是没了存在的必要，不少强者则是纷纷告辞而去，对他们，林动也没怎么挽留，虽然这种时候收揽这些强者会令得道宗实力大为的提升，但他却并不太愿意这么做，他之所以喜欢道宗，是因为这个宗派拥有着值得他守护的感觉，若是突然间汇入如此庞大的陌生强者，难免道宗的风气会有所改变，那是他最不愿意看见的一幕。
所以，他宁愿道宗稳扎稳打，逐步的变得强大，也不愿意道宗受到这种冲击，从而从本质上发生改变。
当林动这般大部队再度回到道宗时，迎接他们的是无数亮晶晶的眼睛，震天般的欢呼，冲上云霄，即便是千里之外依旧能够听见。
所有的道宗弟子皆是激动得眼睛发红的冲出道宗来，这一年来，他们不知道有着多少师兄弟死在元门的手中，然而即便如此，他们依旧只是能够压抑着心中的仇恨，咬牙切齿的望着元门耀武扬威，而他们，却只能龟缩在护宗大阵内，甚至连偶尔出去，都将会提心吊胆，不敢暴露身为道宗弟子的身份。
而这一切，当那一道有些风尘仆仆的青年身影再度出现在他们道宗时，终于是开始改变。
他的身影，依旧是那般挺拔，只是那年轻的脸庞，愈发的坚毅，再然后，离去三年的他，再度将道宗重担扛起，一如三年之前异魔域中，那在最后关头赶来，以一人之力，力挽狂澜的场景。
在他的回归下，那一直将他们压着抬不起头的元门开始退去，最后终是被他招集人马，杀上元门，将那所有的恩恩怨怨，尽数的了结。
留在道宗的弟子或许并没有看见元门之前的那惊人之战，但他们却是始终坚信着，这一次他们道宗，必将在他的带领下，真正的崛起！
而那道身影，最终也并没有让得他们失望，他带着道宗人马而去，最后带着他们梦寐以求的战果凯旋。
虽说道宗成立的历史中，或许也出现过不少极端优秀的弟子，但所有人都是明白，林动，他超越了这个宗派以往的所有前辈。
有一种人，注定被铭记。
道宗的狂欢，足足持续了五日时间，而在这五日内，元门被灭的消息，也是如同风一般的传遍了整个东玄域，于是那种狂欢，开始从道宗蔓延到整个东玄域。
整个东玄域都是轰动了起来。
元门发动的战争，波及整个东玄域，无数宗派，王朝在元门铁蹄之下化为飞灰，但面对着元门的强势，即便他们心怀恨意，但却是只能犹如老鼠般的躲着，而谁也未曾想到，那之前还不可一世，无人能敌的元门，居然便是以一种迅雷不及掩耳之势的速度倒下了，而且还是倒得那般的彻底。
而在元门崩塌，东玄域震动间，道宗的名气，也是趁此达到了一个巅峰的程度，强如元门，最终都是被道宗所解决，这东玄域之上，还能有何宗派与道宗争雄？
如今东玄域诸多超级宗派被元门所灭，以往所形成的格局已是破碎，真正的超级宗派，唯有道宗与九天太清宫尚还屹立，显然，在这场战乱后，这两个超级宗派将会成为最大的受益者。
而在道宗名气达到顶峰时，林动的名字，也是随之响彻了整个东玄域，因为谁都清楚道宗的这场反攻之中，究竟谁才是主宰者，道宗能够一甩以往被压着打的疲态，反攻元门将其所灭，林动之力，居功至伟。
另外借助着林动之名，大炎王朝也是开始让得东玄域人人知晓，这个曾经在东玄域上的一个不起眼的低级王朝，如今在这东玄域，恐怕再没任何一个王朝敢去招惹。
在接下来的一些时间中，东玄域上也是逐渐的有着一些超级宗派重建，但那实力以及名气都是远不如从前，而他们也是颇为的聪明，对于道宗保持着极大的敬意，俨然一副要以道宗为首的姿态，在见识了道宗的恐怖实力后，他们显然是再没了其他的心思。
而对于其他宗派那种以道宗为首的姿态，道宗倒是不置可下他们并没有类似元门的那种野心，一定要东玄域上所有的派都是向他们臣服，他们一切所做的，都只不过是想要保护这个宗派，并且将一些恩怨所了结而已。
反正不管如何，战乱的东玄域已是逐渐的平静，百废待兴，这片大地，终归会再度爆发出比以往更为兴盛的光彩。
而与外界的沸沸腾腾不同，已成为东玄域瞩目的道宗内，在经过最开始的狂欢后，倒反而是最快的平静下来，虽然整个宗派依旧是洋溢在那种轻松自豪的气氛中，但类似前些日的那种忘我狂欢却是逐渐的淡了下去，而一切的修炼以及新弟子的招收，都是在按照以往的惯例照旧而行，只不过那种规模，即便是压制又压制后，还是显得有点过度庞大罢了。
林动漫步走在道宗内，在周围时不时的有着道宗弟子匆匆而过，然后有着一些俏脸晕红的少女弟子眼带好奇与许些羞意的打量着他，远远的便是有着各种林动师兄之类的喊声传来，极为的热闹。
林动对于那些道宗弟子也是面带笑容，微微点头，他望着那些兴奋甚至崇拜的将他给望着的道宗弟子，素来都是紧绷成一根弦的心情，也是有着一种如释重负般的放松，那种感觉，一如三年前他刚刚进入道宗时。
这些年来，林动经历了太多的生生死死，有时候会感到疲累，但他却是明白自己不能停下来，他背负的东西不少，若是他停了下来，或许现在的道宗，已是化为一片废墟，那些笑容灿烂的道宗师弟师妹，或许也将不复存在，而他的家族，他的父母，也将会湮没在那血火之中。
有时候，为了守护，总归是需要付出一些东西的，不过所幸，至始至终，林动都未曾后悔过。
抱着极端放松的心态，林动走上一座修炼台上，然后在那丹河之旁盘坐下来，在他不远处，有着众多道宗弟子正在盘坐静修，吸收炼化着丹河之内的元气。
一种安宁祥和的气氛蔓延着，令得林动有种慵懒的感觉。
距元门覆灭，已是过了十日光景，林动的伤势也早已痊愈，这十日内，他则是彻底的放松下来，外界的任何事情，他都未曾再理会，什么动乱，什么善后，什么异魔，都先暂时的一边待着吧。
而应玄子等人似也是知道他的心境，这段时间诸多事情都未来打扰于他，于是便是任由他成天懒洋洋的混迹在道宗内，而这一幕，倒也是成为了道宗内颇为有趣的画面。
丹河旁，林动懒散的盘坐着，目光望着那奔涌流淌的丹河微微发呆，许久后，他似是有些累了，那双眼开始缓缓的闭上，然后黑暗笼罩视线…
黑暗笼罩，但林动的心境，却是在此时通明如灯，他能够清晰的感应到整个道宗，甚至道宗的每一个弟子，他感受着他们那种浓浓的兴奋以及自豪之感。
那种炽热的情绪，令得林动心中似乎也是有着一种滚烫散发出来，温暖如烈日。
那种感觉，突然觉得很舒服。
因为很舒服，所以林动的身体，再没有了任何的动静…
修炼台上，时间飞快，当那些静修的道宗弟子结束修炼时，却是见到林动的身影依旧如磐石般的纹丝不动，他们望着那道略带一丝慵懒的背影，眼神却是微微的有些恍惚，那种感觉，就如同此时的那道背影，突然的融入了这片天地一般，摸不着，也感应不到。
众多弟子茫然间，然后有着一些窃窃私语声传开，最后越来越多的弟子赶来，他们望着那道身影，眼中充满着惊异。
由于这边的动静越来越大，最后连应玄子他们都是被吸引了过来，他们望着那没有丝毫气息，甚至连体内元力，精神力都是停止流转的林动，面色显得格外的凝重，他们毕竟见识多广，虽然不明白林动为何会如此，但这显然似乎是处于了一种极为玄妙－的状态之中。
修炼一途，苦修为基，但却有着机缘之说，有时候的一朝顿悟，堪比十年苦修，而眼下的林动，似乎则是在这种莫名其妙－的情况下，晋入了一种奇妙－的状态，虽然他们不懂，但也知道，这对林动应该会很重要。
于是，他们开始将众多弟子驱散，将这片修炼台空了下来，以免将林动所干扰。
在道宗之内的一座山峰上，炎主也是望着远处那修炼台上的削瘦身影，那眼中，有着淡淡的神光闪烁，眼神深处，有着一抹惊讶之色悄然的掠过。


第1220章 大符宗
丹河之旁的身影，一坐便是一月有余，这一月时间中，他的身体犹如磐石般纹丝不动，甚至连其气息，都是彻底的消散而去，身体之上的温度，也是冰冰凉凉，犹如一具没有生气的尸体。
这一幕，莫说是寻常的道宗弟子，就连应玄子他们都略感纳闷，毕竟这种情况，他们也从未见过，不过虽然未曾见过，但他们终归还是有着一些眼力，清楚的明白眼下的林动定然打扰不得，因此也是严厉的制止寻常弟子进入那片修炼台。
而一月的时间，便是在不少道宗弟子疑惑以及担忧的目光中，悄然而过，但伴随着时间的流逝，众人也终是能够逐渐的感觉到一些奇异之处，一些感知敏锐者，能够隐约的感觉到，不知不觉间，在这道宗的上空，仿佛是有着一股极端浩瀚庞大的力量在悄然的凝聚着。
那种力量飘渺无形，但却给予人一种近乎实质般的威压，而唯有着一些精修精神力的强者方才能够清楚的感觉到那片力量的确切存在，然后心生骇然。
因为那股力量，竟然是一股浩瀚无尽的精神力。
那股庞大的精神力，即便是触及轮回的强者感应着，心中都是略微的有些发悸，那股力量，犹如一片无尽的大海，笼罩在道宗的天空上，令得无数人道宗弟子战战兢兢。
天空上弥漫荡漾的精神力，伴随着时间一天天的流逝，也是愈发的浓郁，偶尔间，天空上会有低沉的雷鸣声，那种雷鸣之中仿佛是蕴含着毁灭的波动，令人头皮发麻。
而应玄子等人则是会看着天空苦笑，然后将有些无奈的目光投向丹河之旁那道犹如磐石般盘坐的人影之上，那种连他们都是有些承受不住的力量，显然是与林动那玄妙－的状态有些关系，只是不知道这究竟要持续多久，毕竟天空上那些精神力，有点过于恐怖了些，一旦失去控制恐怕整个道宗都得顷刻间化为废墟。
不过担忧归担忧，此时的他们显然也是没有太多的办法，眼下之计，也只能继续的等下去了。
而等待一下，应玄子他们倒并不认为有什么，不过在这等待中，却是又是有着另外的事情发生那是出现在应欢欢的身上，这妮子在灭元门那一日时也是有过不小的消耗，在回来后也只休养了数日方才恢复，而在接下来的一月中，不知为何，她体内散发出来的寒气愈发的浓郁而所有人都是能够感觉到，伴随着她体内寒气的愈发浓郁，她的实力也是在疯狂的暴涨着，那种暴涨的速度，看得应玄子他们心惊不已。
这种暴涨，是在林动进入那种玄妙－状态之后方才发生的，足足持续了半月时间然后方才逐渐的减缓下来但那种寒气，常人已是无法忍受，因此这些时间应欢欢一直在后山冰湖闭关，那里的寒气就连应玄子都仅仅只能坚持十数分钟便是得离去。
应欢欢的这种变化，让得应玄子他们有些不安，应笑笑的不安感最浓，好几次要去冰湖守着应欢欢但最后都是被寒气冻得承受不了，最后才被应玄子强行的抓了出来。
这些突发的事情让得应玄子也是颇感头疼，不过他毕竟是道宗掌教，性子沉稳，所以倒也没过多的慌乱，只是将冰湖封锁，禁止寻常弟子靠近，然后接下来他也是没了办法，只能先等着林动苏醒过来再看看了。
而这样的等待，一直持续到一个半月左右后，终于是有了一些动静。
轰隆。
道宗的弟子依旧是在进行着每日必修的静修课程，不过今日显然是有些不同，自从清晨开始，天色便是显得有些压抑，低沉的轰隆雷鸣声，比往日要急促频繁许多。
而当时间行至晌午时，天色已是近乎暗沉下来，然后众人便是天空黑云滚滚而来，在那云层之中，无数道雷霆犹如雷龙般蜿蜒盘踞，蠕动之间，一种惊人的天地之威散发开来。
轰！
雷霆闪烁，整个道宗都是被那雷光照耀得犹如白昼，整片天地间的元力都是在此时沸腾了起来。
所有的道宗强者皆是面色凝重的望着天空上的暴动，眼角微微抽搐，这种程度的雷暴，就算是一名转轮境的超级强者，恐怕都会被顷刻间轰成灰烬。
无数道宗弟子心惊胆寒的望着天空，然后赶忙撤到能躲避的地方，脸庞上满是骇然，在这种恐怖的天威下，人的力量，仿佛显得分外的弱小。
轰隆隆！
雷鸣之声，越来越急促，璀璨的雷光，仿佛连黑云都是要掩盖不住，那种犹如百万大军即将冲锋之前的阵势，连天地都是有些颤抖。
雷霆的凝聚，最终还是达到了顶峰，然而雷鸣声在那一霎，却是诡异的安静了下来。
不过，这般安静，却仅仅持续了一瞬，然后所有人都是瞳孔骤缩的见到，天空上的雷云被蛮横的撕裂开来，惊天般的雷鸣轰然响彻，那璀璨的雷光，遮掩了世间所有的光芒。
无数道万丈庞大的雷霆，犹如张牙舞爪的雷龙，自天际之上的雷云中冲出，然后化为一片雷浆瀑布，从天而降，直奔道宗而来。
这一幕，骇得无数道宗弟子面色煞白。
雷龙铺天盖地的席卷而下，不过短短数息的时间，已至道宗千丈上空，而也就在此时，有着道宗弟子借助着雷光，见到那在丹河之前静坐了将近两月时间的身影，竟是在此时缓缓的站起身来。
那道身影仰头望着那席卷而来的雷霆，削瘦的身形在雷霆下显得分外的渺小，但当道宗的弟子见到那道身影时，心中的惶恐，却是瞬间烟消云散。
他们始终的相信着，只要有这道身影存在的一天，他们道宗，就将会永远的屹立，因为这是他们所坚信的一种信念。
狂热而毫无理智，但他们就是为此坚信不疑。
无数道雷龙开始汇聚，呼啸之间，犹如滔滔雷海，奔涌而至，犹如携带着灭世之力。
林动仰头，漆黑眸子之中，倒映着雷海，旋即他身形一动，竟直接是出现在天空上，然后任由那恐怖的雷海，将其席卷淹没。
一道道惊呼声，也是在此时自道宗之内爆发而起，不过很快的便是落下来，因为他们见到，那降临而来雷海，突然在距道宗还有千丈距离时，陡然的停下，这般距离，他们已是能够清晰的感觉到其中所蕴含的毁灭性力量。
“快看！”
突然有着惊声传出，无数道目光射去，只见得那雷海中央，突然涌荡起波浪，而后雷光缓缓的堆积而起，片刻后，竟是化为一道千丈庞大的雷霆王座，而在那王座之上，一道削瘦身影，手持权杖，安然而坐。
此时的那道身影，就犹如雷之帝王一般，恐怖的雷霆，尽数受他驱使。
雷霆王座上，那道身影手中权杖轻轻一挥，只见得那犹如要灭世般的雷海便是席卷而下，然后在那无数道震撼的目光中，逐渐的缩小，最后化为一道数千丈庞大的雷霆河流，在道宗上空，缓缓的流淌着。
众人见到那恐怖雷海在林动手中却是乖巧如绵羊，那眼睛都忍不住的睁大了起来，脸庞上满是不可思议之色。
解决掉那恐怖的雷海，那道身影也是缓缓的自雷霆王座上站起，然后他轻轻的摊开双手。
轰。
随着他双臂的摊开，那涌荡在道宗上空将近两月时间的浩瀚精神力，突然犹如是在此时受到了召唤一般，竟是铺天盖地的疯狂涌下，最后尽数的对着林动体内涌去。
而面对着如此恐怖的精神力涌入，林动的身体却是犹如无底洞一般，竟是没有丝毫的不适，那脸庞上反而是有着一抹淡淡的沉醉之色浮现出来。
短短数分钟的时间，天空上那令得道宗无数人心惊胆寒了近两月的浩瀚精神力，便是尽数的被林动吸入体内。
而当最后一股精神力犹如林动体内时，那暗沉的天空，有着温暖的阳光倾泻下来，最后将整个道宗笼罩在了其中。
无数道目光望向天空，那里，一道身影静静悬空而立，他就站在那里，但不论道宗诸多强者如何感应，都是无法的将其察觉，那种感觉，就犹如当日那位四王殿出现时的一般，人明明就在那里，但却根本无法察觉。
“好强大的精神力…没想到林动的精神力，也是达到了这种恐怖的境界。”应玄子望着天空上的身影，许久后，一声轻叹，道。
“若是我所料不错的话，林动的精神力，恐怕已是晋入了那传闻之中的大符宗之境，这家伙…真是一个奇才啊。”瞎眼长老面露感叹之色，他本身便是专修精神力，自然是明白林动的这一步，究竟是有着多么的恐怖。
一名大符宗，足以与一名轮回境的强者相抗衡！
“那条雷河之中拥有着极端纯粹的雷霆之力，对于淬炼元力以及肉身有着极为强大的作用，林动此举，倒是造福了我们道宗啊。”
尘真望着那璀璨的雷河，忍不住的笑道。
应玄子也是笑着点了点头，旋即眼中忧色涌起：“林动终于出关了，不然欢欢那边，真是不知道怎么办才好了。”
“欢欢怎么了？”
应玄子的声音刚刚落下，只见得他面前身影便是鬼魅般的缓缓浮现出来，林动望着前者，眉头微皱，开口问道。


第1221章 怒斗炎主
道宗后山，天空上光影掠过，而后林动与应玄子的身形便是出现在了一座散发着惊人寒气的山峰上，目光透过重重密林，能够见到那里有着一片蔚蓝色的清澈冰湖。
“唉，你去看看吧，如今她体内的寒气越来越恐怖，连我都无法接近了。”应玄子望着远处的冰湖，眼中掠过一抹痛色，轻叹道。
林动微微点头，也不再多说，身形一动，便是出现在那冰湖边缘，目光望去，只见得那冰湖中心的位置，有着一朵冰莲凝聚，而在那冰莲中，一道倩影静静的盘坐着，冰蓝色长发倾泻下来，铺满了那座冰莲。
此时在那道曼妙－倩影身上，正有着浓浓的寒气散发出来，那种寒气之浓烈，甚至连周围的空气都是有着凝结成冰的迹象，极端的霸道。
林动眉头微皱，步伐踏出，身形已鬼魅般的出现在冰莲之前，其身体上有着淡淡黑光涌动，将那侵蚀而来的霸道寒气吞噬化解而去。
冰莲上的应欢欢此时似是陷入沉睡之中，美目紧闭，那张俏美的脸颊此时泛着细微的苍白，娇躯偶尔轻颤，透着一股令人心疼的柔弱。
那种恐怖的寒气，则是不断的从她体内涌出来，林动眼尖，分明的见到随着那些寒气的涌出，应欢欢那本就呈现冰蓝色的长发，颜色愈发的浓郁了一些。
应欢欢娇躯突然剧烈的颤抖了一下，贝齿无意识的轻咬着嘴唇，柳眉紧锁，仿佛是在抵抗着什么，那眉宇间有着一些惶悸之色。
林动见状，心中微疼，面色阴沉，在心中道：“岩，这是怎么回事？之前她还好好的。”
“冰主的力量在急速的苏醒。”岩沉默了一下，道。
“为什么？”林动瞳孔微缩，沉声问道，任何事情都有着起因，应欢欢一直在压制着体内的力量，若没有特殊情况的话，怎么会突然间产生这么大的变化？
岩闻言却是一阵犹豫。
“为什么？！”林动再度问道，声音中多了一丝怒意。
“可能与炎主有关。”岩苦笑了一声，道。
“炎主？那家伙对欢欢做了什么手脚？”林动面色一变，眼中有着怒火涌动，这家伙，果然跟来就没什么好事情！
“这倒或许不是他故意做什么手脚，他是炎主，他拥有着远古的气息，只要他接近了应欢欢，那股气息便是会形成一些引子，然后勾动应欢欢体内沉睡的冰主的力量。”岩道。
“那你为什么不早说？！”林动怒道。
“我以为这应该会来得挺晚的，没想到这么快就出事了。”岩苦笑道。
“那炎主应该也知道他接近欢欢，他的气息就会引动她体内冰主力量苏醒的吧？”林动面色极为的难看，缓缓的道。
“这…他应该是知道的。”岩点了点头，道。
林动眼中的怒火缓缓的收敛，那张面庞再度变得平静下来，不过熟悉他的岩却是知道，这家伙越是平静，那说明心中的怒火便越浓。
林动仲出手掌，轻轻抚着应欢欢那透着刺骨寒意的冰蓝色长发，然后猛的转身，面色阴沉的掠出，下一刻，他直接是出现在道宗天空上，目光一扫，便是锁定了不远处的一座山峰，阴沉的怒声，响彻了天空：“炎主，你给我滚出来！”
这突如其来的怒声，顿时让得道宗内所有人一惊，然后他们抬头，有些疑惑的望着天空上面色阴沉的林动，这还是他们第一次见到林动发这么大的火。
应玄子等人也是迅速的涌出来，面面相觑着，显然不知道这是发生了什么事。
远处的山峰上，火光一闪，而后一道身影便是出现在了天空上林动的对面，而后他眉头微微皱了皱，道：“做什么？”
“你做的好事，你还来问我？！”林动见到他这幅漫不经心的模样，漆黑眸子中怒火更盛，冷笑道。
炎主淡淡的看了一眼那弥漫着寒气的道宗后山，那眼中也是掠过一抹莫名的情绪，而后他道：“这些事情，终归是无可避免的，你这般作法，也仅仅只是自欺欺人罢了。”
“你管的闲事未免也太多了点。”林动冷声道：“这道宗不欢迎你，请吧！”
“在她未曾苏醒前，我都不会走的。”炎主摇摇头，淡淡的道。
“苏醒？你可知道，一旦她苏醒，她还会是她吗？我想要的是应欢欢，不是什么冰主！”林动厉声道。
“你这般想法太过自私。”
炎主皱了皱眉，道：“你可知道她肩上背负着多大的责任？这片天地，异魔潜伏，他们的强大你不是没见过，若是天地大战再启，生灵涂炭，还得依靠她来拯救，到时候她成了应欢欢，这世间万物却是得尽数沦陷在异魔手中，而那时候，你们难道就能独善其身？”
“那么重的担子，她扛不起，如果真要扛的话，我便替她扛！”林动双拳紧握，声音低沉的道。
“你？”
炎主看了他一眼，眼中有着一抹淡淡的轻视，道：“凭什么？凭你获得了吞噬之主的传承？这担子就算是他复活亲自来扛都扛不起，你又有什么资格说这种话？”
“若这种担子有这么容易来扛？还需要你来？！”
“有没有资格，同样也不是你能来评判的。”林动缓缓摇头，未曾再言，只是眼中的那抹执拗，显然是无可动摇。
“真是个自大的家伙，既然你有这个口气，那我倒是要试试，你究竟有没这个资”炎主见状，心中也是升腾起一抹火气冷声道。
“轰！”
他话音一落，只见得其身后天空都是在此时变得赤红起来，而后其脚掌一跺，犹如火焰般的赤红元力猛的席卷而出，直接是化为一只火焰巨掌狠狠的对着林动怒拍而去。
林动见到炎主动手，眼中也是寒芒一闪，也不见得他有丝毫动作，头顶雷云闪电般的汇聚而来，一道千丈雷霆已是犹如雷龙般呼啸而下，狠狠的与那火焰巨掌硬憾在一起。
砰！
惊人的气浪扩散开来，空间都是在两人交手之间变得有些扭曲起来。
林动身形微震步伐却是丝毫未退，那黑眸之中，有着凌厉之色涌动炎主实力的确恐怖，但如今他精神力也是晋入大符宗之境，再加上那触及轮回的元力修为，真要加起来，炎主想要凭借着这具分身就将他压制，显然也是不可能的事。
“你若是不走那我便将你这具分身打散！”
林动显然同样也是被炎主暗中的这些手段激出了怒火，只见得其袖袍一挥，一道光芒呼啸而出，然后在天空急速的膨胀，最后化为一道庞大无比的古老阵法。
“乾坤古阵？”炎主望着那古老阵法，眼神也是微凝显然是对此丝毫不陌生。
“轰！”
浩瀚的元力以及精神力猛的自林动体内喷薄而出，最后化为两道光柱冲进乾坤古阵之中，而后阵法运转，两股力量便是逐渐的融合起来，一股极端惊人的波动，缓缓的荡漾而开。
那股融合的波动之强，就连炎主眼神都是微微一凝。
“倒是有些能耐。”
炎主双手轻合，那双目之中，赤红之色迅速的涌上而后这片天地迅速的变得炽热起来，一轮烈日，缓缓的自其身后冉冉升腾而起。
应玄子等众多道宗之人望着这往日还井水不犯河水的两人竟然直接是开打起来，也是大惊失色，这两人若是在这里动起手来，岂不是要直接将道宗给拆了？
天空上的两人，皆是冷目相对，下一刻，浩瀚之力涌动，就要动起手来。
“住手！”
不过就在动手前的一霎，一道急喝声猛的响彻而起，而后天地间寒气用来，一道倩影便是出现在了林动身前，她张开双臂将后者挡在身后，美目泛着怒意的盯着炎主，冷声道：“你要干什么？”
炎主望着那将林动护在身后的应欢欢，微微一怔，拳头握了握，然后又是松开，偏过头去，淡淡的道：“我说过，在你苏醒前，都不会离开。”
“你！”林动一咬牙。
应欢欢偏过头，冰凉的玉手拉着林动的手臂，轻声道：“先冷静下来，好吗？”
她的声音依旧是那般寒气缭绕，只不过却是有着一丝恳求。
林动看着她，最终叹了一口气，天空上的光阵逐渐的散去，最后连带着元力以及精神力，再度掠回了他的体内。
应欢欢见到林动停下来，颊上也是浮现一抹微笑，对于林动性子，她再熟悉不过，后者素来沉稳坚毅，遇见任何事情都是能够最冷静的对待，但眼下他却是因为愤怒失去了以往的冷静，这在林动身上一般是极为罕见的事，而这代表着什么，应欢欢心中自然也是清楚。
“谢谢。”她轻声道。
“说这些干什么。”林动摇了摇头，他看了看应欢欢，道：“你体内的寒气压制下来了？”
“再不压制下来，你们都要把道宗给拆了。”
应欢欢无奈的道。
“你还真当我不知道分寸啊。”林动笑了笑，道。
“这事情就交给我来吧？”应欢欢看着林动，征询着他的意见。
林动微微犹豫，然后点点头，应欢欢见状，也是轻轻一笑，转过身来，冰彻的美目看着炎主，沉默了一下，道：“炎主，我知道你在做什么，但不管如何，现在的我并不是你所认识的那个冰主，而且不管以后会怎么样，我希望你明白，现在的我，是应欢欢。”
“若是你要继续留在这里，请你能够收敛气息。”
炎主望着应欢欢那有些严厉的目光，旋即也是长长的吐了一口气，道：“你…你应该知道确切情况的。”
“雷主已经苏醒，黑暗之主，洪荒之主他们也会陆陆续续的再度出现，到时候…”
应欢欢美目的微垂，道：“但至少不是现在，不是吗？”
炎主沉默，许久后他似是笑了一下，目光停留在应欢欢那俏美的脸颊上，道：“真没想到，素来冷静得近乎异类的你，竟然也有一天会这样的留恋一个人。”
他虽然笑着，但那声音中，依旧是有着一抹极深的涩意。
“因为我是应欢欢。”应欢欢轻声道。
“或许吧。”
炎主摇摇头，旋即转过身去，挥了挥手：“我会照你所说，尽可能的收敛气息。”话音落下，他的身形便是消失在天空上，对着远处的一座山峰掠去。
林动望着那远去的炎主，默然无语，手掌缓缓的紧握，说到底，终归还是他的实力不够，若是足够的话，即便帮她挑下了那重担，让得她的笑容始终如同当年那般欢乐清澈，那又有谁能出言反对？


第1222章 大雪初晴
一场莫名的闹剧，便是这般的收场，道宗的弟子面面相觑着，然后看了看天空上的人影，这才逐渐的散去，虽然他们并不太清楚事情的始末，但想来能够让得素来冷静的林动师兄这般发怒，那炎主应该也是做了一些有些过分的事吧他们的思想倒是很简单，在这里，不管如何，他们都是坚定不移的支持着林动。
“你身上寒气依旧未能褪去，这些时间便好好的静修吧，尽量别怎么与人交手。”天空上，林动望着体内依旧还有着寒气散发出来的应欢欢，道。
应欢欢微微点头，旋即她轻声道：“不过你可要答应我，别再跟炎主发怒了，现在虽然他只是分身，但日后真身必然会来。”
她也是担心着林动，炎主的实力她很清楚，若真要对恃起来，林动必然会吃亏的，而且，炎主所代表的，还有着那一群在远古都是最为恐怖的家伙。
“这次我也鲁莽了点。”林动叹了一口气，这次的事他知道莽撞了一些，但当他看见应欢欢体内冰主的力量被炎主故意不声不响的引动，可却未曾给予他们丝毫的提醒，这番暗中的手段，才是林动动怒的真正原因，他信任着炎主，方才让他同随，但后者这种背着他的举动，却是真正的犯了他一些忌讳。
他知道炎主他们的心愿很宏大，他们是想要守护着这天地，而林动也尊敬他们，他们对这片天地的付出，无人能及，从某一种角度而言，他们对应欢欢的所作所为或许也并没有错，在他们看来，只有冰主的彻底苏醒，方才能够拯救这片天地。
只是林动却是不想应欢欢因为这种原因而变成一个陌生冰冷的人，但他同样也知道那是冰主的责任，应欢欢与冰主难分彼此，这种责任，她无法选择，但也无法避免。
林动也并非是自私的要应欢欢否认这份责任，他只是希望，能够有着最为完美的办法，若是他拥有着接过这份责任的能力，那么，应欢欢就不用去改变，而这片天地也是能够获得拯救。
他知道这个办法或许有着一丝天真，可他却是甘愿为之付出，他经历着重重生死，遍体鳞伤的一步步的对着那个方向靠近着，他希望能够当改变来临时，获得认可。
在这之前，他想要尽可能的保护着他想要保护的那道音容笑貌，他终归到底，还是一个有血有肉的普通人。
应欢欢望着面色复杂的林动，也是一阵默然，旋即冰蓝色的美目望着那熙熙攘攘极为热闹安宁的道宗，神色略微的有些恍惚，而后她纤细玉指指向一处广场，笑道：“还记得吗？三年前的殿试，在那里我们可是成为了一次对手呢。”
“嗯，然后你被我狠狠的抽了一顿。”林动一笑，道。
应欢欢脸颊上有着一抹淡淡的绯红浮现出来，嗔道：“早知道我那时候就该直接煽动天殿的师兄弟把你揍一顿。”
“那我们荒殿那么多师兄弟也不会怕的。”林动忍不住的笑道，突然间心情好了许多，那一幕幕的记忆，实在是让人心暖。
“好了，你先去静养着吧，你这模样还不适合出来。”心情好下来，林动冲着应欢欢挥了挥手，道。
“嗯。”
应欢欢也是明白自身的状态，这些寒气若是不压制的话，对寻常道宗弟子会造成不小的伤害，当即她冲着林动一笑，挥挥小手：“不要再发火了哦。”
此时的她这般模样，竟是再度恢复了一点三年之前的那份娇俏，这看得林动心头微暖，然后点点头，目送着应欢欢化为一道虹光掠回后山。
林动望着应欢欢消失的倩影，脸庞上的笑容方才一点点的散去，双掌紧紧的握着，眼神变幻，许久后，他长叹了一口气。
这一次他能够将炎主给阻拦下来，但下一次呢？那时候若是真如同炎主所说，他真身降临，然后那些所谓的洪荒之主，黑暗之主都是来了，他难道还能护住应欢欢？
那时候说他想要替应欢欢扛过那重担，这些远古之主，又有谁会相信？他现在的实力已算不弱，可与炎主这些远古的巅峰强者比起来，依旧还有着差距。
“岩……你说，我真能超越那冰主么？”林动在心中略微有些茫然的问道。
“你现在的成就，三年之前又能想到吗？”岩淡淡一笑，道。
“但我却是没把握在那些远古之主尽数来到时，将她给保下来，也没办法让他们相信我够资格担负起属于她的重担。”
岩沉默了一下，冰主的优秀，即便是他的主人符祖都赞不绝口，想要与其比肩，本就是一件极不容易的事。
“你要让他们对你认同，其实也不是没有办法。”岩沉默了半晌，突然说道。
“你有办法？”林动一怔，连忙问道。
“我有办法，但成不成却不知道。”岩顿了一顿，接着道：“而且这还得看另外一个人的意愿……”
“谁？”
“绫清竹。”
“绫清竹？”林动愣了愣，有些茫然的道：“这与她有什么关系？”
“具体情况我不太清楚，但是我在她的身上，感觉到了原本在主人方才具备的波动。”岩的声音中，充满着凝重。
“符祖？”林动瞳孔微缩，心中满是不解，绫清竹怎么又会与符祖扯上关系？
“她怎么会拥有着我主人方才具备的波动，我也不明白，不过你可以去问问。”岩道。
林动皱了皱眉，却是摇摇头，他究其原因是因为应欢欢，但因这个原因去找绫清竹寻求帮助，对后者不公平，而他也实在开不了口。
“算了，我自己想想办法吧。”林动叹了一口气，道。“随你吧，不过她这情况就算是我也是第一次遇见，若是放过了，着实有些可嗬”岩自言自语道。
林动摇着头不去理会他，心中也是有些烦躁，没想到这才安静了没多久，各种事情又是缠绕了上来，若能够一直如前些时候那般安静，该有多好。
心中烦恼着，林动也是自天空落下，刚欲走开，一道声音却是从不远处传来，他一抬头，便是见到应笑笑对着他快步而来。
“笑笑师姐。”林动冲着应笑笑笑了一下，道。
“你这称呼现在我可当不起。”应笑笑白了林动一眼，以这家伙如今的实力，就算是要当掌教都是绰绰有余。
“对了，我来是要告诉你，你的那些朋友在你闭关的时候，都先行回妖域了，不过临走时我们设置了空间阵法，下次你若是要去妖域，数日时间便能抵达。”
“小貂他们回妖域了？”林动一怔，旋即点点头，如今东玄域诸事平静，他们是该回妖域，而且那边还有着四象宫需要照料，以小炎的性格，显然也是妖域才更为适合他。
“另外，”应笑笑突然顿了一下，道：“先前绫清竹姑娘带着九天太清宫最后一批人也是离开了道宗，应该是回九天太清宫去了。”
“她走了吗？”林动呆了呆，旋即眼神复杂，心中不知道是个什么滋味。
“这些时间，九天太清宫的弟子在陆陆续续的撤回去，不过按照速度，一月之前她们就该尽数撤走的，但后来不知为何速度减缓了下来，清竹姑娘也是在道宗逗留了一月，直到先前才带人离去。”绫清竹眼神莫名的看了林动一眼，道。
林动苦笑了一声，他自然是听得出应笑笑话里的意思，这一个多月，不正是他陷入那种古怪状态之中么？绫清竹之所以会逗留下来，恐怕是在担心着他吧，而现在他顺利出关了，她也是悄然而去。
不过她这性子倒还真是丝毫未变，而且也是与应欢欢截然不同，后者爱恨不加丝毫掩饰，在异魔城为了护住他，却是宁愿以命来逼应玄子现身，看似任性，但却是至情，而绫清竹则是任何事情都不显露出来，任何感受也是自斟自饮，从不与外人提起，只是唯有亲近者方才能够感觉到那清冷内心之中所包裹的火热。
“我说欢欢和清竹姑娘的事，你究竟想要怎么样？”应笑笑盯着林动，问道。
“我……”
林动动了动嘴巴，却是一句话都说不出来，面色复杂，哪还有平日里的那番果断，而这种事，又哪那么容易说清楚。
应笑笑叹了一声，也不好多说什么，道：“你去送送她吧，她们应该还没走多远。”
林动点点头，刚欲转身而去，应笑笑又是想起了什么，道：“等等，这一两月中，你爹娘也有消息传来，说让你去办件事情。”
“办事？”林动一愣，他爹娘会让他去办什么事？
“嗯，你爹娘说，让你若是得空，就去北玄域把青檀接回来。”
“青檀……”
听到这个名字，林动的心头也是忍不住的一颤，脸庞上有着一抹异常柔和之色浮现出来，那个倔强得令人心疼的小丫头，也不知道如今怎么样了。
想到三年前那手持着黑色镰刀，身着黑色衣裙，巧笑焉熙的少女，林动心头微热，三年不见，真是想这丫头想得紧。
“嗯，我知道了，我会尽快把她接回来。”
林动点点头，青檀在家里可是个小宝贝，爹娘与他都是疼爱得厉害，这些年跑出来受了这么多苦，也不知道娘为此抹了多少泪，既然如今他回来，也是该找时间去躺北玄域了。
声音落下，林动也就不再停留，对着应笑笑挥了挥手，身形一动，便是掠向道宗之外，应笑笑望着他的身影，无奈的叹了一声。
道宗之外，山林之中，数十名九天太清宫的弟子安静行过，在那最前方，是一名身着白色衣裙，娇躯窈窕出众的女子，她脸颊上虽有薄纱遮面，但那印出来的轮廓，却是完美动人。
苏柔走在她后面，大眼睛却是时不时的看向后方越来越模糊的道宗，最后忍不住的低声道：“师姐，我们就这样走了啊？”
“嗯。”
绫清作薄纱下有着清淡的声音传出，没太多的波动。
“我们至少至少也要和林动大哥打声招呼再走啊。”苏柔建议道。
绫清竹眸子看了她一眼，似是笑了一下，声音略柔的道：“何必在意这些。”
“可可师姐你逗留在道宗这些时间，不就想看看他是否安全完成修炼么，……苏柔咬了咬嘴唇，道。”
绫清竹步伐微顿，手中剑鞘轻轻的敲了敲苏柔小脑袋，道：“再瞎说，回去就关你禁闭。”
苏柔小脑袋一缩，不敢再说话。
“她说的倒是没错，我把你们带来道宗，要走前，起码也要跟我打声招呼吧？”一道笑声，突然的从前方传来，让得绫清竹那握着青锋长剑的玉手微微一滞，然后她缓缓的抬起头，只见得在那前方树林的尽头，一道削瘦的身影，正背靠着一颗大树，面带笑容的将她给看着。
细碎的光斑透过树林，照耀在他的身上，仿佛温暖的阳光，让得人心中犹如大雪初晴。


第1223章 相谈
“林动大哥！”
苏柔那惊喜的声音，也是在此时打破了山林中的平静，少女那秀美的脸颊上，布满着欣喜之色。
“林动师兄。”
那些九天太清宫的弟子也是连忙抱拳行礼，神色中颇为的尊敬，林动于他们九天太清宫有着恩情，而且他们能够报得大仇，那也全倚仗着林动。
林动冲着她们笑了笑，然后将目光看向那身着白色衣裙的清冷女子，后者见到他看来，视线则是微微的移开，那握着剑鞘的玉手轻轻用力，白皙的肌肤上，有着细小的青色血管浮现出来。
苏柔看着两人，却是嘻嘻一笑，道：“林动大哥你有话要与师姐说？那我们先去前面等着吧。”
说完，苏柔悄悄招手，带着那些九天太清宫的弟子跃过绫清竹而去。
绫清竹见到苏柔她们要走，那素来清澈的眸子中终于是掠过一丝细微的慌色，急忙要伸手去抓苏柔，却是被后者调皮的躲了开去，然后她冲着绫清竹扬了扬手：“师姐，不急，我们还有的是时间，你们慢慢聊。”
声音落下，少女已是咯咯娇笑着，带着九天太清宫的弟子行出山林，而后嬉笑声逐渐的远去。
随着她们的远去，这林中的气氛也是再度的变得安静下来，绫清竹立在原地，窈窕娇躯有着极为动人的曲线，衣裙如雪，青丝轻束，气质脱俗。
虽然在刚开始她稍微的慌乱了一下，不过她心境毕竟相当的强大，很快的便是变得平静下来，神色古井无波，只不过那比起平常稍稍快了一点的呼吸，似乎还是暴露了一些她此时的心境。
“这么不辞而别，终归不太好吧？”安静中，林动缓步走了上来，他望着眼前的女子，微笑道。
“我们已经打扰多日，道宗内事务繁杂，安静的走，也能为你们减少一些麻烦。”绫清竹道。
“真是这样吗？”林动站在绫清竹面前，目光盯着那张即便是有着薄纱遮掩的绝色容颜，眼神深处泛起一抹柔色，突然叹了一口气，道：“我们认识…似乎有八年了吧？”
两人在那小小的大炎王朝中相识，那时候的她，是高高在上的天之娇子，而他，却还仅仅只是一个在跌跌撞撞中摸爬滚打的小家族稚弱少年。
那时，他望着那张清淡的绝美容颜，唯有仰望着，那是他第一次见到那么漂亮的女孩，他的心自然也是如同常人般有所跳动，只不过他知道两者间的差距，不得不承认，即便是林动，在那时候遇见绫清竹时，心中都是有着一丝自惭形秽的感觉。
然而时过境迁，八年之后，曾经的少年，却是屹立在这片大陆最巅峰的地方，这段路程，他为之付出多少，或许也唯有他心中明白，只不过，他从未后悔。
当绫清竹听得林动这句话，那清澈的眸子中，也是忍不住的泛起了一些波动，旋即她缓缓抬头，美眸凝视着那张年轻的脸庞，八年前，这张脸庞还带着一些稚嫩，不过在面对着她那停留在其咽喉处的剑锋时，他的目光，依旧是那般的灼热与执着。
“八年时间，我们的身份倒是互相变了变，恭喜你，你超越我了。”
绫清竹红唇微抿，旋即她轻轻自嘲一笑，道：“师傅与我说过，我性子傲，犹如高山之莲，但这对男人而言却是犹如催情之药，因为男人最喜欢的事，便是征服他们无法触及的高山，而你，从一开始，就在想着如何征服我吧？”
“不过你的确很强，以你现在的能力，要征服我也是很容易的吧。”
绫清竹盯着林动，眼眶微红，神色自嘲，她心中的情绪，林动又如何能懂，她知道自己给林动的第一印象是什么，那时候的后者眼中的灼热与执着让得她明白，她成为了他目标，犹如幼狼仰望着高崖上的向阳花，于是他开始劈荆斩刺，奋力向前。
一别数年，再次相见，少年已是今非昔比，虽说并非让她震惊的程度，但因为某种关系的使然，她开始关注起那道身影，之后异魔域的相遇，在那太清仙池内，终归还是让得她平静的心境荡起了一些涟漪。
异魔城的惊天惨战，那道身影的浴血奋战，让得她心神微颤，不过素来心境便是非凡的她却是生生将其压制住，最终她当了一回让得她心跳最快的看客，她并没有给予任何的援手，非是不想，而是她相信，他不会就这样的失败，她相信，当他再度出现在东玄域时，必是王者归来。
而一切，也如同她所相信的那样的成为现实，只是，当他真正耀眼无比的出现在她的面前时，她却是略感陌生，当年的幼狼，终归是来到了高崖上的向阳花前。
只是，那却似乎只是一种征服。
或许他从来都不认为，当年那个在其眼中高高在上犹如谪仙般的女孩，却是会在不知不觉间，以一种无人察觉的方式注意着曾经卑微的他。
即便是她的师傅百般逼问，但她却始终未曾说出过他的名字。
即便是她知道师出无名，却还是莫名其妙－的在他离开后，去那个小小的大炎王朝，以一种从未有过的柔软姿态，拜见着他的父母。
即便明知不缺少她的注意，但她依旧默默的留在道宗，直到他渡过难关，而后她方才默默离去。
她知道应欢欢，在那异魔城时，她便是见到了这个女孩为他所做的一切，她敢爱敢恨，那般心性有时会让得她有些生羡，但她终归不是她，她如莲花般内敛，将所有的一切，都是掩藏在那清冷的内心深处，无人触及。
或许，正因为如此，他对自己，方才始终都是抱着那种试图征服的心态，而非是她想要的那种纯粹情感。
林动怔怔的望着眼前这眼眶微红的绫清竹，这时候的她，有着他从未见过的柔弱，那种犹如外壳般的清冷淡然，在此时仿佛是尽数的消除，他能够感觉到后者心中突然涌起的波动。
林动沉默着，半晌后，方才缓缓的伸出手来，想要将她脸颊上的泪水搽去，但却是被她轻轻的避开，而后她自己搽去，那神色再度变得淡然下来，那一幕，仿佛先前仅仅只是错觉一般。
“倒不是想着什么征服，以前我想得挺简单的，就是想让你刮目相看，我只是想证明一下，你曾经对我的否认是错误的。”林动沉默了一会，而后轻叹了一声，声音柔和。
绫清竹微偏着头，没有看他。
“那时候，你也知道我们之间的差距，那时更发生了那种事，其实我也明白，那恐怕就是懒蛤蟆吃天鹅肉，只不过我这癞蛤蟆，稍微的有些野心，因为我想，等有一天能够再站在你面前的时候，我可以正视着你，而不用再如当年那般去仰望着。”
“我…那时候，只想平等的站在你的面前。”林动脸声音轻缓的道。
听得林动这句话，绫清竹的娇躯也是微微的颤了一下，然后她缓缓的转过头，望着林动，此时后者脸庞上掀起了一道笑容，只是那笑容中，透着丝丝疲色，为了那个简单的心愿，他同样走了很多年。
绫清竹玉手轻轻握拢。
“说这些，只是想告诉你，我并非是你所想的那样，将你彻底征服，然后去享受这种病态的快感，我只是想…和你平等，你知道吗？不然的话，我也不会在听见九天太清宫出事后，第一时间便是赶了过去。”林动声音沙哑的道，很久以前，他的确很想超越她，但在那之中，就奠没有掺杂其他的情感么？这一点，他心中最为的清楚。
绫清竹贝齿轻轻咬着红唇，半晌后，她螓首轻点，有着一道细微的嗯声从那薄纱下传出。
接下来，两人则又是陷入了一些沉默，不过这种沉默却是没了刚开始的压抑，绫清竹亭亭玉立，低头轻轻看着手中的青锋长剑，虽然气质依旧是那般的清冷，只不过比起先前，仿佛是多了一些灵气。
远处，突然有着一些嬉笑声传来，绫清竹这才一惊，抬头看了林动一眼，道：“我要回九天太清宫了。”
“哦。”
林动愣了愣，道：“九天太清宫重建好了？没什么问题吧？”
“嗯，虽然师傅坐化了，不过宫内诸多长老只是受重伤，如今也大多恢复过来，只是我现在是新一任宫主。”绫清竹点点头，道。
林动心中轻叹了一声，这些时间，九天太清宫也是经历大变，宫门被毁，宫主坐化，这一切的责任都是落在了绫清竹身上，但她却从未表露过什么，而是默默的将其肩负着，那种一贯的坚强，有时候也让人心疼。
“你将这玉石拿着，若是遇见问题就捏碎，我立即赶来。”林动递过一块黑色玉石，道。
绫清竹见状，微微犹豫了一下，这才接过，然后轻握在手心，淡淡的温度散发开来。
“那…那我先走了。”绫清竹看了林动一眼，道。
“嗯。”
林动笑着点了点头，绫清竹这才迈步走出，不过没走两步，突然有着一道声音突兀的从后面响起：“等等，他还想问你为什么你身上会有符祖的波动？”
突如其来的声音，让得绫清竹愣了一下，然后她转过身来，却是见到一道光影从林动的体内飘了出来，然后林动那脸庞，便是彻彻底底黑了下来。


第1224章 太上感应诀
林动面色发黑的望着那自作主张窜出来的岩，一把便是对着后者抓了过去，咬牙切齿道：“岩，你这个混蛋！”
岩躲开林动的手掌，也是倍感无奈的道：“你这小子倔得跟石头一样，我不说你就永远不会开口。”
“你管得未免太宽了！”林动恨得牙紧，连连的对着岩抓去，不过就在他要抓住岩时，一道带鞘长剑突然横在了他的面前，将其阻拦了下来。
绫清竹瞥了林动一眼，然后看向岩，柳眉微蹙，道：“你是谁？”
“我是祖石之灵，一直都在这家伙体内。”岩目光停留在绫清竹身上，目光直直的，倒不是因为绫清竹的容颜，而是因为她身上那种唯有他才能够感觉到的一丝极淡但却无法忘记的波动。
“喂，你看哪里呢？”林动上前，将这家伙的目光给挡了下来，面色不善。
“再美的女人在我眼中都是红粉骷髅，你瞎吃什么飞醋？”岩白了林动一眼，分外的不客气。
林动老脸也是一红，怒目而视。
绫清竹也是略感好笑的看了林动一眼，而后对岩问道：“你为什么会知道我具有符祖的波动？”
“符祖就是我的第一任主人，那种波动我自然是极为的熟悉。”岩笑道。
绫清竹这才微微点头，偏过头，那清澈眸子盯着林动，道：“你想要知道我为什么会拥有这种波动？”
被绫清竹那清澈得犹如要直视内心般的目光看着，林动犹豫了半晌，顾左言右，始终未能正面回答一下，这看得一旁的岩暗暗着恼，这小子平日里做事异常果断，偏偏在这两个女人身上就犹豫了许多，感情这东西，真有这么麻烦么？
不过林动虽未正面回答，但绫清竹何等聪慧，缓缓收回目光，沉吟了半晌，方才道：“我们九天太清宫每一位宫主以及其继承者，都会自小便是修炼一篇名为“太上感应诀”的玄妙－武学，这篇武学，并不具备任何的力量，也无法做到退敌之用，但这却是必修课程。”
“太上感应诀？”林动与岩皆是一愣，对视一眼，显然都是未曾听说过。
“这篇武学玄奥异常，我们九天太清宫无数先辈穷其一生，都未能取得丝毫进展，但因为宫规，我们始终未曾将其放弃。”
“而修炼这篇武学，因为需要身心纯净，所以修炼者必须是…处子之身，否则，功法自破。”说到此处，绫清竹那清澈眼中显然是掠过一抹微涩之意，眸子看了林动一眼，顿时让得后者大汗淋漓起来，这样说来的话，当年那一次，他岂不是莫名其妙的就将绫清竹十数载的苦修给破了？难怪她那时候恨不得把自己给剁了。
“但你现在…似乎是修炼成功了？”岩看了看绫清竹，有点讶异的道。
绫清竹微微点头，道：“当初师傅知道我苦修被破，也是异常震怒，不过后来八年，我却是在修炼中发现这“太上感应诀”愈发的通达，直到前些时候，师傅拼尽性命再加上众多长老相助，我这才将“太上感应诀”修炼而成，而也就是在那时候…我身上多出了一些你所说与符祖类似的波动。”
“这种力量，的确很强大，虽说我如今仅仅只是转轮境实力，但若要动用那种力量的话，即便是触及轮回的转轮境强者，也是难以抵挡。”
岩也是点了点头，他见到过绫清竹一剑破开那天元子三人自爆所形成的魔宴世界，那种力量，看似不起眼，但却是惊人之极。
说完，他就看向了林动，但后者却是没多少反映，这不由得让得他恼怒的干咳了一声，这小子，平常的机灵都哪去了！
林动听得他的干咳声，也是一阵无力，他自然是知道岩想要他做什么，但…他娘的，这种话怎么可能说得出口来啊？
所以，面对着岩的提醒，他只能将其无视，然后干笑了一声，道：“时间差不多了，苏柔她们还在等你呢。”
绫清竹淡淡的看了他一眼，林动与岩的那般表情被她瞧得清清楚楚，以她的聪慧，自然是明白他们心中的所想。
“那我走了。”
绫清竹见到林动没有说话，也就点了点头，然后没有丝毫拖泥带水的转身，窈窕的娇躯，在林动眼瞳中倒映着一个动人的曲线。
“你小子也太不争气了吧！”岩见到绫清竹要离开，顿时忍不住的怒骂道。
林动看着绫清竹的倩影，心中反而是轻松了许多，冲着岩挥了挥手，笑道：“何必太过执着，世间千道万法，我就不信没其他的办法能够赶上冰主，走吧，我们也回去了。”
说着，他便是欲转身而去，不过却是突然被岩拉住，然后后者冲着他指了指前面，他一抬头，便是见到那道倩影停住了步伐，接着缓缓的转过身来，阳光照耀在她的身上，犹如一圈动人的光弧，她修长的睫毛轻轻眨动，眸中神色变幻，最后盯着林动，道：“你想要学这“太上感应诀”吗林动怔怔的望着绫清竹，最终长叹了一口气，这时候再矫情就真不是他的性子了，当下点了点头，老老实实的道：“想。
绫清竹立在原地，她看着林动，贝齿轻咬着红唇，握着剑鞘的玉手也是微微用力，那眼神中似是有着一些挣扎，如此许久后，她心中终是轻叹一声，仿佛下了什么决定一般。
“你最近会外出吗？”绫清竹问道。
林动听得这有些莫名的话，也是愣了愣，然后点点头，道：“会去北玄域一趟，把青檀接回来。”
“那我随你去吧。”绫清竹想了想，道。
“啊？”林动满脸的愕然，道：“你能走开？你现在可是九天太清宫的宫主。”
“宫内有诸多长老，离开一些时间倒是无碍。”绫清竹神色淡然，清澈双眸看了林动一眼，道：“不过你得答应我一些事。”
“什么？”
“从今以后，不要再问我有关“太上感应诀”的事，若是我认为时机到了，自然会尝试能否让你将它修炼而成。”绫清竹淡淡的道。
“这是你们九天太清宫的不传之秘，会不会…”林动犹豫了一下，道。
“我现在是九天太清宫的宫主，而且你对我们九天太清宫有着恩情，想来诸位长老也不会反对。”
林动望着绫清竹那平静如水般的绝色容颜，最终轻轻的点了点头，道：“好吧，我答应你。”
“另外…”林动声音顿了顿，旋即微微咬牙，道：“你知道我为什么要找你学这“太上感应诀”吗？”
绫清竹清澈双眸盯着林动，声音轻缓：“每个人有每个人的选择，你选择来找我，而我，也是选择将“太上感应诀”教给你。”
“至于原因，我不太需要。”
“我先让苏柔她们回去了。”
绫清竹说完，也不待林动回答，便是转身行出树林，而林动望着她的身影，还略微的有些发呆，最后长叹了一口气，她这是想要自己愧疚到死么…
“唉，你小子可是撞狗屎运了啊，也亏得是你，不然这“太上感应诀”算是别想了…”岩在一旁看了看，然后也是叹了一口气，这“太上感应诀”显然是九天太清宫的不传之秘，别人就算真与九天太清宫有着恩情，她们也断然不会将这般至关重要的东西拿出来的。
而且，这绫清竹聪慧异常，显然也是隐约猜测到一些林动想学“太上感应诀”的目的，只不过她却并未多说。
这女孩，的确总是喜欢默默承受，但却坚强的从不与人言，那般感受，如人饮水，冷暖自知。
“世间万物，唯情债难还啊。”岩拍了拍林动的肩膀，道。
林动默然，心情复杂，然后他跟着走出去，正好见到绫清竹在与苏柔说着什么，后者则是拉着她的手咯咯笑着，从他的角度来看，正好看见绫清竹那娇嫩的耳尖似乎是有些发红。
“林动大哥，师姐就先交给你了，可要帮我们好好照顾她啊。”苏柔远远的冲着林动扬了扬小手，娇声道。
不过她话刚刚落下，绫清竹那剑鞘便是有点羞恼的敲在了她额头上，顿时让得她小脸苦了下来。
一群九天太清宫的少女嬉闹着，打趣了绫清竹一会，然后便是娇笑着逐渐的远去，清脆的笑声，在这山林间远远的传开。
绫清竹望着她们远去，这才转身，来到林动面前，道：“你打算什么时候动身？”
“那就明天吧，我回去与他们交代一下。”林动想了想，如今道宗已是无事，那便尽快去北玄域将青檀接回来吧。
“嗯。”绫清竹轻轻点头，而后不再多说，林动见状也就转身，两人一路再度返回了道宗。
回了道宗，正巧遇见应笑笑，她见到两人一同回来，眼神不由得有些古怪，但却并未说什么，而林动也没法解释什么，便嘱托应笑笑先将绫清竹安排了一下，接着他去与应玄子说了一说要暂时离开的事，后者虽然有点讶异，但倒是没什么反对，只是叮嘱他此行小心一些，毕竟现在这天地间，都算不得太过的平静。
之后林动又去了后山，与应欢欢说了一番，她听得林动要去接青檀回来倒是颇感赞同，当年她便是与青檀相识，两人关系还算不错，而她本身倒是想跟着前去，但奈何她现在的状态实在不宜出岔子，因此也只能遗憾的留在道宗。
从后山出来，林动目光则是望向了远处的一座山峰，略微想了想，吩咐一名弟子找来两瓶好酒，然后他便是拎着掠上那座山峰，在那山巅上，他见到那道盘坐在岩石上的红发身影。


第1225章 山顶之谈
山崖之上，那道身着金色袍服的红发人影静静盘坐，即便是隔着远远的，依旧是能够感觉到那股古老的波动。
林动慢步而去，那道身影显然也是有所察觉，而后转过头来看了他一眼，神色平淡。
林动在炎主旁边的一块岩石上坐了下来，然后扬了扬手中的酒壶，将其丢了过去，炎主仲手接过，把玩了一下，淡淡的道：“怎么？不发怒了？”
“有用吗？”林动轻叹了一口气，目光望着道宗后山，那里的寒气，在逐渐的散发出来，令得整座山峰都是挂满了冰霜。
炎主的目光也是顺着望去，沉默了片刻，道：“我这次暗中施展的的手段，或许对你以及…应欢欢来说有些不太公平甚至于不耻，你会发怒也属正常，不过，若是再重来的话，我还是会这样。”
“当年天地大战，师傅燃烧轮回封印位面裂缝，这才令得那场浩劫终结，但异魔的确极难对付，虽然之后我们尽力的围杀，但依旧是有着不少的漏网之鱼，而且其中一些厉害者，也是与我们拼得两败俱伤，看似是他们最终败退，可我们也被逼得沉睡或者轮回。”
“如今天地再乱，师傅却已不在，而小师妹，则是成了最后的希望，师傅曾经说过，小师妹最有可能达到他那一步，因此我们所有人都甘愿以生命庇护她，即便最后我们皆是重伤，但对于师傅要用剩余的力量护着她进入轮回，我们都未曾有丝毫的异议。”
林动默然，这事他已是知道，所以吞噬之主最后方才会燃烧轮回，不然的话，这位曾经的天之骄子，也不至于会陨落。
“只有小师妹达到师傅的那一步，这片天地无数生灵方才能够被拯救，也只有这样，我们才能抵御下异魔的侵占。”炎主撕开酒壶，重重的灌了一口，淡淡的道。
“而为了这一步，我们也会竭尽全力甚至…不择手段。”
林动默然，旋即他也是饮了一口辛辣的烈酒，摇晃着酒壶，道：“你们的出发点很伟大，这点我是比不上，而且也不太想去比上，我从一个小地方厮杀打滚的爬出来，经历的同样不少，而我努力修炼的原因也很简单，我想要保护我想要保护的人，只要能够护得他们不受伤害，即便我自己遍体鳞伤，我也并不在意。”
“或许你会说，异魔入侵这是天大的事，因为天地陷落，我想要保护的人也无法独善其身，理的确是这个理，为了保护他们，即便是要我去与异魔死战我也会毫不犹豫，说句实话，这些年来，我也是在以各种的方式与异魔交手着，但…这终归还是有着一些差别的啊。”
“我并不是想让她放弃那种责任…只是，或许我想，我可以帮她接下来，那样的话，她就不用承担这种足重得足以让人崩溃的责任。”
林动自嘲一笑，道：“我从来不认为我够资格去当什么救世主，我也只是一个很普通的人，会自私，会冲动，但是为了我所在乎的人，我能够付出任何的代价。”
炎主望着神色自嘲的林动，许久后，方才轻轻摇头，道：“那种责任，只有她才能肩负下来，其他人，都是不行的。”
“为什么？就因为符祖曾经说过的那句话？”林动一笑，此时他的脸庞在夕阳的照耀下显得分外的坚毅，而后他道：“世界上没有那么绝对的事，或许那几率很小，但我却是不会放弃。
炎主轻轻摇晃着酒瓶，看向林动的眼神突然复杂了一些，道：“虽然在我看来你的这种努力有些天真，不过这种勇气，我却颇为的喜欢，曾经的我，也有过你这般想法，我喜欢她，我同样也想为她肩负起那道责任，可是，最后我放弃了。”
炎主的面色此时有些苦涩，他轻声道：“在那种责任面前，我最终放弃，因为我觉得我的确没有接过来的本事，呵呵，说到底，其实是勇气不够，其他的什么，倒是一些借口罢了，有时候，我倒挺羡慕你这份执着，看似莽撞，但却从不心生悔意…”
林动望着面色苦涩的炎主，也是逐渐的沉默了下来，默默的将手中酒壶缓缓的灌了两口。
炎主脸庞上的苦涩，很快的便是消散而去，他看了林动一眼，道：“我们的意愿，最终还是不会出现什么改变，不过你若是对自己真的有那么大的信心，就用自己的能力去证明吧，我们有着一个机会，不过却是留给小师妹的，现在的你，还没有资格与她争抢。”
林动握着酒壶，他知道，他不可能说服炎主，两人的立场处于绝对的逆反，他想要保住应欢欢，而炎主却是需要觉醒的冰主，两者无法共存，总归是要做出抉择的。
想要改变，那就只有向炎主他们证明，他能够超越冰主，只不过这条路，注定艰难无比。
“你要出去？”炎主突然问道。
“嗯，去趟北玄域。”
“去吧，这里我帮你守着，而且有她在这里，这里出现任何事我都不会袖手旁观的。”炎主道。
“那便多谢了。”林动点点头，道谢了一声。
“这是我的任务，即便你不说，我也会去做。”
炎主摇了摇头，旋即他站起身来，目光望着那弥漫着寒气的后山，眼中突然有着一抹温柔之色浮现出来，道：“其实也很想谢谢你…呵呵，好多年都未曾看见小师妹那般的笑了，她从小便是冷静得可怕，仿佛从不会有什么情绪的波动。”
“看见她的笑容，我有时候倒挺理解你那种拼了命想要去守护的心情了…”
“不过，理解归理解，现实，却总是这般的残酷啊。”炎主轻轻挥了挥手，轻叹道：“去吧，虽然跟你说了这么多，但若等我真身来临时，我们该怎么样还是会怎么样的，到时候，或许动手反而是最为直接的方式。”
林动一笑，重重的点了点头，道：“能够与你们这些远古之主交手，那也算是我的荣幸，不过，即便是拼了性命，我也不会让你们轻易的将她抢走。”
“呵呵，有血性，那一天，我倒是很期待。”
炎主转身，手中酒壶冲着林动扬了扬，后者一声大笑，将那酒壶一饮而尽，而后甩开，大笑而去。
炎主望着林动远去的背影，轻轻一叹，喃喃道：“希望你能一直的这样执着下去吧，这样的话，即便最后失败了，但至少，你不会再去后悔…”
山崖上，突然有着寒气涌来，炎主微微偏头，望着那突然出现在先前林动坐的那岩石上的冰冷倩影，却是未曾说话，只是又将手中酒壶饮了一口。
应欢欢也没有与他说话，只是眸子望着林动那消失在远处的身影，旋即她轻轻的捡起林动丢下的酒壶，仰起雪白的脖子，将那其中残留的酒水轻抿了一口。
“有着这么一个人如同傻子般的护着你，难怪你不肯苏醒过来。”
炎主最终叹了一口气，他盯着应欢欢，苦涩一笑：“只是你应该知道…最终什么都不会改变，那种责任，他也扛不起，你愿意让他去受这种苦么？”
应欢欢依旧没有回答他，她轻轻的蜷起修长的双腿，而后玉手有些柔弱的抱着膝，垂着头，将脸埋在膝间，冰蓝色长发倾洒下来，犹如一朵盛开的冰莲。
她的娇躯似是在微微的抖动着，旋即有着冰冷的水花滴落下来，在触地的瞬间，化为一朵朵的冰花，最后有着近乎哽咽的声音，从那冰蓝的长发下传出。
“我知道…属于我的，我就会去接受，只是…我，我真的，真的好喜欢他。”
炎主望着那不断抖动的倩影，绕是他这般的心境，都是有着淡淡的酸意涌上来，但他却是只能这样静静的看着，一如很多年前那般，无力而可笑。
夕阳笼罩着山崖，雪花凝聚着，而后飘落下来，看上去显得那般的凄婉。
翌日。
待得第一缕晨辉自天际倾洒下来时，林动便是行了出来，他并未惊动其他人，而当他来到宗门处时，那里一道身着白色衣裙的窈窕倩影已是静静而立。
“来得这么早啊。”林动冲着绫清竹笑了笑，道。
“此行前往北玄域路途遥远，尽早动身也好尽快回来。”绫清竹依旧是手持着那柄青锋长剑，清澈的眸子看了林动一眼，道。
“走吧。”
林动点点头，然后他转身凝望了一下后方的道宗，最后轻吐出一口气，不再有丝毫的拖沓，手掌一挥，洒脱转身，身形一动，便是化为一道流光划过天际。
绫清竹见状，也是身化虹芒，迅速的跟上。
而在两人远去时，那道宗后山山巅上，寒气弥漫间，似是有着一道身影，远远的眺望着，犹如石像般，许久都未曾移动。
寒气涌来，又是将她的身形遮掩而去。


第1226章 千万大山
玄域分四方，东西南北，而每一片地域，都是辽阔无尽，各成体系世界，不过由于这四域虽然同处一片大陆，但却是相隔极其的遥远，并且在这玄域之中，有着千万大山，自南而北，蜿蜒扭曲，犹如一条匍匐在的天地之间的巨龙，刚好是将这四域切割而开，想要跨越这千万大山，可绝不是寻常强者能够办到。
而东玄域与北玄域之间，正好被隔断，想要穿行其中，即便是颇有能耐者，也起码也是需要两三月的时间才能渡过，而且这还是没有倒霉的遇见这千万大山之中的诸多凶猛妖兽的前提下。
另外在那千万大山中，终年笼罩着由天地元力汇聚而成的雾瘴，若是没点本事就想穿越，恐怕最终只能迷失了方向，化为那千万大山中无数的皑皑白骨。
面对着这种路程，即便是林动都是感到略微的有点麻烦，不过所幸毕竟如今他实力强横，这般麻烦也是能够在忍受的范围之中，只不过这让得他想到当年青檀小丫头那点实力，却是眼巴巴的从遥远的北玄域跑过来见他，心中就有些心疼得紧。
因为想要尽快的找到青檀，因此林动也是艺高人胆大的直接闯进那千万大山，而后辩明大致的方向，便是直接与绫清竹二人全速赶路，那般效率，自然是常人难及。
不过即便是他们效率再高，想要在短短两三天内就穿越千万大山抵达北玄域也是不可能的事，而林动似乎是明白这一点，所以在经历了初始的迫切后，倒也是逐渐的平静了下来，这时候想着急也是无用。
两日赶路倒是相当平静，绫清竹这一路也只是静静的跟在林动身旁，她性子清冷，即便是面对着林动，话语也并不是很多，只是不知道是否是林动错觉，他倒是感觉到在这种平静中，绫清竹那清澈的眸子较之以往，多了一点心神不宁的味道。
浓雾笼罩的山脉中篝火升腾着，一道雪白衣裙的绝色女子静静的坐在篝火旁，一柄青锋长剑斜靠在她的身旁，而她那清澈的眸子，会时不时的看向远处的黑暗。
远处，有着细微的破风声响起，而后一道身影便是出现在了篝火旁林动拍了拍身体上的落叶，冲着绫清竹一笑，道：“雾气太浓，夜里怕还是不能走，所以今夜就歇着吧。”
这千万大山一到夜里，天地元气就愈发的浓郁这也是导致那种元力凝聚而成的浓郁更重，在这种雾气下，即便是精神力都是受到了一些阻拦，因此在夜中时，林动二人倒是并未再继续的赶路。
绫清竹只是轻轻的嗯了一声。
林动看了看她，这才发现在她一只雪白如玉般的小手，正支着一窜架在篝火之上的烤肉这一幕直接是瞬间让得林动目瞪口呆了下来，他见过绫清竹各种清冷脱俗的一面，何时见到这仙子般的人儿，竟然会做这种五谷之事…虽然那烤肉似乎烤有点焦…
绫清竹似也是察觉到林动的目光脸颊似是微红了一下，而后不动声色的收回玉手，将身旁那青锋长剑给握着。
林动笑了一声，在篝火旁坐下来指了指那烤肉：“这是给我的？”
绫清竹依旧没有回答，只是低头看着手中的长剑。
林动见状也就直接不客气的伸过手将那烤肉取了过来，咬了一口，而后自言自语的道：“有点焦。”
呛。
一截泛着寒芒的剑锋突然出鞘了一点，那等凌厉之气，就连篝火都是瞬间黯淡了许多。
林动手掌也是微微僵了一下，然后他看着那依旧低着头把玩着长剑，甚至连神色都没有丝毫变化的绫清竹，忍不住的笑道：“不过味道还不错。”
说着，他加快速度两三口将那烤肉吞进肚内，抹去嘴角的油渍，道：“真是难得，能够吃到你做的烤肉，这若是放在八年前，我一定会说简直是痴心妄想。”
听得他的话，绫清竹终是抬起头，眸子看了他一眼，道：“莫要再说这些怄气的话了，现在的你也不再是八年之前那个需要隐忍着对付林琅天的少年，你的这些成就都是你自己用努力所换来，虽然我从未说过，但对你还是有些心服口服的，因为我知道，换作我的话，这些事情，我是做不到的。
林动怔怔的望着眼前的绫清竹，篝火的火光照耀在那有着薄纱遮掩的绝美脸颊上，显得那般的动人，旋即他忍不住的咧了咧嘴，一种很罕见的虚荣以及满足感从心底深处弥漫开来，最后扩散到四肢百骸，能够让清傲的绫清竹说出这种话，可真真是不容易啊。
“那你得承认以前那样看我是错的。”林动回过神来，然后得寸进尺的道。
绫清竹略感好笑的望着这不依不饶的林动，她同样见识过后者在面对着元门那等强敌时的凌厉与不惧，却也同样是第一次看见他这有点孩子气的举动，然而他的这般模样，不知为何，却是让得她那清冷的眸子涌上了一些柔软，最后她轻轻点了点头，算是满足了这家伙的得寸进尺。
林动见状，却是忍不住的大笑出来，那神色颇为愉悦，绫清竹略显无奈的瞥他一眼，则是不再说话，娇躯一动，掠自树上，盘坐下来，美目微闭，径直的进入修炼状态去了。
林动则是斜靠着树干，看了看绫清竹，然后也是虚眯着眼睛，闭目养神，他能够感觉到，自从两人进入这千万大山后，绫清竹似乎是有着一些变化，不再像以往那般的令人难以接近，不过至于那太上感应诀，林动则再没有问起过，而绫清竹也从未主动提起，那般模样仿佛两人都不知道这事情一般。
而对于这种情况，林动不仅未失望，反而心中深处，仿佛还是轻轻的松了一口气千万大山辽阔无尽，林动二人也是愈发的深入，虽说这千万大山之中有着无数凶狠妖兽，其中一些妖兽实力也是恐怖异常，甚至比起妖域的一些顶尖强者都是不遑多让，但林动二人却并未遇见丝毫骚扰显然，对于他们两人的厉害，这些千万大山中的妖兽强者也是能够察觉到，这种的硬骨头，还是少招惹为妙。
不过虽然没了妖兽的招惹，但当林动两人在千万大山中赶了将近五日路程时，终于是停顿了下来因为他们发现似乎迷失方向了…
林动站在一颗大树上，目光远眺而去，但那浓得近乎粘稠的元力雾气，却是将他视线降到了最低，这种时候，若非林动还有着精神力可以施展的话怕真是有点寸步难行的味道。
林动无奈的摇了摇头，闪掠下树，冲着绫清竹摇了摇头，道：“没办法，只能先走走了，若最后实在不行，我就动用空间祖符直接撕裂空间出去吧虽然不知道会落到哪里去但总比在这里乱转来得好。”
因为两人初始都是艺高人大胆，并未按照千万大山之中的常规路线寻走，而是直接走得最近的直线，所以如今方才会遇见这些麻烦。
不过虽然麻烦了点但也不是太过头疼的事，若真出问题了，林动也能借助空间祖符的力量撕裂空间而去，只是他从未去过北玄域自然也没办法直接撕裂空间过去。
当然，若是他能够彻底的动用空间祖符的力量或许又自当别论，但空间祖符毕竟是周通之物，如今他尚还未彻底的清醒，要让林动将空间祖符炼化的话，他着实是有些做不出来。
“嗯。”
绫清竹也是轻轻点头，旋即她便是对着前方而去，林动则是紧随着，眉头紧皱的望着四周。
两人这番，又是走了半日时间，但那元力雾瘴依旧没有减弱的迹象，这让得他们有些无奈，不过就在林动考虑是否要动用空间祖符之力时，两人的步伐突然同时的停顿下来，然后目光有些讶异与欣喜对着右方远处望去。
浓雾之中，一道娇小的身影狼狈的奔跑着，她身体上有着不少的伤痕，鲜血伴随着她的奔跑洒落下来，染红地面。
唰唰！
而在她向前奔跑间，后方也是有着破风声紧随而来，十数道黑影快速而来，阴寒的杀意自他们身体上弥漫出来。
咻！
一道黑光猛的自后方掠来，狠辣无比的直接射在那前方那道身影腿上，一道痛声响起，前方那道身影顿时狼狈的翻倒了下去，然后她急忙撑起身子，眼神绝望的望着那闪电般出现在其前方的十数道黑影，咬着牙道：“你们这些叛徒，殿主她不会放过你们的！”
“她自己都自身难保，还想对付我们？”那十数道黑影中，一人冷笑出声，旋即其眼中凶光一闪，一掌拍出，磅礴元力便是凶狠无匹的对着那娇小身影轰了过去。
那少女一见，小脸顿时煞白起来，眼神之中布满绝望，不过，就在她绝望间，突然有着破风声响起，而后一道削瘦的身影犹如鬼魅般的出现在其面前，袖袍一挥，那等凌厉攻势便是烟消云散而去。
突如其来的变故，让得所有人都是一惊，那少女也是怔怔的望着眼前这道身影，然后她便是见到后者转过身来，冲着她笑了笑，道：“小姑娘，我帮你解决掉他们，你带我去北玄域，如何？”
少女闻言，顿时一喜，旋即她似是想到什么，连忙道：“这位前辈，感谢您的大恩，不过能不能请您也救救我辰傀大哥？”
“辰傀？”
听得这个有些耳熟的名字，面前的男子似乎也是愣了愣，旋即他看着眼前的少女，有点惊讶的道：“你是黑暗之殿的人？”


第1227章 再遇辰傀
少女听得林动的话，也是愣了愣，旋即微微犹豫，咬着牙点了点头。
“他们呢？”林动看了一眼那十数名眼神阴寒将他给盯着的黑影人。
“他们也是黑暗之殿的人。”
林动闻言，眉头不由得一皱。
“小子，你是何人？我黑暗之殿的事情也敢插手？若是识相，就立即滚开，免得惹祸上身。”
此时那数十名黑影人也是回过神来，眼神阴厉的盯着林动，厉喝道。
“前辈，还请您救救我辰傀师兄。”少女急忙恳求道。
“呵呵，放心吧，我与辰傀也算是相识，自然是会救他。”林动笑道，当初青檀来东玄域，还多亏了辰傀的照拂，如今既然遇见了，他自然是不会袖手旁观。
那少女闻言，顿时松了一口气，虽然她知道要从那些家伙手里救下辰傀是一件多么困难的事，但眼下也只能是病急乱投医了。
“不自量力的东西，杀了他！”那十数位黑影人见到林动执意要出手，眼中顿时有着寒芒掠过，身形一动，便是闪电般的掠向后者，然而，就在他们即将拔剑时，一道凌厉剑芒，猛然自那浓雾深处席卷而出，剑芒掠过，这十数名黑影身体陡然僵硬，一道细小的血线自他们咽喉处浮现，接着身体便是尽数的栽落而下。
这般变故，仅仅出现在电光火石之间，那少女那口气尚还未落下，便是目瞪口呆的见到这些黑暗之殿的强者，这般被轻易的解决而去。
浓雾中，一道倩影缓步走出，绫清竹看都未曾看那些满地的尸体，她玉手握着长剑，安静的来到林动身旁。
“小姑娘，你叫什么？”林动望着那目瞪口呆的少女，笑着问道。
“前辈…仙子姐姐…”少女有点胆怯的看了林动一眼，再看了看他身旁那气质脱俗得犹如仙女般的绫清竹，眼中掠过一抹惊艳，道：“我叫穆莎。”
“我叫林动，她叫绫清竹，现在可以先带我们去救你那辰傀师兄吗？”林动微笑道。
穆莎闻言，眼中顿时有着惊喜涌出来，旋即她微微犹豫，道：“不过林动前辈…”
“叫我林动大哥吧。”林动摆了摆手。
“林…林动大哥，不过辰傀师兄那边，去抓他的是我们黑暗之殿的两位长老，实力极强，乃是转轮境的强者，而且，他们还带了黑暗之殿的暗部。”穆莎小脸有点苍白的道。
“两名转轮境？”林动倒是有着讶异，这黑暗之殿果然不愧是北玄域的龙头，这番实力，的确比东玄域很多超级宗派强横。
“黑暗之殿底蕴雄厚，存在时间远非寻常超级宗派可比，我师傅曾经说过，在这黑暗之殿中，恐怕还有着轮回境的老怪坐镇。”一旁的绫清竹轻声道。
“倒的确是有些实力。”林动点点头，能够拥有着轮回境的老怪，难怪黑暗之殿在这北玄域屹立不倒。
“对了，辰傀不也是你们黑暗之殿的人么？为什么会去追杀他？难道他叛逃了？”林动皱眉问道。
“怎么可能！辰傀师兄对我们黑暗之殿忠心耿耿，怎么会叛逃！”穆莎急忙道：“只是如今黑暗之殿太过混乱，殿主上位不久，各方权利未稳，暗中发生了不少事！”
“原来是内讧了。”林动这才恍然，对这黑暗之殿他没太大的感觉，只要别伤到青檀，随便他们怎么玩。
“你们新任殿主是谁？”林动转过身，对着前面走去，顺口问道。
“我们新殿主是青檀大人。”少女回道。
林动的脚步噶然而止，他身旁的绫清竹也是微微怔了怔，然后林动缓缓的转过身来，望着那穆莎：“你…你说是谁？”
“青檀大人啊。”穆莎也是看着林动，有些疑惑的回道。
“青檀怎么会成为黑暗之殿的殿主了？”林动脸庞微微抽搐了一下，连声音都是猛的提高了许多，那眼中满是震动之色。
穆莎被林动的神色吓了一跳，连忙退后两步，胆怯的道：“去年老殿主在突破轮回境时失败坐化，临终前他将传承以及黑暗祖符都是传给了青檀大人，她自然便是我们黑暗之殿的殿主啊。”
林动面色阴晴不定，这事情倒是大大的出乎了他意料，青檀那小丫头怎么会成为黑暗之殿的殿主，那丫头除了跟在他身边乱转悠之外，哪还有这般本事？
“林动大哥…您，您认识我们殿主吗？”穆莎看着林动，小心翼翼的问道。
林动无奈的叹了一口气，没好气的道：“她是我妹妹，你说我认不认识？”
“您妹妹？”穆莎目瞪口呆，旋即眼神有些古怪，显然是不太相信林动所言，毕竟这件事，她可从未听说过。
“算了，先去把辰傀兄救下来吧。”林动挥了挥手，辰傀在黑暗之殿地位应该比这小丫头高一些，这些事情他或许知道得最为清楚。
穆莎也是连忙点点头，眼下不管如何，还是先将辰傀师兄救下来再说，因此她匆匆包扎一下伤口，然后便是急忙跟了上去。
在这浓雾的另外一角，一片林间空地，一簇簇的篝火升腾着，在篝火四周的大树上，隐约可见一道道身影，他们那锐利的目光正在不断的扫视着四周。
在空地中央，数十道狼狈的身影被捆在一起，他们身体上布满着血迹，显然是经历了一番惨战，而且看这模样，似乎结果还并不好。
在这群人最前方，一道人影身体被黑色锁链捆缚着，一截尖锐的锁链甚至是从其肩膀处洞穿了出来，稍微蠕动一下，便是有着鲜血流出来，令得他的身体一阵剧烈颤抖。
不过即便是面对着这般痛苦，那道人影却是一声不吭，只是额头上不断有着冷汗滴落下来。
“辰傀，老夫念在你也是我黑暗之殿的人才，倒不想如何为难你们，只要你们随老夫回去，到时候大典上就说那小丫头暗中施展手段害了殿主，夺了黑暗祖符，老夫不仅不会杀你，反而让你统领我黑暗之殿暗部，如何？”在篝火旁，两名身着黑袍的老者正面色阴翳的盯着那男子，阴测测的笑道。
“呸！”
听得此话，那道人影顿时一口口水吐了出来，他满脸讥讽的望着那两名长老，冷笑道：“想要我诬蔑殿主，痴人说梦！”
“嘿，一个小丫头而已，还想成为我黑暗之殿的殿主？我看你才是痴人说梦。”那一名长老嘲笑道。
“哼，那小丫头一上位便想清除我们这些老臣，有这般下场也是活该！”另外一名长老也是阴冷的道。
“如果不是你们不服殿主，暗中试图分裂黑暗之殿，殿主她会对你们下手？”辰傀咬着牙道。
“非是我们不服，只是这黑暗之殿殿主之位，论资历，论本事，可轮不到她这个不知道哪来小丫头片子！”
辰傀冷笑一声：“这是老殿主临终前的任命，你们自己没本事，现在又来恬噪。”
“老殿主的任命？呵呵，那时候就那小丫头在殿主身边，什么任命，还不是她一人说了算。”那长老目光阴冷的盯着辰傀，道：“看这模样，你是不打算配合了啊。
辰傀讥讽的看着他，然后不再说话。
“真是敬酒不吃吃罚酒，既然如此，那就莫怪老夫心狠了。”那长老眼中杀意掠过，也不再有丝毫的客气，手掌一握，便是有着磅礴元力在其手中汇聚而来，而后直接化为一柄元力剑锋，袖袍一抖，狠狠的对着辰傀暴射而去。
辰傀见状，心中也是一声暗叹，不过就在他准备闭目等死时，一道笑声却是突然的穿透浓雾，在这空地中响了起来：“没想到这种老不要脸的东西，果然是到哪里都能够遇见。”
“谁？！”
这般声音，顿时让得场中所有人一惊，那两名黑暗之殿的长老更是厉喝出声，锐利的目光扫视着四周的浓雾。
浓雾中，三道身影缓缓的行出，而后逐渐清晰的出现在了众人眼中。
“辰傀兄，别来无恙啊。”林动目光在空地中扫视一圈，然后停在那满身鲜血的辰傀身上，微微一笑，道。
“你…林动？”辰傀惊愕的望着林动，旋即面色猛的一变，急喝道：“你们快走！”
“走？”那两名黑暗之殿的长老却是怪笑出声，然后手掌猛的挥下，眼中杀意暴闪：“给我杀了！”
“是！”
他喝声一落，周围便猛的爆发出道道应喝声，而后磅礴元力陡然涌动，凌厉攻势铺天盖地的对着林动三人席卷而来。
林动面色淡漠的扫了一眼那般攻势，却是未曾有丝毫的理会，只是迈步走向那两名黑暗之殿的长老。
砰砰砰！
道道攻势，瞬间接近林动周身数丈范围，而就在要落到他身体之上时，那一道道攻势猛的凝固，然后辰傀便是惊骇的见到，那些来自暗部的强者，竟然是在此时凭空的爆炸开来，那番模样，仿佛是被无形的大手生生捏爆一般，显得极端的诡异。
血雾飘落下来，林动的脚步，却是停在了那两名长老面前，旋即他冲着面色剧变的两人微微一笑，只是那笑容中，有着一股冰寒彻骨的杀意涌出来。
“呵呵，小丫头片子？我林动的妹妹，也是你们这些老杂毛够资格这样叫的？！”


第1228章 青檀之事
“你！”
那两名黑暗之殿的长老面带震惊的望着走到他们面前的林动，后者的笑容在那漫天血雾之下显得犹如恶魔一般，令人心生寒意。
“轰！”
不过两人脸庞上的震惊仅仅持续了霎那，便是陡然化为狰狞，下一刻，浩瀚元力猛的自他们体内席卷而出，一声低喝，那两股强悍元力便是喷薄而出，弥漫着浓浓的杀意，狠狠的轰向林动。
林动依旧是面带淡淡笑容的望着两人，没有丝毫出手的迹象，只是就在两位长老那等攻势即将落到他身体上时，他们的身体，却是在此时陡然凝固。
与他们身体同时凝固的，还有着他们体内浩瀚奔涌的元力，一抹真正的恐惧之色，终于是在此时涌上两人的眼中，因为他们发现，他们竟然在这一霎那，失去了对身体的掌控！
这种情况，还是他们这么多年第一次遇见，甚至即便是以往面对着老殿主，他们都未曾如此的无力过。
“你…你究竟是谁？！”两名长老骇然失声道。
“这位朋友，我们是黑暗之殿的人，若是阁下能够放我们一马，我们若是有得罪的地方，立即与你赔罪，到时候我们黑暗之殿也是能够成为阁下的朋友，阁下有任何事情，只需要吩咐一声便可！”
后面那穆莎以及辰傀和那些被抓住的人目瞪口呆的望着变脸变得极为迅速的两位长老，皆是忍不住的倒吸了一口气看向林动的目光犹如见鬼一般，这两名长老就算是在他们黑暗之殿都算是顶尖强者，然而在眼前这看上去不过二十多岁的青年手中，竟是孱弱成这样？
这之中，辰傀的震骇显然最为的剧烈他以往便是见过林动，但那三年之前，林动虽说实力让他惊讶，但也并未超越他太多，可眼下，这才短短三年时间不见，他…他的实力竟然恐怖到这种程度了？
“你们刚刚说的小丫头片子，是青檀吧？”林动微笑的望着这两位黑暗之殿的长老，道。
那两名长老闻言眼神微微一变，旋即咬牙点了点头，犹自还抱着一丝侥幸的道：“阁下莫非与那丫头有渊源不成？”
“刚刚不是都说过一次了么…青檀是我妹妹，而我，是她大哥！”林动笑吟吟的道，只是那笑容显得格外的冰寒。
两名长老瞳孔微微一缩心中暗暗叫苦，谁能想到，那个看似孤单一人的小丫头，竟然还有着一个这么恐怖的大哥，而且还正好被他们倒霉的遇见了。
“原来是殿主的大哥，呵呵都是一家人，今日主要是辰傀这叛徒想要叛逃我黑暗之殿，而我们二人奉殿主之命前来擒拿他。”一名长老干笑道。
“哦，原来是这样。”
林动笑着点了点头，而那两位长老见到林动的笑容，心中寒意猛然升腾起来，他们毕竟也是老奸巨猾之人，立即感觉到了一股浓浓的危险，当即一声暴喝体内元力毫无保留的席卷而出，竟是在这一霎挣脱了林动的束缚，而后身形一动，化为两道黑光，分开暴射而出。
辰傀见到这两道老家伙要逃，面色顿时一急，不过当他在见到林动那平淡的面色时，心中的情绪这才平缓下来，虽然他与林动交之不深，但从后者在当年那异魔域表现出来的手段来看，绝对不是什么善男信女，这种放虎归山的事情，想来他绝不会去做。
也正如他所料，林动只是平静的望着暴射而出的两人，待得他们要冲进浓雾之中时，方才仲出手掌，轻轻一握，指尖有着黑色光芒闪掠而过。
嗤！
就在林动手掌握下的霎那，辰傀等人便是见到，那两名长老前方空间陡然扭曲，两道巨大的黑洞凭空的浮现出来，吞噬之力爆发间，还不待那两名长老惨叫出声，便是一口将他们吞噬了进去。
砰砰砰！
黑洞之中，传出剧烈的低沉震动，那两名长老仿佛是在疯狂的挣扎着，不过这种挣扎并未持续多久，便是在黑洞迅速的旋转间减弱了下去。
黑洞旋转了半晌，林动袖袍轻轻一挥，黑洞便是消散而去，而连带着消散的，还有着那两名实力在转轮境的黑暗之殿的长老。
整片空地，再度在此时变得鸦雀无声，不论是那辰傀还是穆莎都是愣愣的望着眼前这一幕，那两位长老，便这样的…死了？
“辰傀兄，没事吧？”在众人发愣间，林动也是一笑，屈指一弹，众人身体之上闪烁着黑芒的黑色锁链便是尽数的断裂而去。
“辰傀师兄，你还好吧？”那穆莎也是在此时扑了上来，她见到辰傀那伤势，顿时哭了出来。
辰傀安慰她几声，然后有点艰难的站起身来，冲着林动一抱拳，面色复杂的道：“林动兄，我们又见面了，没想到短短三年不见，你却已强到这般地步。”
说起来辰傀也算是天赋极高，不然的话当初也没办法成为东玄域宗派通缉榜第一位的狠人，但这三年下来，他的实力也堪堪晋入死玄境，这般速度其实说来已算是不错，但奈何与林动这般变态比起来，终归还是黯淡了许多。
“辰傀师兄，你还真认识林动大哥啊？”那一旁的穆莎抹了抹眼睛，也是在此时忍不住的道，她先前还以为林动是在说笑呢。
“林动兄可是殿主的大哥，殿主这次总算是有救了。”辰傀眼露欣喜的盯着林动，笑道。
“辰傀兄，究竟是怎么回事？青檀怎么会成为黑暗之殿的殿了？”林动眉头微微皱了皱，道。
辰傀闻言却是一声苦笑，沉默了一下，道：“林动兄还记得三年之前的事吧？当日青檀见你被元门三巨头逼走，生死不明，愤怒之下便是要与那三巨头拼命所幸最后师傅现身，将她救了回去。”
“不过这一回去，青檀却是犹如变了一个人一般，以往的活泼再也不见，反而变得异常的沉默，经常一个人的发呆，而之后不久她便主动向师傅提及，她要进入黑暗裁判所。”
“黑暗裁判所？”林动眉头紧皱，心中却是有些不安涌出来：“这是个什么地方？”
“一个黑暗之殿中权利最大也是最残酷的地方…我们黑暗之殿统率着北玄域，其中自然也是有着不少势力在反抗着我们，而黑暗裁判所便是制裁这些反对者的地方，在那里，没有仁慈，只有着血腥。”辰傀面露苦涩缓缓的道。
“什么？！”
林动听得却是震怒异常，满脸的铁青，在他的眼中，青檀始终都是那个自小都是围绕在他身旁转的小丫头，那丫头心性善良，娇憨可爱以前在青山镇因为害怕吃苦，一直都不肯主动修炼，然而现在，那么一个小丫头，竟然跑去什么裁判所，满手沾满着血腥，这让得他心疼得眼睛都有点发红。
虽然林动这一路修炼而来，同样是血与火的铸造，但他却是宁愿自己满身沾着鲜血也不愿意看见那个一直躲在自己身后的小丫头变得如同他一般！
他奋力的修炼，渡过那重重生死险关，他所求很简单，只是想要保护自己在乎的人，让得她们脸上有着最纯粹的笑容，然而现在，他那个最心疼的小丫头，却是变得如同他一般沾染鲜血！
“她是想挨揍吗？！”
林动双掌握得嘎吱做响，声音近乎是从牙缝里面蹦出来一般，见到他这般暴怒，辰傀也是苦笑了一声，在见过以往青檀是如何的活泼阳光后，再看她如今的变化，的确是让得无比的心疼。
“我曾经问过她…她说，只要有一天她能够将元门抹除，不论她变成什么模样她都不介意。”辰傀眼神复杂的看着林动，道：“她…她这么做…只是想要为你报仇。”
林动脸庞上的暴怒，在此时凝固了一瞬，旋即他闭上眼睛，深深的吸了一口气，声音有些沙哑的喃喃道：“这个笨丫头，就这么不相信我吗？”
他沉默了好半晌，终是挥了挥手，道：“现在你们黑暗之殿又是怎么回事？”
“青檀花了两年的时间，成为了黑暗裁判所的裁判长，掌控了黑暗之殿中最为强大的一股力量，更后来，师傅则是冲击轮回境失败，在那最后关头，将传承与黑暗祖符传给了青檀，并命她成为黑暗之殿新一任殿主。”
“不过黑暗之殿太过庞大，青檀虽说这三年突飞猛进，但在资历上，与一些长老终归还是有着差别，因此一些长老暗中不服，便是试图联合罢免青檀，但如今的青檀毕竟不是当年，她手中同样掌控着不小的力量，这一年来，她便是不断的与黑暗之殿那些反对她的势力明争暗斗。”辰傀道。
林动听得面色阴晴不定，他实在是有些无法想象当年那么一个天真的少女，怎么去与黑暗之殿那些狡诈狠辣的老家伙们斗，这其中想想就知道这个丫头究竟吃了多少的苦受了多少委屈。
“青檀手段不弱，这一年她先是示弱，却是争取着时间吸收着师傅留给她的传承，并且也是暗中将黑暗祖符彻底掌控，而后开始反扑，那些长老则是吃了大亏，两月之前，更是彻底溃败，不过…就在青檀即将稳定局面时，那些长老，却是将黑暗之殿两尊最为古老的大人请了出来，那两位，都是踏入了轮回境的巅峰强者，有了这两位的出面，黑暗之殿内的情势也是再度逆转，除了青檀所掌控的黑暗裁判所外，其余大多人，都是投靠向了那反对的一方…”
“而我们这些支持青檀的，也是在暗中被清洗。”
辰傀苦笑道：“若是我所料不差的话，他们应该是打算在三日之后的祭典之上，彻底逼宫，逼青檀交出殿主之位以及黑暗祖符…青檀，也被他们逼得无路可走了。”
林动原本铁青的面色，在听完辰傀所说后，却是逐渐的平静下来，他闭着双目，半晌后，挥了挥手，道：“辰傀兄，带我去黑暗之殿。”
“不管我有多不喜欢那丫头做这些事，不过，他是我林动的妹妹，谁想要欺负我妹妹，老子就活活宰了他！”
林动睁开双眼，那眼中的戾气，却是令得辰傀等人脚底都是冒着寒气，与他比起来，就算是从黑暗裁判所出来的青檀，都是有些小巫见大巫了…


第1229章 黑暗之城
千万大山之辽阔，实在是有些出乎林动的意料，即便是有了辰傀这种对地形熟悉的人带路，等到他们走出千万大山时，已是在两日之后。
站在那千万大山之外，林动望着后方那几乎连天际都是被截开的重重原始山脉，也是忍不住的轻吐了一口气，然后精神力运转，在这片地方留下了空间印记，如此的话，以后再想过来，就能够凭借着留下的空间印记直接撕裂空间而来，那效率倒是能够快上许多。
“林动兄，我们这才刚刚抵达北玄域边缘，而黑暗之殿的总部坐落在北玄域中心的黑暗之城，想要赶过去恐怕也至少需要数日时间，也不知道我们赶不赶得上祭典开始。”辰傀望着远处，有些担忧的道。
“一日时间足够了。”林动淡淡一笑，北玄域不像那千万大山中有着极端恐怖的元力浓雾，那等速度自然可以施展到极致，想来一日时间，足以赶到那黑暗之城。
“那我们尽快吧，殿主如今逐渐被殿中众多长老孤立，我们这些忠于她的人马，则是在被暗中清除，以各种方式阻扰我们回到黑暗之城，显然他们是想要在此处祭典之上逼宫动手。”辰傀叹道。
林动微微点头，略作沉吟，道：“辰傀兄，那两位踏入轮回境的老怪，是否便是你们黑暗之殿中最强的了？”
此行前往黑暗之城，想来必定不会一番平静，既然如此的话，以林动的性子，自然是要将他们的实力彻底的弄清楚，免得横生变故。
“据我所知，那两位大人的确算是我们黑暗之殿辈分最老的两位，不过你也知道，黑暗之殿底蕴悠久，已是存在上千载，其中究竟有没有隐藏得更深的化石级老怪，我还真不清楚。”辰傀想了想，又是摇了摇头，恐怕这种事情，就连一些黑暗之殿的长老都摸不透，更何况“黑暗之殿并不简单。”
一旁的绫清竹突然轻声道：“我们九天太清宫同样底蕴悠久，因此我也是从一些古籍之上看见过有关黑暗之殿的信息。”
“黑暗之殿的创建者，应该是黑暗祖符的第二代拥有者，而且黑暗祖符与其他的祖符不同，其他祖符当年随着各主的沉睡，都是散落天地，然后被其他的强者所得，但黑暗祖符却是不同，自从黑暗祖符落到第二代手中，随他创建黑暗之殿后，黑暗祖符便一直都是留在黑暗之殿，直到传承至今。”
“哦？”林动眼中也是掠过一抹讶异，他对于祖符最为的了解，这类天地神物，一旦出现，必定引来无数强者的争抢，想要将祖符一直的保持下去，可并不是一件简单的事，就如上一代雷霆祖符的拥有者雷帝，他便是最终因为异魔的觊觎，导致雷府被毁，若非他也是拥有着强大手段，最终封印异魔王，开辟雷界将雷霆祖符隐藏，恐怕那雷霆祖符，也早已散落天地，被他人所获。
而类似黑暗之殿这种，能够一直的将黑暗祖符传承下来的，却是相当的不容易，而这也是说明黑暗之殿实力惊人，不然的话，很难做到这一步。
“我算了算，从第二代到如今，黑暗祖符传承了五位，而青檀应该便是第六代黑暗祖符拥有者。
绫清竹看了林动一眼，道：“这黑暗之殿能够保护黑暗祖符这么多年，必然有着隐藏手段，不过看这模样，青檀应该并未掌握这种手段，不然的话，黑暗之殿的那些长老绝不敢做出逼宫这般事情。”
“所以此行前去，也不得大意。”
林动眼神微微一凝，旋即点了点头，看来他倒是小看了这黑暗之殿，这北玄域辽阔程度不比东玄域差，而黑暗之殿却是这片庞大地域当之无愧的主宰，这里没有任何的势力能够动摇他们的根基，这与东玄域那种多足鼎立显然是截然不同，而这也是从另外一个方面反映出黑暗之殿的强横之处。
“不过对于这北玄域的霸主，我倒也想见识一番。”
林动笑了笑，虽然知道黑暗之殿不是省油的灯，但他却并没有任何的惧意，以他如今的实力，足以跻身晋入这天地间顶尖层次，触及轮回的转轮境实力，再加上堪比轮回境强者的大符宗精神力以及两大祖符之力，寻常轮回境强者，根本难以与他抗衡，除非遇见那种渡过一次轮回劫的轮回强者，这才能够与其争锋。
而这种层次的强者，想来如今这天地间也寻不出多少，真要有，恐怕也至少得是类似青雉那种从远古活下来的超级化石了…
辰傀见到林动那平静的面色，心中也是微微松了一口气，林动如今的实力，他根本就无法看透，不过从他举手投足间便是将两名黑暗之殿的长老抹杀的手段来看，必然是一个相当恐怖的层次，而如今想要帮助到青檀，或许也只有他才能够到。
“走吧。
林动挥了挥手，也不再多说，心神一动间，便是有着浩瀚精神力将众人尽数的驮负而起，而后周身空间逐渐的扭曲，在下一霎，众人已是同时的消失而去，再次出现时，已是在百里之外。
北玄域上，并没有如同东玄域那般繁星多的各类王朝，这里大多数的地域，都是由黑暗之殿所设置的分殿来统领，而一个分殿之主，就犹如一个王朝的帝王，其下也是有着极端复杂的官衔，从这一点之上，黑暗之殿的统治性，就显得比东玄域高上了一个档次，也难怪黑暗之殿能够成为这北玄域之中屹立不倒的霸主。
而虽说千万大山与黑暗之城距离极其的遥远，但以林动这般撕裂空间赶路的速度，却是将这种赶路时间缩短了许多倍，因此，待得第二日清晨时，他们已是抵达了黑暗之城的外围。
站在一座山峰上，林动远眺，只见得在那前方山水交汇处，一座看不见尽头的庞大城市，犹如一头半截身体潜伏在地底深处的远古巨兽，静静的匍匐着。
城市通体呈现暗黑色彩，给人一种肃穆沉重之感，城市上空，有着一道巨大无比的暗黑光罩笼罩下来，将整个城市包裹在其中。
光罩之上，无数符文流转，一种无法言语的浩瀚波动弥漫开来，直接是令得那空间都是呈现了一些扭曲的迹象。
从林动的角度看去，能够见到十数道巨大无比的城门口，而此时，在那些城门口外，有着条条人龙缓缓的涌入，那等人气，惊人之极这城市的规模，算是林动所见之最，按照他的估摸，怕光是这一座城市，就能容纳数千万人，这黑暗之城，不愧是黑暗之殿的总部所在，手笔相当不小，难怪这黑暗之殿为了一个殿主之位能够争得你死我活，这黑暗之殿的殿主，简直就是这北玄域的主宰，翻手间便是掌控无数人的生死。
“祭典是我们黑暗之殿三年一次的盛会，每到这个时候，北玄域中所有大大小小的势力都必须前来朝拜，谁若是敢不来，恐怕第二日黑暗裁判所便是会寻上门去，将其制裁。”辰傀望着那城市之外的浩浩荡荡人群，道。
“这黑暗裁判所还真不是个好地方。”林动听得却是连连皱眉，火气又是忍不住的涌了上来，这丫头，什么地方不好去，却是要去这种血腥堆积的地方。
辰傀见到林动发火，也是只能苦笑一声，从后者在那异魔域一手抹杀数百名元门精锐弟子来看，这血腥手段同样是不弱，但如今一听到青檀也是这般，却是怒得跟什么一样。
“林动兄，因为祭典的缘故，黑暗之城的防卫也将会是最为森严的时候，而眼下殿主受制，那些长老必定是掌控了城中所有的力量，我们若是从城门进去，恐怕会被搜查出来。”
“既然城门不能进，那就直接撕裂那护城大阵吧。”林动倒是随意的道。
“那阵法是我黑暗之殿第一任殿主所创，若是强行撕裂的话，恐怕立即就会惊动城中强者…”辰傀无奈的道。
“这阵法虽然厉害，但想要拦住我，可没那么容易。”林动一笑，道：“辰傀兄，就你与穆莎随我们进城吧，其余人留在外面。”
“嗯。”辰傀点点头，他们这点人，进了城也帮不了多少忙，反而碍手碍脚。
林动袖袍一挥，淡淡的银光自其手中散发而出，然后将他们四人包裹，紧接着周身空间缓缓的扭曲，而四人的身体，便是这般凭空的消失而去。
辰傀仅仅只能感觉到脑海中一阵眩晕，待得他回过神来时，惊天般的喧嚣声陡然冲来，睁开眼来，只见得四人已是身处那黑暗之城天空上，远处的后方，呈现黑色光罩依旧笼罩着，但对于他们的进来，却是毫无反应。
“真是厉害。”辰傀叹了一声，身为黑暗之殿的人，他自然清楚这阵法的强横，但眼下林动，却是能够来去自如，这般本事，真是让人想想都感到恐怖。
林动笑笑，虽然如今他并未炼化空间祖符，但依旧能够动用一些空间祖符的力量，因此要穿过这阵法，倒不算什么难事。
辰傀目光远眺，望向这黑暗之城的最中心，只见得那里突然有着无数黑暗光芒升空而起，犹如一朵盛大的烟花。
他望着那些黑暗光华，神色也是逐渐的凝重起来。
“祭典已经开始了。”


第1230章 逼宫
在巨大无比的黑暗之城最中央，是一座庞大得近乎看不见尽头的黑石广场，广场之上，无数黑色石柱笔直矗立，在那些黑色石柱上，布满着铭文，那些都是对黑暗之殿有过巨大贡献的人，同时他们也是黑暗之殿中曾经的佼佼者。
这座广场，名为黑暗广场，乃是黑暗之城的重地，唯有着每当祭典开始时，此处方才被开放，而能够进入到这里的人，也都是这北玄域中颇有权势者，当然，他们所谓的权势，自然也是黑暗之殿所赋予。
如今的黑暗广场中，黑压压的人海一眼望去，却是无法见到尽头，然而即便是如此恐怖的数量人群汇聚在这里，这片天地，却是安静无声，每一个人都是不敢发出任何的异声，无数道目光噙着畏惧的望向广场最中心的位置，那里有着一座黑色祭坛，祭坛有着万阶石梯，在石梯的最顶峰，是一道黑色王座，坐在此处，足以俯览着整片黑暗广场，那个位置，仿佛就是北玄域的主宰之位。
而此时，在那祭坛王座之上，有着一道纤细的身影，她身着黑色裙袍，裙袍边缘，布满着暗金色的玄奥花纹，隐约间，有着一种肃穆以及尊贵之气散发而出。
她有着一张平静而美丽的容颜，如雪般的肌肤，如弯月般的柳眉，挺翘的玉鼻，特别是那一对静如深潭般的眼睛，看上去就犹如最为深邃的夜空，宁静之下，却是透着无尽的神秘，令得人忍不住的沉醉在其中。
这般容颜，虽然比起以往多了几分冷冽与成熟，但依稀能够见到当年那娇俏的轮廓，除了青檀之外，还能有着何人？
如今的青檀再没了当年的娇憨，眼波流转间，犹如刀锋般冷冽，在那之下，还有着淡淡的血腥之气涌动，令人心神发寒。
她坐于王座之上，目光俯览着那人山人海的黑暗广场，而在她目光所过处，尽是谦卑的身影鲜有人敢与其对视。
当年那开朗娇憨的少女，如今，却是成为了这北玄域的真正霸主，举手投足间，便能掌控万万人的生死权势遮天。
王座上的她冷冽而威严的眸子扫视着全场，最后轻轻点头。
“祭典开始！”
在其螓首轻点间已是有着一道道嘹亮的声音，在雄浑元力的包裹下，远远的传开，最后响彻在这天地之间。
无数黑色光芒冲天而起，仿佛一场盛大的宴会。
黑暗广场中，开始陆陆续续有着从北玄域各方而来的势力首脑出现而后那恭敬的声音，此起彼伏的在这天地间传开。
“北邙宗宗主，率众前来，恭贺祭典殿主天威浩荡，诸域臣服！”
“大雪山山主祝殿主威仪永存，神功盖世！”
“……”
祭典乃是整个北玄域的盛事，几乎北玄域之上百分之八十的势力首脑都会亲自赶来，这便是如同一种诸臣朝拜，谁也不敢缺席，因此如今这广场上，所汇聚的强者，估计拥有着北玄域六成左右的力量，那浩浩荡荡的规模，端的是骇人之极。
道道恭敬无比的喝声，在广场中回荡，祭坛之上的那道尊贵身影，眸子中却是没有丝毫的波动，她的眼光微微一转，看向那黑暗广场最前方处，那里的席位上，有着不少身影漠然而坐，这些人大多都是白发苍苍，身体之上的袍服也是显露出他们在黑暗之殿中非同寻常的身份。
而此时，在这般重大的典礼上，他们却是微闭双目，丝毫未将恭敬的目光投向祭坛之上的那道身影，一些人嘴角微撇，显然是有些不屑。
在这批人影最前方，有着两道苍老得犹如行将就木的身影，这两人发须皆白，脸庞上布满着深深的沟壑皱纹，他们此时微垂着头，犹如是在沉睡一般。
王座上的青檀目光扫过这两道身影，那冷冽的眸子也是微微一凝，袖中的小手缓缓的紧握，眼神深处，掠过一抹冰冷杀意。
繁琐的诸朝恭颂一直的持续着，整片天地都是回荡着这种恭敬的声音，不过谁都是能够感觉到，这一次的祭典，似乎是与以往有了一些不同之处。
各方的势力首脑目光偷偷的打量着黑暗广场，然后视线在最前方的长老团所在的席位处停下，而后目光再转，看向祭坛周围，那里，有着无数身着黑衣的身影，他们的身体上，有着浓浓的血腥味道散发出来，在他们的黑衣上，是一张狰狞的鬼脸，鬼脸额头上，一柄黑色长剑深深的刺入，煞气惊人。
对于这支部队，北玄域所有的势力都并不陌生，他们对其充满着恐惧，因为这是黑暗之殿中最为血腥的地方，同时也是黑暗之殿最强横的一个部门，黑暗裁判所。
眼下，这支黑暗裁判所的强者，将祭坛重重护卫，而且他们正对的位置，正好是长老团所在，那番模样，显然是在戒备着什么。
整个黑暗广场，都是笼罩在一种古怪的气氛之中。
一些首脑暗中对视，在来之前他们便是受到了一些风声，这一次的祭典，恐怕会分外的不平静，只不过那应该与他们没太大的关系，不论是黑暗之殿现在的新任殿主掌权，还是长老团掌权，他们都没有反抗黑暗之殿这个庞然大物的资格。
所以这一次，他们就只需要老老实实的看这场好戏究竟会如何衍变就行了。
恭颂的声音，在许久后终于是逐渐的停下，而在那声音落下时，这天地间的气氛，仿佛都是悄然的凝固。
祭坛上，王座两侧，立着两名黑袍老者，他们见到这一幕，则是将目光投向了王座上的青檀，后者则是玉手轻扬。
“恭迎祖碑！”
青檀缓缓的自那王座上站起，莲步轻移，而后她玉手一握，玉足轻跺地面。
嗡！
广场上那矗立的无数黑色石柱猛的爆发出嗡鸣之声，一道道黑光暴射而出，在广场上空交织，然后化为一座巨大的黑暗石碑，在那石碑上，乃是黑暗之殿历代殿主之名。
而在这石碑出现时，广场上，无数人都是跪伏了下来，甚至连那些长老团的长老都是微微弯身，唯有着最前方的两名老者，仅仅只是将脸庞垂下。
祭坛上，青檀也是对着那座石碑轻轻弯身，而后石碑震动黑暗的光梯成形蔓延出去，最后链接着祭坛。
“恭请殿主祭祀先祖！”
王座之旁两名黑袍老者齐齐大喝眼神之中，有着狂热涌出来，只要成功完成祭祀，那么殿主之名就将会坐实，谁也反对不得。
长老团最前方的两名老者，那紧闭的双目终是微微睁开一丝，旋即那一直放在袖中的手掌，轻轻的伸了出来。
在其后方，一名坐得最近的老者见状眼中顿时一抹寒光涌过，轻轻点头接着站起身来，冷声喝道：“慢着！”
这道喝声，瞬间便是令得广场上的气氛一滞，无数人心头猛跳起来，终于要开始了吗？
“大长老，你为何阻拦殿主祭祀？！”那两名黑袍老者见状，眼神顿时一沉，厉声喝道。
那被称为大长老的老者，面色阴寒的看了一眼两人，而后停留在青檀身上，淡淡的道：“按照殿规，唯有受到长老团全票通过的殿主方才有资格祭祀先祖，不过我们可并不认为如今这新殿主，通过了这项决议。”
那两名黑袍老者面色一变，看向青檀。
青檀冷冽的眸子盯着那大长老，而后冰冷的声音，响彻而起：“大长老，师傅坐化前，已是亲自将殿主之位传于我，这项命令，按照规矩，也将超越长老团。”
“你说老殿主亲自将殿主之位传于你，可却只是你一面之词，按照老夫得来的消息，倒是你趁着老殿主冲击轮回境时，偷袭出手，害其突破失败，而最终被你夺走黑暗祖符，试图掌控我黑暗之殿！”大长老冷笑道。
“你这丫头，年龄不大，却是心狠手辣，连这等欺师灭祖的事情都做得出来，若是殿主之位落在你手，恐怕我黑暗之殿列祖列宗都会死不瞑目！”
“大长老，休得对殿主不敬，若不是殿主亲自所传，你真当黑暗祖符能够轻易传人？你也真当殿主的传承能够被人强夺不成？！”一名黑袍老者厉声道。
“哼，天下谁人不知想要获得传承，必须要传承者心甘情愿方才有可能成功，你在此处试图诬蔑殿主，实乃大罪！”另外一位黑袍老者也是沉声道。
“老殿主对这丫头信任有加，谁知道她是否施展了什么诡计强夺了黑暗祖符与传承。”
那大长老袖袍一挥，旋即目光一转，道：“再说，想要成为一殿之主，你也应该知道，必须手持我黑暗之殿的两大圣物吧？”
青檀眸子冰冷，旋即她眉心处，有着一道古老的符文缓缓的浮现，而随着这道符文的出现，这片天地，竟是逐渐的变得黑暗下来，一股无法言明的古老波动，荡漾而开。
那是黑暗祖符的波动。
大长老感受着这股波动，眼中掠过一抹贪婪之色，旋即冷笑道：“黑暗祖符是其一，还有呢？！”
青檀那如玉般的皓腕，自袖中探出，玉手轻轻一握，只见得黑光在其手中逐渐的凝聚，最后化为一柄通体漆黑，其上布满着复杂玄奥符文的黑色巨镰，这黑色巨镰一出现，不少人都是感觉到一种寒气袭来，那番模样，仿佛那黑镰是催命死神一般。
“这便是黑暗之殿第二圣物，远古神物榜上排名第五的黑暗圣镰，大长老，你现在，还有什么话要说？！”青檀手握黑镰，俏脸冰寒，在那漫天黑暗之下，犹如黑暗之中的死神。
大长老望着青檀手中的那黑色巨镰，老脸也是微微一抖，接着他的目光，却是看向了前面的那两名老人。
在他的目光注视下，那两名老人也是缓缓的睁开紧闭的双目，目光漠然的看着青檀，旋即那没有多少情感的声音，在这天地间，回荡而起。
“小丫头，你确定你手中的黑暗圣镰是完整的么？”


第1231章 相见
“小丫头，你确定你手中的黑暗圣镰是完整的么？”
淡漠的声音，缓缓的飘荡在这黑暗广场上空，却是让得无数人心头剧烈的跳了跳，不少人看向那两道苍老身影的目光中有些恐惧的色彩，想来隐约的已是感觉到了他们的恐怖。
“两位太长老，你们此话是何意？”
青檀眸子也是在此时微微一凝，她纤细小手紧紧的握着手中那冰凉的黑色巨镰，冷声道：“莫非你们认为这黑暗圣镰是假的不成？”
两名太长老缓缓抬起那满是皱纹沟壑的苍老面庞，眼神漠然的盯着青檀，淡淡的道：“黑暗圣镰的确是真的，不过，它却并不是完整的，所以，你也不算是完全的获得了我黑暗之殿的两大圣物。”
“哦？那我倒是要听听，它是个怎么样的不完整法？师傅在将它交到我手上时，可并未说起过此事。”青檀冷笑道。
“这件事，莫说是你，就算是你师傅，他同样也知之不深。”
居左的那位太长老摇了摇头，道。
“胡说！”青檀神色一沉。
“论起辈分，就算是你师傅都得叫我二人一声师叔，这黑暗之殿的秘密，有些他不知道的，有何奇怪？”
两位太长老讽刺的一笑，道：“黑暗圣镰，自古以来便是分为两部分，镰身与镰灵，而如今你手中的黑暗圣镰，仅有镰身，却并不具备镰灵，怎能说是完整的圣物？”
青檀俏脸冰寒，道：“胡说八道，虽然你二人在我黑暗之殿地位尊贵，但辱及圣物，照样可治罪！”
“真是好大的口气，即便你师傅尚在，也不敢对老夫二人如何，你这小丫头，倒的确是有些不识礼数，难怪诸多长老反对于你！”
那两位太长老面色一沉，冷笑道：“黑暗圣镰第一任主人，乃是黑暗之主，不过此神物在当年远古大战时，遭受重创，镰灵被毁，最后落至我们黑暗之殿第一任殿主手中，他穷极一生，最终想出修复之法，在其坐化之际，以身祭刀，再度铸就镰灵，这才令得黑暗圣镰彻底完整。”
“不过后来因为一些变故，镰灵却是无故失踪，说起来这还是你师傅的过错，只不过念在他对我黑暗之殿有功的份上，老夫二人也并未多说什么，但如今，这等错误却是不能再继续让你保持下去。”
青檀玉手紧握着黑暗圣镰，眼神深处，杀意闪烁。
“两位太长老，这种事情我们同样未曾听说，可并不具备多少说服力。”那两名黑袍老者也是沉声说道，他们乃是黑暗裁判所的长老，只不过却是忠于裁判长，与长老团截然不同。
“要判明这黑暗圣镰究竟是否完整其实也并不难，不过就怕这小丫头不敢试。”一名太长老淡淡的道。
青檀眼中寒意涌动，这般时候，她已被逼得还有其他路子可走么？当即她银牙轻咬，道：“那就请太长老明说吧。”
“那判明之物，正是这座祖碑，大家应该也知道，这祖碑有着第一任殿主所留的气息，而黑暗圣镰镰灵便是第一任殿主坐化所化，只要你用黑暗圣镰劈砍祖碑，若这黑暗圣镰乃是完整之物，必然能够在祖碑之上留下痕迹，若是不然，则是毫无作用。”
太长老淡漠一笑，道：“虽说这般举止有些冒犯先祖，不过为了黑暗之殿的安宁，想来先祖们也不会怪罪，怎样？你可敢？”
无数道目光射向祭坛之上的青檀，想要等待着她的回答。
“殿主不可中计。”那两名黑袍老者急声道。
青檀微闭着双眼，却是轻轻摇头，道：“现在，我们还有其他的选择吗？”
两名黑袍老者默然，随着这两位太长老的出面，他们在黑暗之殿中已没有太大的优势，若是拒绝的话，反而让得长老团有了借口。
“好，就依两位太长老！”
青檀猛的睁开双眼，眸子中掠过一抹果断之色，旋即其小手一握黑暗圣镰，娇躯一动，直接是出现在那祖碑之前，而后一声娇叱，镰刀已是划破空间，快若闪电般劈砍在那祖碑之上。
铛！
清脆的金铁之声，在广场之上响彻而起，无数道目光望向祖碑，然后他们瞳孔便是一凝，因为在那镰刀所落下处，那祖碑竟是未能留下丝毫的痕迹。
哗。
一片震动的哗然之声传荡而开。
青檀跃身而回，她望着那光滑无痕的祖碑，俏脸上也是掠过一抹细微的苍白，贝齿紧咬着嘴唇，一丝血迹在嘴角渗开。
“小丫头，你还有什么话好说？”那两位太长老淡淡一笑，道。
青檀压抑着心中的动荡，眼珠一转，俏脸平淡下来，道：“看来两位太长老所说的确属实，原来这黑暗圣镰果真不算完整。”
那大长老闻言，顿时一喜，然而还不待他借此说话，青檀便是接着道：“从此以后，这寻找镰灵，也将会是我的任务，若是不将其寻回，恐怕师傅也难以瞑目。”
“哼，这丫头倒是会打蛇上棍！”大长老心头恼怒，暗哼一声，他倒是没料到青檀眨眼便是将这等事给淡化下去。
“这寻找镰灵，将会是我们黑暗之殿的任务，不过眼下你两大圣物未能聚齐，这殿主之位，也不能算做完整。”那两名太长老倒是面色淡漠，紧追不舍。
青檀眼神逐渐的冰冷下来，她眉心处，那古老的符文愈发的清晰，这片天地，也是愈发的黑暗：“两位太长老，那不知道你们认为我这黑暗祖符，又究竟是否完整？”
她的声音之中，开始多出了一些不加掩饰的冰冷与杀意，显然这两位太长老的咄咄逼人，也是激出了她的怒火。
那两位太长老感受着天地间荡漾的那种古老波动，身体也是微微紧绷，眼中掠过一抹冷色，道：“你这莫非是想要借助黑暗祖符强压老夫二人了？”
“黑暗祖符才是我殿真正殿主信物，两位太长老，我敬你们身份尊贵，方才诸多礼让，但若你们咄咄逼人，或许青檀也只有请出黑暗祖符，清扫本殿了！”青檀冰冷的声音之中，有着血腥之气弥漫出来，此时的她，也是被逼得展现出强硬一面。
两位太长老身体微微前倾，两股恐怖气息也是缓缓的蔓延出来，竟是将那来自黑暗祖符的波动生生的抵御了下来。
这片天地的气氛，瞬间便是剑拔弩张起来“呵呵，你师傅倒的确是收了一个很不错的弟子…”气氛凝固间，一位太长老突然一笑，他目光微闪道：“其实要我们答应你坐这殿主之位也未尝不可，只是却希望你能答应一个条件。”
青檀俏脸神色不变，那紧绷的身体也是略作松缓，这两个老不死的实力恐怖异常，若是动起手来，即便她有着黑暗祖符恐怕也难以占得上风而且那撕破脸皮动手，显然是下下之策。
“太长老但说无妨。”青檀心中念头闪动，而后淡淡的道。
“倒也无他，只是我有一徒孙，对你倒是爱慕得很，若你能够嫁于他，你这殿主之位，我二人倒不再多说。”
那太长老笑了笑，旋即其手掌一挥那长老团中，一名黑衣俊朗男子快步走出，他目光灼热的盯着祭坛之上的青檀，诚声道：“青檀，你也知道我有多喜欢你，若是你能嫁给我，老祖必然不会阻扰你登上殿主之位！”
“太长老，这？”那大长老见到这一幕，却是一愣，连忙低声道。
那两位太长老摆了摆手，只是眼芒闪烁，也不知道是在想什么。
整片黑暗广场都是寂静无声，无数人望着祭坛上的青檀，眼下这幕，倒是出乎了所有人的意料，只是不知道后者究竟是否会为了殿主的位置，来委身下嫁。
祭坛上，青檀小手紧紧的握着黑暗圣镰，白皙娇嫩的手背之上，有着细小的青筋浮现出来，她娇躯微微颤抖着，许久后，深深的吸了一口气，那眸子之中，有着浓浓怒火以及血腥之气涌出来：“裁判军，何在？！”
“在！”
祭坛四周，猛的爆发出整齐低吼声，那成千上万道的黑衣人影猛的站起身来。
“贼子逼宫，试图篡夺殿主之位，扰乱殿规，按律该当如何？！”
“杀！”
整齐如一的喝声，携带着滔天杀气，弥漫了整座广场。
“你好大的胆子！”两位太长老猛然变色，霍然起身，厉声喝道。
“既然你们咄咄逼人，那本殿今日，便要与你们试试，看看这黑暗祖符，能否制裁你们！”青檀同样回以厉喝，旋即她玉手一握，黑暗之光猛的自其体内席卷而出，整片天地，彻底的化为黑暗。
“你这臭丫头，真是不自量力，既然我二人敢出面，你还以为这黑暗祖符能对我们形成压制不成？！”那两位太长老冷笑出声，旋即两人身形一动，直接是出现在半空上，而后双手猛的结出奇异印法，两道光束，直接是射在了那祖碑之上。
“后辈不孝，还望先祖之灵，压制祖符！”
两位太长老齐声大喝，而后那祖碑竟是嗡鸣震动起来，紧接着一道黑暗光华冲天而起，而青檀的俏脸则是在此时微微苍白起来，因为她感觉到体内黑暗祖符的力量，仿佛是在此时被压制了一般。
“先祖之灵？！”
这般变故让得青檀一惊，这才猛的明白过来，为何这些家伙一直要等到祭典开始方才对她出手，原来，他们是有着办法借助祖碑之内的先祖之灵来压制她的黑暗祖符！
“小丫头，你这点能耐，也想与老夫二人来斗，不自量力！”
见到黑暗祖符被成功压制，那两位太长老顿时冷笑出声，而后眼神一寒，两人手指凌空点出，两道弥漫着浓浓轮回波动的黑色光束，瞬间撕裂天际，快若闪电般的对着祭坛之上的青檀暴射而去。
两人联手，青檀顿时察觉到危险气息，银牙一咬，小手紧握着黑暗圣镰，然后就欲催动全力相迎。
轰！
不过，就在她要出手时，这片天地，猛的有着惊雷之声响彻而起，众人一惊，抬起头来，只见得天空不知何时雷云密布，一道数千丈庞大的雷龙直接是自天空降落而下，瞬间便是将那两道弥漫着轮回波动的黑色光束震碎而去，而后雷龙咆哮，狠狠的对着那两位太长老冲了故去。
那两位太长老也是因此一惊，恐怖气息涌动，一掌拍出，与那雷龙硬憾在一起。
咚！
雷芒铺天盖地的暴射而开，那两名太长老竟是被生生的震退了上百步，面色皆是一变，厉声喝道：“谁？！”
轰隆。
天空上，又是一道雷霆落下，旋即在青檀吃惊的目光在落在了她那前方，而后雷光缓缓的凝聚，最后在那无数道震惊的目光中，化为了一道人影。
“保护殿主！”
那两名黑袍老者急忙掠至青檀前方，目露戒备的盯着那道身影，下方数百道黑影也是瞬间掠来，将青檀团团包围。
雷光在那道道目光的注视下，逐渐的散去，一道削瘦的身影，也是出现在了所有人的注视之中。
那道身影一出现，缓缓的转过身来，然后便是望向了那道突然间浑身僵硬的青檀，微微一笑。
青檀的神情，在此时一点点的呆滞，她甚至是连手中的黑暗圣镰都是握不住，刀身一颤，便是哐当一声倒落下地，她呆呆的望着那道人影，那原本冷冽的大眼睛中，突然有着水花一点点的涌了出来。
她轻轻的拨开挡在面前的那些护卫，颤颤巍巍的走出来，然后步伐猛的加快，眼中水花犹如崩堤一般，疯狂的涌了出来。
哇。
最终她狠狠的扑到了那道削瘦身影身体上，哇的一声便是那些裁判军惊愕的目光中大哭了出来，她抱着那道身影，眼泪疯狂的流着，那撕心裂肺的哭声，从祭坛上传出，最后在这广场上空，不断的回荡着。
在这道身影面前，她不是什么黑暗之殿的殿主，她永远都是那个只知道躲在他身后的小跟屁虫，也只有在他的面前，她才会哭得这么肆无忌惮，仿佛再大的委屈，他都能够为她承受与包容着。
林动望着那哭得摇摇欲坠的女孩，鼻子也是忍不住的一酸，伸出手来，将女孩那柔软的身子紧紧的搂着，一如许多年前在那青阳镇时护着身后小女孩的少年。
“乖，不哭，有哥在，谁也欺负不了你。”


第1232章 显威
青檀那撕心裂肺般的哭声，从祭坛上传开，最后回荡在这片黑暗广场上空，却是让得无数人都是呆愣了下来，特别是那些黑暗裁判所的强者，自从青檀进入黑暗裁判所以来，他们所见到的，是一个冷漠得近乎没有多少情感的少女，在她那白皙小手上，不知道染上了多少的血腥，而最终，她在这短短三年的时间中，从黑暗裁判所的一位普通弟子，成为了拥有着最高职权的裁判长，对于她，就连是那两位同样在黑暗之殿拥有着极高身份的副裁判长，都是忽略了她的年龄，对其颇为的尊崇。
然而，见惯了青檀那血腥手段后，如今，他们却是第一次见到，她竟然会有着这般脆弱的一面，眼前这一幕，她完完全全没有了以往的手段与气魄，而是真正的成为了一个与之年龄相符的女孩…
那些长老团的长老，也是目露惊疑的望着这一幕，天空上那两名太长老，则是眼神略显阴沉的盯着那一道削瘦的身影。
青檀的哭声，一直的持续着，仿佛她是要将那压抑了三年的苦与委屈尽数的哭出来一般，到得后来，林动望着那湿透的胸口，也是一阵无奈，手掌揉了揉怀中女孩的小脑袋，道：“喂，还没哭够啊？你现在可是黑暗之殿的殿主呢。”
怀中的女孩轻轻的抽泣着，这才抬起那通红的大眼睛，她望着那张成熟而熟悉的脸庞抽了抽挺翘的玉鼻，道：“林动哥…我就知道，你一定不会有事的。”
“我倒是没事，不过恐怕你会有点事。”林动淡淡的瞥了她一眼，神色略显不善。
青檀与他从小长到大对他性子自然是极端的了解，见到他这模样便是明白，大眼睛骨碌一转，小脸上顿时浮现委屈之色，纤细双手抱着林动不肯松开。
“少来这套，等将这里的麻烦解决掉，待会再收拾你。”
林动冷哼一声道。
听得此话，青檀也是回过神来，那眸子中冷光掠过玉手一握，那黑暗圣镰便是掠至她手中，旋即她松开林动，冰寒的眸子望向了远处天空上的两位太长老，道：“林动哥，这次就让我来解决掉他们。”
“呜。”
奈何她话刚刚落下林动的手掌便是捏上了她的脸，狠狠的一扯，没好气的道：“给我闪开点，有我在的时候，你就别想着动手。”
青檀被林动一番蹂躏，小脸顿时皱了起来这让得后面那些黑暗裁判所的强者眼皮微微跳了跳，想来这一幕对于他们的冲击实在是太大了一些。
“他们很厉害的。”青檀道，这两个太长老实力比起那元门的三巨头更为的恐怖…
“现在你哥更厉害。”林动一笑，轻拍了拍青檀的脑袋，那神色却是逐渐的变冷下来，先前这两个老不死的东西对青檀那咄咄逼人的行止，他可是清楚的看在眼中，一想到青檀这小丫头被他们如此相待，他心中便是忍不住的有着杀意涌起来在家里除了爹娘能够教训这个小丫头外，就连他都舍不得喝骂欺负，眼下这两个来不死的东西，又算哪根葱？
见到林动的面色，青檀心中也是有着一些甜甜之意涌出来，而后乖乖的点了点头。
“这位朋友不知是何方高人？为何要插手我黑暗之殿的事？”此时天空上那两名太长老在见到林动与青檀的亲昵举动后，眉头也是一皱，沉声喝道。
林动缓缓的转过身，面色淡漠的望向那天空上的两位太长老，道：“先前你们欺负人欺负得很爽是吧？”
“这位朋友，这是我们黑暗之殿内部之事。”
“青檀是我妹妹。”林动脸庞上掀起一抹犹如刀锋般凌厉的笑容。
两位太长老面色一滞，神色愈发的阴沉下来，谁都没想到，这个看上去孤单一人的小丫头，竟然还有着这么厉害的一个大哥撑腰，为何以前从没听说过？
黑暗广场上，那无数北玄域的强者也是面面相觑，原来这新任殿主还有如此强横的大哥，看先前这两位太长老对青檀的所作所为，恐怕她这位大哥，是绝不会善罢甘休的了。
“就算你是她大哥，但眼下的事，也需要她给我们黑暗之殿千万殿徒一个交代才行。”两位太长老冷声道。
“交代？”
林动似是笑了一下，那眼眸之中，杀意如潮水般涌出来：“将你们两个老不死的东西解决了，就不需要什么交代了吧？”
眼下这黑暗之殿的情况，他看得倒是分外的明了，不过便是这两个老家伙倚老卖老，仗势欺人，从而也是令得黑暗之殿诸多人不敢发言，只要将他们解决掉，这些事情，自然迎刃而解。
“哼，狂妄的小子，想要解决掉老夫二人，那你就来试试！”
那两位太长老听得此话，眼中也是寒意闪烁，虽然他们能够察觉到林动的难缠，但他们也并没有多少忌惮，轮回境的实力，足以让得他们傲视群雄，他们可不相信，眼前这看上去比青檀大不了多少的小子了真有着抗衡他们的能耐！
“是吗？”
林动也是咧嘴一笑，笑容之中，却是有着狰狞弥漫出来，而后其身形一动，直接是出现在天空之上，双手一握，只见得天空之上，雷云猛的汇聚而来，惊人的雷霆在其中疯狂的凝聚而来。
整片天地的元力，都是在此时剧烈的波动起来。
“轰！”
那两位太长老见状，体内气息瞬间爆发而开，滔天元力席卷，元力之中有着浓浓的轮回波动散发出来，令人感到畏惧。
“大黑暗掌！”
两人手印变幻，而后暴喝出声，竟是同时抢先出手，浩瀚元力化为一道黑暗无比的巨掌震碎虚空，狠狠的对着林动怒拍而去。
林动眼瞳之中，雷芒闪烁，一步跨出，只见得无数雷霆顿时疯狂的呼啸而下，最后夹杂着浓浓天威，重重的劈在那两道黑暗巨掌之上雷芒闪烁间，狂暴的雷霆之力，直接是生生的将那两道黑暗巨掌震碎而去。
唰！
黑暗巨掌崩溃林动身形猛的化为电光掠出，一拳轰出，惊雷阵阵，一道数千丈庞大的雷龙猛的咆哮着冲出。
“黑暗之盾！”
两名太长老厉喝出声，只见得元力疯狂在前方凝聚，化为一道千丈庞大的黑暗巨盾盾面之上，有着轮回波动流转。
轰隆！
雷龙狠狠的冲击在那黑暗之盾上，雷光暴溢间，一股股狂暴到极点的力量，疯狂的侵蚀而进，瞬间便是令得那黑暗之盾上面布满着裂缝最后砰的一声，彻彻底底的爆炸开来。
咻！
在黑暗之盾爆炸的瞬间，那两名太长老瞳孔便是一缩，因为他们见到，一道身影，已是鬼魅般的出现在他们面前，紫金光芒冲天而起，数千道紫金龙纹仰天长啸，古老的龙吟之声响彻天宇。
吼！
紫金龙爪瞬间撕裂空间，带着巨大的阴影，狠狠的拍向那面色有些变化的两位太长老。
阴影笼罩而来，可怕的力量直接是令得空间呈现剧烈的扭曲之感，而后那两位太长老也是暴喝出声，双掌挥出，掌心有着极端浓郁的轮回波动凝聚，这令得他们的手掌，都是呈现一种莹白之色。
咚！
两人双掌与龙爪凶狠硬憾，一道飓风直接是在天空上成形，而后众人便是骇然的见到，那太长老二人，竟是被齐齐的震退上千丈。
一阵惊天般的哗然声响彻而起，那些看向林动的目光有着浓浓的惊骇涌出来，谁能想到，后者以一敌二，竟然还能够占据到如此明显的上风，这家伙，究竟是有着多么的恐怖？
那些长老团的长老，也是面色剧变的望着这一幕，那眼中有着浓浓的不安涌出来。
祭坛之上，那两名黑袍老者也是颇为震动，面面相觑，没想到殿主的大哥，竟然厉害到这种程度…
青檀小脸上同样是布满着欣喜，看来这三年，林动哥真的变强了很多啊。
天空上，林动倒是面色淡漠的望着被震退的两位太长老，这两人的实力，仅仅只是普通转轮境，真要论起战斗力，恐怕还不及当日融入那大天邪魔后的元门三巨头，以他现在的实力，要收拾他们倒不算太大的麻烦。
林动仲出手掌，遥遥的对着两人，轻轻一握。
嗤！
随着他手掌的握下，只见得那两位太长老周身空间瞬间扭曲，而后有着四道巨大的黑洞漩涡凭空自他们四周浮现出来。
嗡。
黑洞漩涡猛然旋转起来，恐怖的吞噬之力爆发而出，形成一股可怕的撕扯力，连那空间都是在此时被生生的撕裂而去。
那两位太长老也是察觉到那种撕扯力的恐怖，当即急忙爆发全力，疯狂的抵御着那种撕扯力的侵蚀，而后面色涨红，一声暴喝，终是自那黑洞囚牢之中突破而出。
轰隆！
不过就在他们刚刚冲出黑洞囚牢时，巨大的惊雷声响彻，而后整片天地仿佛都是在此时变得耀眼起来，天空之上，一道数千丈庞大的雷霆光柱，犹如审判之光一般轰然落下，最后快若闪电般的狠狠冲击在两人身体之上。
嘭！
两道黑影冒着黑烟笔直的从天空坠落而下，最后在那无数道目瞪口呆的目光中，落至那黑暗广场上，直接是将那坚硬无比的广场，都是砸出了一个数千丈庞大的深深沟壑。
而在那沟壑之中，两道狼狈身影勉强的站立着，摇摇晃晃间，一口鲜血终是忍不住的喷了出来。
见到这一幕，那些长老团的长老，眼前顿时忍不住的一黑，一股恐惧之情，疯狂的涌上心头，他们最强大的底牌，竟然便是这样的失败了？


第1233章 镰灵
嗡嗡。
整座黑暗广场，都是在此时爆发出巨大的嗡鸣之声，那些来自北玄域各方势力的首脑目瞪口呆的望着这一幕，脸庞上有着浓浓的骇然之色。
这才多长的时间，那两位实力达到轮回境的太长老，竟然便是落败了？
“怎么可能？！”
无数人心中在翻滚着这一句话，一道道震撼的目光望着天空上那道在漫天雷霆的衬托下，犹如雷神一般的林动，这突然冒出来的家伙究竟是何方神圣啊，竟然…恐怖到这种程度。
天空上，林动面色漠然的望着下方巨坑之中的两人，淡淡的道：“现在，刚才那句话，我有资格再说一次了吗？”
这一次，满场鸦雀无声，就连那些长老团的长老都是紧闭起了嘴巴，除了一些对青檀反对得最凶的长老之外，其余一些人目光显然是有了一些闪烁，眼下这不知道从哪里冒出来的青檀大哥，实力显然是极为的恐怖，若是惹恼了他，这里恐怕没什么人能够将其阻拦。
祭坛上，那些黑暗裁判所的强者倒是眼露喜色，这两位太长老便是长老团的最终底牌，若是连他们都不起作用，恐怕长老团也将再不敢与殿主相抗衡。
巨坑之中，那两名太长老一口鲜血喷出，苍老的脸庞都是变得苍白了许多，显然在面对着林动那种凶狠攻势时，他们同样是受了不轻的伤。
“你！”
那两位太长老咬着牙，却是看向祭坛之上的青檀，厉声道：“这是我黑暗之殿的内事，你竟然还要假手他人，这殿主之位，如何服人？！”
青檀闻言，却是嗤笑一声，道：“两位太长老，你们这话倒是说得真好，只是在先前威压我时，怎么不记得你们自己的身份？你们也知道辈分比我师傅都要高，但先前所作所为，又算得了什么？”
青檀毕竟也不再是当年那娇憨的小丫头，这番犀利言辞一落，不仅令得那两位太长老面色一白，就连广场中无数保持着中立的各方首脑都是暗暗点头，刚才你们倚老卖老的欺负人时为何不觉得羞耻，眼下吃了亏，却是利马拿这个说事，真当旁人都是瞎子傻子么？
“本来对于黑暗之殿的事，我并没理会的心情，不过青檀先是我妹妹，再之后才是你们这黑暗之殿的殿主，既然你们欺负了她，那我这当大哥的，自然要为她讨回公道，今天这事，恐怕没这么容易平下来。”林动眼神冷厉的盯着那两位太长老，平淡的声音中，却是杀意涌动。
林动此话，已是摆明了他今日不会善罢甘休，这不由让得那两位太长老眼神也是微微一变，在与林动交过手后，他们方才能够感觉到后者的厉害，若是正面相斗的话，恐怕他们两人联手，都不会是林动的对手。
那些长老团的长老，同样是面色有些苍白，如果今日这两大太长老被解决，那他们也将会失去最大的靠山，到时候以青檀的手段…光是想想他们身体都感到阵阵寒意。
“你也不要太猖狂了，真当我黑暗之殿这么好欺负不成？！”那两位太长老色厉内荏的喝道。
林动漠然的看着两人，却是不再说话，只是天空上，再度有着雷云缓缓的凝聚而来，一种狂暴的雷霆之力，迅速的汇聚。
“你！”这太长老见到林动软硬不吃，那眼中也是掠过一抹狠毒之色，旋即他们对视一眼，猛的一咬牙，喝道：“好，这是你自找的，你就真当老夫二人没治你之法吗？！”
话音一落，只见得他们其中一人手掌探出，掌心光芒凝聚，最后竟是化为一颗龙眼大小的黑色珠体，在那珠体之上，布满着一道道复杂的铭文。
林动见到这一幕，眼中也是掠过一抹讶色，在那黑色珠体上面，他察觉到一股极为奇特的波动。
黑色珠体自其手掌中冉冉升起，那的两位太长老眼中狠毒之色愈发浓郁，而后一咬舌尖，一口精血喷射而出，落在那黑色珠体之上。
“恭请先祖之灵！”
两人齐齐大喝出声，那黑色珠体顿时爆发出万丈黑芒，而后黑芒逐渐的凝聚，最后竟是化为一道淡淡的光影。
光影身体欣长，面目如玉，只不过那眼中却并没有太多的灵智，但即便如此，即便并不妨碍那从他体内散发而出的恐怖波动，那种波动，既然是连那两位轮回境的太长老都是无法相比！
这道光影的实力，竟是达到了渡过一次轮回劫的巅峰层次！
林动的面色，也是在此时微微一凝，眼中掠过一抹惊色，这黑暗之殿果然是拥有着隐藏手段，难怪能够将黑暗祖符一直的护卫着。
“先祖之灵？！”
随着那一道光影的出现，这黑暗广场中顿时爆发出道道惊呼之声，那光影赫然是这黑暗之殿第一任殿主！
“怎么会这样？他们怎么能召唤出先祖之灵？！”那两名黑袍老者也是惊呼道，眼中满是震骇之“嗡嗡。”
在那一道光影一出现，祭坛之上青檀手中的黑暗圣镰竟是剧烈的震动起来，青檀见状，脸颊微变，她能够感觉到，手中的黑暗圣镰似乎是与那光影有着一种极端亲密的联系。
青檀小手紧紧的抓着黑暗圣镰，眸中神色变幻，片刻后似是明白了什么，顿时惊声道：“这便是黑暗圣镰的镰灵？”
听得青檀惊呼，那两名太长老面色也是变了一变。
“原来如此…怪不得师傅从未说起过黑暗圣镰镰灵之事，原来是有人暗中将镰灵偷走，你们真是好大的胆子，竟然敢私自潜藏圣物之灵，这可是我黑暗之殿的大罪，足以剥夺你二人太长老的身份！”青檀俏脸冰寒，冷声喝道。
黑暗广场中，也是爆发出一些骚动之声，不少黑暗之殿的高层面露震动之色，那黑暗圣镰的镰灵，原来是被这两位太长老给偷走了，难怪他们信誓旦旦的说那黑暗圣镰并不完整…
“哼，这里哪轮到你来胡说八道！”
两位太长老眼中寒芒一闪，心神一动，那道光影一闪之下，便是出现在了林动前方，而后光虹涌动，带起惊天之力席卷向林动。
而在那光影缠住林动时，两位太长老已是如同鬼魅般的冲向青檀，看这模样，竟是想要先将后者擒住，想来到时候林动也得投鼠忌器。
“哼。”
青檀见到两人冲她而来，眸中也是掠过一抹冷光，小手紧握黑暗圣镰，眉心处那古老符文愈发清晰，而后镰刀唰的一声掠出，天地仿佛都是黑暗了一瞬。
铛！
一名太长老率先而至，浩瀚元力奔涌，掌心之间弥漫着轮回波动，竟直接是凭借着肉掌硬生生的与那锋利镰刀硬碰在一起。
火花暴射间，空间都是呈现一种扭曲之感，而后青檀与那太长老皆是后退数十步，不过还不待她稳下体内波动，头顶之上，又是一道光影闪掠而来，另外一名太长老，也是凶悍出手，凌厉攻势当头劈来。
青檀见状，眸中冷光闪烁，不过就在她要再度出手时，一道白色倩影突然出现在其身前，青锋长剑嗡鸣震动，一道并不起眼的剑虹暴掠而出，与那太长老攻势硬憾在一起。
嗤！
剑芒掠过，却是有着血花溅射，那太长老顿时一声惨叫，身形狼狈的倒射而退，手掌之上，鲜血淋漓，然后他怒目望去，只见得一道白裙绝色倩影，静立在青檀面前，手中三尺青锋之上，还有着血迹滴落下来。
这出手之人，自然便是一直隐在旁边的绫清竹。
这番变故，又是引得无数道惊异目光射来，而当那些目光落到那白衣倩影之上时，眼中皆是掠过一抹惊艳之色。
此时那些黑暗裁判所的强者方才回过神来，急忙簇拥而上，将青檀重重护住。
青檀也是讶异的看了一眼那现身的绫清竹，后者那等气质以及薄纱下的美丽轮廓令得她微微一怔，然后看了一眼天空上的林动，小嘴轻轻的嘟了一下，看着绫清竹，道：“你是谁啊？”
绫清竹也是微微怔了一下，旋即道：“我叫绫清竹。”
青檀盯着她，嘀咕了一声，道：“你不会是林动哥的…”
虽然她话未曾说完，但绫清竹自然是听明白了过来，这般时候，绕是以她的清冷性子，那脸颊都是忍不住微红一下，然后静静的摇了摇青檀眼中依旧是有着一丝疑色，但终归是没再问下去，只是有些担忧的看向了天空，那里有着极端恐怖的波动散发出来，那道先祖之灵的实力，显然极为的可怕，也不知道林动哥能不能对付得了。
嘭！
天空上，惊人的能量波动席卷而来，林动退后了数步，他看了一眼下方发生的一幕，眼神则是变得阴沉了许多，这两个老杂毛，倒的确是不要脸皮了，既然趁这般时候对青檀出手。
在林动那阴沉的目光下，那两位太长老脸庞也是抖了抖，但旋即在见到那道镰灵时，这才稍稍安心，这镰灵说起来便是他们黑暗之殿的守护神，只要这东西在他们手中，想来这家伙也奈何他们不得。
林动似是知道他们心中所想，那黑眸之中，寒芒掠过，袖袍一挥，一道黑光掠出，最后化为一道手持长刀的黑影守护在青檀与绫清竹旁边，有着这具吞噬天尸守护，想来这两个老杂毛也再不敢偷袭出手。
“真以为凭借这东西，就能护得住你们吗？”
林动缓缓抬目，望着眼前那道镰灵，一声冷笑，刚欲动手，岩的声音，却是在此时突兀的在其心中响起，让得他微愣下来。
“嘿嘿，同为神物之灵，这小东西就交给我来吧。”


第1234章 手段
“你？”
林动听得岩这话也是陡然间愣了愣，旋即暗感好笑，自从岩苏醒以来，这家伙一直都是缩在他体内，至于正面战斗的什么事情，基本上极少主动出手，类似今儿这一遭，还真是头一次。
“你行吗？”林动在心中有点怀疑的问道，眼前这镰灵实力强横，足以和渡过一次轮回劫的轮回强者媲美，就连他想要将其解决也是极感棘手，如今的岩虽然恢复了许多，但貌似也没这等战斗力吧？
“嘁，我好歹也是远古神物榜上排名第二的神物老大好不？在那远古时期，我巅峰状态，这种渡过一次轮回劫的对手也并不算什么。”岩嗤笑一声，道。
林动心头一笑，对此他倒的确并不怀疑，从大荒芜碑在那远古时期镇压诸多异魔王的恐怖战绩来看，显然这些由符祖炼制出来的超级神物也是拥有着极端惊人的战斗力，只不过当年那场大战他们同样受到了极重的创伤，至今都是未能恢复回来。
“天地大战，这黑暗圣镰受创最重，当年黑暗之主手持此镰曾与异魔皇交过手，但最终她却是惨败，而黑暗圣镰的镰灵也是在那时候被异魔皇抹杀，从而导致黑暗圣镰威力不再。”岩缓缓的道。
林动听得眼神微微一凝，那黑暗之主竟然如此凶悍？竟然敢去与那异魔皇交手，这从那娇滴滴的表面上，倒还真是看不出来。
“如今这镰灵，应该是那第二任黑暗祖符的拥有者在坐化时以身祭刀所化，不过以他的能力显然不可能重铸镰灵，所以应该是这黑暗圣镰中本就有着镰灵的残迹存在，最后被他融合修复，这才锻造出了新的镰灵。”
岩对于这种情况显然是极端的了解，一眼便是看穿了端倪。
“不过眼下这镰灵灵智极浅，显然还处于幼生阶段，我要收它，简直是轻而易举。”岩笑道。
林动闻言，这才微微点头：“既然如此，那这东西，便交给你了在天空上的林动与岩交谈间，那下方的太长老二人，却是面色有些变幻的望着矗立在青檀面前的那吞噬天尸，从后者身体上，他们感觉到一种极端危险的波动。
“这个小子不知道究竟是什么来路，不仅本身实力恐怖，竟然还有着这么厉害的傀儡！”太长老二人对视一眼，皆是从对方眼中看出一抹不安，今日的事情，恐怕真没那么容易了结掉。
“现在只能期盼镰灵能够对付那林动了…”
两人心中一声暗叹，然后抬起头，眼神却是突然一凝：“那是？”
在他们目光所望处，只见得林动身体之上突然有着淡淡的白芒涌出来，而后白芒在其身前凝聚，也是化为了一道光影。
那道光影倒是与镰灵有些相似，不过却并未在其身体上感觉到类似镰灵的那种恐怖波动，这才令得他们微微放心。
“哼，这小子难道想要凭借这东西来对付镰灵么？真是不自量力。”两人一声冷哼，眼中却是有着一抹讥讽掠过。
此时这片黑暗广场无数道目光同样是汇聚在天空那两道光影上，谁都看得出来，那道镰灵就已经是两位太长老最后的手段，若是这般手段再失效的话，恐怕这些反对青檀的长老，就得倒大霉了。
岩闪现而出，他目光看了一眼前方那眼神有些空洞的镰灵，却是摇了摇头，有些嘘唏的道：“没想到如今的你，竟是会变成这样，不过这总比烟消云散来得好，给你足够的时间，总归能够彻底恢复的。”
镰灵空洞的目光看着岩，那没丝毫波动的眼中似是泛起了一些涟漪。
“不对？”
下方的两位太长老见状，眼神却是微微一变，手印急急变幻，便要操控镰灵出手。
“黑暗圣镰的镰灵，可不是你们这些货色能够操控的。”岩嘲讽的看了他们一眼，而后手指凌空点出，一道温和的白芒便是笔直掠出，直接是射进了镰灵眉心之中。
嗡嗡！
而随着那道温和白芒的射进，镰灵的身体突然微微的颤抖起来，道道白芒自其体内暴射出来，在这些白芒涌动间，只见得镰灵的胸口处，有着一道道黑色的纹路浮现出来。
这些黑色光纹，在那温和白芒的照耀下，以一种惊人的速度消融而去，那两位太长老的面色则是在此时剧变下来，因为那黑纹正是他们用来操控镰灵的咒文，但眼下，竟是直接被逼出了镰灵身体之中。
“糟了！”
两人面色惊悸，急忙要催动咒纹，但随着一道细微的咔嚓之声，那一道道符文便是尽数的消散而去，而在咒纹消散的霎那，他们立即便是感觉到，他们失去了对镰灵的操控权。
“怎么会这样…”
两人面色瞬间煞白下来，目光骇然的望着岩，他们实在是无法想象，后者怎么会如此容易的便是将他们的咒纹破解，那种能力，就算是那些渡过了轮回劫的巅峰强者都无法办到啊！
岩手印变幻，道道光印落到镰灵身体之上，而后他的身体爆发出璀璨光芒，光芒凝聚间，最后便是在无数道目光的注视下，再度化为了一颗黑色的珠子。
岩手掌一招，那黑色珠子便是落到他手中，然后抛向林动：“将镰灵放在黑暗圣镰中，这样才能让它逐渐的恢复。”
林动接过那黑色珠子，眼中还有些诧异，显然是没料到这镰灵如此容易便是被岩给解决掉了。
“没什么好奇怪的，在主人所炼制的这些超级神物中，我排首位，自然是有着压制它们的手段。”岩笑了笑，虽说他一直未曾展现过太过惊人的手段，但不论如何，他都是远古神物榜上排名第二的超级存在，小觑它的话，恐怕就只能是自己愚蠢了。
林动微笑着点了点头，然后屈指一弹，那黑色珠子便是掠向青檀：“接着。”
青檀见状，手中黑暗圣镰也是爆发出黑色光芒，光芒一卷，便是将那黑色珠子吸进镰身之中，而后那黑暗圣镰顿时嗡嗡的颤抖起来，一道道惊人的波动自其中爆发开来，那本就硕大的体型更是膨胀了一圈，再由娇小的青檀将其给握着，那一幕看上去倒是很有些奇特的美感。
而此时广场中的那些长老团的长老，则是面色煞白的望着这一幕，特别是那大长老，身体都是忍不住的颤抖起来，步伐踉跄的退后了两步。
整片广场都是在此时骚动起来，眼前这一幕代表着什么他们都很清楚，这些长老团的逼宫，显然是彻彻底底的败了。
祭坛之上，青檀小手紧握着黑暗圣镰，那俏脸却是变得冰寒下来，她手中镰刀轻挥，一道黑芒掠过，只见得半空中那座祖碑先是缓缓的消散而去，而随着祖碑的消散，那种对她体内黑暗祖符的压制也是彻彻底底的散去，整片天空，都是在此时再度的缓缓黑暗下来。
“两位太长老，你们私藏镰灵，如今可是知罪？！”
听得青檀那冰冷之声，那两位太长老身体也是一颤，刚欲说话，却是见到天空上的林动缓缓的落下，而后不远处那手持黑色长刀的恐怖傀儡也是飘然而至，手中黑刀，闪烁着令他们心寒的光芒。
一股毫不掩饰的杀意，从林动体内蔓延出来，显然只要接下来这两位太长老有丝毫的异动，就将会迎来雷霆般的攻击。
两位太长老面色变幻，那股浓浓的死亡之感涌上心头，终是让得他们眼中掠过一抹恐惧之色，而后挣扎了片刻，终是颤颤巍巍的道：“今日是之事，实乃老夫二人被蒙蔽心智，还望殿主从轻发落！”
“扑通！”
他们两人这话一出，那长老团处，众多长老立即便是跪了下来：“殿主，我等也是受大长老要挟，还望殿主从轻发落！”
那大长老闻言，面色顿时铁青下来，身体因为恐惧不断的颤抖起来。
场中局势变幻之快，让人目不暇接，谁能想到，之前还咄咄逼人的长老团，却是如此迅速的溃败下来。
那些来自北玄域的各方首领，也是心中暗叹，看向祭坛之上的那道倩影的目光，充满着敬畏，从此以后，她显然将会是黑暗之殿真正的殿主！
“殿主，此次的事情牵扯太大，若是尽数责罚的话，恐怕对我黑暗之殿大为不利。”那两名黑袍老者见到这一幕，低声说道。
那两名太长老显然也是明白他们对黑暗之殿的重要性，这才投降得极为干脆。
青檀微微点头，旋即其小手一挥，只见得数十道黑色光符闪现而出，最后掠过长空出现在那两位太长老以及众多长老面前，符文之上，有着浓郁的黑芒闪烁。
“诸位长老，今日你们逼宫已是死罪，但念在你们对本殿有功的份上，暂饶不死，但死罪可免，活罪难逃，这是我黑暗之殿的黑暗圣符，想要保下一条命，那便将其服下。”青檀冷冽的声音，在广场之上响彻着。
那些长老闻言，面色皆是一变，他们对这黑暗圣符可并不陌生，若一旦吞服，他们的性命便是彻底掌握在了青檀的手中，稍有反抗，恐怕便是得尸骨无存。
“只要你们日后对本殿忠心耿耿，本殿自然不会对你们动手，而这黑暗圣符也完全无用，本殿也犯不着平白的削弱我殿的实力…”
青檀眸子扫过那些犹豫的长老，淡淡的道：“怎么？看这模样，似乎诸位长老依旧心有反意啊“我等不敢！”
那些长老闻言，终是狠狠一咬牙，将那黑暗圣符吞进体内，不管如何，保得一条性命总比死了好，而且青檀也说得没错，他们是殿内精英，只要不心生歹意，青檀也不会对他们出手，这样的话，更是能够保证青檀不会秋后算账。
那两位太长老见状，最终也是一声长叹，将那黑暗圣符吞进了体内。
青檀见状，这才微微点头，那冷冽的目光却是看向了唯一一位没有获得黑暗圣符的大长老，冷声道：“至于这罪魁祸首，却是死罪难逃，诸位长老，还不将其擒下？”
“你！”
大长老面色一变，刚欲暴喝，却是感觉到周围众多长老目光不善，还不待他反抗，他们已是齐齐出手，磅礴元力，将其压制得动弹不得。
“将其送入黑暗裁判所，以殿规处置！”青檀道。
这话一出，那大长老眼中顿时涌现绝望之色，那些长老身体也是一颤，进了黑暗裁判所，那可真是连死都是一种奢侈了，这个时候他们方才有些庆幸，还好先前表态得快。
广场上，那些各方首领见到这一幕，心中皆是有些震动，算是真正的领略了一次这新任殿主的手段，短短数招下来，不仅保存了黑暗之殿的实力，而且还将力量尽数的控制在手中，最后这番狠辣手段，更是将那些心存侥幸的长老震慑一番，想来以后，再也不会有人敢犯今日之事。
这手段…真厉害啊。
天空上，林动却是面色平静的望着青檀将这残局完美收拾，但其眼中却不见丝毫的喜悦，反而是面无表情的收起吞噬天尸，然后落至那祭坛之上。
青檀见到林动面无表情的掠回，她对林动的性子几乎都是了解到了骨子里，自然是清楚此时后者在想什么，当即小手轻挥，将那些黑暗裁判所的强者尽数的遣退下去，仅仅只留下那两名低垂着头的黑袍老者。
将众人遣退，她这才微低着头走到林动身旁，伸出小手轻轻拉了拉他，而后者却只是淡淡的瞟了她一眼，冷哼了一声，道：“真是好手段啊，哪学来的？”
他是真有些恼了，虽然他明白以青檀想要坐稳这黑暗殿主之位，这些手段在所难免，但以大哥的身份来看，他却并不喜欢那个娇憨可爱的小丫头拥有着如此可怕的心机，因为有时候，玩弄手段，又何尝不是对自身的一种侵蚀？
青檀小嘴轻撅了撅，然后她抱着林动，把小脸埋在他怀中，柔弱的香肩轻轻抖动着，接着有着委屈的哽咽声音传出来：“我也不想带着那些面具啊，可是…你都不在了，都不管我了…只要能为你报仇，就算变成你不喜欢的模样，我也无所谓的。”
林动身体也是在此时微微一僵，旋即他望着怀中的女孩，一声长叹，这丫头，真是把他的死穴抓得准准的啊，这一句话下来，他还能责备她个什么？


第1235章 魔袭而来
黑暗之殿这番逼宫之乱，随着林动的及时出手以及青檀最后那番恩威并施的完美手段，终是这般未掀起太大涟漪便是被镇压了下去。
而且这番镇压，不仅未令得黑暗之殿伤筋动骨，更是令得青檀彻底的掌控了黑暗之殿中的所有力量，从此以后，她将会是黑暗之殿真正的殿主，当然，以黑暗之殿在北玄域的地位来看，要称如今的她是这北玄域至高无上的女皇那也未尝不可。
这种情况，显然是林动从未想象过的，他从没料到过，有一天那一直跟着他身旁的小跟屁虫，却是会拥有着这般让他愕然无语的成就。
不过这种所谓的成就，却让得他颇感无力，他原本想要的，只是一个依旧如同以往那般娇憨可爱，只要她成天快乐着就行的小丫头而已…
当然，这种无力如今已是出现，再没办法更改，林动也就只能接受着，特别是每当他要发怒时，那丫头便是立即委屈下来，一副任打任骂的模样让得他又好笑又好气，而最终的教训，也只能是不了了之。
但让得林动犹自松一口气的是，至少青檀这丫头在面对着她时，依旧是那股如同许多年前那般发自真心的情感，那种情感，纯粹得没有丝毫的杂质，而其实这才是让得林动最终任由这丫头装可怜糊弄过去的主要原因。
虽然他极为的疼爱她，但若她所学来的那种心计会用在亲近人身上时，林动那才会真正的感到失望，这是他最大的忌讳。
随着祭典的完毕，接下来的数天时间林动便是停留在黑暗之城，因为这黑暗之殿刚刚稳定下来，青檀显然是要做不少的善后之事，所以也没办法在这时候就强行把她给拎走。
眼下，就只能先等这丫头将事情处理完毕了。
这是一片幽静的竹林，林中有着竹屋两三间，碧绿之中透着清澈，倒是令人心旷神怡。
林动则斜躺在竹屋之前，嘴中叼着一片竹叶，温暖的阳光落在他的身上，令得他浑身都是懒洋洋的，他这般躺了一会，又是撑起一些身子，望着前面不远出，那里，一身白色衣裙的绝色女子手持三尺青锋，剑锋轻舞，窈窕曼妙的身姿犹如蝴蝶一般，有着一种惊心动魄的美感。
看一个女子舞剑，特别是当这位女子拥有着倾国倾城般的容颜时，那一幕显然是极为的赏心悦目。
他目不转睛的盯着那道美丽的身影，而后者仿佛也是有所察觉，那剑锋微微一震，纤细指尖轻弹，十数道竹叶犹如锋利剑芒般对着林动暴射而去。
后者纹丝不动，任由那些凌厉竹叶搽过身子，在那地面上射出深不见底的痕迹。
绫清竹收剑而立，莲步移来，而后看了林动一眼，忍不住无奈的摇了摇头，她所修炼诸多武学都是求个心静，而她对于自己的心境也相当自傲，即便是对着那千军万马，绝世强者，也难以打破她的心境，唯有眼前这个家伙，仅仅只是看一看，便是令得她无法静下心来。
这还修炼个什么？
林动望着绫清竹那眼神，也是忍不住一笑，道：“坐下来休息吧，你随我穿越千万大山来到北玄域，也挺累吧？”
绫清竹犹豫了一下，玉手轻拂，这才轻轻的坐在竹屋之前，而后她伸出纤细修长的玉手，挡着那从竹林缝隙中照耀下来的一缕阳光，阳光照在她玉手上，渗透下来的光芒，让得她那清冷的眸子缓缓的柔和了许多，旋即她微偏过头，望着林动那张慵懒的脸庞，贝齿轻咬着嘴唇，沉默了半晌，突然道：“你是因为我让你别再问有关“太上感应诀”的事，所以你才一直不问的吗？”
林动因为她这突然的话愣了一下，旋即他转过头望着绫清竹那有着薄纱遮掩的绝色容颜，而后者见到他目光直视过来，却是将视线偏移开去。
“你让我别再问，自然是有着你的理由。”
林动笑了一笑，然后他望着绫清竹因为他这句话微垂下的眸子，又是道：“而且，我也挺希望自己学不了的，有些东西…我怕还不起。”
绫清竹道：“你对我九天太清宫有着恩情，你要求学“太上感应诀”，那也是情理之中。”
“那换作其他人对你们九天太清宫有恩情，你也愿意教？”林动盯着绫清竹，道。
“你！”
绫清竹清冷眸子中陡然涌上一抹怒意，径直的站起身来，就欲拂袖而去，不过林动却是突然伸出手来，一把将其玉手抓住。
手掌相触，犹如完美的玉石，娇嫩柔滑，带着点点冰凉，令人爱不释手。
两人的身体，仿佛都是在此时僵硬了一下，绫清竹也是没想到他这般举动，先是一愣，旋即急忙要抽手而回，但随即便是被林动紧握着，再也挣扎不得。“你，你做什么！”这般时候，即便是以绫清竹的心境，都是猛的涌上一抹慌乱，急声道。
林动拉着她的手，倒并没有其他的举动，只是笑了笑，道：“那你还用什么恩情来搪塞我？”
绫清竹银牙轻咬了咬，手中传来的温度令得她身躯都是有点发软，旋即她偏过头来望着林动那带着笑容的脸庞，最终心叹一声罢了，也就不再挣扎。
阳光照耀下来，竹林中却是安静异常，两人一坐一站，手掌握着，光芒在两人身上犹如形成一圈光弧，那一幕，显得格外的宁静。
这一幕，不知道持续了多久，那远处，突然有着一些脚步声传来，而后青檀的声音也是由远而近：“林动哥！”
这番声音，瞬间打破了宁静，绫清竹急忙自林动手中将手抽回，然后侧身在一旁，不去看他，只是那脸颊上，似是有着绯红涌上来。
林动意犹未尽的笑笑，然后站起身来，远处一道倩影飞奔而来，最后脚尖一跃，便是犹如乳燕般冲进了他怀中。
在她后方，还跟随着一些黑暗之殿的强者，不过他们见到这一幕，却是赶忙低头，然后小心翼翼的退了出去。
林动望着那赖在怀中的黑裙女孩，手掌宠溺的摸摸她那小脑袋，道：“今天的事忙完了？”
“差不多了吧。”青檀伸着懒腰，将那玲珑有致的曲线尽数的展现出来，而后她抱着林动手臂，笑嘻嘻的道：“林动哥有没想我？”
“这才多大一会，想你干嘛？”林动好笑的摇摇头，道：“不过爹娘却是想你想得紧，你赶紧将事情安排妥当，跟我回东玄域见见爹娘“到时候回去了，爹肯定又会板着脸，娘说不定也会打我屁股，林动哥你可要护着我。”青檀犹豫了一下，有些惧怕的道，她当初离家出走可是还跟柳妍吵过架的…
“自作自受，咎由自取，自己回去领罚。”林动淡淡的看了她一眼，却是摇摇头。
见到林动说得坚决，青檀小脸顿时苦了起来。
一旁的绫清竹听得他们这般对话，也是略微莞尔。
“清竹姐，你在这里住得还好吧？”青檀也是看向绫清竹，娇笑着问道。
绫清竹闻言，也是微笑着点了点头。
林动望着两女笑谈，也并未插话，片刻后方才拉过青檀，道：“青檀，问你个事，你应该知道异魔吧？”
“异魔？，青檀一怔，然后点点头，道：“我如今也是黑暗祖符的掌控者，自然是知道异魔。”
“那你们这北玄域，可曾发现过什么异魔踪迹？”林动问道，异魔在东玄域搞出那么大的动静，不可能会在北玄域毫无动作吧？
青檀皱着柳眉想了想，然后摇了摇头，道：“我们黑暗之殿统率北玄域，这么多年，自然是与那些邪恶的异魔交过手，不过都算不得太过惊人的对碰。”
林动暗感讶异，莫非这北玄域，还真这么干净不成？以那“魔狱”滔天的手段，就连天妖貂族都能侵入进去，这黑暗之殿虽然也是极为的厉害，但也不见得能够令得那“魔狱”束手吧？莫非他们是忌惮黑暗祖符不成？
想不明白，林动只能摇摇头，将这事暂时的放下。
青檀因为忙完殿内事务，倒是留在了他们这，三人笑谈间，转瞬夜色便至，而这丫头似乎一到了林动身旁便是倍感慵懒，月光笼罩时，她便是趴在林动怀中甜甜睡去。
林动望着那犹如小猫般趴在他腿上睡去的青檀，也是微微一笑，然后小心翼翼的将她抱起，放进竹屋之中的床上，盖好被子，这才悄悄的退了出来。
月色中，绫清竹素雅而立，她望着走出来的林动，突然轻声道：“青檀她很喜欢你啊。
“她从小便是这样。”林动点了点头，道。
绫清竹看了他一眼，刚欲再说什么，她与林动神色突然微微一变，然后缓缓的抬头，只见得在那一轮明月下，一颗竹树之上，一道黑色身影，静静而立，一对黑色的眼睛，在月色下，显得分外的诡异。
林动望着这道身影，瞳孔却是微微一缩。
“呵呵，你便是四王殿所说的那个林动吧？”那道黑影望着林动，轻笑一声，道。
“不知道阁下又是魔狱之中的哪尊大人物？”林动冷笑道。
黑影微微抬头，月光下，有着一张布满着魔纹的脸庞浮现出来，而后他咧嘴一笑，森白的牙齿，弥漫着阴森的感觉。
“在下魔狱七王殿，正为阁下而来。”


第1236章 七王殿
“在下魔狱七王殿，正为阁下而来。”
当这句话自那道黑影嘴中传出时，林动的面色也是陡然间剧变下来，他的身体都是在此时紧绷起来。
以他对魔狱的了解，这个诡异而庞大的异魔组织，最为恐怖的人，应该便是这些以王殿命名的人，如当日出现在破碎空间之中的十王殿，那个家伙的实力，按照林动的推测，恐怕是堪比渡过两次轮回劫的巅峰强者，而眼下这位所谓的七王殿，恐怕实力也丝毫不会比那十王殿弱。
这还是林动第一次真正的亲自面对着这种魔狱之中的顶尖人物。
在东玄域所遇见的那四王殿虽然恐怖，但毕竟只是一道分身，无法给予林动太文的威胁，但眼下，这现身的七王殿，却是真正的本尊！
黑芒以及雷光缓缓的自林动眼中涌出来，元力精神力皆是奔涌而动，此时的林动，伊然是一副如临大敌的模样。
一旁的绫清竹也是美眸冰彻的盯着那七王殿，玉手之中的青锋长剑发出细微的嗡鸣之声。
“真是没想到，为了我这小人物，你们魔狱竟然能够出动这般大人物。”林动盯着那七王殿，缓缓的道。
“呵呵，阁下莫要自谦，你如今可不算什么小人物，即便是在我们魔狱必杀的名单中，你也算是名列前茅，所以让我出动，也是应该的。”七王殿含笑道。
林动冷笑一声，道：“不过你这种大人物出动，一旦动起手来必是惊天动地，你也不怕苏醒过来的雷主找上你？”
听得林动此话，那七王殿脸庞上的笑容显然也是微凝了一下，旋即他轻声道：“以往雷主便是在这北玄域沉睡，因此这里我们倒是极少前来不过如今他苏醒过来，却是离开了北玄域，所以若是能够尽快将你解决掉的话，想来他也无法感应。”
“是吗？！”林动眼中雷光黑芒愈发浓郁，一股浩瀚的波动，悄然的散发而开。
七王殿微笑着点点头而后他目光环视一圈，道：“不过这里倒并不是动手的好地方，所以怕是要请你随我们走一趟了。”
他话音一落，手掌挥下只见得周遭空间扭曲，竟是有着五道黑影诡异的浮现出来，这些黑影皆是令得林动瞳孔一缩，因为他发现，这五人，居然全部都是异魔王级别的实力！
显然，此次这魔狱为了对付他是真正的下了大手笔！
这五道黑影一出现，掌心中顿时有着黑芒暴射而出，而后黑光飞快的在半空交织，而后化为一道光阵，将林动二人笼罩进去。
“清竹退！”
林动见状，顿时一惊一把拉住绫清竹身形瞬间暴退，不过就在他们要退出那光阵笼罩范围时，周遭空间猛的剧烈扭曲起来。
“挪移！”
低喝之声，猛的自那五道黑影嘴中暴喝而出，而后空间扭曲到极致，光阵闪烁，那五道身影以及那位七王殿，林动绫清竹二人竟然便是这般诡异的凭空消失而去，唯有着那残留的空间波动，显示着先前此处所发生的事情。
嘭！
就在他们身形刚刚消失之际，那座竹屋之中，猛的爆发出惊天波动黑暗光芒席卷而出，整座竹屋都是瞬间化为虚无接着一道娇俏身影闪掠而出，她脚踏虚空，小脸布满着寒霜与杀意的望着那片扭曲的空间。
咻咻！
周围天地间，道道急促破风声响彻而起，几乎短短数个呼吸间，上千道身影便是掠至这竹林周围，他们望着半空中俏脸冰寒的青檀，急忙跪伏下来。
“你们这些废物，黑暗之城闯进如此之多的异魔竟然都无法察觉，留你们有何用？！”青檀玉手一握，那黑暗圣镰闪现出来，她眼神噙着怒火的望着周围那些黑暗之殿的强者，怒叱道。
感受着青檀体内那惊人的杀意，那些黑暗之殿的强者骇得面色发白，急忙求饶：“殿主息怒！”
“殿主。”
两名黑袍老者也是急忙掠来，他们在距青檀有些距离的地方停下，然后目光一扫这片扭曲的空间，道：“殿主，这些潜入异魔实力极端的恐怖，寻常强者根本无法将其察觉”
青檀银牙紧咬，那眼中有着极端冰寒的杀意涌出来，这番模样，哪还有先前赖在林动怀中睡觉的半分乖巧，伊然是一位真正手握生杀大权的北玄域女皇。
她深吸一口气，倒是未曾再将怒火发泄在这些手下身上，这些潜入的异魔的确极为的恐怖，甚至连她都是在先前一霎方才有所察觉，不过她也明白这是她大意了，因为只要到了林动身旁，她就会不自觉的收敛着这些年学来的谨慎，但哪料到，这次这般，却是害了林动。
“殿主，这空间波动甚是浓烈，不过我看应该不是远距传送，而且林动大人也非寻常人，长时间空间挪移必定被他打  破，所以，恐怕这空间挪移的地方，就在黑暗之城四周。”那两位黑袍老者倒是经验丰富，看了眼那扭曲的空间，沉吟道。
青檀眼中冷芒闪烁，旋即小手猛的一挥，沉声道：“吩咐下去，将城内执法军，黑暗裁判军尽数派出，给本殿寸寸搜寻黑暗之城方圆千里之内，一旦有方向爆发出惊人的能量波动，立即汇报！”
“是！”
那些黑暗之殿的强者闻言，急忙应道，然后赶紧转身前去吩咐命令，再不敢留在这里触火气正处于鼎盛的青檀。
青檀望着飞快散去的诸多人影，小手紧握，银牙咬得咯吱做响，那眉心处，古老的符印不断的闪现着，显然，对于林动在她眼皮底下被人劫走之事，她心中极其的暴怒。
而随着青檀命令发布下去，整座黑暗之城，都是在此时骚动起来。
这是一片茂密的山林，天际之上，银月高悬，淡淡的月光倾洒下来，为这片山林都是披上一层薄薄的银纱。
嗡。
天空中，原本宁静的空间突然剧烈的波动起来，而后黑光闪现，待得光芒散去时，数道身影已是凭空的出现在了天空上。
“呵呵，这地方倒是挺不错，想来用来埋葬两位应该也足以让你们满意了。”七王殿微笑的望着林动二人，道。
那黑暗之城毕竟是黑暗之殿的地盘，而且那叫做青檀的黑暗之殿殿主身怀黑暗祖符以及完全修复的黑暗圣镰，若是在那里动手的话，难免横生枝折，因此他们也是施展了一些手段，将林动二人挪移出黑暗之城，这样一来，更是能够确保他们的计划万无一失。
林动面无表情的望着眼前这般有些恐怖的阵仗，一位实力起码达到渡过二次轮回劫实力的七王殿，五位堪比轮回境强者的异魔王，这般阵容，就算是如今的他，都是足以感觉到浓浓的危险。
“待会动起手来，我会尽力阻拦，你便找机会离去找青檀。”林动偏过头，对着身旁的绫清竹轻声道。
绫清竹看了他一眼，却是微微摇头，道：“你不要与我说这般话，眼下这困境即便是你也难以应付，我若是走了，即便是将青檀找来也是无用。”
见到她这般时候倔起来，林动不由得微恼，但绫清竹却是怡然不惧，那清清冷冷的目光让得他只能无奈的摇摇头。
“想来待会交手动静应该不小，看来我需要提前做点准备。”半空中，七王殿笑吟吟的望着两人，旋即突然轻轻扬手。
轰！
随着他手掌扬起，只见得那五尊异魔王体内突然有着滔滔魔气涌出来，而后化为一片巨大的魔气光罩，将这片山林都是笼罩在其中，这样的话，也是隔绝了任何能量波动的传出，显然，这七王殿今日是要必杀林动了。
见到魔气光罩成形，那七王殿方才微笑着点点头，旋即他低头望着林动，道：“除了符祖之外，你还是我第一次见到能够将多道祖符掌控的人，所以虽然如今的你尚未进入轮回境，但在我们魔狱的必杀名单上，却是仅次于远古八主他们。”
“不过可惜今天过后，或许你的名字便是要从我魔狱必杀名单之上除去了。”
随着七王殿此话落下，他脸庞上的笑容也是愈发的浓郁，而后，滔天般的魔气，粘稠得犹如一片魔海般，疯狂的自其体内席卷而开，这片山林都是在此时变得逐渐的枯萎下来，甚至连天地元力，都是发出哀鸣之声。
“咻！”
七王殿笑容满面，而后一指轻轻点出，只见得一道粘稠得犹如液体般的魔气光束，猛然撕裂空间，快若闪电般的对着林动暴射而去。
林动见到这七王殿出手，眼神之中，也是猛的涌出浓浓杀意，袖袍一挥，一道黑影暴掠而出，黑色长刀怒斩而下，狠狠的与那那道魔气光束劈砍在一起。
铠！
刺眼的火花在半空中暴射而开，空间震裂，那手持黑色长刀的吞噬天尸，却是直接被震退数十丈，这一幕看得林动眼神一凝，自从得到这一具被吞噬之主强化过的吞噬天尸后，还是第一次见到它彻底的落入下风。
“呵呵，这是吞噬之主的吞噬天尸吗？比起当年倒是强了不少，不过，这东西今日可救不了你们的性命。”
七王殿淡淡一笑，而后他双目缓缓闭上，在其身后那滔滔魔海开始剧烈翻涌，一股恐怖的威压，逐渐的弥漫开来。
显然，这位七王殿，要开始动杀手了。


第1237章 魔皇甲
恐怖的魔气威压，浩浩荡荡的弥漫开来，直令得这片天地为之失色。
这是林动首次真真切切的感受到这些魔狱最高层的恐怖战斗力，这般波动，强大得甚至是令人连抵抗的勇气都被削弱了下去。
林动深吸一口气，脸庞之上布满着凝重，以他如今的实力，凭借着触及轮回的元力修为以及大符宗境的精神力，足以在与寻常轮回境的强者交锋中占尽上风，而若是再借助着两大祖符的力量，更是能够与那种渡过一次轮回劫的巅峰强者争锋，但从某种程度而言，这已经是一种极限。
可眼下，这七王殿，却是拥有着堪比渡过两次轮回劫的恐怖实力。
这场危机，可真不是那么容易渡过的了啊…
林动双掌缓缓紧握，漆黑眸子盯着那悬浮天际，在身后滔天魔海衬托下犹如一尊魔神般的七王殿，一丝丝的凶芒，却是逐渐的从眼神深处攀爬出来。
这些年他经历了太多的生生死死，也早已磨练出他那面对着任何对手都不再有所畏惧的心性，眼前的对手，虽然恐怖，但若他真的认为这样就能轻易解决掉他林动的话，未免也想得过于天真了一些。
轰隆。
天际之上，雷云飞快的汇聚而来，轰隆隆的雷鸣之声，在这片天地中回荡着。
林动此时的双瞳，已是逐渐的变得诡异一只眼瞳深邃如黑洞，一只眼瞳璀璨如雷，一黑一银，诡异之中有着一种极端狂暴的波动散发着。
“呵。”
七王殿淡笑的望着这般状态的林动，而他眼中却没有丝毫的笑意只是有着无尽的冷漠，下一刻，他手指凌空点出。
轰。
身后磅礴魔海在此时奔腾而起，直接是化为一头万丈庞大的九头魔蟒，魔蟒仰天嘶啸，魔气喷吐间，遮天蔽日。
魔气涌动间那九头魔蟒已是暴掠而出，冲击之下，空间都是寸寸崩裂。
“轰隆隆！”
天际之上雷云疯狂翻涌，旋即那雷浆般的瀑布，犹如一条银河，夹杂着惊天动地般的声势倾泻下来，最后狠狠的冲击在那九头魔蟒身体雷光疯狂的在九头魔蟒身体之上跳跃闪烁着，虽说雷浆过处魔气尽数消退，但紧接着便是有着更为恐怖的魔气涌出来，最后反将那蕴含着雷霆祖符之力的雷浆，尽数的腐蚀而去。
九头魔蟒那泛着无尽魔气的巨嘴，已是在此时穿破空间，狠狠的对着林动所在的这片空间噬咬而来那一霎，空间尽数的蹦碎而开。
林动望着那疯狂噬咬而来的九头魔蟒，眼中雷芒爆闪，手掌高高举起，只见得一道千丈雷霆猛的降临而来，直接是化为一柄千丈庞大的雷枪。
林动手持雷枪，喉咙间也是有着一道低吼之声传出，下一刻，他身形如电暴射而出，竟直接是冲进那九头魔蟒狰狞巨嘴之上。
璀璨的雷芒猛的在那九头魔蟒巨嘴之中爆发开来，它仰天尖啸，只见得那雷芒却是穿透它庞大的身躯，最后犹如万道雷剑，自它身躯内部，暴射而出。
砰！
九头魔蟒砰的一声，竟直接是在此时爆炸开来，漫天魔浆四射，一道雷光瞬间暴掠而出，林动在那催雷芒的包裹下，犹如雷神降临。
“吞噬！”
林动撕裂九头魔蟒，手掌陡然对着那远处天空的七王殿遥遥抓下，指尖黑芒闪烁，只见得后者周身空间瞬间崩溃，四道巨大无比的黑洞漩涡浮现而出，形成囚牢将其困于其中，而后吞噬之力暴涌，那股撕扯力便是要将七王殿撕碎而去。
“哈哈。”
然而面对着林动的攻势，那七王殿却是一声大笑，笑声之中充斥着嘲讽之色，而后滔天魔气犹如冲击波一般自其体内冲击开来。
砰砰砰！
四道黑洞漩涡，撕裂之力刚刚成形，便是直接是硬生生的被那魔气冲击震散而去，这七王殿的实力，远远的超越了林动。
“大天魔手眼！”
七王殿手掌猛的一抓，只见得天地黑暗下来，一只犹如从位面之外穿透而来的魔手遮天蔽日的凝聚而出，在那魔手之上，竟是生有一只巨大的黑色眼睛，那眼睛眨动间，犹如有着毁灭之力迸射而出。
咻！
一道蕴含着毁灭之力的魔光，陡然自那魔手眼中射出，一闪之下，便是将林动笼罩。
魔光暴射而下，林动手掌一抓，一道千丈庞大的黑洞漩涡便是在其前方成形，而后疯狂旋转，犹如大口一般，生生的将那道魔光硬接而下。
砰砰砰！
黑洞漩涡中，爆发出惊天般的爆炸声响，紧接着那黑洞都是变得扭曲起来，最后砰的一声，竟是生生的爆炸开来。林动的身形，直接是被那种可怕的冲击波震飞数千丈，而后喉咙一甜，一抹血迹自嘴角浮现出来，那七王殿的攻势太过恐怖，即便是他使用吞噬之力化解，依旧未能彻底阻拦下来。
“真不愧是拥有着两大祖符的人呢，我这般攻击，即便是渡过一次轮回劫的强者都无法阻拦，却是被你给接了下来。”七王殿望着林动，笑吟吟的道。
林动眼神冰寒，却是未与他半句废话，双手印法陡然变幻，而后隔空猛按而下：“大荒芜经！”
奇特的波动冲进大地之中，而后这片山林立即以一种惊人的速度荒芜起来，数千里之内，浩瀚无尽的力量在大地之下奔涌，最后飞快的对着林动所在的地方汇聚而来。
嘭！
足足数千丈庞大的能量光柱，在此时冲破大地，最后尽数的灌注于天空之上的那一道削瘦的身影之中。
这般恐怖的能量灌注，直接是令得的身体都是膨胀了一圈，而后他猛的仰天长啸，那啸声之中，夹杂着古老的龙吟之声。
吼！
紫金光芒在此时喷薄而出，整整八千道紫金龙纹在林动周身腾飞，龙纹飞舞间，连这片空间都是剧烈的扭曲起来，下方大地，更是被这种威压震得崩塌下去。
林动皮肤表面，一股股紫金光芒穿梭着，他目光凶狠的盯着天空上的七王殿，身形猛的暴掠而出，而后直接接近那七王殿，一拳轰出，八千道紫金龙纹随之飞舞，拳风过处，空间破碎。
“砰！”
而面对着林动这般凶狠攻势，那七王殿却是丝毫不退，粘稠的魔气在其手臂之上缠绕，而后也是一拳轰出，硬生生的与林动憾在一起。
双拳硬碰，周遭数千丈的空间，都是寸寸破裂。
砰砰砰！
两人身体一震，眼中皆是有着凶光闪烁，而后拳影呼啸，竟是铺天盖地的对轰而去。
龙纹魔气，疯狂奔涌。
轰隆隆！
空间破碎的声音不断的响彻着，天地间，狂风大作，飓风旋转，那等恐怖的攻势，将这片山林，瞬间夷为平地。
咚！
又是一记凶悍得无法言语的对碰，林动那双拳之上，鲜血淋漓，甚至连血肉都是被震散而去，露出其下犹如白玉般的骨骼，所幸他拥有着洪荒龙骨，不然的话，恐怕在这种冲击下，连骨骼都是得化为粉末。
不过绕是如此，他体内气血也是极端的震荡，这般硬碰，看似凶悍，但却是受创不轻。
林动身形倒飞而去，目光死死的盯着七王殿，此时的后者那双拳上，同样是有着黑色的鲜血滴落下来，这令得他布满着笑容的脸庞略微有些阴沉，想来他也是没料到对付一个林动，竟然会令得自己身体出现伤势，虽然这对于他而言，并不算什么。
“真是个不简单的家伙啊，不过这样的话，我更想亲手杀了你了！”
七王殿阴沉的脸庞上，再度有着笑容浮现出来，旋即他嘴巴一张，那片粘稠的魔海，直接是被他一口吞进体内。
嗡嗡。
他的身体，在此时剧烈的颤抖起来，粘稠的黑色物质自他体内渗透出来，最后竟是在他身体表面，缓缓的凝构成一件看上去极端狰狞血腥的黑色魔甲。
魔甲几乎覆盖了七王殿整个身体，甲胄之上，有着犹如钢铁般凌厉的线条延仲出来，每一道弧度，都是充满着可怕的狰狞之感，这件魔甲一出现，一种无法言语的恐怖波动，也是缓缓的散发出来，那种感觉，就犹如一个域外的毁灭之物，降临在了这世间。
林动望着这一幕，瞳孔也是微微紧缩。
“林动，你可知道这是何物？”七王殿冲着林动微微一笑，笑容中，却是弥漫着杀意。
林动身体紧绷，体内力量尽数的运转，但却并未回答。
“这是魔皇甲，由我们的皇亲自传给我们，一般只有在与远古八主交手时，我们方才会动用，今日，为了你，我却是将它催动了出来，所以…”
七王殿脸庞上的笑容，逐渐的狰狞起来。
“所以…今日就算你死在了我手中，你也没半点好可惜的了！”
林动那布满着鲜血的双掌缓缓紧握，那原本激烈跳动的心脏，反而是逐渐的平静下来，眼下这场战斗，已是让得他感觉到死亡的味道，不过，不管你是什么人，想要杀我，也必须付出你难以想象的代价！
林动双目陡然圆睁，一股疯狂的戾气，缓缓涌出！


第1238章 力战
魔气奔涌，七王殿脚踏虚空，身体之上那狰狞的魔皇甲闪犹如金属般的冰冷光泽，此时的他，有着一种嗜血的狰狞之意蔓延开来。
那种可怕的威压之感，让得人心跳仿佛都是加剧起来。
八千道紫金龙纹在林动周身盘旋，古老的龙吟之声响彻而起，在其双瞳之中，一黑一银更是显得分外的诡异。
“嘭！”
林动的身形，再度在此时暴掠而出，双拳挥动，直接是化为庞大无比的紫金龙爪，狠狠的对着那七王殿怒轰而去。
然而，面对着林动这般凶悍攻势，那七王殿眼中却是掠过一抹讥讽之色，身形竟是纹丝不动，任由那足以将山脉夷平的龙爪攻势重重的落到他身体之上。
铛铛！
清脆的金铁之声爆发开来，七王殿身体纹丝不动，那身体表面的魔皇甲之上更是未能留下丝毫的痕迹，他淡漠的笑望着林动，眼中的嘲讽愈发的浓郁，然后他仲出那同样被黑色甲胄覆盖的手指，凌空一点。
咻。
一道魔光直接从其指尖迸射而出，而后瞬间便是将那凝聚了八千道紫金龙纹的龙爪撕裂而去。
“这点能力的话…”七王殿笑着，旋即他眼中便是倒射着一道身影暴掠而来，后者的一条手臂，都是在此时呈现了一种液体般的形状，在那流淌的液体中，雷光闪烁，黑芒凝聚隐约能够见到两道古老符文在浮沉着。
“竟然能够将祖符之力运用到这种程度了…”七王殿眼中终是掠过一抹细微的诧异，旋即那手掌也是陡然紧握，接着一拳轰出，没有任何的花俏，有的只是那股近乎毁灭般的力量。
砰！
液体般的拳头与那覆盖着魔皇甲的魔拳硬憾在一起，那片空间仿佛都是在此时凹陷了下去，魔气喷涌间，林动的身形，再度被生生震退。
唰。
不过在退后的瞬间，他袖袍猛的一挥，一道光华暴掠而出瞬间迎风暴涨，化为一座巨大的石碑，石碑之上有着古老的波动弥漫而出。
“大荒芜碑？”那七王殿望着那石碑也是一怔，旋即摇了摇头，道：“没想到连这般神物也是在你手中，不过可惜，这大荒芜碑似乎还未恢复巅峰实力，不然的话今日倒也是有些麻烦。”
嗡。
在其说话间，大荒芜碑已是带起滔天光华，狠狠的对着七王殿镇压而去，碑底之下，有着巨大古老光阵浮现着。
林动祭出大荒芜碑，却并未就此停下屈指一弹间，有着七彩虹光浮现，只见得那玄天殿也是闪现出来，殿身摇晃间，无数神物化为一道洪流呼啸而出，直接对着七王殿席卷而去。
“玄天殿…”
再度被祭出的玄天殿也同样是令得七王殿笑了一声，没想到这远古神物榜前十的超级神物中，在这林动手中，还是有着两尊。
“手段的确层出不穷可惜无用。”
七王殿笑着，那眼中有着冰寒之色陡然涌出来，而后他手掌猛然一抬，竟直接是生生的将那镇压而来的大荒芜碑抵住，而后一脚踹出，那道席卷而来的神物洪流也是尽数的蹦碎而去，无数神物在那等恐怖之力下，化为粉末。
砰砰！
七王殿仰天大笑，拳影呼啸，重如泰山般的落在大荒芜碑之上，竟是反而将大荒芜碑逼得节节败退，而玄天殿的神物洪流，也是被他尽数的抵御而下。
身披魔皇甲的七王殿，实力的确是恐怖得近乎逆天了。
林动望着那将大荒芜碑与玄天殿攻势尽数阻拦下来的七王殿，眼神也是愈发的凝重，旋即他深吸一口气，双目却是缓缓闭上，而后在其眉心处，一道光线浮现，而后缓缓的撕裂开来。
随着那道光线的裂开，一只神秘莫测的眼睛，也是出现在了林动眉心，赫然便是祖符之眼。
当这枚祖符之眼出现时，那七王殿的眼神方才微微一凝：“祖符之眼？”
林动周身的气息，在此时迅速的收敛，只是那眉心中的祖符之眼，却是愈发的明亮，下一霎，他紧闭的双目陡然睁开。
唰！
在其双眼睁开的霎那，那祖符之眼内的光芒也是强烈到极致，而后一道弥漫着黑芒与雷光的光束，陡然自其中暴射而出。
那道光束极端的奇特，其中犹如布满着无数细小的古老符文，看似微弱的光芒，却是有着一股令人恐惧的波动散发出来。
那道光束，瞬间穿透了空间，哪一个眨眼间，已是抵达那七王殿身前，然后狠狠的射在其胸膛之上。
嘭！
耀眼的强光伴随着能量冲击波扩散而开，而后光芒逐渐的弱去，只见得那七王殿依旧脚踏虚空，只是在其魔皇甲胸膛处，却是有着一道裂纹浮现出来。
林动望着这一幕，面色也是忍不住的一变，没想到他即便是催动了祖符之眼，也仅仅只是将那魔皇甲撕裂出一道口子，这东西，究竟是有着多么的恐怖？
那七王殿同样是低头看着胸膛处的裂纹，那眼神却是变得极端阴沉下来，显然魔皇甲被林动攻破令得他极为的暴怒，这是他们的皇所赐予，这般损伤对于他们而言，却是近乎耻辱。
“你也该玩够了吧？”
七王殿脸庞上的笑容尽数的淡下去，旋即他猛的一步踏出，身形犹如瞬移般出现在林动面前，那魔掌翻手便是对着林动怒轰而去。
吼！
八千道紫金龙纹升腾，而后化为一道紫金巨龙与那七王殿硬生生的轰撞在一起，然而接触的霎那紫金光芒便是碎裂而开，紫金巨龙更是被生生撕裂。
林动借此身形暴退，但那股侵蚀而来的强大冲击波，依旧是令得他喉咙间传出一道闷哼声，这身披魔皇甲的七王殿的确是太厉害了。
一掌震退林动，那七王殿眼中气爆闪，显然是不再打算继续拖下去，身形一动，再度追上拳挥动间，滔天魔气涌动。
砰砰砰！
天空上两人再度凶狠对碰，不过如今的林动显然是开始尽数的落入下风，那七王殿身体之上的魔皇甲厉害无比，即便是他催动着祖符之力，也是难以对他造成太大的伤害，反而他面对着七王殿的攻击，却是得小心谨慎，若非是洪荒龙骨相护，他怕早已彻底重伤。
不过绕是如此，他的情况也并不妙。
轰！
又是一记强猛对碰，林动喉咙微甜，一口鲜血自嘴中涌出，旋即被他一口吐出去，那眼中神色，却是愈发的狰狞。
“你身上的宝贝还真是不少，这具肉体，若是再精进的话，恐怕也是能够与洪荒之主略作争锋了，但可惜，我可没那兴趣留这么一个有潜力的敌人。”七王殿望着即便是在他如此凶猛攻势下，依旧能够屹立到现在的林动，也是忍不住的摇摇头，道。
话音落下，他眼中杀意也是愈发浓郁，不过就在他刚要出手时，后方猛的有着凌厉剑芒暴射而来，这般剑芒虽说威力不弱，但远远达不到伤他的程度，因此他只是淡淡一笑，反手一握，便是生生的将那道剑芒捏爆而去。
“这点能力…”
他嘲讽一笑，然而笑声尚还未完全落下，只见得一道倩影却是突兀出现在其前方，一柄青锋长剑犹如毒蛇出洞，快若闪电般的刺向他。
“不自量力。”
青锋长剑之上，突然有着奇特的光芒覆盖而上，那刚刚将一句话说完的七王殿，瞳孔猛的一缩，刚欲暴退，那青锋长剑却是陡然加速，快若闪电般的刺在他那魔皇甲上面的裂纹之上。
嗤！
足以让得渡过一次轮回劫强者全力攻击的魔皇甲，却是在此时被那青锋长剑生生刺进半寸，黑色的血液顿时顺着剑锋流了出来。
“你！”
胸膛传来的剧痛，瞬间便是令得七王殿面庞扭曲了起来，旋即他猛的一掌轰出，面前绫清竹娇躯也是倒飞而出，一口鲜血自薄纱下喷出，而后将薄纱也是震飞而去。
七王殿眼神狰狞，他望着那倒飞而出的绫清竹，眼中杀意暴涌，手掌一握，只见得一条粘稠的魔气洪流瞬间自其手中暴掠而出，然后贯穿长空，狠狠的对着绫清竹暴射而去。
绫清竹望着七王殿这般攻势，也是轻咬银牙，玉手紧握长剑，就欲强行相迎。
不过还不待她出手，一道身影已是快若鬼魅般出现在其身前，而后一声低吼，漫天紫金龙纹奔涌，一拳硬生生的轰在那魔气洪流之上。
嘭！
空间在此时崩塌，林动的身体却是如遭重击，一口鲜血喷了出来，那手臂都是在此时扭曲了一些，剧痛自手臂上传来，令得他额头上顿时有着冷汗冒出来。
“林动。”绫清竹见到林动这番模样，也是一惊。
“呵呵，真是好感人的一幕，既然你想保住她，那我就偏要在你面前将她给杀了！”七王殿望着这一幕，嘴角的狰狞愈发的浓郁，而后他身形一闪，便是诡异的消失而去。
“小心！”
在那七王殿消失时，林动面色一变，急忙反身一把将绫清竹抓了过来，而也就是当他在将绫清竹护于怀中时，七王殿的身影已是出现在其后侧，旋即森然一笑，那蕴含着滔滔魔气的掌风，便是毫不留情的落在了林动后背之上。
噗嗤。
林动又是一口鲜血喷出来，他怀抱着绫清竹，身形急速的自天空坠落而下，然后两人皆是重重的落进那片山林之中。
“哈哈。”
七王殿大笑着，下手却是毫不留情，手掌一招，又是数道魔气洪流成形，闪电般的贯穿长空，狠狠的轰下去。
丛林中，林动抱着绫清竹略显狼狈的躲避着，但依旧被一些攻势所波及，不过每一次的攻击，都被他用后背给强行接了下来，其怀中的绫清竹，倒是丝毫未伤。
砰砰！
这般恐怖攻势，直接是令得下方的大地崩裂开来，林动翻滚而开，背上血肉模糊，但他却是死死的撑着身子，身体剧烈颤抖间，嘴角的鲜血愈发的浓郁。
“这个王八羔子。”林动倒吸一口冷气，强忍着闹中传来的阵阵眩晕，咧嘴骂道。
绫清竹望着那近在咫尺的脸庞，后者紧咬着牙，脸庞上的鲜血令得他看起来有些狼狈，但这幅模样落得她眼中，却是令得她忍不住的紧咬着红唇，那素来清冷的眸子，都是在此时变得极端的温柔与紊乱下来。
“为什么要这么拼命？”
绫清竹望着此时格外狼狈，但却令得她心头有些颤抖的林动，贝齿咬着红唇，喃喃道。
“连个女人都保护不了的话，那还叫什么男人？”林动咧嘴一笑，粗重的呼吸扑打在绫清竹那娇嫩的脸颊上，体内传来的剧痛，即便是以他的肉体之强横，都是令得他额头上不断有着冷汗滴落下来。
这次的伤，可不轻。
“你这个傻子。”
绫清竹略有些失神的盯着林动，旋即那眼中仿佛是有着一些雾气浮现出来，纤细玉手轻抚着他的脸庞，然后不顾那污渍，玉手轻轻的将他嘴角的血迹搽拭而去，而后她轻咬银牙，那素雅的俏脸上，竟突然有着一抹妩媚的笑容浮现出来，直令那百花都是失了颜色。
“我突然很想把太上感应诀教给你了，怎么办？”


第1239章 修炼之法
林动听得绫清竹这突然间的话语，却是愣了一下，旋即苦笑道：“现在可不是学“太上感应诀”的时候。”
绫清竹轻咬着银牙，道：“你能施展手段将他阻拦一些时间吗？”
林动想了想，道：“虽然这家伙很厉害，但要阻拦他一些时间，应该不难。”
话音落下，他手掌一握，一片光阵顿时在其掌心浮现出来，而后光阵迎风暴涨，转瞬间便是化为一片巨大的光阵，将这片山林笼罩在其中。
咻。
乾坤古阵一成形，林动又是将大荒芜碑以及玄天殿射出，令得它们冲进阵法之内，有了它们加固阵法，也是彻底的将阵法稳固下来，这般防御，想来就算是那七王殿再厉害，也得耗一些时间。
不过这些防御一做出来，体内经脉抽搐间，又是令得林动额头冷汗加剧了一些，他此时显然受伤不轻，甚至连脑中都是有着阵阵眩晕涌来，只不过却是被他生生的压制了下来，这个时候，若是失去意识的话，恐怕就真是难逃一死了。
“这乾坤古阵应该能阻拦一些时间，不过也就仅仅只能拖延而已。”林动喘了两口粗气，望着那笼罩了山林的乾坤古阵，阵法之上有着极端浩瀚的能量在涌动，隔绝了这里与外界。
“你放心吧，我可没那么轻易被解决掉，若真是逼得急了我便自爆了这乾坤古阵，那时候这家伙也得吃不了兜着走。”林动咬咬牙，虽然那样代价实在是大了点，但这般时候，也顾不了许多了。
绫清竹微微摇头她看了林动一眼，俏脸红了一下，道：“你能先放开我吗？”
林动这才发现两人此时姿势过于暧昧了一些，他整个人扑在绫清竹娇躯上，虽然那种柔软感觉很让人想入非非，但毕竟着实有点不妥，当即他勉强的笑了笑刚欲强行撑起身子，体内的伤势终是被牵动，又是一口鲜血喷了出来身体一软，反而重重的压在了绫清竹娇躯之上。
绫清竹被他这般重压，也是轻呼了一声，微微侧头，然后便是见到林动那紧咬着牙的脸庞，那上面还残留着一些痛苦之色。
望着他这般少有的狼狈绫清竹那眸子之中也是逐渐的涌上柔软之色，她微微犹豫了一下，然后玉手轻轻的将林动额头的汗水温柔的搽去。
“必须得先离开这里了，待会我自爆乾坤古阵，你便带我离去吧，吞噬天尸会断后。”林动的嘴贴在绫清竹娇嫩耳边咬着牙说道，他能够感觉到阵阵虚弱之感在飞快的涌来。
这一次真是亏大了，不仅要赔上乾坤古阵，而且多半是连吞噬天尸也多半难以收回了，这些年来，除了异魔城那次，他还是第一次吃这么大的亏。
然而听着他的话，绫清竹却是微微摇头，然后她突然伸出玉手在那犹如羊脂玉般的小手中，有着一枚暗红色的丹丸。
“你…你把它吃了。”绫清竹将那丹丸放在林动嘴边，轻声道。
“疗伤的？”林动一愣，倒是不疑有他，直接将那丹丸给吞进体内，不过紧接着他便是感觉到不对，丹丸入体，一股奇怪的火热猛的涌了出来，而且，在那种火热涌来时，他脑海中的眩晕陡然翻涌起来，那种眩晕，令得他眼皮都是缓缓的垂下。
“你…你给我吃的是什么？”视线模糊间，林动喃喃自语道。
“没什么，等你睡醒，一切都好了。”绫清竹纤细玉手轻轻的抚着林动的脸庞，她轻咬着红唇，轻声道。
林动心中隐约的察觉到一些不对，但此时他本就重伤在身，那种虚弱感疯狂的涌来，最终却是令得他无法清醒过来，眼皮一搭，视线便是尽数的黑暗。
绫清竹望着昏睡过去的林动，张完美无瑕的脸颊上，火红一片，她轻咬着银牙，声音犹如蚊蝇的喃喃自语：“太上感应诀是我们九天太清宫不传之秘，而且也根本传不了旁人，因为准确说来，这太上感应诀根本就没有修炼之法，那只是一种玄奥的感应，而那种奇特而强大的力量，便是来自那感应之地。”
“而…而想要让你也感应到那种奇特存在，除了在出生的那一霎传承之外，就唯有…唯有以双修之法，共同感应。”
若是此时林动还苏醒着的话，必然会因为此话目瞪口呆下来，他从未想到，这“太上感应诀”竟然是需要这般方式，难怪今日当他在竹林与绫清竹说起那句话时，后者会突然间发怒了原来…
绫清竹轻轻撑起身子，而后看了林动一眼，轻咬红唇，道：“躲在他体内的那个人，你也出来。
林动身体表面光芒顿时闪烁起来，而后岩飘荡而出，他看着脸颊滚烫的绫清竹，忍不住的干笑一声，只是那眼神略微的有些古怪。
“你给他吃的…吃的是…”岩看着绫清竹，干笑道：“是春药？”
“他要学“太上感应诀”，便只有这一个法子。”绫清竹眸子微垂，脸颊如血，道。
此时岩也是有些失语，他终是明白过来，或许从林动想要学“太上感应诀”的那时起，绫清竹便是想到了这一幕，所以她才会提出让林动以后别不要再问她有关“太上感应诀”的事，显然当时的她也是略微的有些挣扎。
虽然她性子素来清淡，看上去对什么都不是很在意，但对于这种事情显然也没办法以平常心而待，特别是在这个对象是林动的时候，她这一路来，只是安静的跟着他的身旁，显然是要借着这种法子来抵消掉心中的某些挣扎以及下定某些决心。
只不过事到临头时，她依旧还是少了一些勇气，方才用出这种让人有些哭笑不得的方式，说到底，不管再清傲的女子，在这种事面前，终归还是一个会胆怯会羞涩的普通女子。
岩叹了一声，能够让得眼前这清冷而内心高傲聪慧的女子，用出这般近乎掩耳盗铃的笨办法，林动这家伙，也还真是有“本事”了。
“能麻烦你一个事吗？”绫清竹突然抬起头看着岩，道。
“什么？”
“这件事情，你不要告诉他。”绫清竹玉手小心翼翼的将林动嘴角的血迹搽去，轻声道。
“为什么？”岩一愣，显然是有些不明白她的想法，类似绫清竹这般女子，显然是对于这种事情极端看重的，平日里更是薄纱遮面，寻常男子想看她真容都是极难，她身子清清白白，这辈子就只是被林动误打误撞的沾染过，眼下这般付出，可并不容易。
“我不喜欢他对我只有愧疚。”绫清竹摇了摇头，道。
岩苦笑着点点头，看来绫清竹果然很明白林动为什么要学“太上感应诀”，若是后者知道他这个要求对于绫清竹而言需要付出多大的话，以他的性子，怕还真是会感觉亏欠她很多，而这一点，则是高傲的绫清竹不想看见的。
“我会尽量为你保密。”
“谢谢了。”绫清竹嫣然一笑，那霎那间绽放出来的风情，让得岩这般状态都是怔了怔，旋即暗叹着飘然远去。男女之间这东西，果然够让人头疼的。
见到岩远去，绫清竹这才用冰凉的玉手贴着自己脸颊，旋即她望着林动，玉手一挥，一道光芒自其袖中掠出，接着竟是化为一间闪烁着光芒的竹屋，这东西显然是一件灵宝，只不过除了遮风避雨之外，却是并没有太大的作用。
绫清竹将林动轻轻的抱起，然后走进竹屋，将其小心翼翼的放在那床榻之上。
她坐在床缘边，美眸怔怔的望着那张熟悉的脸庞，许多年前，当这张脸庞还有些稚气的时候，她便是遇见了他。
或许那时候的她也根本没想到，有一天，她会对他作出这些事情来，若是早知道的话，恐怕在那山峰上，她就直接干脆先一剑把这家伙杀了，那就什么烦恼事都没了。
她看着林动，好半晌后，那张绝美的容颜也是有着点点异样的火红涌起来，最后她轻咬着银牙，缓缓的站起身子，玉手带着一些颤抖的将那束腰，轻轻拉开。
雪白的衣裙滑落而下，一具完美得没有丝毫瑕疵的胴体便是这般的暴露在了竹屋之内，她微微的颤抖着，看上去，有着一种惊人动魄的美感。
“当年你便是这般，现在…现在就活该你遭报应。”
她看着床榻上那昏迷过去，但浑身却是散发着滚烫温度的男子，想起那过往的种种以及之前他护着自己时所看见的那张满脸鲜血看上去略显狼狈的脸庞，她忍不住的轻轻一笑，只是那眸子中却是有着水花凝聚着，顺着脸颊滴落下来。
最后她玉手一挥，竹屋之内，尽数的黑暗下来，但那黑暗之中，却是有着一番春光，绽放而开，此时，春意盎然了时间。


第1240章 感应
天空之上，魔气弥漫，七王殿脚踏天空，面色阴寒的望着下方那巨大无比的古老阵法，眼中寒芒在急速的闪动着。
“乾坤古阵…这小子，还真是让人惊讶无比啊。”
他盯着那熟悉的古老阵法，却是一声冷笑，对于这阵法的威力，他在远古时期便是亲自领教过，不过那时候这阵法可是在符祖的手中，当年不知道有着多少异魔被这阵法所抹杀。
“五王听令，给我将这阵法侵蚀了。”
七王殿手掌一挥，冷笑道，这乾坤古阵虽然威名赫赫，但眼下林动显然并未获得其真髓，再加上他的实力远远不可能与符祖相比，这又能让得他有几分忌惮？
“是！”
天空上，那维持着魔气光罩的五尊异魔王也是立即应喝，而后那魔气光罩之上有着铺天盖地的魔气涌来，接着落在那乾坤古阵之上，魔气荡漾间，虽然不断的被那乾坤古阵的力量净化而去，但紧接着便是有着更多的魔气涌来，不断的消耗着乾坤古阵的力量。
七王殿则是凌空盘坐下来，目露冷意，林动表现得越是不凡，他心中的杀意便越是浓郁，今日一番交手，他已是能够察觉到后者的棘手，若是任由他成长下去，恐怕日后不会比远古八主他们更好对付。
这种潜在的敌人，还是尽快解决掉为好。
想到此处，七王殿那看向乾坤古阵的目光愈发的阴森。
“我倒是要看看，你这乾坤古阵能够坚持多久。”
朦胧之中，林动的意识仿佛是在没有尽头的飘荡，他的这股意识也是极为的模糊，只不过偶尔能够感受到一些莫名的柔软。
意识犹如婴儿般脆弱飘荡之中，模糊之中，似乎是有着另外一股柔和意识缠绕而来，将他紧紧的护着，那种感觉，极为的美妙。
因为美妙－，他的意识也是靠近过去将那道意识缠绕住，一道炽热掠过，模糊中似乎身体动了动隐隐约约的，仿佛有着女子咬着红唇轻哼的声音传来。
意识飘荡在天地间，在那遥远处，突然有着朦胧的光芒出现，那种光芒，仿若是混沌之光天地初开时所诞生的东西。
他的意识，在距那片混沌还有着一些距离时便是停滞不前，那种光芒令得他的意识阵阵刺痛，也令得他不敢靠近过去。
而在他意识彷徨无助间，那道柔软的意识将他轻轻的包裹，犹如一只纤细的玉手将他拉着，缓缓的靠近那片混沌。
不过那种光芒，显然对于那道柔软的意识也是有着刺伤性，因此那道意识波动着，但她却是始终紧紧的包裹着他的意识，不让他受到那种刺痛的伤害。
但林动模糊的意识，依稀能够感觉到她的那种强忍的痛苦，当即有着一种莫名的情感涌出来，他的意识猛然膨胀许多反而是将那道柔软意识包裹住。
隐约的，仿佛是有着女子细微的惊呼声传出，他的意识犹如是将她狠狠的揉进了身体之中，然后那两道意识，终是冲进了那片混沌之地。
一种无法言明的奇特感应，在此时悄然的诞生，那种感觉，就犹如凌驾了某种世界…
时间缓慢的流逝，一股极端通畅的感觉，在那意识之中如潮水般的涌出来，最后，模糊的意识，开始缓缓的苏醒。
当林动再度睁开眼时，葱郁的山林印入眼帘之中，他躺在地面上，微微抬头，便是见到那泛起剧烈涟漪的巨大阵法。
脑中还残留着一点眩晕，林动微微甩了甩头，将其甩开，然后坐起身子，漆黑眸子中，还有点茫然。
“你醒了？”
前面有着柔软的声音传来，林动抬起头来，然后便是见到绫清竹俏立在不远处，一对清澈眸子静静的看着他，此时在其脸颊上，又是有着薄纱遮掩，纤细而玲珑的娇躯，在白色衣裙的包裹下，展现出近乎完美般的曲线弧度。
她此时原本束着的三千青丝却是铺散而开，犹如瀑布般落至挺翘娇臀处，不知为何，此时的她看上去有着一种慵懒的味道，那种味道，让得林动刚苏醒过来的心都是跳了跳。
“你怎么变了？”林动看着此时的绫清竹，忍不住的道。
“什么变了？”绫清竹走近过来，然后在林动面前轻轻的蹲下来，那素来清冷的眸子，泛起一抹淡淡的笑意。
“变得更好看了。”林动笑道，他看着绫清竹，有些讶异，后者似乎很少主动的做出这种有点亲近的举动，即便对象是他。
而林动此言倒的确没错，现在的绫清竹，不知为何，却是有着一种惊心动魄般的美丽之感，那并非是简单的容貌之美，更是气质之上的凸显。
对于他这马屁之言，绫清竹淡淡一笑，旋即轻声道：“别看了，乾坤古阵很快就要支撑不住了，还是先想办法怎么脱离困境吧。”
听得此话，林动方才猛的回过神来，他抬头望着那剧烈颤抖起来的乾坤古阵，也是急忙起身，不过旋即他便是发觉到不对。
“我的伤？”
林动满脸的惊讶，他发现自己先前体内与七王殿交手时所出现的伤势，竟然是在此时尽数的痊愈，甚至连先前消耗的元力，也是尽数的恢复。
“是你给我那丹药的缘故？”林动看着绫清竹，他依稀记得在昏迷前绫清竹给他吃了什么东西。
绫清竹眸子微微偏开，道：“太上感应诀，我已经教给你了。”
“啊？”林动再度发愣，这又是什么时候的事？
“你静心下来自然能够有所感，不过我所能够做的也就只有这些了，至于以后你能将这“太上感应诀”修炼到什么程度，就得全靠你自己了。”绫清竹站起身子，道。
林动皱了皱眉头，旋即他也是站起身来，目光看向绫清竹，道：“我昏迷的时候，发生了什么事情？这太上感应诀…”
绫清竹微微摇头，道：“你答应过我，不再提起太上感应诀的，既然如今已经成功了，就不要再问了，行吗？”
林动望着绫清竹那微微垂下的眸子，沉默了一下，旋即苦笑着点点“你先感悟下这太上感应诀吧，眼下这局面，若是能够脱身，那自然是好的。”
绫清竹偏过头，轻声说道，然后她便是转身走到那一处山坡上，美目怔怔的望着远处，神情略有点恍惚。
林动望着她那窈窕的柔软身段，轻风吹拂而来，如墨般的青丝轻轻飘舞，不知为何，他心中涌上一股淡淡的难受之感。
他使劲的摇了摇头，将心中情绪逐渐的平静下来，然后双目微闭，感应着那突然间在意识中多出来的一种玄妙东西。
那种东西，并非是任何实质之物，反而更像是一种飘渺无形的东西，但在触及间，仿佛又是有着一种莫名的力量，从那莫名之处，悄然涌来。
这，究竟是什么？
林动心中满是茫然。
天空上，乾坤古阵的颤抖越来越剧烈，隐约的能够察觉到在那之外的滔天魔气，面对着五尊异魔王的联手侵蚀，这乾坤古阵，显然无法继续的坚持下去。
嗡。
嗡鸣的声音，终是从天空传来，而后那巨大的乾坤古阵猛的黯淡下来，光芒减弱，然后便是化为一道光影自天空掠下，钻进林动身体之中。
而随着乾坤古阵的掠回，那大荒芜碑以及玄天殿也是掠回来，但这模样，显然都是消耗了极大的力量。
在乾坤古阵败退时，这片山林便是彻彻底底的暴露在了那滔天魔气之下，而在那魔气海洋之中，七王殿凌空盘坐，那对弥漫着阴森的目光，淡漠的注视着再度出现在视野之中的林动二人。
“呵呵，看来今日你们最终还是难逃一死啊。”七王殿缓缓的站起身来，他望着林动，嘴角却是有着一抹狰狞缓缓的涌起来。
林动那紧闭的双目，也是在此时逐渐的睁开，漆黑眸子之中，安静一片，没有丝毫对眼下这般绝境的惶色。
七王殿望着那眼神平静的林动，眉头却是微微一皱，隐约的，不知为何感觉到一些不对劲的地方，不过旋即他便是将这种感觉丢弃，这才不到半个时辰的光景，这林动，又怎会让他平白的多出这般感觉？
“接下来，你二人便在此处做一回亡命鸳鸯吧。”
七王殿手掌仲出，在那掌心之中，有着极端恐怖的魔气凝聚而来。
林动的眼神，依旧平静，下一霎，他身形出现在绫清竹身前，轻声道：“在我身后，放心，我把你带出来，就一定会把你带回去。”
绫清竹望着眼前这道削瘦的身影，贝齿紧咬着红唇，那清冷眸子深处，有着一抹淡淡的柔软在涌动着，旋即她以一种罕有的柔和姿态，轻轻的嗯了一声。
林动则是抬头，紧握的双手缓缓松开，那漆黑眸子中，一股凌厉到极点的波动，暴涌而来！


第1241章 雷弓黑箭
凌厉自林动眼中掠过，旋即他突然伸出手掌，那天空上雷云顿时铺天盖地的凝聚而来，轰隆一声，只见得一道数千丈庞大的雷龙便是呼啸而出，最后直接是在林动掌心凝聚起来。
雷光闪烁，雷龙飞快的缩小着，数息之后，便是化为一道巨大的银色雷弓，出现在了林动手中。
林动面色冷冽，目光盯着天空之上的七王殿，一只手掌，缓缓的将那雷弓嘎吱的拉成满月，而在那弓弦被拉至极限时，只见得深邃的黑芒飞快的凝聚而来，再接着，便是化为一只黑色的长箭。
嗡嗡。
黑色长箭箭锋处，黑芒流转，犹如是有着细小的黑洞漩涡成形，箭在弦上，尚未射出，然而这片空间已是呈现了一种崩塌的迹象。
天空上，七王殿的眼神也是因为林动此举凝了凝，旋即冷笑道：“真是厉害，雷霆祖符做弓，吞噬祖符做箭，能够将两大祖符运用到这般地步，你还真是出乎我的意料，不过，你我之间实力相差如鸿沟，手段再多，也无法将其弥补。”
“这样么…”
林动闻言，脸庞上也是有着一抹淡淡的笑容浮现出来，然后他深吸一口气，片刻后，突然有着一道混沌之光闪烁而来，落至那黑色长箭之这道光芒缓缓的在长箭之上流淌，它没有丝毫可怕的波动，反而那长箭之上涌动的吞噬之力，却是在此时极端的内敛起来，周遭本来被压迫得崩塌的空间，竟是缓缓的恢复过来。
那番模样，犹如林动手中的雷弓黑箭，尽数的失去了力量一般。
林动的脸庞上，一抹细微的苍白悄然的涌上，然后他冲着七王殿微微一笑，笑容之中，弥漫着无尽的狰狞。
林动那扣着弓弦的手指，在此时猛然松开，他的眼神，也是在这一霎，凌厉到了极致。
嗡！
黑色长箭，几乎是在脱离弓弦的那一霎那便是消失而去，而天空上的七王殿，瞳孔却是在此时猛的紧缩起来。
下一瞬，滔天魔气犹如大海一般自其体内席卷而出，身体表面的魔皇甲黑光流溢，魔气席卷，他便是犹如那脚踏天地的魔神，声势骇他前方的空间，在此时突然的被撕裂，一道黑光暴掠而出，然后还不待七王殿有任何的反应，便是狠狠的射在了他的身体之上。
砰！
七王殿的身体，在此时剧烈的颤抖起来，他步伐连连后退，旋即一道尖锐的惨叫声，陡然自其嘴中传出，漫天魔气，都是在此时翻滚起来。
黑光从七王殿胸前贯穿而进，竟然直接是生生的将那魔皇甲洞穿了过去，又是从其后背穿出，最后依旧去势不减，狠狠的射在天空之上那魔气光罩之上。
咔嚓。
由五尊异魔王联合布下的魔气光罩，在此时未能取到丝毫的阻拦作用，一道清脆之声下，那魔气光罩便是尽数的爆裂而开。
冰凉的月光，顿时再度倾洒而下。
噗嗤。
五尊异魔王面色皆是一白，而后一口黑色鲜血喷出来，他们目光隐隐有些骇然的望着半空中的那道身影。
“七王殿？！”五人急忙出声，在那魔气翻涌间，七王殿的身形有些踉跄，他有些呆滞的望着胸口处的血洞，黑色鲜血滚滚的流出来，那里，魔皇甲竟是被射出了一个约莫半个拳头大小的洞，洞口边缘，一道道细微的裂纹浮现着。
这魔皇甲，竟是被被攻了？！
“怎么可能…”
七王殿手掌摸着那些流出来的黑色血液，喃喃一声，旋即他瞳孔猛然怒睁起来，厉喝道：“怎么可能？！”
他身后滔天般的魔气，在此时紊乱下来，他那面庞上，也是掠过一抹苍白之色，显然是被这一箭伤得极重。
下方，林动面无表情的望着咆哮中的七王殿，那拉着弓弦的手掌，已是布满了鲜血，他的手掌同样是在微微的颤抖着，显然先前那一击，也是倾尽了他的力量。
“看来你这破甲，没你想的那么强。”林动冷笑道。
“你！”七王殿面庞扭曲，刚欲暴怒出手，却是见到林动那雷弓再度对着他举了起来，弓弦上，仿佛又是有着黑色长箭凝聚而来。
这一次再看着这黑色长箭，七王殿身体却是抖了抖，他不明白为什么林动的攻击突然间凌厉到了这种程度，在这之前，他即便是施展出祖符之眼，也不过是堪堪将他的魔皇甲撕裂出一道细小的裂纹，哪能像现在这般，不仅洞穿了魔皇甲以及他的身体，甚至连天空上的魔气罩都是给摧毁而去。
那般攻击，以他这般实力，怎么可能施展得出来？！
胸口的鲜血还在不断的流出来，七王殿眼神阴沉着，手掌一挥，只见得滔天魔海涌来，竟是将他的身形尽数的掩盖而去。
“林动，你也少在我面前装腔作势，先前那般攻势虽然厉害，但我才不信你能连续施展出来，今日不论如何，你这条命，我都是收定了！”
魔海之中，七王殿阴森森的声音，四面八方的传出，令得林动根本无法分辨出他的位置，显然这家伙也是极端的聪明，在吃了一次大亏后，再也不敢用身体来做靶子。
林动面无表情，只是握着雷弓的手掌缓缓紧了起来。
“五王，你们也随我出手，杀了这小子！”
魔海中，又是有着厉喝声传出，看来为了能够击杀林动，这七王殿已是不得不放下身份，要与其他五位异魔王联手了。
绫清竹听得此话，也是走到林动身旁，玉手紧握着青锋长剑，眸子冰彻的盯着天空上翻腾的而魔海。
林动袖袍一挥，吞噬天尸再度闪现出来，目光急速的闪烁着，那七王殿说得倒没错，先前那般攻势，消耗丝毫不比施展祖符之眼弱，原本以为凭借这般攻击斩杀这七王殿，但他总归还是小觑了异魔那顽强到极点的生命力。
而且以他现在的实力，也的确没办法连续的施展，眼下，怕也只能依靠吞噬天尸拖延一下时间，等他稍作恢复，便是继续催动雷弓黑箭，想来那七王殿再厉害，怕也是会受到无以伦比的重创。
不过，在他不能出手的这段时间，也不知道光靠绫清竹与吞噬天尸，能否将七王殿以及那五尊异魔王的攻势阻拦下来。
“杀！”
在他心中念头急转间，那魔气海洋中，已是有着充满着杀意的暴喝声响起，而后六道魔气光束，陡然对着他们暴掠而来。
“你先恢复。”绫清竹望着七王殿等人再度攻来，银牙轻咬，她显然也是明白此时林动的状态，而话音一落，她手中长剑也是涌上淡淡的光芒。
林动见状，顿时微急，那七王殿虽然被他所伤，但这些异魔生命极端的顽强，即便是这般伤势，依旧还有可战之力，绫清竹这般实力，如何能与他们正面相斗？
“哼，你们这些异魔，竟然敢跑到我黑暗之城来撒野，真当我黑暗之殿是软柿子不成？！”
然而，就在林动也要强行出手时，那遥远处，猛的有着一道怒叱之声传来，而后天际流光闪现，这片天地顿时黑暗下来，下一霎，只见得一道娇小倩影，已是带着两道人影暴掠而来，最后出现在了林动身旁，赫然是青檀以及那两位黑暗之殿的太长老。
“青檀？！”林动见到这突如其来的援手，也是一怔，旋即心中终是悄然的松了一口气。
“林动哥，你没事吧？”青檀看着林动那满手的血迹，顿时急急的道：“我们搜寻了黑暗之城千里之内都未找到你们的踪迹，还好先前这片有着能量波动传来，这才急忙赶了过来。”
林动摇了摇头，他抬头望着天空，青檀等人及时赶到，他倒是不用太忌惮这七王殿了。
天空上，魔海翻涌，那七王殿六人也是停了下来，他们望着赶来的青檀等人，面色也是有些阴沉，没想到原本以为手到擒来的事情，却是突然间变成这幅模样，他不仅未能将林动斩杀，反而被后者一箭射出了极重的伤势。
“七王殿，怎么办？”那五尊异魔王看向七王殿，低声问道。
七王殿眼中狰狞闪烁，他捂着胸膛，脸庞上掠过一抹苍白之色，旋即森然道：“既然来了，那就一并解决掉，一群蝼蚁，也妄想让我铩羽而归，痴人说梦！”
“是！”
听得七王殿声音之中满含的杀意，那五尊异魔王眼中也是杀意浮现，魔气涌动间，就欲彻底的出手将下方众人尽数的抹杀。
下方青檀见状，也是一声冷哼，小手一握，黑暗圣镰闪现出来，而后镰身之上光芒一闪，那镰灵便是被召唤而出。
一旁那两位黑暗之殿的太长老也是面色凝重，体内元力运转，随时准备动手。
而随着双方眼神渐冷，一场惊天动地般的大战，显然就要在此展开。
轰。
不过，就在那剑拔弩张的气氛浓烈到极致之时，这天地间，突然有着极端响亮的雷鸣之声响彻而起，场中众人皆是一惊，急忙抬头望向远处，只见得那视线尽头处，黑暗的天地突然璀璨起来，一片雷霆世界以一种惊人的速度蔓延而来，而在那漫天雷霆之中，一道高壮的身影，脚踏雷电而来，一股无法言明的恐怖气息，在此时弥漫了天地。
那七王殿在见到这一道脚踏雷电而来的身影时，那素来阴森的脸庞上，终是涌上了一抹惊悸之色，失声惊叫。
“雷主？！”


第1242章 雷主
“雷主？”
当这两个字自那七王殿嘴中传出时，在场所有人身体都是猛的一震，而后眼神惊异的望着远处，那里，雷霆世界轰隆隆而来，而在那漫天雷霆之中，一道壮硕的身影，脚踏雷电朝前一跃，便已是万丈之外。
林动同样是有些震动的望着那道踏雷而来的身影，这一位，便是炎主口中的那远古八主之一的雷主吗？
那道人影的速度极快，当雷声响彻时，尚还在那天际之边，而待得雷声落下时，只见得这片天空之上雷光汇聚，一道身影，便是这般的出现在了所有人的视线中。
而此时林动等人方才能够看得清晰，来人身材高壮，约莫中年之岁，一头银色短发，他模样颇为的粗犷，浓眉之下，有着一双雷电双瞳，那双瞳内，充满着冷肃与狂暴，在其赤着的胳膊上，有着一道雷霆图腾，一种古老而狂暴的波动，若隐若现的散发出来，令得天地都是在微微的颤抖着。
轰隆！
天地间，雷霆愈发的密集，他那对银瞳缓缓的凝聚在七王殿等人的身上，一种无法形容的压迫，弥漫而开。
“我正在满世界的寻你们，没想到，你们还真的敢露面。”
他盯着七王殿，低沉的声音，仿佛雷鸣一般，突然的在这片天地间响起，空气都是在那声音之下嗡鸣震动起来。
“雷主，你还真是阴魂不散啊！”七王殿望着雷主，面色阴沉，咬着牙道。
“我与你们，还有任何话好说吗？”雷主淡淡的道，他脸庞始终都是那副不言苟笑的模样，给人一种极端冷厉的感觉。
“既然找到了你们，那你便告诉我一声，其他那些家伙如今都躲在哪里吧。”雷主盯着七王殿，声音低沉。
“嘁，你还是跟以往一般喜欢说大话，凭你一人，还想与我魔狱抗衡？若是如此的话，当年你们会被逼得尽数沉睡吗？”七王殿冷笑道。
“至少，你还没资格与我这般说话！”
雷主漠然的道，旋即其眼中猛的有着雷芒掠过，大手猛的一握，只见得无尽雷霆在其掌心凝聚而来，直接是化为六颗不过拳头大小的雷球，雷球之上，雷弧疯狂的跳跃着，一股毁灭般的波动，弥漫而开。
“咻！”
他没有任何的客气，手掌一挥，那六颗雷球瞬间消失而去，下一霎，便是自那七王殿六人身后空间之中暴射而出。
七王殿急忙转身，滔天魔气暴涌，那被魔皇甲覆盖的手掌一拳轰出，便是狠狠的轰在那那颗雷球之上。
嘭！
雷光疯狂的暴溢而开，七王殿喉咙间也是传出闷哼之声，身形直接是被震飞数百丈，那条手臂，一片焦黑，想来若不是有着魔皇甲保护的话，光是这一道攻击，就足以废掉他一只手臂。
啊！
不过七王殿能够将这般攻击抵御下来，那另外五名异魔王却是没了这等实力，雷球冲来，他们身体之上的魔气几乎是瞬间崩溃而去，狂暴无比的雷霆之力在他们面前爆发开来，五人皆是发出凄厉惨叫声，而后他们的身体，竟然是在那等雷芒之下，以一种惊人的速度被消融而去短短数个呼吸间，五名轮回境强者，便是被雷主一手抹杀！
这般一幕，看得林动也是微微一惊，那黑暗之殿的两名太长老，更是目瞪口呆，浑身颤抖不已，这般实力，几乎让人没有反抗的勇气。
“你！”七王殿见到五尊异魔王转瞬间被雷主抹杀，也是气得咬牙切齿，那眼中杀意暴涌。
“现在可以说其他的那些家伙躲在哪里了吗？特别是天王殿那个家伙，他躲了万千载，还不肯露面吗？”雷主眼中没有泛起丝毫的波动，淡淡的道。
“天王殿？”七王殿闻言，眼中却是掠过一抹讥讽之色，道：“雷主，当年你便是差点死在他手中，如今还敢找他？我看，你还是将冰主请出来吧！”
“看来你是不想说了…”
雷主微微摇头，那雷瞳之中猛的有着惊人的杀意爆发而出，旋即他一步跨出。
就在他步伐跨出时，那七王殿瞳孔也是一缩，魔气爆发而开，而其身形却是化为一道黑光闪电般的暴退。
“你的速度，太慢。”
他身形刚退，其身后空间便是被撕裂而开，雷主的身影鬼魅般的浮现出来，他五指紧握，雷光在手臂之上疯狂跳跃，而后一拳轰出，重重的轰在那七王殿后背之上。
嘭！
雷光迸射，七王殿身形化为一道黑光坠落而下，直接是将一座山峰震得塌陷而去。
雷主双指并曲，凌空一点，一道恐怖至极的雷光喷涌而出，狠狠的冲进那崩塌的山峰之中，顿时那一片片废墟化为灰烬，那其中的一道身影尚还来不及躲避，便是被生生轰中，而后一口黑血喷出来，身体在下方地面搽出一道上万丈的深深痕迹。
这先前将林动逼得险象环生的七王殿，在这雷主手中，竟是处于完败的下场！
七王殿狼狈的自地面上撑起身子来，他目光狠毒而狰狞的望着雷主，身体之上的魔皇甲出现了不少的凹痕，他的实力，处于渡过两次轮回劫的层次，而雷主，却早已是渡过三次轮回劫的最巅峰强者，若不是他有着魔皇甲护体，根本现在早已是重伤。
“这么多年了，你还是只能依靠这一身龟壳吗？”雷主望着狼狈的七王殿，摇了摇头，道：“这魔皇甲虽然厉害，不过我若是要杀你，它恐怕还保不住你的性命。”
七王殿面色阴沉，一声冷哼，双手猛的结印，只见得滔天魔气席卷而出，他的身躯竟是扭曲起来，下一刻，他居然生生的化为十数道身影，而后对着四面八方逃窜而去。
那些身影，全部都是拥有着相同的气息，甚至连魔气波动都是如出一辙，显然，这些都是真正的七王殿，而并非是什么残影幻象。
雷主淡漠的望着施展出这般手段的七王殿，下一霎，他的身影，诡异消失。
轰！
就在雷主身影消失的那一霎，远处雷声响彻，林动抬目望去，只见得一道雷光犹如瞬移一般出现在一道黑影之前，一拳便是将其轰爆，下一瞬，这道雷光，又是出现在了另外一道黑影之前…
砰砰砰！
雷光闪烁，雷主展现出了一种恐怖得无法形容的速度，那十数道黑影尚还未冲出多远，便已是被尽数的轰爆。
砰！
最后一道黑影，也是被一拳轰中，但却并未再爆成魔气，而是身体倒射而出，黑血狂喷，显然，这家伙的真身被轰了出来。
“在我面前想要逃跑，是你变愚蠢了吗？”雷主漠然的望着那狼狈异常的七王殿，雷瞳之中，有着一股令人心悸的杀意缓缓的涌起来。
“既然你不愿意说，那你也没了什么作用，还是杀了吧，免得污了这片天地。”雷主大手一握，只见得无尽雷霆在其上空凝聚，最后化为一方约莫千丈大小的雷印，雷印之上，有着狂暴无比的波动弥漫而开。
轰！
雷印一成形，却是没有丝毫的犹豫，直接是蕴含着毁灭之力，狠狠的对着那七王殿镇压而去，那片大地，都是在瞬间崩塌成一片深渊。
七王殿望着这般攻势，眼神也是一变，滔天魔气涌动，死死的抵御着雷印，不过那等魔气，在雷印的镇压下，却是愈发的薄弱。
雷印距离七王殿越来越近，后者牙齿也是紧咬起来，眼中凶芒不断的闪烁着。
轰！
雷印最终轰然落下，然而，就在雷印即将镇压七王殿时，只见得他身后的空间，突然撕裂道一道千丈裂缝，而后一只千丈庞大的魔手，猛的自那裂缝之中探出，将那雷印生生顶住，并且一掌拍飞而去。
这般变故，令得林动等人都是一惊，目光望去，只见得在那千丈魔手之上，竟是有着一道黑影负手而立。
“呵呵，雷主，这么多年不见，何必这么心狠手辣？”魔手之上的黑影冲着雷主微微一笑，道。
“五王殿？”
雷主望着那道现身的魔影，那雷瞳之中终是掠过一抹波动，道：“总归是出来了一个够看的啊。”
“雷主，我知你厉害，不过我若是要救人走，你也拦不住，所以也犯不着白白费力气了，你放心吧，我魔狱与你们这些远古之主，终归会有一战的。”
那道魔影轻笑一声，旋即他目光看了一眼林动所在的方向，道：“你便是那个林动？的确是有些不简单，没想到连老七都失手了，不过这一次，便算你好运吧…”
话音落下，他袖袍一挥，便是有着魔气将那七王殿席卷着，身后空间，再度崩裂而开。
“哼！”
雷主见状，却是一声冷哼，一拳轰出，漫天雷霆化为一头万丈庞大的雷狮，而后呼啸而出，携带着狂暴无比的力量，冲向那五王殿。
“呵呵，还是这般不客气，不过现在可不是与你动手的时候，雷主，等着吧，这一次，我异魔必会逆转万千载之前的那场战争。”
那五王殿袖袍挥动，一朵巨大无比的魔莲绽放而开，竟是将那雷狮抵御而下，而后他的身形，则是退进了那片空间裂缝之中，接着魔气散去，终是彻彻底底的消失在了这天地之间。
雷主望着那五王殿消失的地方，屈指一弹，一道细微的雷芒便是穿透空间，悄无声息的消失而去。
做完这些，他方才转过身来，目光盯着林动，半晌之后，方才缓缓的道：“你就是炎主所说的那个林动？这一代的雷霆祖符掌控者？”


第1243章 魔狱之事
林动听得雷主此话也是微微一怔，后者能够感应到他体内的雷霆祖符倒是并不奇怪，不过听其口气，显然是在与炎主交流之中便是听说过他。
虽然不知道炎主是如何与他说的，但林动依旧还是点了点头，旋即冲着雷主拱手道：“今日多谢雷主出手援助了。”
虽然今日的事，真要彻底的拼起来，那七王殿也并不容易如愿，只不过那样一来，林动他们也得付出一些代价，因此雷主这援手之情，倒不得不谢过。
“斩杀异魔，本就是我的义务与责任，我追踪他们这么久，今日却是因为你方才有了一些线索，所以你倒也不用谢我。”雷主摇了摇头，声音洪亮如雷鸣。
雷主的目光扫视着林动，眼神之中有着一种审量的味道，最后他微微点头：“没想到以你这般实力，竟是能够攻破那七王殿的魔皇甲，看来真如炎主所说，你并不简单。”
他的声音中，罕见有着一抹惊讶之色，他与这些异魔最高层打的交道实在是太多，自然也是很清楚那魔皇甲的厉害，正凭借着这个，那七王殿方才能够在他手中坚持这么久的时间，而眼下林动，却是能够凭借自身力量将魔皇甲攻破，这即便是雷主，也是忍不住的有些微讶。
林动只是笑了笑，那般攻击，乃是汇聚了他体内两大祖符之力，再加上那种感应太上而来的神秘力量方才最终取得这般战果，说起来，也算是他如今最强的手段了，但可惜，即便是如此依旧只能够击伤那七王殿，想要将其斩杀，却是不可能的事。
雷主看着林动，他突然犹豫了一下，看着林动，道：“我能看看雷霆祖符吗？”
他此时的语气，比起寻常说话要放低了一些声量雷霆祖符，乃是天地神物，而他则是在雷霆祖符诞生后的第一任掌控者，不过他也很清楚，这般天地神物，会自主寻主，当年的他在沉睡之时，便算是与雷霆祖符断了那层关系而现在，它的新任掌控者，是林动，甚至于连他都没有资格说什么收回。
林动听得雷主此话，倒是一笑，然后点点头伸出手掌，掌心雷芒凝聚，而后一道古老的雷符缓缓的浮现出来。
雷霆祖符一出现，便是闪烁着雷光轻轻的飘向雷主，后者则是满脸的怀念，仲出大手抚摸了一下，笑道：“好久不见啊，老伙计。”
嗡嗡。
雷霆祖符发出细微的嗡鸣之声，雷弧缠绕在雷主手掌上雷霆祖符同样是有灵，而对于这第一任掌控者，它同样是有着感情。
雷主抚摸着雷霆祖符，好半晌后，方才将其递还给林动，道：“如今你是雷霆祖符的掌控者，应该也明白这个身份的一些责任，希望你不要辱没了它的名头。”
林动握着雷霆祖符，雷光在其手中闪烁着，旋即他看着雷主，道：“你不想将雷霆祖符拿回去吗？没有了雷霆祖符，雷主之名，总归是有些缺陷吧？”
雷主摇了摇头，道：“远古八主，的确与八大祖符拥有着莫大的联系，但从某种方面而言，我们只是相当于战友一般的关系，它们是自主的，拥有着挑选新掌控者的权利，如果现在的雷霆祖符是无主之物，那我自然会带走它，但它已经为自己挑选了新的主人，既然如此，那便是最适合它的，祖符的选择，是不会出错的。”
说到此处，雷主一笑，笑容有些傲意：“而且你也别太小看我们了，的确祖符能够让得我们的实力锦上添花，但即便是缺少了它们，我们同样强大，因为在那长久的相伴中，我们已是同化了它们的力量。”
林动闻言，却是一愣。
“不明白？”雷主脸庞上缓缓的泛起一股神秘之色，旋即他缓缓的伸出手掌，手掌一握，只见得雷光闪烁出来，那雷光之中，有着林动极为熟悉的力量，那是，雷霆祖符的力量。
“现在的我们…虽然不再拥有祖符，但我们…却是能够称为…人类形态的祖符。”
“所以，即便是缺少了祖符，但我们依然拥有着那种力量…只不过一些极端厉害的手段，还是要借助祖符方才能够施展罢了。”
林动脸庞上也是有着惊异之色涌出来，难怪当炎主，雷主这些远古之主再度苏醒后，并没有太过执着于祖符，原来祖符的力量，已是在他们体内生根发芽…
“不过一般只有将祖符掌控到极致的程度，方才能够在离开祖符后，依旧具备着这种力量，虽然在我们之后，祖符都是经历了不少优秀的掌控者，但能够达到这一步的，却是极为的罕见。”主看着林动，道：“你或许以后能够达到这一步，而且你体内｜还有着两道祖符，那时候力量同化下，真不知道究竟会发生什么样的变化。”
林动默默的点头，如今的他对于祖符的掌控也算是炉火纯青，但显然，或许正因为是身怀两大祖符的缘故，他暂时还无法达到类似雷主他们这种极致的地步。
雷主看着林动将雷霆祖符收进体内，突然沉默了一下，道：“那个，冰主的事，炎主已经与我说过了。”
林动一愣。
“当然，其他那些苏醒过来的家伙，应该都知道这个事情了。”
林动嘴角微微抽了抽，眼角余光看了一眼身旁的绫清竹，她却只是静静的望着远处山林，仿佛并未听见他们所说一般。
“事情突然间变成这样，我们也挺吃惊，原本我们以为，以小师妹的心性，恐怕始终都不会对某个男子有着男女之间的感情。”雷主看向林动的目光也是有点古怪，当日在听见炎主与他的传音后，他也是呆愣了好半晌，显然一时间是有些无法想象那冷得犹如万年玄冰的小师妹，有一天竟然也会这样…
“她不是她。”林动轻叹了一声，道。
雷主淡淡一笑，眼神略含深意的盯着林动，道：“或许吧…既然小师妹发了话，那我们暂时也不纠结于此，不过…还是要提前告诉你一声，她，终归是需要回来的。”
“因为…这片天地，还需要她。”
林动深吸一口气，旋即缓缓的点头：“我知道，我会以我的方式来改变，或许这在你们看来有些可笑，不过，放弃的话，不是我的性子。”
雷主似是知道他会这般回答，也就不再多说什么。
“你接下来准备去哪？”林动将话题转移而开，问道。
“先前我在那七王殿身上留下了雷印，能够隐约的感知他们的大致方位，我需要去探测一下，这些家伙潜藏这么多年，总是让得我有些不安。”雷主沉吟道。
“你一人？”林动一怔，虽然雷主实力的确恐怖，但那魔狱也不是省油的灯，例如先前出现的那位五王殿，显然便是一尊堪比渡过三次轮回劫的巅峰强者，而在此人上面，应该还有着四位实力同样处于这个层次的强大存在，雷主一人想要与他们周旋，显然太过的勉强。
“魔狱之中的那几位最强大的家伙，当年同样受了极重的伤势，如今应该都是在竭力的恢复之中，所以真要正面抗衡的话，魔狱终归还是不太敢的，不然的话，也不会一直的潜伏着。”雷主道。
话音落下，雷主淡淡一笑，道：“而且即便是遇见危险，我要脱身的话，就算是那最为强大的天王殿也留不住。”
他的话语中有着许些傲意，身为雷霆之主，他的速度，在远古八主中也是出类拔萃，想要留他，可并不容易。
“天王殿？”林动看了看雷主，先前他从七王殿的嘴中也是听过这个名字。
“应该是魔狱的老大吧，一个极端难缠的家伙，当年异魔皇麾下最强的存在，我当初与他交过手，算是不敌他。”提起这个名字，雷主的眼神显然也是凝了凝，道：“不过这家伙当年被小师妹重伤，想来依旧还未恢复过来，不然他出现的话，也就只能小师妹才能对付他了，而那时候…”
说话此处，他看了看林动，那意思很明显，一旦这天王殿现身，那么…冰主也就该出现了，这或许便是他们给予林动的最后时间了。
林动手掌微微紧了紧。
“我也该动身了，这一次魔狱对你的行动失败，接下来应该不敢再有什么动作了，在魔狱那几位最厉害的家伙都未曾彻底恢复前，想来他们会‘安静一些。”雷主道。
林动点点头：“小心。”
雷主行事颇为的雷厉风行，该说的一说完，便是不再有丝毫的停留，冲着林动等人大手一挥，周身雷光便是璀璨起来，而后雷鸣响彻，雷光闪烁间，其身形竟已是消失不见。
林动望着雷主消失的地方，也是轻叹了一口气，这天地间，真是越来越乱了啊，那些潜伏的异魔，真不知道会在什么时候，彻底的迸发。
而一旦到了那时候，显然，第二次的天地大战，也将会再度开启。


第1244章 再回异魔域
经历七王殿突袭的事之后，黑暗之城的防御显然是变得森严了许多，不过林动他们却是明白，这种防卫对于七王殿那种程度的强者而言没有太大的效果，总归到底，还是要依靠他们自己的力量。
但好在如今那七王殿的突袭被阻，而其本身也是重伤而退，按照雷主所说，接下来他们应该不敢再轻易的有动作，毕竟魔狱虽然厉害，但如今这天地间局势微妙，在没有一些把握的情况下，他们也并不敢轻易的大肆行动。
而林动数人，再度在黑暗之城停留了一些时日，青檀也终是将黑暗之殿人心稳定了下来，到得现在，这黑暗之殿，算是彻彻底底的被她握在了手中，再也不会有任何的反对之言出现。
这般时候，便是可以先将这小丫头带回东玄域了。
黑暗之城，一片广场之上，天空中有着一片空间阵法缓缓的扭曲着，一股股狂暴的空间力量散发出来。
“殿主，空间挪移阵法已是布好，随时可以动用。”那一名黑袍长老望着成形的挪移阵法，抬头对着身旁的黑裙女孩恭声道。
青檀闻言，也是轻轻点了点头，道：“程长老，在我暂时离开的这段时间，黑暗之殿便由你代本殿掌管，若是殿内有大事出现，便捏碎我给你的传音符，我会立即赶回来。”
“是。”那位黑袍长老闻言，顿时恭敬的应道。
青檀吩咐完毕，也是螓首微点，而后娇躯一动，便是出现在不远处的平台上，那先前还略显威严的小脸立刻有着甜甜的笑容涌起来，她抱住林动的手臂，笑道：“林动哥，我们可以动身了。”
林动闻言，也是笑着点点头，看向绫清竹：“那我们走吧。”
话音落下，他袖袍一挥，三人便是出现在那空间挪移阵法之中，而此时下方则是有着无数黑暗之殿的强者恭敬的跪伏而下。
“恭送殿主。”整齐而恭敬的声音，在此时响彻起来。
青檀冲着下方微微点头，而后小手一扬，那空间挪移阵法便是被她催动，顿时有着狂暴无比的空间波动弥漫开来，璀璨的银光涌动间，将三人的身形尽数的包裹。
“嗡。”
璀璨光芒爆发而开，众人眼睛一眯，待得再度睁开时，天空上的三人已是凭空消失而去，唯有着那片空间，还呈现着一片扭曲之状。
待得林动三人撕裂空间出现在东玄域时，已是三日之后，这般效率比起去时显然是要快上数倍，而且以后再想过去，也是能够凭借着留下的空间印痕轻易的抵达，再也不用去穿越那千万大山。
林动自扭曲空间中走出，他目光四处扫视了片刻，突然望向了北方，眼中掠过一抹莫名之色：“那里是？”
绫清竹也是望了望，旋即她看了林动一眼，轻声道：“那里是异魔域的方向。”
“异魔域啊…”
听到这个给林动留下极深印象的地方，他也是忍不住的轻叹了一口气，当年，正是在那里，他们三兄弟被元门逼得狼狈逃离。
“既然来了这里，那便再去一趟异魔域吧。”林动道，他还记得，当年在那异魔域中，曾经有着一位名为焚天老人的前辈，于他有着一些恩情，只不过后来他却是肉身坐化，将一位异魔将所镇压。
当年的那异魔将在林动眼中，几乎无可匹敌，反而如今，或许他反手将便是能够将其抹杀无数次。
绫清竹与青檀自然是对他的决定没什么异议，当下都是微微点头，林动见状，也不迟缓，手掌一挥，三人已是化为流光掠过天际。
短短数分钟的时间，数千里已是跨越而过，一座巨大的城市，则是出现在了林动视野之中，他望着这座还有些熟悉的城市，眼中也是有些缅怀，当年正是在这里，他们三兄弟，与元门展开了血拼，不过那结局，却是并不太好。
林动注视着异魔城，好半晌后方才再度动身，而下一刻，他们已是出现在了那异魔域之外，前方的空间微微荡漾，一道古老的阵法若隐若现，这是自远古便是流传下来，封印着异魔域内魔气的阵法，当年林动他们想要进入，却是必须等到封印减弱时方才能够进去，但眼下，他们显然已不再需要这等繁琐，袖袍一挥，只见得那片空间便是被生生的撕裂而开，三人身形一动，直接是掠了进去。
在他们进入异魔域时，那下方还有着不少等待着异魔域开启进入冒险的强者，他们望着三人这般可怕的手段，皆是一脸的呆滞。
对于这些小插曲，林动自然是未曾理会，进入异魔域后，他视线一扫，数年时间不见，这异魔域依旧有着淡淡魔气缭绕，放眼望去，一片古老与沧桑，那地面上一道道巨大的沟壑，显露着当年这里曾经爆发的惊天大战。
林动三人并未过多停留，直往异魔域深处而去，以他们如今的实力，这曾经东玄域的禁地显然没有丝毫的阻碍性，因此不过十数分钟的时间，他们已是抵达异魔域深处，凭借着三年之前的记忆，林动最终逐渐的停下了身子，目光看向不远处，那里，一座巨大的赤红岩浆火山矗立，而在那火山之殿，隐约可见一道犹如石像般的身影。
林动身形缓缓的飘近，只见得那道影子，犹如雕像，身体之上布满着尘埃，而那岩浆火山中也是有着一股能量波动散发出来，令得人无法的靠近，不过如今这种波动对于林动而言，显然不具备丝毫的效果。
“前辈。”
林动冲着那道坐化的人影微微弯身，虽然如今他的实力恐怕早已超越了这位焚天老人，但后者当年却是在坐化之际，将体内残余的生气尽数的凝结成丹，然后送予了他，这份恩情，林动可一直的记着。
“这位前辈已经坐化，难道还能有救？”绫清竹清眸望着那道布满着尘埃的人影，轻声道，当年这里所发生的事情，她也是在场，自然是明白林动受了这位前辈不少的好处。
“他的情况有些特殊，并不算完全的坐化，当年为了镇压那异魔将，他将自己炼制成活死人，以此求得不灭，欲要将那异魔将永远镇压，所以，从某种方面而言，焚天前辈并不算彻底坐化。”林动微微摇头，当年在离开时，他曾问焚天老人，后者则是说，若想要救他，等他参悟了轮回再来，现在的林动虽说还未真正的晋入轮回境，但也算是触及轮回，精神力更是达到了大符宗的境界，即便是寻常轮回境强者，也完全不是他的对手。
“这位前辈将自己炼制成活死人，不过眼下他体内死气鼎盛，生气尽消，所以方才会坐化而去，若是能够为其补充足够的生气，应该能让他再度苏醒。”青檀大眼睛看了一眼这道坐化的人影，想了想，说道。
林动颇感赞同的点点头，旋即他冲着那道人影微微一笑，道：“焚天前辈，当年你将体内生气尽数的送予我，今日晚辈便将这番恩情还与你。”
话音一落，其手掌伸出，顿时有着滔滔元力席卷而出，那元力之中，生死之力完美相融，而林动心神微动间，那元力之中便是有着浓浓的白芒被剥离而出，在那白芒之内，弥漫着精纯到极点的生气。
生气弥漫，林动屈指一弹，只见得那浩瀚生气竟是化为一道巨大的漩涡，漩涡中心，便是焚天老人那道坐化的人影。
生气弥漫，然后源源不断的对着那道人影身体之中涌起，那等生气，竟是令得这岩浆火山山顶，都是有着碧绿生长出来。
这焚天老人体内的死气显然是极端的浓郁，想要令得他体内的生死达到平衡，即便是轮回境的强者办起来都是有点难度，不过好在如今林动今非昔比，体内元力浩瀚如海，再加上精神力相助，待得半日时间过去，只见得那焚天老人干枯如树皮般的皮肤，竟是逐渐的变得松缓下来，一丝淡淡的生气，开始从他体内渗透出来。
在灌注着元力时，林动心神微动，那传承自吞噬之主的一些轮回之意，也是融入那生气之中，而后漫天生气涌动着，竟是化为一枚乳白色的丹丸，直接顺着焚天老人天灵盖没入他身体之中。
这焚天老人本就是触及轮回的顶尖强者，距那轮回境仅有一步之遥，当年他突破失败，如今若是能够有着这吞噬之主的轮回之意相助，或许还能够再度冲击轮回境。
当那白色丹丸没入焚天老人身体时，后者那近乎已经死去的身体，突然在此时猛的颤抖了一下，而后那苍老的脸庞上，有着红润浮现，那对眼睛，也是缓缓的睁开。
他茫然的睁开眼，再然后，便是见到了身前那一脸微笑的年轻脸庞。


第1245章 唤醒焚天
焚天老人缓缓的睁开双眼，他望着面前的青年，那张脸庞依稀的有着一些熟悉的味道，但他皱着眉头，却始终都无法让得自己神智彻底的清晰，虽然将自己炼制成活死人令得他能够长久的存在下去，但显然，与正常人来相比，终归还是有些区别的。
“你…”焚天老人茫然的道。
“焚天前辈，晚辈林动，三年之前曾与你有过一面之缘。”林动微微一笑，旋即手掌一握，一尊赤红鼎炉便是出现在了其手中，赫然便是当年焚天老人曾经送予他的焚天鼎，这件宝贝，也是伴随着林动渡过不少难关，甚至还救过他的性命。
“这是…焚天鼎？”
焚天老人怔怔的望着那焚天鼎，那茫然的眼神顿时清醒了一些，他连忙将其接过，目光打量着林动，片刻后，他似是记起了什么，震惊的道：“你…你是当年那个小家伙？”
焚天老人并没有太过清晰的时间概念，在他有些模糊的意识中，他记得似乎是在很多年前见过林动，然后还将这焚天鼎送予了他。
只不过，他却是未能想到，当他再度从那黑暗之中走出来时，那个当年在他眼中嬴弱不堪的青年，却已是拥有了极端可怕的实力。
他能够清晰的感觉到从林动身体之上传来的压迫，那种压迫感，令得他体内的生死之气都是略微的有些迟缓。
林动轻轻点头，道：“当年前辈告诉我，想要救你，便参悟了轮回再来，虽然如今尚未到那一步，不过我想，应该还是具备了将你唤醒的力量。”
焚天老人怔然，他眼神复杂的看着眼前的青年，当年他只不过是以为自身已必死，这才将体内残余生气尽数的赠予林动，那原本只是临死前的一点无心之举，但哪料到，在这数年之后，那留下的一丝善缘，却是让得他拥有着如此大的回报。
“小友…谢谢了。”
焚天老人长叹一声，对着林动缓缓的行了一礼，他当年给予林动的与如今他所收回的好处相比，显然是不值一提。
“前辈勿要客气，如今你体内生死之气再度平衡，若是能够晋入轮回的话，说不得便是能够脱离那种活死人的状态，再度拥有真正的生命。”林动摆了摆手，道。
“唉，想要摆脱这般活死人状态，哪有那般容易，这种状态，根本不可能再拥有轮回的感应。”焚天老人叹了一口气，道。
“那可不一定，前辈你感应下体内。”林动神秘一笑，道。
焚天老人闻言也是一怔，而后依言略作感应，紧接着他那苍老的脸庞上便是有着一抹惊色浮现，而后惊色变浓，最后化为狂喜之色：“这…这是轮回之意？”
“我曾自一前辈那里获得传承，这些轮回之意今日便送予焚天前辈了。”林动笑道。
焚天老人激动得身体连颤，老泪纵横，他原本以为这一生都将会在这种令人生不如死的状态之中渡过，但哪能料到，如今不仅被唤醒，而且他的体内，再度拥有了轮回之意，凭借着这些轮回之意，他也是拥有了再度冲击轮回境的资格。
林动望着激动无比的焚天老人，也是微微一笑，虽然后者当日给予他的机缘或许如今看来不值一提，但对于他这种为了镇压异魔，甘愿将自身炼制成活死人的大义却是极为的敬佩，如今能够帮其一把，他心中也是极为的欢喜。
“小友，大恩大德，没齿难忘，以后若是有任何差遣的地方，老夫定是万死不辞！”焚天老人激动的对着林动行了一个大礼，但却被后者连忙阻拦了下来。
“前辈太客气了。”林动笑着摇了摇头，刚欲说话，其神色突然一动，因为他感觉到这座岩浆火山突然发出细微的颤抖，那山底之处，竟是有着浓郁的魔气散发出来。
“糟了，那被我镇压的异魔将也苏醒了。”焚天老人见状，却是一惊，他坐化在此处，镇压着那异魔将，如今他苏醒过来，那异魔将竟然也是随之苏醒了过来。
轰！
就在他声音落下间，那岩浆底部魔气愈发的浓郁，下一刻，魔气竟是顺着岩浆巨山侵蚀而上，短短十数息的时间，整座岩浆巨山，都是被那魔气化为灰烬。
随着岩浆巨山的消失，一股极端惊人的魔气，顿时冲天而起，在那魔气之中，一道魔影仰天咆哮，那等声势，比起当年，竟然要强上无数。
“这…”
岩浆老人感受着那股魔气的强横，面色却是猛的一变，骇然失声道：“怎么可能？这异魔将竟然进化了？！”
林动闻言，眼中也是掠过一抹惊异之色，他自然是能够感觉到，这异魔将的实力突飞猛进，显然是达到了异魔王的层次，这家伙倒是厉害，竟然能够在被封印的这些时间中，依旧储存足够的力量，冲击异魔王的层次，而且还被他冲击成功了。
“糟了，林动小友，你们快走，这异魔将已是成功进化成王，堪比轮回境的强者，我也完全镇压不住了。”焚天老人急忙道。
“桀桀，焚鬼，你想不到吧？本将不仅未在你的镇压下化为灰烬，反而是绝境中寻找得突破，现在，本将已是晋入王级，你如何与我相斗？”魔气滔天，那道魔影脚踏天空，他望着焚天老人，那阴森的脸庞上，满是森寒之色。
焚天老人一咬牙，就欲拼命将这异魔王阻拦，但却是被林动伸手阻拦下来，后者望着那猖狂大笑的异魔王，却是笑着摇了摇头道：“连魔狱七王殿我都交过手，你这一个刚刚突破到王级的异魔竟然也敢在我面前猖狂，倒是有些可笑。”
猖狂大笑声噶然而止，那异魔王有些惊异的盯着林动，旋即讥讽一笑道：“你这小子说些什么胡话，七王殿那是能够与远古之主争锋的大人物，你这家伙，在大人眼中，就是蝼蚁般的存在。”
林动笑笑，却是不再与其争辩，只是那双眸之中有着冰寒之意缓缓的涌上来。
轰隆。
突然间，雷鸣之声在这片天地间响彻，天空之上雷云汇聚而来，无数雷霆在其中狂暴舞动，一股极端惊人的波动，弥漫而开。
那异魔王也是因为这般变故一惊，猛的抬头，然后眼神惊疑的望着这一幕一股浓浓的不安，涌上心头。
“这小子不对。
这异魔王也不是傻子，见到这般阵仗，他心中也是的有着危险升起，目光闪烁，竟是猛的暴退而去。
原本准备与其大战的焚天老人见到他这般仓惶而退而是一惊，不过还不待他说话，林动却是淡淡一笑，手掌一握，只见得那异魔王四周空间猛的扭曲，四道巨大的黑洞漩涡成形，将其困于其中，狂暴无比的撕扯力爆发间，顿时将那异魔王骇得疯狂运转魔气死死抵御。
轰！
天空上，无数道雷霆汇聚在一起，最后竟是化为一道数千丈庞大的雷龙张牙舞爪的呼啸而下，最后直接是在那异魔王骇然的目光中，冲进那封锁的黑洞囚牢之中。
嘭！
一轮雷日，在那天空上爆发而开，狂暴而雄浑的雷霆之力弥漫而开，竟是连这异魔域上空的魔气，都是被清楚了不少。
凄厉惨叫声，自那雷日之中传出，然后焚天老人便是目瞪口呆的见到，那一道刚刚方才完成晋级而猖狂无比的异魔王，竟是在此时以一种惊人的速度消融而去，转瞬间，便是彻彻底底的消失在了这天地之间。
“这…”
焚天老人眼中满是震动之色，他与这些异魔打了不少的交道，自然是明白他们那顽强到极点的生命力，一尊异魔王，就算是同为轮回境巅峰强者，也难以彻底的将其击杀，唯有着施展手段将其封印，但眼下，这林动却是在举手投足间，将一尊异魔王干干净净的抹杀，这般能力…委实太恐怖了一些。
林动轻轻拍了拍手掌，天空上的雷云消散而去，那眼神倒是未有丝毫的波动，以他如今的实力，寻常异魔王完全不是他的对手，更何况，这家伙－不过只是堪堪晋入王级而已，这就敢在他面前猖狂，倒的确是不知道死字是怎么写的。
“林动小友的手段，真是让老夫震撼，先前倒是我失态了。”焚天老人缓缓的回过神来，目光直直的望着林动，叹道。
他这才明白，当年那个需要受他恩惠的青年，如今究竟是强到了何种恐怖的程度。
林动笑了笑，目光望着那消散而去的异魔王，旋即望向下方的大地，眉头忍不住的皱了皱，按照常理而言，若是陷入封印之中，那家伙即便不会越来越虚弱，但也不可能在封印中晋级，为何这家伙会这般的奇怪？
要知道，将级与王级之间，可是拥有着极端庞大差距的，想要晋入，可是难上加难。
林动皱眉沉思了片刻，却是没什么理路，只能摇摇头，或许是这家伙运气好吧。
“焚天前辈，眼下这异魔被除，你是打算去哪？”林动将目光看向焚天老人，道。
“我这老命，是小友所赐，既然这家伙被除掉，若是小友不嫌弃，我便跟在你身旁吧。”焚天老人想了想，笑道，他本是远古之人，如今倒是一人不识，也没个安身之地。
“我正好要回我的家族，既然焚天前辈无处可去，若是有兴趣的话，倒是可以留在我家族之中做个客卿长老。”林动一笑，正好，焚天老人实力强横，他若是能够留在林家的话，倒也是能够帮他保护一下家人。
焚天老人本就无处可去，对此倒是没什么异议，当即笑着点点头。
林动见状，也就不再此处继续的停留，手掌一挥，便是对着异魔域之外掠去，而在他要掠出异魔域时，却是忍不住的再度回头看了一眼这片大地，不知为何，在其内心极深处，升腾起一丝淡淡的不舒服之感，只不过这种感觉极淡，一闪即逝，而他眼中也是掠过一抹疑惑，身形闪烁间，已是消失而去。


第1246章 相遇
异魔域之外，林动四人闪现而出，焚天老人望着如今这万千载之后的天地，也是忍不住的有些嘘唏。
“那我们便直接先赶向大炎王朝吧。”解决了焚天老人的事，林动心情显然也是颇好，当即笑道。
听得他的话，青檀与焚天老人倒是没有丝毫异议，倒是绫清竹贝齿轻咬着红唇，那清眸盯着林动，缓缓的摇了摇头。
“我就先回九天太清宫了，大炎王朝与我们宫门在两个方向，我们便在此处分别吧。”
“你不去大炎王朝了？为什么？”林动也是有些错愕的看着绫清竹。
绫清竹清澈的眸子停留在林动的脸庞上，旋即她淡淡一笑，道：“你这是要带我去你家？你知道这会代表什么吗？”
林动哑然，若是将绫清竹带回大炎王朝的话，那岂不是有着媳妇见公婆的味道？
“若你愿意去的话，想来我家里人都会很欢迎。”林动想了想，道。
“等你先将自己的心弄明白后，再来说这句话吧，不然的话，或许你心中总归会感觉到一点不公平。”绫清竹似是微微一笑，道。
林动愣下来，那面色不由得微微有些变幻，他自然是明白绫清竹所说的意思，她内心高傲，即便林动要带她回大炎王朝，她也必须要林动心甘情愿得没有半丝的累赘。
绫清竹望着林动的面色，心中轻轻一叹，薄纱下的红唇掀起一抹略显苦涩的笑容，而后不再犹豫，转身飘然而去。
“你已习会太上感应诀，我也没了留下来的必要，九天太清宫尚在重建中，我身为宫主不宜离开太久，所以便先回宫参悟轮回了，我们便在此分别吧。”
林动望着绫清竹那转身的优美倩影，心头却是忍不住的有点难受，旋即猛的一咬牙，伸手一把将她那娇嫩如玉般的皓腕抓住。
“你万里迢迢陪我去北玄域，还将太上感应诀教我，若就这样让你走了，岂不是让人说我林动狼心狗肺。”林动皱了皱眉，道：“不管怎么样，你得先去大炎王朝，然后我再亲自将你送回九天太清宫，不然我可不让你走。”
绫清竹被林动抓住，再听得他这话，不由得暗暗羞恼，这人怎么这么霸道。
她本是极为独立有主见的性子，但眼下见到林动那皱起眉头来的脸庞，轻咬了咬银牙，却竟是无法将其挣脱开来。
一旁的青檀见到两人这般举止，小嘴顿时撅了起来。
“我们走吧。”
林动也不待绫清竹出言反对，直接是拉着她对着大炎王朝的方向掠去，后者挣扎了两下，但林动大手却是犹如铁钳般纹丝不动，最终她只能放弃下来，她轻咬着银牙望着身前的那道身影，这人真是没个道理可讲。
大炎王朝都城。
如今的这座都城，比起当年的繁华，显然是而强盛了太多，而大炎王朝也同样不再是那小小的低级王朝，因为着林家的存在，这个曾经的低级王朝，莫说是其他的超级王朝，就算是那些超级宗派，都不敢对其丝毫的人不敬。
而其中缘由，东玄域之上，人人皆知。
在都城偏北区域，有着一片极为辽阔的院落，这是整个大炎王朝最为尊贵之处，即便是那不远处的帝都皇宫，都难以与其争色。
而这片院落，正是林氏宗族所在。
在院落周围的一些高处，隐约可见一些犹如磐石般的身影静静盘坐，这些是虎噬军中的强者，虽说小炎他们回了妖域，但却是留了一千虎噬军在林氏宗族，拥有着这股力量，显然也不敢有人对林氏宗族心生凯觎之心。
现在的林氏宗族在大炎王朝地位崇高，不过随着如今林啸成为新一任的族长，也是下了一些狠手段整肃族风，一些打着林氏宗族的名号为这个宗族抹黑之人，几乎是处于最严厉的惩罚，而面对着林啸的雷厉手段，整个林氏宗族却是没一人敢出声反对，即便是那些资历甚老的族内长老，都是唯唯诺诺，因为他们很清楚，林氏宗族如今的地位，究竟是怎么来的。
而林啸的整肃，也是取到了不小的作用，林氏宗族再不复那种骄纵之气，而且也是鲜有听见林氏宗族子弟仗着这般名声在外胡作非为的事，这无形中又是令得林氏宗族在大炎王朝之中的名声悄然的上升着。
帝都天际之边，突然有着流光暴掠而来，数个闪烁间便是掠进帝都，然后直奔林氏宗族而去，那些守护在林氏宗族中的虎噬军强者也是有所察觉，不过很快他们脸庞上的警惕便是消散而去，取而代之的，是一种狂热的尊崇之意，那身子更是连忙单膝跪下。
林氏宗族内，众多子弟皆是见到他们这般动作，旋即都是一惊，眼中陡然有着狂喜涌出来，他们很明白，能够让得这些实力极端变态的强者这般敬畏相待的人，在这林氏宗族中，仅有着那一位林氏宗族的传奇人物。
“少族长回来了！”狂喜的骚动，顷刻间，便是在林氏宗族中爆发开来。
听得族内的骚动，那正处于议事厅中的林啸也是有所察觉，旋即连忙带着一些林氏宗族的族老出来，而在他们出来时，那议事厅之前的院中，光芒闪烁，数道身影便是闪现了出来。
“动儿。”
那站在林啸身后的柳妍，一眼便是见到现身的林动，当即脸颊上便是有着惊喜之色浮现出来，一旁的林啸也是满脸舍笑。
林动冲着他们笑了笑，侧开身子，然后那躲在后面的女孩便是显露了出来。
“你青檀？”柳妍与林啸皆是一愣，旋即那眼睛顿时睁大了起来。
“爹，娘。”
青檀原本还有些小心翼辈，但当她在见到柳妍那脸颊上滚落下来的泪水时，眼眶也是立即红了起来，一声泣唤 犹如乳燕般对着柳妍扑了过去。
柳妍急急的将她给抱着，她望着眼前女孩那熟悉的脸颊，多年不见，那小脸上再没了当年的青涩稚嫩，只不过这反而看得柳妍心疼不已，这种心疼比起林动在外闯荡尤要浓烈。
“你这小丫头 这么多年都没个音讯，你想气死爹娘啊？！”柳妍心疼的抱住青檀，然后忍不住的有些恼怒的伸手在女孩那翘臀上狠狠的拍了一巴掌。
青檀嘤咛一声，小脸羞红的直往柳妍怀里钻去这模样，哪还有半分黑暗之殿殿主的威严与冷厉，完全是一个离家出走被抓回来的小女孩模样。
“哼，终于知道回来了？”林啸板着脸，道。
“爹，我可想你了。”青檀拉住林啸大手，笑嘻嘻的道。
林啸哼一声想要责备两句，但望着青檀那笑颜如花的小脸，却又是一句话都说不出来，这小丫头，这些年来也不知道在外面吃了多少的苦 这光是想想都让人心疼。
“回来了就好，不过以后再敢不打招呼就离家出走 我让你哥揍你！”林啸哼道，不过旋即他紧绷的脸庞也是缓缓的松开，伸出粗糙的大手磨挲着青檀小脑袋，叹道：“不过回来就好，回来就好啊。”
“爹，娘，对不起嘛，我知道错了。”听得林啸那如释重负般的声音 青檀眼眶一红，这些年她同样经历了太多太多，因此对于林啸柳妍这种纯粹到极点的爱护之情，她也是懂得它的重要与珍贵。
林动望着有些嘘唏的父母，也是微微一笑这下子，他们一家子总归算是团圆了。
柳妍抱着青檀 眼睛突然看见了站在林动身侧的绫清竹，然后就愣了愣，此时的后者，已是将面纱摘下，露出那绝美的容颜，不过虽然如此，柳妍显然还是将她给认了出来，当即那眼中有着一抹欣喜之色浮现出来，急忙拉了拉林啸的衣袖。
被她一番提醒，林啸也是田过神来，笑容满面的望着绫清竹，干咳了一声，道：“动儿，带客人回家，也不介绍一下吗？”
林动无言，你们不都已经见过了的吗？
一旁的绫清竹也是因为眼前的阵仗脸颊略微的泛红，她以往来，只是以林动朋友的名义来拜访过林啸柳妍二人而已，哪曾像现在这般暧昧，而且周围还有那么多眼巴巴将她给盯着林氏宗族的族人。
“晚辈绫清竹，见过两位长辈。”绫清竹微赧，盈盈一礼，轻声道。
她本就是有着倾国倾城般的容颜，气质清冷，眼下这番罕见的赧然模样，却是惊艳连一旁的林动都是微微失神。
在那周围的院墙上，不知何时爬满了林氏宗族的子弟，那外面的一些大树上都是有着人爬上去，而后他们目光皆是惊艳的望着那亭亭玉立的绝色女子，口水流了一大片，紧接着，一片哗然声爆发而开。
“哇，好漂亮的仙子姐姐啊”
“她与林动哥是什么关系啊？”
“你这不是废话吗？林动哥都将她带到我们林氏宗族见爹娘了，你说是什么关系？”
“林动哥真是厉害。”
虽然那些艳羡的哗然声是在外面传开，但绫清竹何等感知，自然是听得清清楚楚，当即那绝美的脸颊上便是有着一抹滚烫浮现出来，旋即她轻咬着银牙，恨恨的剐了一旁冲着她尴尬发笑的林动。
“呵呵，原来是清竹姑娘啊。”
柳妍笑着走上来拉着绫清竹，目光却是不断的在后者身上扫来扫去，那眼中的喜色与满意，却是怎么收都收不住，而在她这番目光下，绫清竹眸子中的涩意也是愈发浓郁，但又只能硬着头皮，迎接着柳妍那种目光。
柳妍显然是对绫清竹极为的喜爱，她知道后者性子清淡，不喜这种被人注视的场合，便是连忙拉着她与青檀进了屋，再然后，那院子周围便是爆发出无数不甘的嚎叫声，这让得林动暗恼，凌厉目光一扫，那院墙上顿时有着无数道道身影狼狈的缩了回去，顿时那院子周围，响起一连片的哀嚎声。


第1247章 安宁
宗族内因为绫清竹的热潮，久久的未曾褪去，庞大的院中，全都是有关于此的嗡嗡讨论之声，不少林氏宗族的子弟满脸的羡慕，虽然他们仅仅只是偷看，但绫清竹那等绝美容颜与清冷气质有时候连林动都会感到惊艳，更何况他们这些涉世未深的子弟家伙。
而对于绫清竹所带来的这般骚动，林动也是倍感无奈，不过此时他倒没机会与其交流，柳妍似乎对绫清竹极为的喜爱，拉着她与青檀二人进了屋，许久都未有动静，显然是聊得极为的开心。
林动见她们这阵仗，也就只好打消了插入的念头，在与林啸说了一会话后，后者突然笑着拍了拍他的肩膀，指着前面。
林动转身目光望去，只见得有着数道身影快步而来，那最前方的一人，是一位俏丽的女子，她年龄看上去比林动略大，那容颜有着一分熟悉的感觉。
在她后面，则是三名男子，虽然多年不见，但林动还是一眼将他们认了出来，当即那平静的心境，都是在此时荡起阵阵涟漪。
这四人，赫然便是林霞，林宏，林山以及林长枪，他们都与林动有着血缘之亲，当年在那青阳镇，更是一同长大，虽说小时有些摩擦，但如今看来，却也是令得人心头阵阵温馨。
“林动！”
那俏丽女子望着林动，她望着这比起当年离开青阳镇时成熟了许多的青年，眼中有着难掩的惊喜，不过在那惊喜中，还有着一点拘束，毕竟他们也是有些年头没见面，而且如今的林动，可不再是当年那个跟随着他们在青阳镇打闹的孱弱少年，现在的他，莫说这林氏宗族，就算是这东玄域，恐怕都寻不出多少人能够让得他重视。
当年青阳镇中的少年，如今已是屹立在这天地之间的巅峰强者。
“林霞姐，好久不见。”
林动望着林霞，脸庞上却是有着柔和的笑容浮现出来，他同样是看见了林霞他们眼中的拘束，旋即笑着伸出手臂，将林霞抱了一下，接着又是将林宏三人重重的抱了一下，他经历了太多的生生死死，因此对于这种拥有着血缘关系的直系兄妹之情，还是相当的看重。
林霞四人见到林动这般举动，脸上都是有着喜色涌出来，那种拘束终是被打破，那林霞更是拉着林动叽叽喳喳的说个不停，倒是再没有当年那副大姐头的凶悍模样。
那林宏，林山两兄弟也是冲着林动嘿嘿直笑，小时候他们没少找林动的麻烦，但后来也是在林动手中吃了不少的亏，如今的他们在林氏宗族内地位极高，有时候只需要说一下当年他们是如何与林动对恃的事，就足以让得林氏宗族那些子弟对他们投来近乎崇拜的目光。
林长枪倒依旧是那副圆滚滚的模样，这家伙从小便是喜欢青檀，经常跟在他身后借机找青檀，那时候与林动的关系颇为的不错。
林动与四人说笑着，望着他们那熟悉的面庞，依稀还能够想起当年发生在那青阳镇中的一幕幕，如今想来，即便是林动这种经历杀伐的心境，都是忍不住的涌上淡淡的温馨之感。
在那房间中，正与柳妍说着什么的绫清竹微微抬头，眸子顺着窗户望出去，正好见到林动那张脸庞，此时，那张脸庞，仿佛是回复了一些当年的稚嫩，那种稚嫩，令得她突然的想起与他第一次相见时…
“清竹姑娘？”柳妍轻叫了两声，绫清竹连忙回过神来，她望着柳妍那泛着笑意的目光，绝美的脸颊顿时掠过一抹绯红，连忙道：“您叫我清竹便好了。”
柳妍笑着点点头，道：“清竹啊，动儿没怎么欺负你吧？以后若是他敢欺负你，你便告诉我，我去收拾他。”
绫清竹听得俏脸生晕，不知如何作答，只能在心中将那林动恨得半死。
接下来林动将焚天老人也是介绍给了林啸，后者听得对方的身份，顿时恭敬有加，这等超级强者就算是在东玄域，那也是足以成为一宗之主，这若是成为了他们林氏宗族的客卿长老，可实在是他们林氏宗族的大幸。
而因为林动的关系，焚天老人对林啸也是格外的客气，两人一番商谈，他便是决定了暂留林氏宗族，反正如今他在这天地间已无认识之人，能够留在这里指导一些小家伙，倒也挺适合他。
林动见到他们聊得开心，也是笑着退开，而后漫步在这庞大的林氏宗族中，沿途间还能够遇见一些林氏宗族的子弟，而他们在见到林动后，皆是眼神狂热，这一点，甚至连林氏宗族的那些长辈，也是无法避免。
林动也是因为他们的目光有些苦笑，然后转了方向，寻了一些幽静小道，漫步而进。
行走在碎石林间，细碎的光斑投射下来，笼罩在他的身体上，那种慵懒的感觉，令得他唇角泛抹淡淡的笑意。
在家的感觉，真好啊。
林动身形掠上一颗茂盛大树，然后便是在那树枝上斜靠着着懒散的坐下来，双目微眯间，竟是逐渐的睡过去。
他这一睡，便是一下午光景，待得夕阳斜落时，他方才逐渐的睁开双目，那黑色双眸中，平静得犹如深邃的夜空，令人沉醉。
那泥丸宫内浩瀚如海般的精神力，仿佛是悄然的凝实了一些。
林动仲着懒腰，身形一动，便是掠下，刚欲回走，那目光却是突然望向了林子深处，在那里，他隐约的感觉到一些精神力波动的传出，而且那种波动，有着一丝久远的熟悉之感。
“真是熟人不少啊。”
察觉到这道精神力波动，林动也是微微一叹，想了想，然后便是迈步走进深处，再之后，他便是在那林中深处的小溪旁，见到了一道静静盘坐的倩影。
那道倩影身着素裙，气质恬静，有侧面看去，那张脸颊也是颇为的俏美，从这脸颊上，林动能够看见一些熟悉的感觉。
林可儿，当年他们在炎城相识，还正是因为她的到来，林动方才会去那古墓，然后与绫清竹发生那等荒唐之事。
此时，那林可儿似是在修炼之中，在她的周身荡漾着雄浑的精神力，那般程度，约莫是天符师顶峰的层次，只要再进一步，便是能够晋入仙符师小乘的境界。
这林可儿当年林动便是发现她修炼了精神力，没想到数年时间不见，也算是小有成就。
林动望着这一幕，淡淡一笑，心神微动，便是有着一股精神力悄然的散发而出，然后将那林可儿的精神力包裹。
那林可儿精神力突然遭到包裹，她本人也是猛的一惊，精神力立即呼啸而起试图挣扎，但紧接着，那股神秘的精神力却是散发出一种温暖的波动。
在那种温暖之下，她的精神力犹如尚在母亲怀中的胎儿一般，逐渐的陷入沉睡，而在那种沉睡间，她的精神力却是悄然的增涨着，然后不知不觉，竟是冲破了那一层阻碍她许久的薄膜，真正的晋入了仙符师的境界。
那种温暖，不知何时的散去，林可儿也是猛的睁开眼，她先是一愣，旋即急急转身，目光望向远处，那里，似是有着一道削瘦的身影，缓缓远去。
“林动…”
她轻咬着嘴唇，望着那道远去的洒脱的身影，神情微微的恍惚，那许多年前的一幕幕记忆逐渐的涌出来，她轻轻一笑，只是那笑容中，有着一些微涩之意。
待得林动回到歇息的院中时，正好见到柳妍从房间中退出来，他见状连忙笑着迎了上去。
“她们都休息了？”
柳妍微笑着点点头，她望着如今比起自己已经高了一个头的林动，脸颊上满是欣慰之色，她犹自还记得，在那很多年前，少年为了能够给林啸赢回来一株灵药奋力苦修的模样，然而如今时间流逝，当年那稚弱的少年，却是成为了整个林氏宗族都为之自豪的存在。
“动儿啊，清竹人挺好的，娘亲也喜欢。”柳妍拉着林动的手臂，轻笑道。
林动闻言不由得尴尬的一笑。
“不过…你这次回来，为什么不把欢欢也带回来？”柳妍看着林动，道：“那个女孩我同样很喜欢，我看得出来，她也很喜欢你的。”
林动苦笑，如今的他也正处于这般头疼之事中，他对绫清竹与应欢欢都有着感情，她们为他所做所付出的，他也是能够感受到，但奈何这两女都是心气高傲之人，一个如今贵为九天太清宫宫主，另外一个更是冰主轮回转世，这般优秀女子，常人得其一已是天大福分，但如今他却是一并沾惹，却真不知道是福是愁。
柳妍是过来人，一见到林动这份神情便是明白过来，掩嘴一笑，道：“敢情动儿还是抢手货呢。”
林动微窘，无奈的道：“娘您就别取笑我了。”
“这事情，还是只能你自己做主，不过，这两丫头我都很喜欢。”柳妍微笑道，那话语中的意思倒是让得林动干笑一声，娘这要求，难度还真是有点不小啊。
柳妍看着林动，突然轻叹了一声，道：“还有青檀…”
“青檀怎么了？”林动微怔。
柳妍嘴巴动了动，最后也是略感苦恼的摇了摇头，伸出手来扯了一把林动耳朵，没好气的道：“没什么，我先回去休息了，你也早点歇息。
说完，她便已是迈步离开，同时还忍不住的摇摇头，这儿子太出色，也真是让人头疼啊，青檀这小丫头…


第1248章 鹰宗
因为如今诸事平静下来，所以林动此次在林氏宗族待的时间不短，他这八年大多都是在外面闯荡，现在难得一家人都是团聚，他自然也是想要多抽出些时间来陪伴着亲人。
他很清楚，虽然现在这天地局势看似平静，但那之下，却是有着无尽暗流涌动，潜伏万千载的异魔逐渐的现身，谁也不敢肯定他们接下来会做什么，可隐隐的，林动却是能够感觉到一些不安，或许，这般暂时的宁静，便是暴风雨来之前的预兆吧。
正是在这种隐隐的不安趋势下，林动对这般与亲人相处的气氛愈发的留恋。
不过让得林动奇怪的是，绫清竹竟然也是并未再提起过要先行回九天太清宫的事，不过林动见得柳妍对她喜欢之极，每日聊天，因此倒也不敢去说这茬，她们喜欢，那就随她们吧。
而这般安静的过了数日时间，悠闲中的林动突然被林啸叫去。
“要我去趟大荒郡？去干嘛？”庭院中，林动听得林啸的要求，倒是微微一怔，有些疑惑的道。
“如今的大荒郡可与以往不一样了。”林啸道。
“不一样？”林动也是愣了愣，大荒郡是他离开青阳镇，炎城之后的第一个历练之地，当年的那里，算是颇为的混乱，各方势力云集，当然，在那里还有着大荒碑，那个隐藏在其中的远古宗派，而林动的吞噬祖符便是自那里所获，此番他前往大荒郡，便是想再进那大荒碑，他的一切际遇，都是从那里开始，说起来算是承了那古老宗派的一个天大恩情。
涅盘丹，但最后还是那位守护者帮他净化了诅咒之力…
“如今的大炎王朝，其实说来有着两大强大势力，以往的那些四大宗族除了我们林氏宗族，其他的基本都是没落了下去。”林啸点点头，道。
“而这两大势力，一个是我们林氏宗族，另外一个，便是大荒郡的鹰宗，这一次大炎王朝被诸多王朝围攻，北面边境，基本都是鹰宗在帮忙抵御。”
“鹰宗？”林动有些讶异，这是什么时候冒出来的势力？
“这鹰宗的前身，仅仅只是大荒郡的一个武馆，不过在五年之前，这鹰之武馆突然鹊起，短短一年时间便是一统了大荒郡，而且据我们所知，他们的势力，还辐射到了周围的不少王朝，实力极为惊人。”林啸道。
“鹰之武馆？”
熟悉的名字令得林动眼睛眨了眨，而后眼中有着浓浓的惊愕浮现出来，这人生倒真是奇特，没想到当年那个还需要他出面帮忙庇护的小小武馆，在这数年后，也是拥有这般惊人的成就。
“他们怎么会突然这么强？”林动好奇的问道，他可是记得清楚，当年鹰之武馆在大荒郡可是相当的弱小，怎么可能拥有着一统大荒郡的实力。
“据说是他们获得了大荒碑内的宗派传承，如今鹰宗总部便是设置在大荒碑周围，将其团团护住。”林啸笑了笑，道：“如今鹰宗的宗主，应该是那个叫做姜雪的女子吧。”
“姜雪…”
林动微微恍惚，脑海深处，有着遥远的记忆涌出来，在那灯火昏暗的房间中，漂亮的女子轻褪长裙，将那雪白的胴体暴露在了他的面前，那个坚强的女孩，为了能够让得武馆获得庇护，甘愿流着泪委身相求于他…
没想到，当年那个女孩，如今却是成为了那一统大荒郡的鹰宗宗主，这般变幻，真是让人感到奇妙。
显然，在他离开大荒郡之后，那里，又是发生了一些事情。
“让我过去，是与鹰宗有关？。”林动问道。
“嗯。”
林啸点点头，皱眉道：“那座大荒碑的事，你应该是知晓一些，据说最近出了不小的问题，鹰宗没办法解决，向我们林氏宗族求援，你看你如果有时间的话，就去一趟吧。”
“大荒碑出问题了？”
林动眉头也是一皱，旋即毫不犹豫的点点头，大荒碑中的那远古宗派说起来与他有着恩情，他的吞噬祖符便是从那里所获，当年在那里获得大荒囚天指，也不知道帮他渡过了多少生死难关，如今他们有难，他自然是必须全力相助。
“快去快回吧。”
林啸点点头，如今的林动再不是当年那个需要他管教着的少年，以后者的实力，莫说这大炎王朝，恐怕放眼这浩瀚天地间，能够阻拦他的人都不多。
林动笑笑，也没拖沓，不过此番他倒只是单独前往，这大炎王朝虽说辽阔，但以林动的速度而言，却仅仅只是半个时辰的光景，他已是进入了那大荒郡的地域范围之中。
林动身处天空，低头望着下方的地域，眼中满是缅怀回忆之色，当年他从炎城走出，穿越诸郡，最后方才抵达这大荒郡，在这里，也算是他磨练初始的地方。
他辩明了一下方向，便是径直对着大荒郡中央地带而去，而随着接近那片中央地带，他也是能够有些惊讶的见到，不少城市都是有着一面巨大的旗帜迎风飘扬，而那旗面纸上，有着一只翱翔的苍鹰，那鹰目中凌厉的神光，令人望而生畏。
显然，这应该便是“鹰宗”的旗帜，只是没想到这鹰宗对大荒郡的掌控如此之强，恐怕真要说起来，鹰宗才是这大荒郡的控制者，皇室的力量，都是在这里失去了效力。
林动心念转动间，身形已是掠过数千里，而后心神微动的抬头，只见得在那前方，竟是有着一座巨大的城市矗立在古老的平原之上，在那城市上空，能够见到无数身影掠过，那等繁华之景，丝毫不弱于帝都。
林动目光远眺只见得在那城市最中心的位置，一座巨大无比的石碑静静的矗立着，望着这道熟悉的石碑，他眼中也是掠过一抹复杂之意。
“大荒囚天指…”
林动轻轻一笑，当年这般武学，可是他最为强大的杀手锏，不知道帮他渡过了多少的生死劫难，而这武学，正是从大荒碑中所获。
林动身形化为一道流光掠过天际数个呼吸后，已是出现在了那巨大的石碑之前，他负手而立的望着石碑，眼中的缅怀愈发的浓郁。
不过这座石碑如今似乎是成为了鹰宗的圣物，寻常人根本不准靠近，在这周围也是有着极端森严的防卫，这些防卫最初始并未发现林动的靠近但后来随着他负手而立的怔然发呆，这才猛的回过神来，当即那尖锐的警报之声便是响彻而起，无数道身影从四面八方的涌来，然后将林动包围得水泄不通。
在林动出现在大荒碑之前时，这座城市的一处高楼上一名身着鹅黄长裙的女子高挑而立，她拥有着相当俏美的容颜，只不过那柳眉间，有着一些因为身居高位而带来的威严，此时的她，手持香茗，只是眼神静静的望着远处似乎是有些出神。
“咳。”
一道轻咳声突然从后方响起，那女子一怔，偏过头来，只见得一名头发花白的中年男子站于后方她微微一笑，道：“爹。”
中年男子走上前来，他望着眼前这座规模宏伟的城市，长叹了一口气道：“雪儿倒是好能耐，短短数年时间却是让我们那小小的一个武馆，成为了这大荒郡的主宰。”
“只是承前人之福罢了。”女子微微摇头，道。
中年男子有些心疼的看着她，苦笑道：“不过你这丫头也不小了，什么时候操心下自己的事，这大炎王朝无数年轻俊杰对你倾心爱慕，莫非你就没一个能看上眼的？”
这位俏美女子，正是当年那鹰之武馆的姜雪，而这中年人，则是其父亲，姜雷。
姜雪轻轻的抿了一口香茗，淡淡一笑，道：“这又不着急的。”
“不着急…我看，你怕是早就心有所属吧？”姜雷苦笑一声，道。
姜雪那端着香茗的玉手微微一颤，抬头微笑道：“爹你说什么呢？”
“你还真以为爹不知道当年那一夜里你去林动房里是做什么的吗？”姜雷叹道。
姜雪那如玉般的脸颊上一抹绯红瞬间涌了上来，她红着脸看着姜雷，一时间竟是失了以往的伶俐：“爹，你…你怎么知道的？”
姜雷望着她这罕见的娇羞姿态，苦笑更甚：“前些时候的事，你应该也听说了吧？林动出现在了帝都，现在的他，可不再是当年那个需要我们带他出迷雾森林的少年了…”
姜雪轻咬着嘴唇，旋即轻笑一声，道：“他能有这成就，我又一点都不意外，他本就该是翱翔九天的鹰隼…恐怕现在的他早便是忘记了我，但这又如何呢？我记着便好了。”
“你这傻丫头啊。”姜雷被她这番言语弄得大感无语，只能无奈摇头。
“呜！”
而就在两人谈话间，突然城市上空有着急促的警笛之声响彻而起，这令得两人面色顿时一变，而后有着破风声快速的传来：“宗主，有人潜入禁地，如今已在大荒碑之前！”
那姜雪闻言，一张俏美的脸颊顿时冰寒下来，身形一动，便是闪电般的掠向大荒碑所在的方向，同时那冷喝之声，响彻全城：“鹰宗弟子听令，封锁城市，启动阵法！”
在城市中央位置，天空以及地面都是被密密麻麻的人海所弥漫，那一道道泛着凌厉的目光，皆是锁定着大荒碑之前的那道削瘦身影，大有命令一到，便是同时出手将其轰成肉泥的迹象。
然而，面对着他们这种包围，石碑前的人影却是纹丝不动，他双手负于身后，静静的望着古老的石碑。
咻。
“宗主！”
远处，一道破风声迅速传来，一道倩影便是出现在了那无数人目光之中，当即有着惊天般的恭敬喝声响起。
姜雪俏脸冰寒，玉手轻挥，而后那对俏目便是看向了石碑之前的那道削瘦身影，柳眉微皱，道：“不知阁下是何人？为何闯我鹰宗禁地？！”
听得她的声音，那道犹如出身般的身影终是微微一颤，然后他缓缓的转过身来，目光看向姜雪，脸庞上有着一抹柔和的笑容浮现出来。
“姜雪姑娘，好久不见，别来无恙啊。”
姜雪娇躯在此时逐渐的僵硬，她望着那张依稀有着熟悉轮廓的年轻脸庞，然后周围那些鹰宗的弟子便是惊愕的见到，这在宗内素来严厉冷肃的宗主，竟然是在此时呆滞了下来。


第1249章 故人
“你…你是林动？”姜雪娇躯僵硬，怔怔的望着那道带着笑容的年轻脸庞，那袖中玉手也是缓缓的紧握起来，那略微有些艰难的声音，从她那红唇中带着点点颤抖的吐出。
“林动？”
她的声音不大，但依旧被附近的一些鹰宗弟子听见，旋即身体皆是猛的一震，眼露惊骇的盯着眼前的青年，这个名字，如今在大炎王朝可是人人知晓。
眼前之人，便是那传说中灭掉了东玄域上最强大的超级宗派的林动吗？
林动望着姜雪的神情，轻笑着点点头，他看着她的目光也是有些奇特，在那很多年时，姜雪还只是一个一心想要护着鹰之武馆的坚强女子，然而数年不见，没想到她却是成为了这鹰宗的宗主，看来际遇这东西，并不只是他一人能够拥有。
姜雪终是逐渐的回过神来，她深深的看了一眼林动，而后玉手轻挥，那天空上鹰宗大批的人马便是退散而开，她莲步轻移，走至林动身旁，道：“没想到还能够再看见你。”
林动看着她，后者眼中的复杂情绪令得他有些莫名，但还是笑着道：“我也没想到数年时间不见，你们鹰之武馆却是变成显赫大炎王朝的鹰宗了，真是厉害。”
“这点成就，恐怕都不入你的眼吧？”姜雪幽幽的道，鹰宗虽说发展不弱，但要与那些超级宗派比起来依旧差了不少，而如今东玄域上最强的超级宗派都是被林动所灭，这等实力，一个小小的鹰宗，又怎能与其相比。
林动被姜雪这有些幽怨的语气弄得干笑两声，却不好如何接话。
“哈哈，林动，真的是你啊－？”后方，突然有着大笑声传来，一名头发花白的中年男子掠来，他望着林动，脸庞上满是大喜之色。
“呵呵，姜雷大叔。”林动望着中年男子那依稀有些熟悉的脸庞，脸庞上也是有着笑容浮现出来，拱手笑道。
姜雷满脸喜色的盯着林动，那眼中甚至是有点激动之色，后者如今在这东玄域可是传奇般的人物，想要与他攀上一些交情的人如同过江之鲫，然而谁又能想到，这般传奇人物，在那多年之前，却还代表着他们鹰之武馆出战…
“以你如今的身份，恐怕早便是忘了我们这些人吧？”姜雪见到林动与姜雷谈得热络，甚至都没多看自己几眼，这令得她轻咬了咬红唇，有些怨念的道。
一旁的姜雷倒是无奈的看了姜雪一眼，这丫头这些年主持鹰宗大事，向来都是滴水不漏，怎么眼下这番，却是与多年之前一般无二了啊？
“姜雪姑娘莫要取笑我了，当年若不是你们将我带出迷雾森林，说不定我早在其中喂了狼。”林动苦笑道，他也不知道姜雪的怨气从何而来，莫非是因为当年那一夜？
想到此处，他目光突然扫了扫姜雪那玲珑有致的娇躯，眼中不由得浮现一抹尴尬之色，想来任何一位女子来主动献身却被拒绝，心中都会有些怨愤吧…
他的目光虽然只是一扫，但如今的姜雪毕竟不再是当年那个需要求着他的女孩，那锐利目光依旧有所察觉，当即俏美脸颊上飞上一抹红霞，有些羞恼的盯了林动一眼。
“咳…此番回来，是因为我爹收到了你们的求援消息，所以过来看看这大荒碑究竟出什么问题了。”
林动收回目光，旋即有些疑惑的道：“另外，你们与这大荒碑，也有什么关系吗？”
姜雷笑着点点头，道：“在你后来，雪儿也进了这大荒碑，并且获得了这大荒碑碑灵的认可，也获得了那远古宗派的传承，因此我们方才创建了鹰宗，守护着大荒碑。”
林动不由诧异的看了姜雪一眼，那大荒碑的碑灵他也是接触过，但他仅仅只是获得了大荒囚天指，至于宗派传承，更是无从说起，没想到，姜雪竟然也能这么厉害…
“与我说说大荒碑的问题吧。”林动指了指大荒碑，道。
说起正事，姜雪黛眉也是微微皱起，道：“如今的大荒碑内与以往有些不同…你可还知道，那碑中的守护者？”
林动点点头，当年他便是被那守护者追杀得上跳下窜，不过最后他还因为涅盘丹的事，承了人家一个情。
“你应该也知道，大荒宗在那远古时期，曾遭遇异魔攻占，这里发生了惊天大战，最后宗派被毁，异魔也是在这里损失不小，同样也是导致地底深处有着魔气存在，而在这些年中，那些魔气逐渐的渗透出来。”
“果然是因为魔气啊。”
林动轻叹了一声，在来时他便是有所察觉，但没想到还是这些老对手，如今他也是能够想到当年这大荒碑所发生的事，恐怕还是因为吞噬祖符引起，就如同那被异魔攻占的雷府一般。
“大荒碑中，残存着许多大荒宗的先辈意志，这些年来，那守护者逐渐的吸收了那些意志，也起了一些变化，这种变化初始是好的，但后来随着魔气的涌出，守护者却是逐渐的被侵蚀，不过他也是意志无比坚韧之人，即便是被魔气侵蚀，但却始终不肯伤及大荒宗的任何一物，只是每日凄厉嚎叫…”说到此处，姜雪也是一声轻叹。
林动面色略有些阴沉，对于那位大荒宗的守护者，他也是相当的敬佩，后者身死道消，但却依旧是凭着那股对宗派的守护之情，变成这番模样，要永恒的守护着宗派，这份执着，连他都是心生敬意，没想到如今却是被那些魔气害成这样。
“大荒碑中的魔气，在这些年中，不知道为何越来越浓郁，前些年的时候，竟是要冲出大荒碑。”姜雪道。
林动眼神微凝，若是让得那些魔气冲出大荒碑，恐怕这大荒郡将会生灵尽灭了。
“我们施展了很多手段，都是无法封锁魔气，后来，就在魔气要冲出来时，却是出了一事。”说到此处，姜雪与姜雷的面色都是略感复杂起来。
“什么事？”林动有些疑惑的道。
“那些冲出来的魔气，被茵茵化解了…”姜雪道。
“茵茵？”林动一愣，脑海之中顿时浮现出当年那个眨巴着大眼睛，可爱至极的小女孩，她竟然能够化解魔气？
“嗯，具体情况我们也不知很清楚，不过能够解除这番麻烦自然是好的，后来我们便将茵茵放在大荒碑中，说来也是奇怪，有了她坐镇大荒碑内，那些魔气竟是随之消除，只不过很快的又是涌了出来，这样一来，他们便是形成了对恃，直到现在…”
林动眼中满是惊异之色，魔气的难缠程度，他相当明白，茵茵只是一个连修炼都未怎么修炼的小女孩，怎么会对那些魔气拥有着克制性？
“看来这大荒碑中，应该是有什么东西在作祟。”林动淡淡一笑，他与异魔打的交道实在是太多，眼下这情况一眼便是能够看出来，恐怕这大荒碑内，潜藏着什么东西。
姜雪点点头，她也是隐隐有所感觉，但却无法确定。
“开启大荒碑吧，进去看看便知。”
林动一挥手，道。
有了他在这里，姜雪显然也是放心了一些，当即也不犹豫，玉手轻扬，只见得那大荒碑上便是有着光彩爆发出来，而后将他们两人包裹而进，待得光芒散去时，两人也是消失了踪影。
林动微睁开双眼，眼前的景象已是出现了变幻，依旧是那片荒凉的空间，无数废墟存在着，显示着这里曾经的繁华。
林动目光一扫，然后便是见到那些不断从大地下渗透出来的丝丝黑气，当即那双瞳便是微微一眯。
吼！
而在林动打量着这片天地时，那遥远处，突然有着凄厉的嚎叫声传来，那嚎叫声中，充斥着痛苦与愤怒。
是那守护者。
林动眉头微皱，袖袍一挥，他与姜雪二人便是对着那个方向暴掠而去，短短数分钟后，只见得一座孤峰凸起，此时，在那山峰上，一道黑影单膝跪地，不断的仰天咆哮，一股浓浓的魔气不断的从他体内散发出来。
而在那道黑影后方的一块巨石上，一名黄裙小女孩正有些无奈的坐着，她摇晃着雪白的小腿，一道光芒从她体内散发出来，将那道黑影笼罩着，同时也是不断的在消除着他身体上的魔气。
“茵茵。”姜雪望着那小女孩，顿时挥了挥手。
“姐姐！”
小女孩惊喜的抬头，那大眼睛便是见到掠来的姜雪，当即小小的身躯跃起，径直的对着姜雪扑了过来，后者连忙将她给接着，微微一笑，纤细玉指指着后方的那道身影：“看看这是谁？”
小女孩顺着看去，然后便是见到一脸微笑的林动，那大眼睛中先是闪过一抹思索，接着小脸上顿时有着欣喜涌出来。
“大哥哥？”
林动听得这一如当年的稚嫩叫声，再看看这么多年竟然依旧保持着当初形态的茵茵，心中虽然疑惑，但还是笑吟吟的点点头。
在他点头时，那小女孩已是欣喜的跑了过来，那乌黑的大眼睛中满是喜色，此时，一缕轻风吹来，吹起她额间的发丝，然后，林动便是见到了一道淡不可闻的奇特符印，当即身体便是猛的一震。
那是轻回印？
这茵茵，居然也是轮回者？！


第1250章 碑中之魔
林动颇有些震动的望着姜茵茵光洁额间的那轮回印，眼中满是惊愕之色，他怎么都是无法想到，这个早在许多年前便是相识的小女孩，竟然…也会是轮回者？！
“怎么了？”姜雪也是注意到林动的眼神，当即问道。
林动伸出手掌轻轻摸了摸姜茵茵小脑袋，后者那小脸上也是有着纯真的笑容浮现出来，她显然对林动有着相当深刻的印象，即便是多年未见，依旧是将其认了出来。
“她这么多年，一直都是这样的？”
林动问道，与茵茵相识那是在七八年前了，然而那时候的她便是十二三岁的小女孩模样，可如今这么多年过去，这番体形，竟然是没有丝毫的变化，甚至连那眼中的纯真，都是与当年一般无二。
姜雪苦笑着点点头，她望着姜茵茵，叹道：“我不太清楚这究竟是怎么回事，但她这些年来的确一直未曾变化过，而且我也是能够隐隐的察觉到她的一些不一般…”
林动与茵茵笑了笑，然后让她先去那山峰上等着，这才对着姜雪道：“她是轮回者。”
“轮回者？那岂不是说…茵茵以后会被另外的记忆所侵占？”姜雪闻言，俏脸顿时微微一变，显然是听说过轮回者的传言。
林动摇摇头，道：“这就得看她轮回之前究竟什么来头了…”
按照常理而言，若只是那种渡过一两次轮回劫失败，而后轮回转世的轮回强者，那或许还会是现在的记忆占主导，但若是类似冰主那等可怕的存在，那究竟日后身体主导权属于谁，还真是有点不太好说。
“那你知道她轮回之前是什么人物么？”姜雪问道。
林动再度摇头，一般说来，只要渡过一次轮回劫，并且不是连元神都是被彻底的抹除，那么便是有运转轮回之意进入轮回的能力，当然，即便是进入轮回，依旧还是有着被泯灭灵性的风险，所以若不是万不得已，大多巅峰强者，都不会选择这一条路子。
远古时期，晋入这种层次的巅峰强者也不算少，光凭借着一道轮回印，即便是他也是看不出什么来，不过若只是那种渡过一两次的轮回强者，那想来对于茵茵而言，反而会是一件好事。
林动此时也是落到了那山峰之上，而后他的目光便是望向那道被魔气包裹的黑影，如今与当年显然是大变了模样，当年他浑身仅有骸骨，可现在却是有着血肉生长出来，虽然依旧干瘦，但却具备了人形，而且最令得林动讶异的是，他那眼瞳之中，仿佛有着无数残缺的意念在凝聚，不过不论这些意念如何杂多，却是有着相同的一点，那是…守护。
“这些年来，飘荡在这片空间的那些残缺意志，都是融入了他的体内，现在的他，倒是有些奇特，也说不清楚他究竟算是什么…”姜雪轻声道。
林动微微点头，看来这大荒宗的凝聚极端的强大，这些弟子，即便是死去上千载，但那股执念，依旧是要守护着他们的宗派。
而眼前的守护者，似乎便是他们意志的凝聚吧。
林动心中一声轻叹，这般感觉，就犹如在那道宗之时一般，想来若是道宗被毁，那片地域，也会有着无数道宗弟子守护宗派的意志残存。
“吼！”
此时那黑影似乎是极为的痛苦，他双拳重重的砸在自己胸膛，双膝跪地，那望着这片空间的茫然眼中，却是有着极端之深的眷念。
“喂，大家伙，你怎么样了？”此时那茵茵也是见到黑影极为的痛苦，急急的道。
她的体内不断的有着光芒飘荡出来，然后笼罩向那道黑影，不断的净化着他体内的那种魔气。
林动望着这守护者这般痛苦，眉头也是大皱，旋即其手掌一握，突然有着无尽雷霆自其手中弥漫而开，那雷霆之中，充斥着极端狂暴而纯正的力量。
雷光凝聚成一颗雷电光球，直接是冲进那守护者的身体之中，紧接着，噼里啪啦的雷弧便是从他体内弥漫出来，而他身体之上弥漫的魔气则是在此时被尽数的消除而去。
随着魔气的消退，那守护者眼中的痛苦与茫然也是逐渐的散去，那双目中，再度有了一些杂乱的灵智在凝聚，最后，他对着林动深深的弯腰一礼。
林动袖袍一挥，将其阻拦了下来，道：“当年我承你抹除涅盘丹诅咒之力的情份，今日以此相报。”
守护者虽然有了灵智，但显然并不太深，而且恐怕他也记不得当年所做的那些事，毕竟现在的他，只不过凝聚了众多意志而成的存在，既不像人，又不像尸，也与焚天老人之前那种活死人的状态完全不同。
林动也明白他的状态，所以也并未多说，只是那略显森寒的目光转向下方渗透着一丝丝魔气的大地，冷笑道：“他体内魔气强盛，而且并非是无根之水，光凭借茵茵的压制也不是长久之计。”
“斩草要除根，想要解决这问题，还得将隐藏在这下面的东西解决掉。”
他在这里获得吞噬祖符，也因为如此，他方才有着如今的成就，所以说起来，这大荒宗对他也算是有着大恩，而如今见到他们既然连死去的意志都是不得安宁，心中也是多了一丝冷厉与恼怒。
“你护着茵茵。”
林动对着姜雪说了一声，后者也是点点头，掠至茵茵身旁，将其拉着。
“我倒是要来看看，究竟是什么东西在此处作祟。”
林动一声冷哼，只见得天空之上雷云猛的汇聚而来，无数雷霆闪烁，竟是将这片空间都是囊括在了其中。
轰轰！
无数道雷霆疯狂的倾泻下来，最后尽数的落在下方的大地上，那种狂暴而纯正的力量蔓延开来，那些原本弥漫在天地间的魔气，也是在雷霆的清扫下，以一种惊人的速度被荡除而去。
然而，即便是面对着这般清扫，那下方大地，依旧是没有丝毫的动静。
“倒真是忍得住！”
林动冷笑，屈指一点，只见得一道万丈庞大的雷龙张牙舞爪的呼啸而下，最后狠狠的轰在下方的大地上。
嘭！
整片空间仿佛都是在此时颤抖起来，那片大地，直接是被撕裂开一道巨大无比的深深沟壑，那沟壑深处，黑暗无比，仿佛有着无比邪恶的气息在涌动着。
撕开大地，林动面色冰寒，再度一指点出，只见得无数道雷霆转向，铺天盖地的对着那大地深处轰击而去。
咻！
不过，这一次，那大地深处终于是有了动静，然后林动便是见到滔天般的邪恶魔气犹如魔龙般自那地底深处涌出来，魔气弥漫间，直接是将那些狂暴无比的雷霆尽数的抵御下来。
“还是耐不住了吗？那就给我滚出来！”
林动见到这隐藏在暗中的异魔终于是有了动静，眼瞳之中，雷光浮现，手掌上抬，只见得天际之上雷云疯狂的涌动着，无数的雷霆汇聚而去。
雷云最终缓缓的散去，但在雷云散去时，一颗千丈庞大的雷日，却是显露了出来，可怕的雷霆波动带着嗡鸣之声，远远的传开，令得这片空间，都是在微微的颤抖着。
“去。”
林动眼神冷漠，屈指移下，只见得那轮雷日顿时坠落而下，直奔那深渊深处而去，沿途过处，雷芒闪烁，仿佛连空间都是灼热了起来。
“哼！”
雷日疯狂的坠落而下，那深渊之底，也是有着一道阴冷之极的声音传出，旋即无数魔气涌出，竟是化为一道万丈庞大的魔气光盘。
咚！
雷日径直的落在那魔气光盘之上，两股同样可怕之极的力量在此时爆发而开，直接是将那本就巨大的深渊，再度蛮横的撕裂开来，一道道巨大的沟壑，犹如蜘蛛网一般的遍布着下方的大地。
璀璨的雷光，疯狂的闪烁着。
咔嚓。
那魔气光盘上，终是有着裂缝浮现出来，最后砰的一声，彻彻底底的爆炸开来，不过就在魔气光盘炸裂时，又是有着一只巨大的魔手自深渊中探出，一拳便是将那雷日震散而去。
砰砰！
不过虽然雷日散去，那种狂暴的雷霆之力，依旧是将那只魔手反震得遍体鳞伤，雷霆祖符的力量本就狂暴，攻击性强横无匹，这家伙托大的想要硬拼，自然是要吃亏。
“你竟然获得了雷霆祖符？！”
这般硬碰，也是令得那深渊之底传出一道阴冷之声，旋即滔滔魔气涌出，最后掠出深渊，在那魔气中，仿佛是一道身影，缓缓的走出。
林动目光望着那魔气萦绕处，那道身影越来越清晰，最后终是走出黑雾，而在其走出黑雾时，那张苍白而俊逸的脸庞上，却是有着一抹诡异之色浮现出来，同时那阴冷笑声，传荡开来。
“上一任吞噬祖符掌控者死在本殿手中，没想到，今日这一任吞噬祖符以及雷霆祖符的掌控者，也得死于本殿之手，看来本殿还真是你们这一脉的克星呢。”


第1251章 九王殿
林动面色平静的望着那自魔气之中缓步走出来的魔影，感受着后者身体之上涌动的惊天魔气，他双目微微一凝。
“不知道你又是魔狱之中的哪一尊王殿？”林动淡淡的道，在那魔狱之中，有资格称为殿的，恐怕也就那十尊王殿了，只不过就是不知道眼前这家伙，又是第几殿？
“没想到你对我魔狱倒是有些了解，看来这些年来，外面也是有了一些动静，呵呵，算算时间，其实倒也差不多了。”听得林动所言，那魔影也是微感讶异，旋即道：“本殿在魔狱之中，排行第九。”
“九王殿么。”
林动眼中倒是没多少意外之色，此人虽然魔气惊人，但显然顶峰程度也该与那七王殿相仿，而且眼下他这般模样，显然是曾经遭受极端严重的重创，现在都未能彻底的恢复实力，按照林动的推测，现在这九王殿的实力，恐怕也就堪堪能够媲美渡过两次轮回劫的强者，说起来，还不如之前曾经大战过的那位七王殿。
“看来当年率魔进攻大荒宗的，便是你了吧？”
那九王殿闻言，却是阴冷一笑，道：“这大荒宗的确是被本殿所灭，而且那上一任的吞噬祖符掌控者，也是为本殿所杀。”
“吼！”
他这话音一落，只见得山峰上那守护者猛的发出凄厉的咆哮声，狂暴能量席卷而开竟是要对着那九王殿冲去，那咆哮声中，蕴含着飘荡了千载岁月的仇恨。
姜雪见状，连忙运转元力将其压制下来，以他现在的实力怎么可能会是那九王殿的对手。
“没想到还有点残漏的…不过这样连死了都不如。”九王殿漠然的看了那守护者一眼，旋即眼中也是有着阴毒之色涌出来：“不过你们这大荒宗倒的确是有点能耐，虽然最终被本殿所灭，但却将本殿手下尽数斩杀殆尽，那老鬼更是拼得一死，将本殿封印在这大地之底。”
林动冷笑道：“看来现在你逐渐的挣脱了封印，你想要将魔气渗透出去是想通知魔狱的人你所在的位置吧？”
九王殿瞳孔微缩，不置可否的冷笑一声。
“不过可惜，你这家伙着实是有些倒霉。”林动眼中掠过一抹讥讽恐怕这九王殿也是完全没想到，魔气尚还未泄露出去，便是被茵茵所净化，如今她更是进入这片空间，将魔气镇压，令得他始终无法彻底的恢复。
“这个该死的丫头！”
那九王殿眼瞳陡然间涌上暴戾之色他凶狠无比的望着山峰上的小女孩，那副狠毒模样将后者也是吓了一跳，连忙躲在姜雪身后，只露出一只乌黑的大眼睛将他给偷偷看着。
“看你如今模样，显然重创未愈，看来这倒是一个挺不错的机会。”
林动笑望着九王殿那漆黑眸子中，却是有着浓浓杀意在涌动，他很清楚魔狱的强大，那些王殿，皆是拥有着可怕的实力，以他如今的实力，遇见这些王殿也是极其的棘手，但眼下这九王殿却是不同，他被上一任吞噬祖符掌控者封印上千载实力至今未能恢复，若是能够在这里将其解决掉的话，那对于魔狱，也必然会是一个不小的损失。
“想要杀本殿？就怕你这小子没这资格！”
九王殿也是厉声大笑，虽说他在体内林动察觉到了两大祖符的波动，但后者毕竟尚未踏入轮回境，而他眼下虽然也未能完全恢复，可也绝对不是寻常渡过一次轮回劫的强者可比，更何况林动？
林动面色漠然，滔天般的元力已是携带着杀意席卷而出，他很清楚一位王殿在魔狱之中的地位，若是让这九王殿逃脱出去，无疑将会是大患，所以，今日无论如何，此獠必杀！
“轰！”
然而在林动眼中杀意涌动时，那九王殿眼中同样是杀意爆闪，他脚掌猛然一跺，只见得那大地之下，突然有着魔气冲出，只见得无数道魔尸自地底冲出，狰狞无比的对着林动冲杀而去。
“堂堂魔狱九王殿，何必还用这些手下来当炮灰？”
林动见状，却是一声冷笑，手掌一握，一圈千丈庞大的黑洞漩涡浮现而出，吞噬之力暴涌间，直接是尽数将俺写魔尸吞噬而进。
砰！
九王殿阴冷一笑，手掌一抓，只见得那无数道魔尸竟是堆积在一起，最后血肉消融而去，化为一柄巨大的黑色骨剑，邪恶的魔气，不断的涌出。
九王殿手握黑色骨剑，身形一动，已是犹如鬼魅般出现林动上空，而后那骨剑便是携带着邪恶魔气，洞穿空间，狠毒无比的对着林动天灵盖暴刺而去。
“唰！”
不过就在骨剑此处时，一道雷光也是猛的奔涌而来，最后重重的与那骨剑硬憾在一起，火花溅射间，空间都是被震得扭曲下来。
“魔尸雷！”
攻势被阻，那九王殿也丝毫不急，反而一声冷笑，只见得他那手中骨剑爆裂而来，竟是化为数十颗黑色的雷光，疯狂的冲下林动。
那雷光之中，弥漫着无尽的邪恶，甚至还有着腥臭之气弥出来，这般魔气，即便是轮回境的强者被沾染上了，也必然会受到污染与重创。
“滚！”
然而，面对着这般邪恶魔雷，林动却是怡然不惧，手掌一握，只见得一道仿佛雷霆形成的古老符文在其掌心成形，而后一拳轰出，直接是生生的轰在那些魔雷之上。
砰砰砰！
纯正而刚烈无比的雷霆之力肆虐而开，竟是丝毫不惧那邪恶魔雷，雷光所过处，那魔雷则是犹如遇见烈日的残雪，尽数的被消融而去。
而那九王殿见到魔雷无效，眼神也是微微一沉，林动身怀两大祖符，那种力量对于魔气而言，克制性着实是不小。
林动却是丝毫不理会于他，他眼神一凝，只见得整条手臂再度化为液体之形，两大祖符的力量在迅速的凝聚在一起。
嘭！
那仿佛有着无数古老符文沉浮的手掌直接是呼啸而出，最后狠狠的对着那九王殿怒拍而去。
九王殿见到林动那液体般的手臂，眼神也是微变，旋即其印法猛的一变，只见得魔光在其身体之上凝聚，而后一套狰狞的魔甲便是显露而出。
咚！
林动那蕴含着两大祖符力量的掌风，重重的落到那九王殿魔甲之上，但却是有着低沉之声传出，那足以将一尊异魔王瞬间抹杀的攻势，竟然仅仅只是在那魔甲之上留下一道浅浅的印痕。
“魔皇甲？”林动感受着手臂章传来的反震之力，再看看那九王殿身躯之上熟悉的狰狞铠甲，冷声道。
“没得想到你这小子倒还有点见识。”九王殿冷笑一声。
林动眉头微皱，在与七王殿交手时，他已是知道这魔皇甲的厉害，这东西乃是那异魔皇所赐，正是因为有着这东西的相护，这些家伙，方才能够屡次不灭。
九王殿仗着有魔皇甲护身，眼神再度凶狠下来，手握滔天魔气，凶狠无匹的的对着林动席卷而来，而后者也是将元力与精神力尽数的施展开来，再加上两大祖符的力量，竟也是未能让得这九王殿取得多少的上风。
虽然按照正常情况，林动想要抗衡渡过一次轮回劫的强者便已是极限，若是遇见类似七王殿那种渡过两次轮回劫又有着魔皇甲的对手，则是要尽数落入下风，但眼下这九王殿真要说来，因为重创未复，本身实力，也完全比不得七王殿，所以林动方才能够与其拼得不分上下。
天空上，两道光影呼啸，每一次的对碰，都将会将那空间震得扭曲不堪，滔天魔气与元力对碰，那等声势，骇人之极。
姜雪也是俏脸凝重的望着天空上的战局，有种有些庆幸，还好林动及时的赶了过来，不然的话，这里谁能够阻拦得了这九王殿？
“哈哈，小子，你能耐的确不弱，不过终归伤不了我，这样拖下去，你又能奈我何？依本殿来看，你还不如让本殿离去，也好护得这里安宁，至于我们的恩怨，来日再来解决，如何？！”九王殿再度与林动力拼一记，依旧未能取得上风，目光一转，大笑道。
“放虎归山的蠢事，你以为我会来做？”林动冷笑道，若是让这九王殿逃了出去，以后必成大患。
“好，那我们就来看看谁能坚持得更久！”九王殿眼中狰狞一闪而过，厉声笑道。
林动身形悬浮自天空，目光冰寒的盯着那九王殿，眼芒同样是疯狂的闪烁着，以他如今的实力，想要对这九王殿造成重创，恐怕也就唯有施展雷弓黑箭了，不过那般攻势对他消耗太大，若是这样都抹杀不了九王殿的话，那则是真有些头疼了。
“顾不得这些了。”
林动也是果断之辈，他有着自信，施展这般攻势，就算杀不了这九王殿，也能将其重创，到时候再施展手段将其镇压便是。
心头念头一动，林动也就不再犹豫，手掌一握，璀璨雷芒凝聚，化为一道雷弓，而后手指拉动弓弦，一只黑色长箭，悄然的凝聚。
他的眼中，只有着那九王殿的存在，眼瞳深处，杀意暴涌，今日，必杀此獠！
滔天般的杀意，从其体内弥漫而出，那种坚定意志，无可更改。
而就在他对九王殿的杀意暴涨到极限时，那山峰之上的守护者仿佛也是有所察觉，当即嘶吼出声，那吼声中，仿佛包含着无尽的感激。
嗡嗡。
在那守护者咆哮时，突然有着无数道光华从其体内暴射而出，然后那些光华悬浮在天际，林动目光一扫，旋即瞳孔便是微微一缩。
只见得在那最前方的那道光华中，有着一道苍老身影盘坐，他一身黑袍，那对眼瞳，竟是漆黑如墨，赫然便是当年将那吞噬祖符传给林动的黑瞳老人！


第1252章 诛杀
林动有些发怔的望着那光华之中的黑瞳老人，他自然是将后者认了出来，当年他便是从后者的手中，获得了吞噬祖符。
而他，也是上一任的吞噬祖符的掌控者。
“前辈…”
林动目光一扫，只见得那周围无数的光华中，竟都是有着身影盘坐，旋即他心头便是明悟过来，这些便是大荒宗那无数强者残留在这片空间的意志，即便是身死道消，可他们依旧还是要固执的守护着他们的宗派。
“你这老鬼！”
那九王殿也是见到一幕，面色顿时一变，怨毒无比的望着那黑瞳老人：“你将本殿封印上千载，以为这样就能将本殿抹杀吗？”
“哈哈，不过你看看这千载之后，本殿依旧存活，而你却是化为灰烬，我们之间，谁赢了？！”
黑瞳老人望着狂笑中的九王殿，那苍老的脸庞上却只是浮现一抹淡淡的笑容，而后他站起身来，对着林动缓缓的弯身。
周围那无数道光影，也是同时弯身行礼。
“吾等愿随之诛魔。”
似是有着古老无比的声音，从天空中传开，而后他们的身体，竟是爆发出万丈光芒，在那弥漫天际的光芒中，弥漫着一种极端强大的轮回波动。
林动漆黑的眼瞳中，在此时也是有着流光波动，那黑色长箭微微的颤抖着，那浓郁的轮回波动弥漫而来，竟是令得箭身上，有着绚丽的光华流溢开来。
一种无法形容的可怕波动，在此时缓缓的散发出来，竟是令得这片空间都是剧烈的颤抖起来。
林动也是因为黑箭的变化而有所动容，他能够察觉到，这支黑箭上面，被这些大荒宗的无数强者赋予了最后的意念，那道意念纯粹到极点，他们要将这毁掉他们宗派的九王殿诛杀…
“没想到，上千载后，这般意志依旧如此的强烈。”
林动轻轻一叹，那黑眸之中，冷光缓缓的凝聚，手中黑箭，将那逐渐变色的九王殿锁定。
“前辈，当年我承您传符恩情，今日，便为你们大荒宗讨回这份大仇！”
林动的面色，愈发的冷厉，而后空间波动，一股神秘的力量破空而来，最后覆盖在了黑箭之上，那种混沌之色，瞬间便是令得黑箭之上弥漫的轮回波动内敛而去。
一只黑箭，朴实无奇。
然而就是在这支黑箭的锁定下，那九王殿的身体都是微微颤抖了一下，那魔瞳之中，有着一抹罕见的恐惧之色涌出来。
这一刻，他真正的察觉到了死亡的味道。
这种浓郁的死亡味道，即便是千载之前面对着黑瞳老人临死反扑时，也未曾出现过！
“小子，你真要与我死拼吗？！我们何不各退一步？”九王殿厉声喝道。
林动眼神漠然的盯着他，然后嘴巴一动，两个冰冷的声音清晰的吐出：“蠢货！”
九王殿一愣，那脸庞顿时变得狰狞下来：“敬酒不吃吃罚酒的小子，你真以为这样就能抹杀本殿？不知天高地厚的东西！”
伴随着他厉喝传出，只见得滔天魔气自其体内席卷而出，他身体表面的那魔皇甲也是剧烈的蠕动起来，一道道诡异无比的符文自那魔甲之上浮现出来，而那九王殿的身体，也是随之膨胀，短短数息之间，便是化为一尊巨魔，那等魔气，遮天蔽日。
林动望着那将魔气催动到极致的九王殿，眼中凌厉却是愈发浓郁，那紧紧扣着弓弦的手指都是微微颤抖起来，一丝丝的鲜血渗透出来，最后顺着弓弦滴落而下。
长箭之前的空间，也是在此时缓缓的蹦碎开来。
箭虽未射，但仿佛连这片天地都是有些承受不住那等恐怖威压。
“去吧。”
林动心中发出一道喃喃之声，而后那紧扣着弓弦的手指，终是带起一缕鲜血，猛的松开。
呜！
尖利无比的声音，在此时响彻天地，那道黑箭犹如是在此时撕裂了空间，几乎是在瞬息中，已至那九王殿前方。
吼！
九王殿喉咙间也是爆发出低吼之声，那滔天魔气涌动，在前方的化为一座座狰狞的魔气门户。
砰砰砰！
然而那等强大的防御，却是在那缕黑光之下被摧枯拉朽般的摧毁而去，短短数息之间，滔天魔气尽数崩溃，还不待那九王殿有其他的反应，那黑箭已是在其紧缩的瞳孔中，狠狠的射中他的身体。
铛！
隐约的，似乎是有着一道金铁之声传出，那魔皇甲之上竟是在此时崩裂开一道道裂纹，黑箭毫不留情的射穿魔皇甲，然后洞穿了那九王殿的身体。
嘭！
九王殿身形倒飞出上万丈，身体之上的魔皇甲竟是不断的蹦碎开来，凄厉以及难以置信的惨叫声，自其嘴中爆发而起。
九王殿最终落地，他满身都是流满了黑色的鲜血，他缓缓的低头望着那破碎的魔皇甲，脸庞上还犹自残存着无法置信之色。
当年那黑瞳老人拼尽全力都无法办到的事，眼下，却是被林动一箭所破！
那由他们尊贵无上的皇所赐予的魔皇甲，竟然，被破了？
“怎么可能？！”
九王殿喃喃自语，下一刻，他的眼神陡然变得凶狠起来，不过他身躯刚刚一动，便是爆发出凄厉惨叫声，只见得无数道光华，从其体内暴射而出。
砰砰砰！
而在那等光华暴射间，他的体内，仿佛也是有着炸雷正响起，而没伴随着一道雷爆声响起，这九王殿的面色便越是苍白，气息也是迅速的被削弱。
“砰！”
当最后一道剧烈的爆炸声响起时，那九王殿的身体竟然是生生的爆炸开来魔血溅射而开，令得地面之上都是变得粘稠邪恶起来。
魔血渗透在地面上，隐隐间，一道魔光，便是要对着地底钻去。
“我说过今日，必将你斩杀！”
林动冷漠的声音，在此时响起，只见得他手掌一挥，温和的白芒猛的暴射而出，化为一道光罩，将那道微弱的黑光所笼罩。
嗤嗤。
白光笼罩下来，那黑光之上顿时爆发出阵阵白雾凄厉的惨叫声随之响起。
白光缓缓的手回，只见得在那白光之内，一道巴掌大小的魔影悬浮，那般恐惧的面容，赫然便是那九王殿，只不过此时的后者，全然没了先前的威风与凶气。
林动注视着被白光包裹的九王殿，心中也是忍不住的一叹这些异魔生命力真是顽强得可怕，即便是遭受到这般恐怖无比的攻击，竟然还能够残存，别看这九王殿现在伤得极重，甚至连魔体都是被毁，但若是让他逃了恐怕百年时间，又是能够逐渐恢复过来。
难怪当年就算是远古八主与他们对恃，也未能见到将这些魔狱高层斩杀多少反而个个如同百足之虫死而不僵，而眼下，这九王殿，终是落到了他的手中，虽然这是因为他借助了大荒宗那无数先辈强者残留在这片空间的意志。
“在这里将你解决，想来日后天地大战也是能够减轻一点负担吧。”
林动冲着那白光之中的九王殿淡淡一笑，旋即他伸出双手只见得手心之中，雷霆祖符以及吞噬祖符皆是浮现出来。
两大祖符，一左一右，刚好是将那九王殿夹在其中。
“似乎还不够。”
林动见这阵仗，却是想了想，再度将大荒芜碑也是召出，而后悬浮在其上方，接着又是取得玄天殿，置于九王殿下方。
“你…你！”
九王殿见到这一幕，面色终于是剧变起来，这四大神物皆是对于异魔有着极强的克制性，即便是他生命力极端的顽强，但这般时候，还是感觉到了浓浓的不妙。
林动却是丝毫不理他，掌心再度一握，祖石也是浮现出来，而后掠进那白光之中。
五大超级神物，散发着淡淡的光芒，然后逐渐的连接在一起，而后那白光开始缓缓的缩小。
“啊，不要啊！”
白光越来越小，其中的那九王殿也是愈发的缩小，他那凄厉的惨叫声传出来，面对着这种五大超级神物的净化，就算他实力滔天，也是必死无疑！
然而里弄对他的惨叫却是理也不理，双目微微闭上，而那白光，也嘴中是缩至米粒大小，最后砰的一声，竟是爆裂开来。
一股无法形容的波动扩散而开，下方的大地，都是被掀起百丈土浪，一片狼藉。
林动双目这才睁开，他望着那散开的白光，苍白的脸庞上也是有着一抹如释重负般的笑容浮现出来，这魔狱九王殿，算是彻彻底底的被他抹杀了…
“真是不容易啊。”
林动袖袍一挥，将这五大神物收入体内，这些魔狱的最高层的确恐怖，即便是这般状态，想要彻底抹除也是费了他如此大的力…
“前辈，此魔已诛，你们的心愿，想必也该了去了。”林动抬头，望着天空上悬浮的那些光影，再看向那黑瞳老人，轻声说道。
天空上，所有的身影都是在此时对着林动深深鞠躬，那黑瞳老人的脸庞上，仿佛也是有着欣慰之色散发出来。
“多谢小友，大恩无以为报，唯有聊表心意。”
天空上，黑瞳老人他们的身体开始逐渐的散去，而随着他们身体的散去，突然有着一道道奇特的光点成形，而后这些光点，对着林动飘掠而来。
林动伸出手掌，轻触着那光点，旋即身体微微一震，眼中却是有着惊喜之色涌出来，这是¨轮－回感悟？


第1253章 差之丝毫
漫天绚丽的光点缓缓的飘摇，整片天地仿佛都是在此时变得梦幻了一些，那种奇特的轮回波动荡漾着，令得人的眼神都是略微的有些迷幻。
林动则是静静的盘坐在天空上，他望着那漫天的光点，漆黑眸子深处却是有着一些激动闪烁，如今的他，已是触及轮回，不过这距离突破到轮回境，依旧还是有着不小的差距，眼下这番机遇，倒是能够将那差距大大的弥补一番。
这些轮回感悟，乃是大荒宗无数先辈强者所遗留，他们即便是身死，都是守护着宗派，如今那毁灭他们宗派的凶手被斩杀，他们也是彻底的放下了心中的执念，那些意念开始消散，而这些轮回感悟，则是被他们遗留了下来。
“姜雪姑娘，这与你也是一番际遇，可莫要错过了。”林动望着山峰的姜雪，微笑道。
姜雪闻言，也是轻轻点头，摸了摸茵茵的小脑袋，然后盘膝而坐，她自然也是很清楚这些轮回感悟对她有着多么的重要。
倒是一旁的茵茵坐在石头上，小手托着小脸蛋，甩着两条雪白的小腿，显得甚是无聊，那些轮回波动在其眼前飘过，却是未能让得她有丝毫的心动，只不过那大大的眼睛深处，仿佛是有着奇异之光在缓缓的涌动着。
林动凌空盘坐，身体却是在此时犹如形成了一个黑洞，天空之上那些绚丽光点源源不断的涌来最后尽数的没入林动身体之中。
而伴随着这般磅礴的轮回之意涌入林动体内，一种淡淡的玄妙之感，也是悄然的在林动心中荡漾而起，原本清明的神智，也是在这种轮回之意的侵润下逐渐的模糊，那隐约之间，仿佛是要坠入某种无法言明之地，但却始终无法彻底的进入。
于是，便是唯有沉侵在那种感觉之中，逐渐的体悟。
这是一片黑暗之地，没有任何的光亮黑暗之中，有着无尽的邪恶缓缓的流动，那种邪恶并不属于这个世界。
在那黑暗的中央，有着知道血红色的灯盏，那灯丝如骨，缓缓燃烧间，渗透着无尽的魔气。
突然间，其中一盏血灯波动了一下那原本邪恶的火焰，竟是在此时逐渐的微弱下来，最后彻彻底底的消散而去。
嗡。
随着那盏血灯的熄灭，这黑暗深处，突然有着一些波动传出，隐约间似乎是能够见到一些黑影在那黑暗之中模糊的出现。
“老九…被抹杀了。”
黑暗中，有着沙哑的声音，缓缓的传出。
“死了？呵，这倒霉的家伙…当年说是去对付上一任吞噬祖符的掌控者，结果便是失去了音讯，不过看其血灯尚在，应该只是被封印了吧，怎么如今突然被抹杀掉了？”另外有着阴柔的声音传出。
“应该是遇见厉害的对手了，老九同样有着魔皇甲寻常人可极难将其抹杀。”
“我们这些年倒一直是在探测他的方位，不过却是没什么反应，看来当年那吞噬祖符的掌控者，也是有些能耐。”
“说不定是那些远古之主动的手，我在妖域感应到了空间之主的存在，不过只是短暂的交过手，便是各自退走了，我看这家伙的实力，也是尽数恢复了。”
“你们自己行事小心一些，前些时日我们便是差点被雷主察觉。”
“雷主？怕他做什么？若他真敢来的话，正好先将他解决掉。”
“雷主若是有难的话，那其他那些远古之主也会被逼得提前现身，而我们准备尚未周全，交起手来，恐怕又是当年那般结果。”
“那现在怎么办？”
“不过时间也差不多了，我们也是该可以启动计划了。”
“嘿嘿，怎么要动手了么？忍耐上千载，真是有些迫不及待了啊…”
“嗯，准备吧，这一次的天地大战，他们不会再有希望了…”
黑暗中的邪恶之气逐渐的淡化而去，那些模糊的身影，也是悄然散去，那等寂静之下，仿佛是有着一场笼罩天地的风暴缓缓涌动。
当林动从那种玄妙的状态中退出来时，已是十日时间过去，他睁开依旧有些朦胧的双眼，望着眼前熟悉的天地。
呼。
他沉默了片刻，深吸一口气，眼中开始恢复清明，伸出修长的手掌，元力凝聚间，他能够感应到其中涌动的浓郁轮回波动。
这般波动，比起以往，强悍了许多，不过，那种程度，依旧未能彻底的达到轮回境。
“还差一点点啊。”
林动喃喃自语，他能够感觉到，此时的他，处于转轮境的巅峰，距那轮回境，几乎仅有半步之遥，只要半步能够跨出去，他便会是真正的轮回强者！
只不过，这一步，想要跨出去，可并不容易。
林动淡淡一笑，倒也并未感到失望，虽然他机缘深厚，但若是轮回境真的这般好晋入的话，也不会让得那无数惊才绝艳之人，一辈子都是无法触及了。
他有着一种感觉，这一步，或许不久之后，便是能够真正的踏出。
“你醒了？”
不远处，也是轻柔的声音传来，林动抬头，然后便是见到姜雪笑吟吟的将他给看着，此时后者的眼神也是深邃了不少，显然同样是获得了不小的好处。
林动微微点了点头，目光看了一眼山峰上，那守护者如今正呆呆的盘坐着，眼中不断有着茫然之色浮现，但更多的是一种大仇得报的解脱“以后随着修炼，它的灵智应该能够恢复一些，说起来，还得谢谢你为大荒宗所做的，我继承了大荒宗的传承，便代他们为你感谢一声。”姜雪轻声道。
林动摇了摇头，道：“大荒宗与我也是有着恩情，做这些倒是应该的，而且若是让那九王殿跑了，日后也会是我的大麻烦，所以感谢倒是不必了。”
姜雪微微一笑，玉手仲出，只见得在其手中有着一片黑光，黑光之中，似乎是一件破碎的甲胄，隐隐的，有着邪恶之气散发出来。
林动见状，眼神却是一凝，这东西赫然便是那魔皇甲。
“九王殿是你所杀，这也应该是你的战利品，而且我想其他人应该也无福消受。”姜雪道，这魔皇甲之上的魔气太重，对于那些异魔是神物，对他们而言，却毫无作用。
“嘿，这可是好东西，虽然魔气甚重，不过将其投入玄天殿内炼化，我想那股能量，应该能够让大荒芜碑彻底恢复。”岩欣喜的声音在此时响起。
林动眉头一挑，让大荒芜碑彻底恢复？想那黑暗圣镰完全恢复后，便是足以与渡过一次轮回劫的巅峰强者媲美，而这排名第三的大荒芜碑若是彻底恢复，那该有多强？应该能抗衡一下那种渡过两次轮回劫的巅峰强者吧？
林动想到此，心中也是有着热意升腾起来，然后也不客气，伸手将这破碎的魔皇甲接过，丢进玄天殿内，让其逐渐的炼化。
“我们也先出去吧。”这大荒碑中的问题被解决，林动四处一扫，道。
姜雪也是点点头，伸手召过茵茵，后者也是喜滋滋的跑过来，因为以前需要她来镇压这里的魔气，所以姜雪也只能让她一个人留在这里，如今能够出去，这小丫头自然是兴奋之极。
林动见状也是一笑，袖袍挥动，空间扭曲间，三人再度回神时，已是在那大荒碑之外。
在大荒碑外，姜雷等人一直的等待着，他们见到林动三人出来，再看着姜雪那面带笑容的脸颊，顿时也是大松了一口气，显然那最困扰他们的问题，已经被解决了。
“姜雷大叔，既然眼下问题被解决，我也先回帝都了，若是还有什么需要帮忙的地方，只管来找我便是。”
林动对着姜雷一笑，道，他原本以为很快能够解决问题，没想到又是耽搁了十日光景，眼下事情解决，倒是不能再停留了。
姜雷听得他要离开，也是一怔，旋即看了姜雪一眼。
“此番便麻烦你了。”姜雪倒是微微一笑，只是美眸深深的看了林动一眼。
林动笑了笑，然后伸手摸了摸茵茵的小脑袋，道：“日后茵茵若是有什么变化的话，你们便来找我，我会尽力相助。”
“大哥哥。”茵茵倒是有些不舍的抓住他的衣袖。
林动冲着她温和的一笑，再度闲聊一会，终是掠出身形，很快的便是消失在了天际之边。
姜雪驻步，她眸子望着远处消失的身影，脸颊上的笑容方才缓缓淡化，最后轻轻一叹，有些苦涩。
“若是喜欢的话，怎么不留留他啊？”姜雷也是无奈的道，这丫头，倒也是固执，从头到尾都不曾表明一下自己的心意。
“我喜欢他，这种感觉，就让我一个人承受便好了，而且，以后又不是没机会见面的…”姜雪微微一笑，旋即玉手轻挥，转身而去，那般模样，倒是颇为的洒脱。
姜雷见状，也唯有苦笑一声。


第1254章 回宗
“事情解决掉了吗？”林氏宗族内的客厅中，林啸望着归来的林动，笑了笑，问道。
林动点了点头，而后将那大荒碑中所发生的事情粗略的说了一遍。
林啸听得那大荒碑中竟然有着如此强大的异魔，面色也是变了变，旋即暗叹着点了点头，此番如果不是林动的话，恐怕又将会是一场灾难爆发吧。
“爹，我出来也有些时日了，明日或许就得先回道宗。”林动将一枚玉牌递给林啸，道：“若是再出现什么问题的话，便捏碎玉牌，我会立即赶来。”
出来这么久，也是得回道宗一趟了，而且他能够感觉到祖石之中周通的元神开始恢复，想来他醒过来的时候，是很想看见道宗吧。
林啸接过玉牌，点了点头，而后笑道：“对了，你也去看看清竹姑娘吧，你将人带回我们家，然后又消失十来天时间，可是太过不礼貌了。”
林动望着林啸那幸灾乐祸的笑容也是无奈的摇摇头，他原本以为很快就能解决的，没想到竟然拖延了这么久。
出了客厅，林动便是直往后院，而后便是在那庭院中见到正与柳妍交谈的绫清竹，后者见到他出现，只是淡淡的瞥了他一眼便是收回了目光。
“呵呵，动儿回来了啊。”柳妍倒是冲着林动一笑，然后上前拍了拍林动的肩膀，对着绫清竹那边使了个眼色接着笑着远去。
林动苦笑一声，冲着绫清竹走去。
绫清竹长袖轻挥，美目微垂，淡声道：“我明日便回九天太清宫。”
虽然她语气依旧那般的清淡，但林动却是能够听出一丝极淡的恼意绫清竹被他这样带来林家，心中本就是有点惶然，结果这段时间林动却是直接消失而去，这无疑令得她有些尴尬与羞恼，不过她性子倒是极好，每日与柳妍以及林动长辈相处倒是笑颜相对，滴水不漏但难免心中会对林动这种不打招呼的失踪而心生恼意。
把她带来林家，而他却消失，这是个什么道理？
林动连忙拉住她的衣袖苦笑道：“这次的确是我的不对，我原本以为很快能解决问题的，没想到耽搁了这么久。”
林动说着，目光打量着绫清竹那绝美的脸颊，道：“这次我遇见了那魔狱的九王殿。”
“九王殿？”
绫清竹脸颊终是一变，与七王殿交过手的她自然是明白魔狱这种以王殿命名的异魔的恐怖之前即便他们联手都未能在那七王殿手中讨得好处，这次林动竟然独自一人遇见了一位九王殿？
“你没事吧？”
绫清竹月眉微蹙，清眸看向林动，那眸中有着一丝关切担忧之色。
“还好，那家伙因为封印实力大减，不过依旧让得我与他大战了数天数夜…”林动目光微转而后他斜瞥着绫清竹，口若悬河的将那与九王殿的战斗扩大了数倍凶险的说了出来，那番模样，倒是经历了九死一生，方才最终险险获胜。
不过他这番苦肉计对绫清竹貌似没太大的作用，那张绝美的脸颊一直清淡如水，任由林动说得天花乱坠都没有半点的动静。
林动见到无效，也只能无奈的住嘴，聪明的女人果然难对付。
绫清竹见到他越说越小声这才看了他一眼，而在见到后者那悻悻的脸庞时，红唇微微掀了一下，那眼神也是柔和了一些，虽然林动说得有些夸大，但遇见那九王殿，一番大战必然是少不了的。
“下次若是再这样，我可不来了。”
原本有点悻悻的林动听得突然自一旁传来的声音，也是一愣，旋即干笑着点点头：“放心，下次去也带着你。”
心中却是松了一口气，总算是打发过去了。
“你明日便要回去了？”林动话音一转，问道。
“嗯。”绫清竹螓首轻点，道：“毕竟出来这么久了，我身为宫主，虽然宫内有着诸多长老维持，但也不能一直这样。”
“正好，明日我也打算先回道宗一趟，那便将你送回九天太清宫吧。”林动点点头，道。
“我回宫内便会闭关参悟轮回，或许短时间内不会出关。”
绫清竹看着林动，道：“太上感应诀，你要多加感悟，你那般感应毕竟不是自身感悟而来，如今仅仅只算是触及门槛，你想要达到那些远古之主的程度，还远远不够。”
林动怔了怔，旋即眼神复杂的看着绫清竹，她果然是知道自己学“太上感应诀”的目的啊。
绫清竹避开他这般目光，然后对着院外而去：“你奔波了一阵，先休息吧。”
不过在其转身时，一只大手却是突然握住了她那如玉皓腕，大手上传来的温度让得她娇躯一颤，急急的想要挣脱，但那手上的大力却是让得她挣脱不得，她那素来清冷的绝美脸颊上，在此时闪过一抹细微的慌色：“你…你要干什么…啊？”
手上突然有着大力传来，然后她便是轻撞在了一道结实的胸膛上，再接着她那纤细柔软的腰肢，被一双手臂紧紧的环住。
“谢谢。”
还不待她挣扎，一道蕴含着歉意的轻声便是在其耳边响起，绫清竹娇躯微静，俏立在原地，脸颊泛红，也不知道该怎么动作才好，若是换作旁人的话，恐怕她早就拔剑将其劈成两半，但对着眼前这家伙，她却是无法下狠手，只能贝齿轻咬了咬下唇，任由着他这番轻薄。
不过林动倒也并没有进一步的什么动作，他只是将怀中的人儿抱住想要以此来缓解一些心中的歉意。
庭院幽静，许久后，那院门处一道娇俏倩影浮现出来，她一眼便是见到了里面拥在一起的两道身影，顿时小嘴便是微撅了起来故意的轻哼出声。
她的声音虽然轻微，但立即便是将庭院中气氛打破，绫清竹猛的回过神来，羞恼的盯了林动一眼，那番风情令得后者心头都是忍不住的颤了颤。
不过还不待他说话，绫清竹已是落荒而逃，那番模样哪还有平日的清冷淡然。
林动干咳了一声，望着门口那撅着小嘴的青檀，也是尴尬的笑了笑。
青檀走到林动身旁俏目也是看了看绫清竹离去的方向，道：“林动哥，我记得还有一个道宗的应欢欢来着吧？你这样…”
林动听得这话，顿时脑袋大了一圈，这丫头还真是哪壶不开提哪壶，当即仲手狠狠的揉了揉青檀小脑袋道：“大人的事，小孩子别瞎管。”
“我可是黑暗之殿殿主，哪还是什么小孩子！”青檀不乐意的道。
林动伸出手指，笑着弹了一下青檀光洁额间。
“林动哥，你明天要去道宗了？”青檀突然抱着林动手臂，笑嘻嘻的问道。
林动点点头看这妮子的模样，应该是从林啸那里听来的吧。
“我也要去！”
“不行，留家里陪爹娘！”
“爹娘都答应了！”
“不行！”
“那我发元神传信给欢欢，把你刚才做的事告诉她！”
“……你欠揍？”
翌日，林动望着那巧笑焉熙跟在自己身后的青檀，只能没好气的摇了摇头，总算是见识了一番这丫头的缠人大法。
在他身旁，绫清竹优雅而立，此时正与前来送行的柳妍微笑着交谈。
“爹娘，那我们便先走了。”林动见到时辰差不多，也就对着林啸柳妍一笑，说道。
“嗯，小心一些。”
林啸二人也是点点头，如今的林动毕竟不再是当年的少年，他们知道自己这儿子如今的成就，那足以让得他们自豪。
林动也是笑应着，然后也就不再拖延，袖袍一挥，三人已是化为流光远远遁去，眨眼间便是消失在了那天际之边。
离开大炎王朝，林动先是将绫清竹送到九天太清宫，经过这段时间的修整，九天太清宫已是再度完善，林动二人在此处略作停留，也是告辞而去，直奔道宗。
短短半日光景，道宗山脉已是进入视野之中，林动目光一扫，身形便是微微一顿，只见得在那远处的一座山峰上，一道倩影坐于峭壁之间，冰蓝色的长发倾泻开来，一种寒气散发出来，令得周围的地面都是结满了坚冰。
“欢欢？”
林动见到那道熟悉的倩影，却是一怔，连忙带着林动飞掠而去，此时的后者也是察觉到他的归来，那冰彻的美目中，泛起了道道波动。
“你回来啦？”应欢欢起身，那俏丽脸颊上有着微笑浮现出来。
“你怎么在这里？”林动笑问道。
“等你啊。”应欢欢纤细玉手将耳畔一缕冰蓝长发挽起，微笑的轻声，仿佛是在说着一件很平常的事。
林动愣住，他望着应欢欢那美丽的脸颊，那冰蓝色的眸子，犹如一颗蓝宝石一般，冰彻见底，而在那仿佛能够冻结空间的冰寒中，有着一些柔意在涌动着。
与之前相比，现在的她，似乎变得安静了一些，隐隐的，他感觉到一丝不对劲，但又是说不上来。
“你这状态哪还能乱跑？”
林动轻叹一声，对着她伸出手来，道：“我们先回去吧。”
应欢欢轻点螓首，微微笑着，纤细而冰凉的玉手握着林动手掌，感受着他那掌心的温度，她那冰冷的心中仿佛也是泛起了一些涟漪，唇角的笑容，愈发的轻柔。
“你是欢欢？”
那跟在林动身旁的青檀望着眼前与当年大变了模样的应欢欢，却是有些惊愕，道：“你怎么变成这样了？”
“不要瞎问。”
林动拍拍青檀小脑袋，然后也是将她拉起，直接是对着那道宗掠去，而在进入道宗时，他目光转向后方的一座山峰，冲着那里微微感谢的点点头。
炎主站在那里，望着进入道宗的三人，而后神色复杂，轻轻一叹。


第1255章 师徒相聚
进了道宗，林动独自在前，身后青檀与应欢欢倒是凑在了一块，两女以前就认识，还联手对付过元苍，关系还不错，如今见面，倒是都有些物是人非的感觉。
回去的路上，能够见到不少道宗的弟子，他们见到林动回来，眼中皆是有着惊喜之色，远远的便是有着各种热切的呼喊声传来。
林动冲着那些脸庞尚还有些稚嫩的新进弟子微微一笑，从他们的眼中，他能够看见一种灼热，那种灼热，是一种信念的凝聚，拥有着这种信念，若是很久以后道宗遭遇大荒宗那般劫难时，或许，他们的意志，也将会一直的守护着道宗。
三人掠上主峰，在那主殿之前，应玄子以及一众道宗长老已是在此，他们见到林动归来，那脸庞上也是有着笑容浮现出来，虽说如今东玄域战乱平定，但唯有林动在宗内时，整个道宗内的士气方才能够彻彻底底的凝聚在一起。
林动冲着应玄子行了一礼，略作交谈后，微微一顿，道：“掌教，周通师兄应该可以出来了。”
听得此话，应玄子的脸色也是变得复杂了许多，旋即他轻轻点头，后面一众长老也是将眼神投射了过来，在林动未曾出现之前，周通便是道宗的上一任传奇，若是他能够在道宗安然的修炼到现在，恐怕早已成为了道宗掌教。
林动手掌轻握，温和的白光自其掌心散发出来旋即祖石浮现而出，他屈指一弹，一道光华便是自祖石之中掠出。
光芒逐渐的凝聚，最后在众人的注视下，化为一道黑衫身影正是周通。
周通逐渐的睁开紧闭的双目，将那位于前方的应玄子一众人收入了眼中，旋即他身体微僵，神色复杂，嘴唇微微蠕动，最后他缓缓的屈下身子，双膝跪地那略显沙哑的声音，在殿前传开。
“师傅，弟子回来了。”
应玄子的身体也是在此时颤抖起来那素来保持着镇定的面庞也是在此时浮现一种难以自制之色，他缓缓的上前一步，手掌轻轻的摸着周通的头，那眼中满是湿润，轻声道：“回来就好，回来就好啊。”
周围众多长老也是默然轻叹他们都知道周通这些年所受到的痛苦，当年道宗的传奇，却是差点沦落为元门手中的工具，并且用来对付他们道宗。
“当年…是弟子不识大局，意气用事，不听师傅之言方才遭此劫难。”望着应玄子那湿润的眼睛，绕是以周通这般心性，都是忍不住的流下眼泪。
“呵呵，年轻人哪会没个冲动的时候。”应玄子温和的一笑，旋即微微沉默，道：“只要你别怪为师怯弱，不肯为你报仇就好。”
“师傅身为一宗掌教，自然是要为整个宗派做想。”周通摇摇头，从始至终他都未曾怪过应玄子丝毫，就如同当初林动一般，他们都理解应玄子，因为有时候，做出那种看似怯弱的决定，比他们用命去拼斗，要更加的具备勇气。
应玄子抹了把眼角，然后他望着那庞大无比的道宗，声音却是突然在此时响彻了整个宗门：“我的弟子，整个道宗，欢迎你再回来。”
他的声音，远远的传开，无数道宗弟子都是在此时将目光投向了主峰，然后微微一怔，接着眼中顿时有着惊喜之色涌出来。
“那是…周通师兄？！”
虽然很多新人对这个名字颇感陌生，但他们却是从一些老一辈弟子脸庞上看出了一种激动之色，而后，那惊天动地般的欢呼声响彻而起。
“周通师兄，欢迎回来。”林动望着那沸腾起来的道宗，也是冲着眼睛通红的周通微微一笑，道。
周通自站起身来，他望着眼前的道宗，脸庞上同样是有着一抹激动的笑容浮现出来，然后他冲着林动抱拳道：“林动师弟，这一次，多谢你了。”
若不是林动使用祖石护住他的元神，并且为其温养，恐怕现在的他，早便是烟消云散。
林动笑着摇了摇头，再度伸出手来，掌心中银光闪烁，空间祖符便是浮现了出来：“既然周通师兄已经恢复过来，那这空间祖符，也该物归原主了。”
周通望着那空间祖符，也是微微一怔，他看着林动，后者面带微笑，眼神真诚，那之中并没有丝毫对这空间祖符的贪恋。
周通沉默了一下，缓缓的伸手接过空间祖符，目光盯着祖符，眼神之中充满着复杂，最后他似是轻笑了一声，指尖轻划，一道淡淡的血纹自空间祖符之上闪现出来，而后被其一指点破。
咔嚓。
血纹悄然的破碎而开，周通的面色却是瞬间惨白下来，他温柔的抚摸着空间祖符，轻声道：“抱歉，连累你与我受魔气侵染这么多年，不过，相信下一任掌控者，会比我更完美。”
空间祖符绽放着一圈圈的光芒，隐约的有着嗡鸣声传出，那声音中，似乎同样包含着不舍的情感。
“周通师兄…”
林动怔怔的望着这一幕，身为祖符掌控者他自然是知道周通做了什么，他单方面的切断了与空间祖符之间的关系，而这种切断对于他而言，显然是造成了极大的重创，甚至连他的元神，都是在此时变得薄弱了许多。
“林动师弟，身为祖符掌控者，你应该也知道未来天地间的劫难，我想，空间祖符，你会比我更适合。”周通冲着林动微微一笑，再度将空间祖符递出：“我能够感觉得出来，它也是这样认为的。”
林动面色复杂，眼瞳盯着那闪烁着银芒的空间祖符，轻轻一叹。
而在其有些犹豫间，一只冰凉的玉手轻轻的握住他的手掌，他偏过头然后便是见到应欢欢那张俏美的容颜，她轻声道：“这是空间祖符与周通师兄共同的选择。”
林动苦笑一声，终是点了点头，伸出手掌，接过空间祖符，道：“周通师兄，我不会让得它蒙尘的，祖符掌控者的责任，我也会一力担之。”
嗡。
淡淡的光芒自空间祖符之中散发而出，而后犹如液体一般，悄然的融入林动的掌心之中，此时他能够感觉到，一种古老而纯净的力量，在体内荡漾而开。
吞噬祖符与雷霆祖符也是在此时散发出阵阵波动，条件反射般的在抗拒着空间祖符的同体，不过紧接着祖石突然爆发出温和的光芒，在那种光芒的照耀下，那种彼此间的抗拒，也是悄然的减弱下来。
银色的光芒涟漪，一圈圈的自林动体内散发出来，周围的空间，都是在此时犹如湖水般，泛起剧烈的涟漪。
“林动师弟，空间祖符已是认同你，接下来你便尽快将其掌控吧。”周通望着那融入林动体内的空间祖符，微笑道。
林动点点头，他能够感觉到伴随着空间祖符的入体，他体内的元力，正在以一种惊人速度膨胀起来，这令得他平静的心境中泛起阵阵涟漪。
有了空间祖符这奇兵，他终是可以正式的冲击那轮回之境。
林动转身，让应欢欢先照应着青檀，而后也就不再迟缓，身形一动，直接出现在不远处的山峰上，径直的盘坐而下。
现在的他，需要将这空间祖符彻底的炼化掌控。
耀眼的银色光芒，不断的从林动体内散发而出，犹如一轮银色耀日，将那道宗内无数道目光都是吸引了过去。
应欢欢见到林动进入修炼状态，玉手轻扬，寒气直接是在那山峰周围化为冰罩，将其护在其中。
“看来他是想要借机冲击轮回境了。”应玄子他们眼光都不弱，一眼便是看出林动的目的，当即都是有些感叹，谁能想到，当年的小小弟子，如今却是将他们都是远远的抛在了身后。
“林动师弟的天赋比我还强，我道宗有他，定会长盛不衰。”周通也是点了点头，林动如今的成就，即便是他都是相当的佩服，当初元门之外的那一战，也是让得他彻底的认同了林动的实力。
“接下来便安静的等待吧。”应玄子微微一笑，若是林动能够晋入轮回境，他就真的会跻身晋入这天地间顶尖的层次，那时候，能够超越他的人，或许屈指可数。
“臭林动哥，又把我一个人丢下。”青檀微撅着嘴，嗔道。
应欢欢将她拉着，笑道：“我来替他照顾你。”
青檀闻言，这才甜甜一笑，大眼睛看向那座山峰，眼中掠过一抹旁人难以察觉的微涩之意，其实，能够这样的看着你，我也心满意足了。
远处，山峰之上，炎主也是将视线投向林动所在的山头，他负手而立，轻声道：“三道祖符了么…林动，你还真是挺是不一般呢，竟然能够做到这种程度，不过…”
“你想要超越小师妹，那就让我们来看看，你究竟有没这个资格吧…”


第1256章 位面裂缝
磅礴而浩瀚的元力犹如洪水一般，在林动体内狂暴的涌动着，而且这般元力，还在以一种惊人的速度涨动着。
在林动的身体之中，三道祖符形成三角之状，遥遥对望，彼此间也是有着一些抗拒，而在它们中心位置，祖石静静的悬浮着，那种温和的力量，不断的化解着祖符之间的抗拒。
而在这三道祖符中，吞噬祖符与雷霆祖符相对而言要显得温和一些，毕竟他们已是被林动炼化，至于那新来的空间祖符，则是抗拒得有些明显，这种抗拒或许并非来自它的本意，而是祖符之间天生的排斥，这种力量，常人得其一已是天大机缘，想要共存一身，那几乎是无法想象的事。
自古以来，除了那位符祖之外，可还没人能够成功的将八大祖符成功的纳入体内，虽然现在的林动，还仅仅只拥有着三道祖符，但那般困难度，也非常人能够想象。
林动心如止水，并未刻意的消除那种抗拒之感，他只是静静的将心神沉侵下来，感悟着那种奇异的空间之力。
想要成功的掌控空间祖符，那便必须对它的力量，了如指掌。
不过所幸，拥有着掌控吞噬祖符与雷霆祖符的经验，如今的林动并未过于的慌乱，他心神沉侵在那空间祖符之中，他能够隐约的感觉到，周身的空间，仿佛是在此时蠕动起来，那种感觉，仿佛连神秘的空间，都是具备了生命力。
他的心神，在此时缓缓的蔓延出身体，然后融入那空间之中，最后顺着空间，如同水波一般的扩散开来。
空间，无所不在。
心神与空间交融，犹如无形的眼睛，将那天地万物，收入林动的心中，他看见了山峰之外笼罩着他的冰罩，也看见了那无数注视着这里的道宗弟子，然后他的心神，再度顺着空间，远远的飘散而开。
百里，千里，万里，以及更远的地方…
林动的心神仿佛是在此时化为游魂，以一种常人无法感知的状态，远远的扩散而开，而在这种扩散刚开始时，他能够感觉到心神融入空间所产生的那种阻涩感，但随着他心神越发的遥远，那种阻涩感也是悄然的减弱，知道最后彻彻底底的消散。
林动的心神，沉侵在这种无边无际的飘荡之中。
这般飘荡，不知持续了多久，林动发现他的心神开始模糊，这才猛的惊醒过来，而后便是骇然的惊觉，他的心神，如同化了这天地间虚无的空间，竟是再无法的召回。
他此时方才骤然的明白过来，这种空间历练，究竟是有着多么的凶险，若是心神无法回体，那么他的肉身，也将会犹如石雕，再也无法动弹。
一种慌意，在此时涌上心间，旋即林动强行令得自己冷静下来，这种时候若是乱了心神，恐怕就真是再无回归之时。
既然这是一种历练，那总归是有着破解之法。
林动心神渐稳，旋即他不再抗拒，而是顺着心神，彻彻底底的融入这空间，许久后，他突然发现自己的心神似乎开始顺着空间而升起，最后地面开始远去，耳旁风声大作，一切的景物，都是在以一种惊人的速度变得渺小。
云彩掠过，耀日远去，黑暗涌来，犹如无尽的虚无，令得人心生骇然，这种地方，即便是轮回境强者也是无法抵达。
这般漂浮，不知持续了多久，林动心神突然一震，向着后方望去，只见得在那遥远的黑暗虚无处，一道道数十万丈庞大的光线交织的蔓延而开，这些光线，仿佛是形成了一个巨大得令人毛骨悚然的阵法，上面所弥漫的能量，仿佛连天地都是为之颤粟。
林动心神望向那超级阵法之后，那里似是有着一道约莫十万丈庞大的黑色裂缝，裂缝犹如魔龙般蠕动着，在那之后，是没有丝毫光亮的无尽黑暗，犹如是恶魔之地，可怕的邪恶魔气，不断的涌出，但却是被那超级阵法阻拦而下。
林动的心神有些骇然的望着那黑暗裂缝，这…莫非这便是那传说之中的位面裂缝？而在那裂缝之后，便是那些异魔来自的地方？！
林动的心神，在此时剧烈的颤抖着，那般裂缝，仅仅只是看上一眼，就让得他差点心神失守，那种邪恶，犹如是要让人无尽的沉沦在其中。
若这裂缝便是那位面裂缝的话，那么眼前这约莫数十万丈庞大的超级阵法…应该便是那位符祖大人所留吧？
也唯有那等存在，方才能够布置出这种毁天灭地般的阵法，生生的将一个位面所封印。“符祖…”
林动心中喃喃自语，他的心神注视着那超级阵法，他能够感觉到，那阵法之中，随便涌出一股能量，都是能够将一名渡过两次轮回劫的巅峰强者轰得魂飞魄散。
那位存在，果然不愧是从古至今最为强大的人。
那种力量，令人敬畏。
而在林动心中为那超级阵法的可怕力量而震撼间，他突然猛的发现自己的心神竟是在此时逐渐的对着那阵法飘荡而去，这番发现，瞬间差点将其骇得满身冷汗，那种地方，不论是超级阵法还是那位面裂缝，都绝对不是现在的他能够抗衡的。
然而，此时不论他如何挣扎，都是无法摆脱那种吸引，因此他竟然只能眼睁睁的看着，自己的心神，距那超级阵法，越来越近。
而随着逐渐的接近，那自超级阵法中蔓延而开的力量威压，也是令得他心神颤抖起来，那种感觉，仿佛心神都是要在此时消散而去。
不过随着林动的接近，那超级阵法仿佛也是有所感应，一道淡淡的光晕散发而开，竟是在那阵法之上，隐隐的形成了一道几乎遮掩了天地的虚影，那道虚影无法看清他的模样，但却是有着一种凌驾天地般的波动散发出来。
那是…符祖？！
虚影仅仅只有半个身子，但俯身之下，犹如这天地都是在其身下，而后，他缓缓的仲出巨手，一根手指，穿透虚空，对着林动按下。
在这种攻击下，林动几乎是没有丝毫的躲避之力，下一瞬，那根手指，已是落到了他的身体之上。
砰！
一指按下，他的心神几乎是在瞬间爆碎开来，而后他便是感觉到黑暗在飞速的退去，日月再现，云彩浮现，而他的心神却是飞快的坠落而下。
唰唰！
这般下坠，持续了许久，旋即林动猛的睁开眼睛，脸庞上浮现浓浓的骇然之色，冷汗如雨般的自额头上浮现出来，瞬间打湿了整个身体。
“我回来了吗？”
林动微微有些颤抖的伸出手掌，然后握了握，看向外面的冰罩，视线透过冰罩，能够看见道宗那繁华的景象，无数道宗弟子在其中穿梭。
先前与此时，犹如隔着生与死。
林动仰起头，有些惊悸的望着天空，在那遥远得无法触及的地方，有着一道符祖燃烧轮回创造出来的超级阵法，以及那一道，可怕的位面裂缝。
那里，就算是轮回境强者都是无法抵达，但他却是借助着空间祖符，将其窥测。
只是那种感觉，太过可怕了…
先前的那一霎，林动几乎有种魂飞魄散的感觉，若是那道虚影对他有着任何一点杀心，恐怕他早已彻底的湮灭而去。
林动抹去额间的冷汗，然后他手掌突然一顿，他视线看向手掌，只见得他的手掌，竟是呈现淡淡的银色光泽。
这般变化让得他愣了愣，而后看向身体，他的身体竟然也是在此时呈现银芒，一种奇特的感觉，出现在其心间。
他微微闭目，只感觉周遭的空间仿佛与他融合在一起，只要心念一动间，便能够出现在千里之外的空间。
“这是…我掌控了空间祖符吗？”
这种奇特的感觉，让得林动一惊，旋即眼中有着狂喜之色涌出来，只有掌控了空间祖符的人，方才能够与空间无比完美的融洽。
林动迅速闭目，而后他便是感觉到，在其身体之中，空间祖符静静的悬浮，与吞噬祖符，雷霆祖符呈三角之状，只不过先前的那种抗拒，在此时却是尽数的消除而去。
嗡嗡！
在林动狂喜的注视着那安静下来的空间祖符时，后者突然震动起来，然后林动便是察觉到，随着空间祖符的震动，他体内浩瀚的元力突然在此时犹如涨潮的潮水一般，以一种惊人的速度凝聚而来。
而在元力疯狂的凝聚间，一种奇特的变化，开始在浩瀚的元力之中产生。
那仿佛是一种力量凝聚到极致之后的变化。
元力犹如海洋般荡漾在林动的身体之中，而在那元力海洋之底，奇特的波动，悄然的孕育着，一种无法形容的玄妙霊之感，绽放而开。
那是…轮回的波动！
林动的眼中，惊喜涌上，他知道，他要真正的跨出那阻碍了无数强者毕生修炼的一步！
一步之后，便是轮回！


第1257章 晋入轮回
岁月如梭，不知不觉，便是两月时间过去。
这两月之中，山峰之上的冰罩始终未曾散去，所有人都只能隐约的看见冰罩之内的那道削瘦身影，不过随着时间的推移，即便是道宗内的普通弟子，都是能够隐约的感觉到，似乎是有着一股相当恐怖的元力波动，正在那冰罩之内缓缓的凝聚。
道宗的上空，厚厚的云层滚动，那云层乃是由浩瀚的元力凝聚而成，其中元力汇聚的程度，达到了一种极端可怕的地步。
那种元力威压，即便是转轮境的超级强者，都是为之胆寒。
道宗主峰，应玄子等人也是仰头望着天空，眼中满是惊叹之色，如今的他也算是触及轮回的强者，但那最后一步，却是迟迟难以跨出，那不仅仅是缺少轮回感悟，而且他体内的元力，也未能凝聚到那种近乎变态的程度。
而元力不足，自然也是无法孕育出那种轮回之意。
“不过，看这模样，应该也快了吧？”
应玄子喃喃自语，他能够感觉到，天空之上凝聚的元力，已是逐渐的达到极限，若是再让其堆积下去，恐怕连这片天地都是被压塌而去。
轰！
而就在他的声音刚落下不久，那天空上，突然有着奇异的声响传出，无数道宗弟子惊异的抬头，只见得那弥漫了千里的元力云层，竟是在此时缓缓的扭曲了起来。
一缕缕奇特的光束穿透云彩，照耀下来，而凡是被这般光芒照耀的人，神情皆是有些恍惚，体内的元力竟然是悄然的膨胀了一些。
“轮回之光…”
不远处山峰上的炎主望着那些光束，眼神微微一凝，而后视线投向冰罩笼罩的山峰，这林动，终是踏出这一步了啊。
“踏入了轮回境，倒也算是够资格跻身晋入顶尖层次了。”
炎主自语一声，旋即他眉头突然一皱手掌轻轻的抚着胸口，眼中掠过一抹阴沉之色。
“这些家伙…终于动手了啊。”
轰！
天空上的元力云层翻涌的速度越来越快，下一刻一道巨声响彻，众人便是骇然的见到，那云层竟是被生生的撕裂而开，一道千丈庞大的元力洪流，猛的倾泻而下。
元力洪流，如同自九天之上行垂挂而下的瀑布带着一种惊天动地的声势，降临而下，最后在无数道骇然的目光中，落自那冰罩之上。
咔嚓。
坚固无比的冰罩，在此时却是瞬间崩溃开来，冰屑四溅。
那距此处不远处的青檀见到冰罩破裂脸色顿时微变，纤细小手一握，便是有着黑光流转而开，不过在其要出手时，一只冰凉的玉手却是将其握住阻拦了下来。
“这是冲击轮回境必须要经历的，外人帮不得，否则于他有弊无利。”应欢欢道。
青檀闻言，这才轻咬着嘴唇，有些担忧的望着那座山峰那种可怕的元力冲击实在是有些可怕，也不知道林动能否承受得住。
冰罩破碎，无数道目光望去，只见得那盘坐在山峰之上的那道削瘦身影也是在此时站起了身子，他望着那降临而来的元力瀑布，脸庞上却是没有丝毫的惧色，黑色眸子中，反而有着一种狂热在涌动。
嗡。
林动身体微微一震，只见得一圈巨大的黑洞，自其头顶上方浮现而出。
元力瀑布降临而来，最后狠狠的冲进那黑洞之中，那般可怕的冲击，即便是那黑洞都是剧烈的颤抖起来，林动的身体更是猛的后退一步，体内的脏腑仿佛都是被冲击得移了位置。
不过这般时候，他也是明白，万不能有丝毫的退缩！
黑洞旋转速度骤然加快，吞噬之力运转到极致，将那种可怕的元力疯狂的吞噬而去，然后尽数的传进林动身体之中。
而由于浩瀚元力的入体，林动的皮肤表面都是有着无数元气犹如蛇一般的蠕动着，看上去颇为的可怖。
元力瀑布，一头连接着天上云海，一头便是灌注进那黑洞之内，那番模样，犹如天地之间的一条巨龙，委实有些壮观。
不过，要成功晋入轮回境，显然并不是这么的容易。
就在那第一道元力瀑布降临而下不久，那元力云层突然再度翻涌起来，紧接着，众人便是有些惊骇的见到，足足十道元力瀑布，自那云层中呼啸而下，犹如白龙，张牙舞爪的冲向林动。
望着这一幕，应玄子等人也是有些色变起来，青檀更是将小手紧握着，一旁的应欢欢，那冰蓝眸子，也是紧紧的盯着山峰上那道身影。
呼。
林动同样是发现了这般变故，他瞳孔微微一缩，旋即眼中狠色掠过，若是连这一关都过不去的话，还谈什么超越那些远古之主？！吞噬祖符！”
低沉喝声，陡然自林动吼传传出，旋即一道纯粹到极点的黑芒自其天灵盖中呼啸而出，黑光在其上方飞快的凝聚，最后化为一道巨大无比的黑色符文。
符文缓缓的蠕动着，一种极端惊人的吞噬之力爆发开来，在那般吞噬之力下，众人也是骇然的感觉到，体内的元力，竟是有着忍不住要喷薄而出的迹象。
“那便是吞噬祖符吗？”
应玄子等人有些震动的望着那神秘的黑色古老符文，这便是那传说之中的吞噬祖符吗？
吞噬祖符急速的蠕动着，旋即迅速的化为一道黑色人影，那道身影，倒是与林动一般无二，只不过那对眼瞳，犹如黑洞一般，深邃无吼！
黑影仰天咆哮，一股恐怖无比的吞噬之力自其嘴中爆发出，那天空都是被吞噬得呈现扭曲之状，那十道元力瀑布也是受到牵引，带着惊天般的声势，疯狂的对着那道黑影嘴中灌注而去。
而随着这般恐怖的吞噬，山峰之上的林动体内，竟是不断的传出低沉的爆炸之声，接着众人便是惊异的见到，他的身体，竟然是在此时以一种惊人的速度膨胀起来。
短短十数个呼吸间，林动便是化为一道百丈巨人，那身体之中，元力如同怒蟒般疯狂穿梭。
嘭！
林动那膨胀到极限的手臂，突然是在此时爆炸而开，血肉溅射，无数道宗弟子都是惊骇失声，失败了吗？
紧张的目光投射而去，却是见到，那爆炸开的手臂下，玉色的骨骼闪烁着奇异光芒，在那骨骼之内，仿佛是有着龙吟声传开。
生气涌动，那骨骼之上，血肉再度以一种惊人的速度生长出来，不过此时林动的身体各处，却是不断的爆炸开来，那般血肉模糊，看得人心惊胆颤。
不过不论身体各处被元力如何撑得爆炸而去，但很快的便是有着血肉生长出来，只要骨骼脏腑健在，血肉便是能够迅速的生长。
而以林动那洪荒龙骨的厉害程度，这元力灌注虽然恐怖异常，但也极难摧毁他的骨骼。
砰砰砰！
所有人都是能够心惊肉跳的听见那山峰上传出的肉体爆炸声音，浓浓的血雾从那里弥漫出来，最后遮掩了山峰。
而这般爆炸声，足足从晌午持续到黄昏，终于是开始逐渐的减弱，所有人都是望着那里，浓浓的血雾粘稠得令人视线都是难以穿透而进，谁也不知道，林动有没有承受住那种可怕的元力灌顶。
天空上，最后一道元力瀑布，也是被那吞噬祖符所化的黑影吞噬而进，而后那道黑影也是剧烈的颤抖着，最后化为一道黑光，遁入那血雾之中。
吼！
而就在的黑影掠进血雾时，一道低沉的龙吟声，猛的自那血雾之中传出，那龙吟之中所蕴含的威压，令得无数人索索发抖。
璀璨的紫金光芒，陡然撕裂血雾，紫金光芒冲上天空，接着便是化为一条巨大的紫金巨龙，巨龙张开龙嘴，将那粘稠的血雾，尽数的吞进体内，而后仰天长啸。
惊天的龙吟扩散而开，天空上那些元力云层，竟然也是在此时呼啸而下，最后被那紫金巨龙，生生的吞进。
天空上，再度有着温暖的阳光倾泻下来，那种可怕的元力威压，也是缓缓的消散而去。
但那每一道目光，都是停留在那紫金巨龙身上，那里，强光射出，紫金巨龙开始扭曲缩小，最后终是在众人的注视下，再度的化为一道削瘦的身影。
那道身影，身着黑衫，黑色头发随风轻扬，那张年轻的脸庞，泛着淡淡的紫金光芒，一种无法言语的威严，散发而开。
他那漆黑的眸子，深邃如夜空，令人为之沉迷。
他脚踏虚空，修长的双手，缓缓的紧握，而后他猛的仰头长啸，啸声清澈，千丈光柱自其体内暴射而上，而后在那九天之上，化为一道巨大无比的光盘。
光盘轻轻转动，无始无终，犹如轮回。
“轮回之盘…”
应玄子等人望着那巨大的光盘，眼中也是猛的有着惊喜之色涌出来，当这光盘出现时，他们便是知道，林动真正的晋入了轮回之境！
天空上，林动的视线同样是停留在那光盘之上，那唇角，也是有着一抹淡淡的激动之色浮现而出。
“终于到达这一步了啊。”


第1258章 动荡之始
天空之上，云层散去，那种笼罩着元门两月之久的可怕威压，也是在此时逐渐的消退而去。
林动悬浮天空，他感受着体内那浩浩荡荡，近乎无穷无尽般的元力波动，唇角也是有着笑容浮现出来，果然不愧是轮回境啊，跟之前相比，真是天壤之别的差距，难怪即便是他拥有着诸多强大手段，再加上两大祖符之力，都依旧是被那七王殿搞得异常狼狈。
不过，若是现在再遇见那家伙的话，林动却是有着自信让得他讨不了多少的好处！
“林动哥。”
下方有着娇声响起，而后青檀掠来，她上上下下的打量着林动，在见到后者并无大碍后，她这才拍着胸松了一口气。
“没事吧？”应欢欢也是紧随而来，冰彻的眸子中，有着一抹关切之色。
林动摇摇头，道：“我此次闭关了多久？”
“两年。”青檀道。
林动一惊，旋即变得见到青檀嘴角的戏谑，当即便是怒得捏住那娇嫩的俏脸，道：“不想活了，连我都敢消遣？！”
“我错了。”青檀被一通收拾，这才捂着小脸委屈的道，望着她这番模样，林动却是忍不住的暗乐，旋即温柔的拍拍她的小脸蛋，他知道这丫头是想逗他放松开心来着，想想这小丫头如今那黑暗之殿殿主的地位，却是在他面前依旧保持着乖巧，他心中也颇感欣慰。
“你闭关了两月时间。”应欢欢见到他们兄妹打闹，也是一笑，道。
“两月啊。”林动这才点点头，他迷失在那空间之中，已经失去了时间的概念，所幸，最终还是脱了身。
“呵呵，林动师弟，恭喜了。”
随着青檀二人上前，那应玄子，周通等人也是迅速的掠上天空，周通更是冲着林动抱拳一笑，道。
林动回以一礼，目光看了周通一眼，只见得后者元神愈发的凝聚，想必要不了多久的时间便是能够凝聚肉身，到时候，他也将会彻底的恢复过来。
“我修炼这么多年，依旧卡在转轮境顶峰，你这小子不过修炼十数载便是能够将我超越，真是…”应玄子感叹道，颇有些嘘唏，这世间，总归是有些怪物的。
林动笑了笑，刚欲说话，其神色突然一动，只见得那不远处的一座山峰上，突然有着炽热的波动爆发开来，那是，炎主？
“怎么了？”
应玄子他们也是察觉到这般变故，面色皆是一惊。
咻。
远处，一道红芒急速的掠来，然后出现在林动他们面前，正是炎主，只不过此时他的身体通红一片，隐隐的有些虚幻的迹象。
“怎么回事？”林动见到他这般变故，也是微微一惊，连忙问道。
“乱魔海出事了。”炎主的眼神有些阴沉，道。
林动瞳孔微微一缩，道：“是异魔？”
“嗯，他们开始有动静了。”炎主点点头，道：“如今乱魔海彻底的混乱起来，无数异魔从那海底涌出来，我想或许再有一些时间，消息就会传到东玄域了。”
林动面色凝重，能够让炎主都这般神色，想来那所谓的动静，绝对不小。
“他们似乎发现了我真身所在的位置。”炎主道。
林动面色微变，他去过炎主所在的那岩浆空间，在那里，似乎是还封印着一个什么相当可怕的东西：“是因为那里所封印的东西？你所镇压的，应该也是一处镇魔狱？”
炎主点点头，犹豫了一下，道：“我所镇压的镇魔狱，数量是三处镇魔狱中最少的，但那里，却是有着一尊极为厉害的东西…”
“什么？”
“异魔皇曾经的坐骑，大天邪魔王！”
嘶。
周围应玄子等人皆是猛吸了一口冷气，虽然他们并未听说过所谓的大天邪魔王，但却是听说过异魔皇的名头，那可是连符祖大人都唯有燃烧轮回方才成功封印的可怕存在，这等大人物，即便只是坐骑，那都是极端的可怕。
“大天邪魔王…”林动眉头紧皱，当日那天元子三人所寄身的便是一头大天邪魔，那实力惊人异常，而这大天邪魔王，必然也不是省油的灯，不然也不可能让得炎主来镇压。
“这大天邪魔王的实力，恐怕仅仅只是比魔狱最强的天王殿稍逊一些，若是让它逃了出来，那可不好对付。”炎主凝重的道。
林动点点头，一旦这大天邪魔王逃出，那魔狱的实力则是会水涨船高，于他们而言，可是极大的压力。
“需要我帮忙吗？”林动问道。
“需要，不过却不是来帮助我。”炎主没有丝毫的犹豫，现在的林动，有着帮助他们的能力与资格。那去帮谁？”林动疑惑的道。
“去帮洪荒之主！”炎主沉声道：“据我得来的消息，现在魔狱也是发现了洪荒之主的位置，当年他受创极重，现在正是恢复的关键期，我想要你去保护他。”
“魔狱也是知道洪荒之主现在的状况，若是此时洪荒之主受创的话，那将会大大的减弱我们的实力。”
洪荒之主同样是远古八主，这算是如今他们最为顶尖的战力之一，如果他出了什么意外，那堪称一种灾难性的后果。
“雷主他们呢？”
林动有些疑惑的问道，这种大事，雷主不可能会没察觉啊。
“魔狱此次有备而来，雷主之前去探测魔狱，如今正被困住，而空间之主与黑暗之主也是被魔狱盯上，暂时脱不了身，所以，我需要你的帮忙。”炎主道。
“至于生死之主，她当年受创仅次于小师妹，如今的话，恐怕还在轮回吧…”
“生死之主并未陨落？”林动有些讶异，生死祖符极为的奇特，已是化为人形，那便是慕灵珊那个小丫头，而据不死圣鲸鱼族所说，生死之主是他们的先祖，可当他们开启陵墓时，却并未发现骨骸，仅仅只有着一枚蛋，而那蛋，便是生死祖符所化，后来孵出了…慕灵珊。
“生死之主掌控生死，她可是最不容易陨落的人，她应该还活着，只不过还未觉醒罢了。”炎主道。
林动忍不住的咂了咂嘴，真不愧是掌控了生死祖符的强人啊，连想死都这么困难…
“为了全力镇压那大天邪魔王，我这具分身也会散去，所以你也得赶快过去。”炎主道，他身体之上波动越来越剧烈，显然是分身即将消散的迹象。
林动点点头，乱魔海发生这么大的动乱，他也的确是要去看看，古家以及唐心莲，慕灵珊她们，若是乱魔海被波及的话，恐怕她们所在的地方也是难逃平静。
炎主将这些话说完，目光也是看向一旁的应欢欢，犹豫了一下，道：“这次怕是不能继续守在你身旁了。”
应欢欢轻轻点头，她那纤细冰凉的玉手轻轻握住林动大手，轻声道：“他不会让我出事的。”
炎主心中轻叹了一声，旋即也就不再多说，身体之上，突然有着火焰席卷出来，而后火焰散去时，他的身体，也是凭空的消失而去。
林动知道，炎主将这分身收了回去，看来此次的危机的确不小，不然也不会逼得他将所有的力量收回去。
“掌教，我便直接动身去乱魔海了，那魔狱如今所有重心都放在乱魔海，东玄域应该暂时还算安全。”林动也没有过多的犹豫，偏头对着应玄子道。
“林动哥，带我去！我还没去过乱魔海呢！”青檀急急的道，生怕林动将她一人给丢在这里。
“还有我。”应欢欢那冰彻的眸子也是看向林动，道。
林动见状，不由得一声苦笑，略作沉吟，便是点了点头，青檀如今是黑暗祖符掌控者，而且手持黑暗圣镰，即便是遇见那种真王级别的异魔王也足以一战，而应欢欢现在的实力连他都是有些看不透，能够跟来，也是极大的战力，当然最关键的是，他不太放心将她一人的落下。
“那便我们三人去吧。”
青檀闻言顿时喜滋滋的笑起来，甚至连那俏丽的小脸，都是变得娇媚许多。
“既然如此，那你们便去吧，互相照应也好。”应玄子点点头，以三人的实力，这天地间皆可去得，也不用太过的担心。
“掌教，我们走后，道宗便暂时封锁宗门吧，以防万一，若是有问题的话，便通知于我，我如今炼化了空间祖符，能够极快的赶回来。”林动道，虽然魔狱的力量都投入到了乱魔海，但总归还是要小心一些。
“好。”
应玄子点头应道。
林动见状，也就不再多说，伸出手臂，便是将应欢欢与青檀揽入怀中，手臂勾勒处，两道小蛮腰一个柔软如蛇，一个冰凉如玉，倒是让人有些爱不释手的感觉。
不过林动此时可没心情感受怀中的温香软玉，心神一动，银色光芒便是自其体内爆发出来，周遭空间都是在此时剧烈的扭曲起来。
嗤！
而待得那银色光芒散去时，众人望去，只见得空间扭曲间，三道人影已是消失不见，唯有着狂暴的空间波动扩散而开。
应玄子望着三人消失，也是轻轻吐了一口气，目光与周通等人对视一眼，暗叹一声，这天地间，终是开始乱起来了啊。


第1259章 再至乱魔海
乱魔海，北冥海域。
一望无际的大海之上，片片云朵懒散的漂浮着，阳光透过云层，将那海面上照耀得波光粼粼，显得格外的宁静。
不过这番宁静倒并未持续太久，那天空上的空间突然扭曲起来，银光浮现间，三道身影便是凭空的闪现而出。
海风迎面而来，林动目光抬起，他望着这熟悉的大海，眼中也是掠过一抹缅怀之意，当年从东玄域逃出后，他便是来到了这里，然后背负那些想要讨回来的东西，费尽一切的苦修着。
那时候的他，在这片望不见尽头的大海上，经历着诸多凶险，但所幸最后他都活着走了出来…
他初来乱魔海时，平凡得犹如石头，一文不值，他离开乱魔海时，他的名声，已是传遍乱魔海，而现在，当他再次出现在这片大海上时，他的实力，已让得他不会再仰视任何人。
当年被追杀得疲于奔命的青年，如今，已是蜕变，翱翔九天。
“乱魔海…我林动，又回来啦！”
林动仰头，一声长啸，啸声如雷，轰隆隆的在海面之上远远的传开。
应欢欢望着那神色有些激昂的林动，微微一笑，道：“当年在离开东玄域后，你便是来了这里吗？”
林动深吸一口气，笑着点点头，谁能想到当年他在刚刚来到这里时，是如何的卑微如蝼蚁？若不是他机灵，恐怕早便是葬身在这片大海。
“很辛苦吧？”应欢欢那冰彻美目盯着林动，轻声道：“虽然那三年我一直很想你，但你也一定过得很辛苦吧。”
她能够感觉到林动对这里的那种复杂情绪，所以她也是能够猜到他在这里的种种经历，他经历着那重重生死，或许有时候，连他都不知道自己究竟能否真的活着重回东玄域…
想到这些，应欢欢贝齿忍不住的轻咬着下唇，对自己当初在见到林动回来时，还使着性子有些自责，自己固然不轻松，可他呢？他所经历的那些，谁又能知道？只不过他习惯将这些隐藏在那笑容之下，让人无从感知。
林动望着那眼中柔意流淌的应欢欢，心头也是微微被触动了一下，旋即他伸出手臂，揽着那纤细的小蛮腰，笑道：“谁让我是男人，这些东西，自然是要承受着。”
“林动哥，当初这里谁得罪你了？我们去把他们掀个底朝天！”青檀也是说道，那大眼睛中倒的确是有着一些杀意在涌动着，虽然在林动面前她一直保持着乖巧娇憨的模样，但不管怎样，她都是黑暗之殿的殿主，她从黑暗裁判所走出来，那白嫩的小手上所沾染的血腥，足以让人胆寒。
“我可不需要你来帮我找场子。”林动笑着弹了一下青檀雪白的脑门，然后目光四处扫了扫，道：“也不知道我们现在身处乱魔海的哪片海域。”
虽然他凭借着空间祖符，能够在相当短的时间中从东玄域赶到乱魔海，但毕竟这乱魔海辽阔无尽，他也没办法精准的定位。
“先往前走走看看。”辨认无果，林动也是摇摇头，然后拉着两女朝着前方飞速的掠去。
乱魔海极为的庞大，每一片海域也是拥有着惊人的面积，而他们所处的这片海域似乎有些偏僻，三人高速掠出十数分钟，却是未曾见到半道人影。
而就在林动为此皱眉间，应欢欢那白玉般纤细的玉葱指突然指向了西面的方向：“那边有人，而且还有异魔的波动。”
林动目光顺着那个方向，却是并未察觉到什么，当即心中微惊，应欢欢的实力，似乎让他越来越看不透了。
“我只是对异魔的波动特别敏感而已。”
似是察觉到林动的惊讶，应欢欢玉手轻轻握了握林动，道。
林动点点头，此时也不宜多说什么，抓着两女，银光闪烁，下一瞬，已是出现在百里之外，拥有了空间祖符，他的赶路速度，显然不是以往可比。
中途数次撕裂空间而去，如此数分钟后，林动终是停了下来，目光望向前方，只见得在那片大海上，有着十数条巨船飞速而过，而在那些大船后方，黑云滚滚而来，邪恶的魔气，令得天空都是昏沉了许多。
那些魔云速度很快的便是追赶上那些大船，而后无数异魔冲出来，犹如雨点般的冲上那些大船，而后漫天厮杀声，惨叫声，此起彼伏的响起来。
不过他们的抵抗，显然没有起到多少的效果，短短十数息间，便是有着三条大船被血洗，浓浓的血腥味道，在天空上扩散开来。
林动远远的望着这一幕，只见得在那些大船上，悬挂着不同的旗帜，这些应该是商船…
商船？不知道古家的商船在其中没？
林动心头微动，视线仔细扫去，然后他便是惊讶的见到，在那靠前方的一条巨船上，赫然悬挂着一个古字旗帜。
“这里面竟然真有古家的商船？看来这里离天风海域应该并不远。”林动眉头微皱，旋即袖袍一挥，银光闪烁，三人再度凭空消失而去。
巨大的商船之上，人头涌动，不少人脸庞上都是布满着恐惧之色，他们望着后方那些铺天盖地涌来的魔云，绝望的尖叫声，此起彼响彻着。
在那古家商船船头处，数十位古家子弟簇拥在一起，他们的面色也是有些苍白，不过比起其他人，他们毕竟算是训练有素，至少手中的武器，还被他们紧紧的握着，身体表面，也是有着元力在涌动着。
“云天大哥，我们现在怎么办？”一名古家子弟望着那些飞速接近的魔云，连说话的声音都是有些颤抖。
而此时，这些古家子弟正望着他们中心位置，那里有着一名俊朗的男子，此时的他，也是面色难看的望着眼前的困境。
“这些东西太厉害，我们挡不了。”他略作沉吟，旋即一咬牙，果断的道：“雅儿，你们分批乘小船走！”
在他身旁，有着一名身着淡绿色衣裙的少女，少女模样娇俏，她听得他的话，俏丽的脸颊上却是露出一抹苦笑：“云天大哥，他们的速度你不是没见到过，我们根本逃不掉的。”
这少女模样极为的熟悉，仔细看去，赫然便是当年林动来到这乱魔海时所遇见的第一个人，古雅，这世间之事，真是有种说不出的巧妙。
如今的古雅，再不是当年那个胆胆怯去的少女，她的容貌清秀，那股气质，即便是在这些优秀的古家子弟之中，也算是出类拔萃。
那古云天听得此话，身体也是一僵，他望着周围那些神色惶然的古家子弟，最后一声悲叹：“我们好不容易冲出武会岛，没想到还是难逃此劫。”
周围的那些古家子弟也是面露哀色，这短短一月时间的变故，几乎是让得他们有种身处梦境的感觉，谁能想到，整个乱魔海如今竟然会变成这般模样？
呜呜。
后方，刺耳的破风之声飞快的传来，一片魔云已是追过来，在那魔云之中，无数道血红暴戾的目光，将船上的所有人盯住。
“桀桀。”
魔云之中，一道魔影走出，他目露讥讽的望着这条古家的商船，然后舔了舔嘴唇，嗜血之意伴随着那滔天魔气，缓缓的席卷而出。
古云天等人见状，本就苍白的面色，更是浓郁了数分，这魔影的实力，恐怕都足以媲美转轮境的超级强者，这怎么可能是他们能够抵挡的？
“难道我们今日真是要葬身此处吗？”古云天喃喃道，一旁的古雅也是紧咬着银牙，小手握着长剑，微微的颤抖着。
周围的众人，脸庞上也满是绝望之色。
“古雅？”
而就在他们绝望等死间，一道有些讶异的声音突然从后方传来，众人闻言皆是一惊，急忙转头，只见得在那半空中，三道身影凭空闪现。
古雅望着最前方那道年轻身影，先是一怔，旋即小脸上顿时有着惊喜涌出来：“林动大哥？！”
“林动？”
古云也是一惊，目光望去，只见得那半空中青年脸庞带笑，正盯着他们，那般模样，不是林动还能是谁？
“你们怎么在这里？”林动冲着古雅二人一笑，问道，说着，他目光还看了一眼前方的那片魔云。
“林动大哥，快走！”
古雅也是在此时回过神来，小脸上的惊喜迅速的散去，急急的道：“你快走，这些魔怪实力极强，快走啊！”
林动望着古雅那焦急的模样，倒是笑了笑，不仅未走，反而落自船头，冲着古雅笑道：“我们还真是有缘呢，第一次来乱魔海第一眼就看见你，第二次过来，还是如此。”
古雅见状，顿时苦笑着摇了摇头，这林动大哥往日倒还机灵，怎么眼下反而变笨了？没看见如今这近乎绝境般的情况吗？
“唉，林动兄，真是好久不见了…算了，现在说什么也无用了，咱们能够死在一起，也算是有缘了。”古元天叹息一声，那魔云已经对这他们席卷而来，林动想跑，都来不及了。
“我可不想死。”
林动笑着摇了摇头，然后他目光看向那片魔云，道：“你们在怕这东西？”
林动说着，却是笑着伸手，手掌一握，本来还晴朗的天空，突然有着雷霆凝聚，而后直接是化为一道千丈庞大的雷霆巨手，一巴掌便是对着那魔云拍了过去。
砰砰砰！
雷手席卷而过，那片魔云直接是瞬间爆炸而开，其中无数异魔，被雷霆缠绕而上，顷刻间便是化为飞灰，烟消云散。
雷手掠过，然后便是散去，天空上的刺耳尖啸噶然而止，连带着停下来的，还有着周围那众多商船上尖叫的声音。
所有人都是呆呆的抬着头，那里，无数异魔化为灰烬飘落，唯有着那一道最为强大的魔影呆若木鸡般的悬浮着，仿佛是还未从眼前这般变故中回过神来。
当然，呆若木鸡的不止是他，就连古云天等人，脸庞上的神情，也是在此时瞬间凝固下来。
原本极为混乱的海面，在此时，鸦雀无声，那番模样，看上去显得分外的滑稽。


第1260章 万魔围岛
海面上，寂静无声，海风吹拂而来，逐渐的将先前的血腥味道吹散而去，海面上的数十条巨船上，无数人都是呆呆的望着天空，显然还未曾从这急转的变化之中回过神来。
那先前将他们杀得溃不成军的凶狠魔怪，便是这样的…全死了？
怎么会这样？
一些人有些艰难的扭着头，面面相觑着，眼中满是茫然。
“没想到还有个异魔将呢…”林动倒是没有理会那突然安静下来的海面，只是将目光望着天空上那呆若木鸡般的异魔将，轻笑道。
“林…林动大哥？”
古雅等人终是逐渐的回过神来，他们缓缓的扭过头，眼神有些茫然的盯着林动，嘴巴动了动，有点干涩的声音传出：“这是你做的吗？”
无怪他们这般的震惊，在当初武会岛上，他们见识了林动的不凡，但那也仅仅只是在年轻一辈中崭露头角而已，虽然后来在炎神殿他也是狠狠的露了一把，但那距离转轮境的超级强者，显然还是有些距离，但先前那一手，显然就算是转轮境的超级强者都是有些难以办到…
这才短短一年多时间不见，他怎么强到了这种地步？
“先把这家伙解决掉再说吧。”林动笑了笑，将目光望向天空上那异魔将，漆黑眸子中，有着一缕寒芒掠过。
天空上，那异魔将也是猛的惊醒过来，他目光骇然的直接望向古家所在的巨船，厉声喝道：“小子，你竟敢杀我异魔族的人，找死不成？！”
“呵呵，连那九王殿我都敢宰了，更何况你这般喽啰？”林动戏谑的笑道。
“你说什么？！”
那异魔将瞳孔陡然一缩，他望着那脸庞带着笑容的林动，心中却是升腾起一丝不安，目光一转，突然有着强大魔气爆发而开，而在魔气弥漫间，他的身形，却是在此时化为一道黑光，猛的暴退而去。
他竟是直接选择逃走？
海面上那些人望着这不战便退的异魔将，也是下巴猛的掉了一地，这可是能够媲美转轮境的超级强者啊，放在这乱魔海，那也是一方巨擘，怎么眼下，却是如此的不堪？
是因为那人吗？
一些目光偷偷的转移而去，望向古家巨船船头处的那一道削瘦身影，这是什么实力？尚未出手，便是将敌人震慑得远遁而去？
林动眸子漠然的望着逃窜的异魔将，却是缓缓的伸出手掌，而后轻轻握下。
嗡！
就在林动手掌握下的瞬间，所有人都是见到，那远处的空间，竟是猛的扭曲起来，然后生生的崩塌而下，而那异魔正好处于崩塌的中央，那种可怕的挤压之力，几乎是顷刻间，便是将其身体挤爆而去。
啊！
凄厉的惨叫声响彻起来，一道魔气自那崩塌空间处逃遁而出，不过还不待他掠出，一圈黑洞便是成形，旋转之间便是将其吞噬而进。
空间平复，黑洞散去，而那实力堪比转轮境超级强者的异魔将，却是在此时消失得干干净净…
海面上，再度安静无声。
林动收回目光，手掌也是缓缓的垂下，那目光望向一旁呐呐无语的古雅等人，笑道：“你们怎么在这里？”
古雅率先回过神来，目光奇特的看了林动一眼，道：“我们一路被这些魔怪追杀着，然后便是逃到了这北冥海域，还好遇见了林动大哥，不然的话…”
“这是北冥海域么…”林动微微点头，北冥海域的话，倒的确距天风海域不是很远，难怪会在这里撞见他们。
“林动兄，谢谢了。”
古云天也是清醒过来，面带感激的对着林动抱拳，那目光中有着一些敬畏之色，见识了先前那一幕，他已是明白，现在的林动，可不再是当年了。
林动笑着摆了摆手，略作沉吟，道：“我想去武会岛，你们能带我吗？”
那洪荒塔便是在武会岛上，所以想要帮助洪荒之主的话，他就必须先到武会岛去。
“武会岛？”
听得这话，古云天他们面色皆是微微一变，眼中有着一抹惧色涌出来。
“怎么了？”林动见到他们这般神色，不由得问道。
“林动大哥，现在的武会岛，可不好去，那里几乎是死地…”古雅咬着嘴唇，道：“我们便是从武会岛逃出来的。”
林动眉头微皱，看来武会岛已经遭到攻击了啊。
“林动大哥，你离开那么久，不知道如今乱魔海的局势，在两月之前，乱魔海突然出现了很多先前那种魔怪，他们的实力极端的厉害，所过之处，任何势力都是无法阻挡，而且他们手段极为的残忍，魔气过处，一物不存。”古雅道。
“我们天风海域也是受到波及，刚开始的时候，三大洞天以及我们五大家还组成了联盟，但仅仅只是与那些魔怪第一次交锋，便是损失惨重，溃不成军。”
林动微微点头，魔狱实力恐怖，光凭天风海域那些势力怎么可能与他们抗衡，溃败也是意料之中的事。
“那之后，我们便是退守到了武会岛，因为随着那些魔怪的接近，岛上的洪荒塔却是突然爆发出防护，将整个岛屿保护着，这才令得那些魔怪攻势受阻。”
“因为在武会岛能够受到保护，这消息传开，天风海域的各方势力都是蜂拥而来，全部都是挤到了岛上去．但这样一来，也是将越来越多的魔怪引了过去，现在的武会岛，几乎是被团团包围，根本进出不得，我们也是趁乱逃出来的。”
古雅说到此处，眼神微微黯淡：“梦琪姐与古嫣姐她们都还在岛上，若是那里的防护一旦被破，岛上的人，恐怕无人能够幸免。”
周围的那些古家子弟也是眼露悲色，他们古家的所有人马都是汇聚在武会岛，若是武会岛被破，古家也将会荡然无存。
林动点点头，那般防护，应该便是洪荒之主弄出来的，不过眼下他处于恢复的关键时期，显然无法尽全力，而异魔之所以围而不攻，恐怕是在等待着真正的强者来临。
“我想去一趟。”
林动看向古雅他们，笑道：“若是你们不愿再去的话，便帮我画一张海图就行。”
“林动大哥，我带你去吧！”古雅轻咬银牙，道。
“小雅。”古云天闻言，顿时一惊，那武会岛如今近乎死境，莫说进去，就算是靠近一点，那都是必死无疑。
“有林动大哥在，不会有事的！”古雅摇了摇头，眸子望向林动，那言语间，竟是对后者有着极大的信心。
林动身侧，应欢欢与青檀皆是狐疑的看古雅一眼，然后转向林动。
林动察觉到她们的目光，不由得干咳了一声，而后点点头，道：“那你便随我去武会岛吧，我受人之托，需要去那里保护一个人。”
“保护一个人？”古雅等人一愣，那武会岛算是他们五大家的地方，林动去那里保护谁？
“洪荒塔里面的那一位前辈。”林动一笑，倒并未解释得太详细。
“洪荒塔？”古雅他们一惊，旋即隐隐的有些明白过来，那洪荒塔本就格外的神秘，他们五大家都未能探测清楚，如今那从洪荒塔中散发出来的防护，难道便是那位神秘前辈的手段？
“时间紧迫，我们便先行赶去武会岛吧，云天兄，你们先行找个安全的地方避避，待得武会岛危机被解后再回来吧。”
林动也不再多说，目光看向古雅，在见到她点头后，其袖袍一挥，只见得银光散发出来，将他们三人包裹而进。
银光闪烁，空间扭曲，待得银光散去时，他们四人，竟已是凭空消失而去。
古云天望着消失的四人，也是忍不住的叹了一口气，心中升起一些希冀，林动如今的手段太过莫测，不过他若是真的能够解开武会岛的绝境，那他们古家，也是能够求得活路了…
“希望你真的能够拥有着解局的能力吧…”
有了古雅带路，接下来林动便是不再有丝毫的停歇，沿途偶尔能够遇见一些异魔横行，不过他皆是随手除去，却是未曾再有所逗留。
而以他如今的速度，短短一个时辰，便是进入了天风海域。
随着进入天风海域，林动则是察觉到这天地间的魔气波动陡然强盛了许多，甚至连海面上都是有着阴风呼啸。
林动四人再度撕裂空间掠出，数分钟后，身形逐渐的停缓下来，略显凝重的目光望向前方。
在那海面的尽头，滔天般的魔气席卷开来，天空上的魔云层层弥漫，无数尖利啸声不断的传出，那等恐怖的魔气，令得平静的海面上，不断的掀起阵阵滔天骇浪。
林动视线透过那异魔大军，只见得在那最深处，一座岛屿现入眼中，在那岛屿之外，包裹着一层深黄色的能量光罩。
目光穿越能量光罩，隐约的能够见到，在那岛屿之上，一座古老的石塔，若隐若现。
那是，洪荒塔。


第1261章 熟人
庞大的光罩，犹如一只巨碗，从天空上倒扣下来，然后将整个武会岛都是笼罩在了其中。
光罩呈现深黄色彩，那种光芒，厚重深沉，光芒流转间，给人一种无可摧毁之感。
而此时的武会岛上，放眼望去，皆是人山人海，不论是靠海的岸边还是岛屿之上，几乎全部都是人影，不过此时，这些人的脸庞，皆是有着惊惶之色。
他们目光有些惊惧的望着光罩之外，那里，天空呈现黑暗之色，一朵朵魔云漂浮着，而在那魔云之中，能够见到无数魔影穿梭，刺耳的尖啸声远远的传开，在海面上掀起惊涛骇浪。
魔气不断的压迫着深黄色的光罩，令得光罩之上散发出阵阵涟漪，看得无数人心惊肉跳，他们都明白，这光罩已是他们最后的防护，若是光罩破碎的话，那他们也将会彻底的暴露在那些异魔的獠牙之下，再无丝毫的活命机会。
而此时，在岛屿的中心，一座楼阁之中，气氛压抑，一道道人影坐在其中，每一人的面色，都是显得格外的阴沉。
“那能量光罩，似乎是在减弱…”
压抑的气氛中，突然有着人低沉的开口，那话却是让得所有人眼皮都是忍不住的跳了跳。
“各位现在说说，这究竟怎么办吧？”此次开口的，是一名头发花白的老者，他眉头紧皱着在其身后，站着两名娇躯高挑，容颜美丽的女子，赫然便是古梦琪以及古嫣。
“这还能怎么办？我们现在除了蹲在这武会岛，还有着其他的办法吗？”在他们对面一名灰袍老者冷冷的道，在这老者身后，一名男子站立，那模样同样不陌生，正是那魏家的魏真。
“那你就在这里等死吧。”古家那位老者撇了他一眼，淡淡的道。
“古统，我魏家逃不了难道你古家就能脱身不成？”那灰袍老者闻言顿时一怒，冷喝道。
楼中众人见到这两家又是吵起来，都是无奈的摇了摇头。
“你们能安静点吗？现在吵架有什么用？！”
在楼中最显眼的地方有着三道人影端坐，他们衣袍不同，但说话重量却是相当不低，因为他们正是这天风海域三大洞天之主。
而在这三大洞天之主中，也是有着一位熟人，正是那邪风洞天的洞主无轩当年还与林动有些恩怨纠葛，不过此时的他，面色也是因为眼下局面而略显阴沉。
“武会岛之外的异魔实力极强，正面抗衡，我们恐怕不是对手。”那无轩看了一眼楼阁之外，远远的还能够看见那光罩之外滔天的魔气。
“现在若是想要有所变数的话恐怕只能倚仗洪荒塔之内的那位前辈了。”天风洞天的洞主叹了一口气，道。
此话一落，楼阁中的众人便是将目光看向了那五大家族，毕竟这洪荒塔一直都是他们在掌管，以前谁都没能察觉到，在那洪荒塔中，竟然有着一位如此恐怖的存在。
见到他们的目光，五大家家主皆是苦涩的摇了摇头，那古家家主古统苦笑道：“以前的话我们还能进入洪荒塔，但却无法进入最后一层，可自从那深黄光罩出现后，我们也进不去了。”
“那位前辈，似乎不想我们打扰。”苏家家主也是点了点头，道。
“要不…你们再试试吧？”无轩想了想，他们如今也算是黔驴技穷，但又不想放弃这最后的救命稻草。
五大家家主闻言，叹息一声，只能点点头，但他们都清楚，这不过是无用之举罢了。
无轩见到那低沉的气氛，视线与两外两大洞天洞主交织了一下，皆是暗叹摇头，这次，难道真是大劫难逃了吗？
“不好了！”
而就在楼中众人皆是心中压抑时，突然有着人慌张的跑进来，急声道：“武会岛之外似乎有人闯进来，如今想要进岛！”
众人闻言，皆是一愣，现在竟然还有人敢来武会岛？这不是找死么？
众人心中讶异，然后皆是涌出楼阁，视线眺望而去，只见得在那深黄色的光罩外，仿佛是有着数道人影凌空而立，在他们周围，魔气翻腾着，那远处，已经开始有着大量异魔对着他们包围而来。
“那是…小雅？还有…林动？”古梦琪与古嫣望着光罩之外的那数道身影，她们先是一怔，旋即惊声道。
“林动？”
这名字一出，众人皆是一愣，那魏家众人面色则是有些不太好看，显然是记起了这曾经让得他们魏家吃亏的人。
无轩眉头也是微微皱了皱，对着名字显然还有着记忆。
“爷爷，那是林动，曾经帮过我们古家，你快让他们进来，不然异魔包围过来就麻烦了。”古梦琪急忙道。
她话刚落，魏家那家主却是立即道：“不行，先不说这能量光罩许出不许进，而且眼下我们可没搞清楚他们的底细，那些异魔的狡诈你们不是不知道，万一他们是异魔假装，这让得他们进了岛，那责任你们来负责吗？”
听得此话，周围众多强者也是微微点头，眼下这时候，显然还是不要横生枝折得好。
古梦琪俏脸微变，有些恼怒的望着那魏家家主，她自然是知道这家伙是在故意找机会使绊子。
那古统也是只能叹了一口气，这种时候，他们古家显然不敢做这种违众意的事，这魏家，真是令人讨厌。
古梦琪见状，只能一跺脚，娇声大喝，元力包裹着清脆的声音，传出岛外：“林动，你们快离开，小心异魔！”
岛外，林动听得古梦琪的声音，眸子一扫，便是明白了过来，而后他淡淡一笑，这些家伙，还是这么讨厌啊。
“不用了，我自己进来。”林动冲着岛内一笑，道。
“嗤。”
那魏家众人闻言不由得一声嗤笑，这光罩就算是那些异魔都攻不破，林动想要进来？真是痴人说梦。
“林动，还不速速滚开！”魏真大喝道，不过他眼中倒满是讥诮之色，他能够见到那些包围而来的异魔，这林动，恐怕是想逃都逃不掉了。
一旁的无轩眉头倒是皱着，隐隐的感觉到一点不对劲，出于谨慎，他却是并没有出言嘲讽。
“嗯？”
而就在无轩皱眉间，那光罩外，突然有着璀璨的银光闪烁起来，感受着那扭曲的空间，无轩面色猛的一变，那眼中陡然有着骇然涌出来。
在无轩那骇然的目光中，只见得他们前方的空间，也是在此时扭曲着，再接着，银光闪烁，四道身影，已是凭空浮现出来。
“这…”
周围无数人望着突兀出现的四人，瞳孔皆是一缩，眼中满是震动之色，这林动，竟然还真的进来了？
“是你让我滚开的？”林动闪现出来，漆黑的眸子，却是看向了那面色僵硬下来的魏真，淡笑道。
魏真被林动盯着，不知为何心中涌上一抹恐惧之色，急忙退到那魏家家主身后。
“林动，你要干什么？！”那魏家家主也是感到一些不妙，但依旧厉声喝道。
“恬噪！”
林动眼神一寒，屈指一弹，一股劲风暴掠而出，下一瞬，那魏家家主身体猛的一震，一口鲜血喷出，身体狼狈的倒飞而出。
周围众人见状，顿时倒吸一口冷气，那无轩眼皮也是急促的跳了跳，这魏家的家主，可是在去年的时候，晋入了转轮境啊！
“你！”那些魏家的强者见到家主被攻击，顿时一怒，不过还不待他们出口喝斥，林动一步跨出，一股恐怖的气息席卷而出，将整座岛屿都是笼罩在了其中。
那股气息所造成的恐怖威压，直接是令得原本喧哗的岛屿都是顷刻间变得安静下来。
“轮…轮回境？！”
这一次，就连那无轩都是再忍不住心中的惊骇，失声叫道。
“轮回境？！”
楼阁上，那些来自天风海域各方势力的首领身体皆是一震，目露骇然的望着那道年轻的身影，这…这竟然是一名轮回境的巅峰强者？！
古统目瞪口呆，他听古梦琪她们说起过林动，但也仅仅只是将其当做一个小辈而已，但哪料到，他所认为的小辈，如今竟是一位轮回境的巅峰强者！
一旁的古梦琪，古嫣也是玉手惊讶的掩住红唇，美目中，满是惊色。
“呵呵，原来是无轩洞主啊。”林动目光微动，望向了那无轩，轻笑一声，有些玩味的道。
被林动盯着，那无轩身体都是僵硬了下来，额头上冷汗浮现出来，一声干笑，道：“原来是林动小哥啊。”
他不是不识抬举的人，虽然如今的他也算是触及了轮回，但他却是明白这与真正的轮回境有着多么巨大的差距，若眼下林动要对他出手的话，恐怕他根本就没有反抗之力。
林动淡淡一笑，倒是没怎么理会于他，只是将目光望向了古梦琪，古嫣两女，抱拳笑道：“梦琪姑娘，古嫣姑娘，好久不见。”
林动无视了这里所有的首领，只是对着古梦琪两女打着招呼，这举动显然是有些不妥，但在他那恐怖的实力下，却是无人敢有丝毫不满，一个个低垂着头，脸庞上满是敬畏之色。
古梦琪两女对视一眼，旋即也是冲着林动嫣然一笑，笑颜动人，美目之中满是欣喜之色。
林动招呼了一下两女，然后便是转目，将视线投向岛屿中心的那座石塔之顶，这一次在那里，他察觉到了一股极端古老与强大的气息。
洪荒之主么…


第1262章 洪荒之主
楼阁之前，气氛诡异，那些来自天风海域各大势力的头头脑脑，却是在此时噤惹寒蝉，特别是那些魏家的强者，更是满脸煞白，那魏真身体都是在此时有些颤抖，目光恐惧的望着林动，他怎么都想不到，当年那个名不见经传的的小子，如今竟是达到了这种恐怖的地步。
以他现在的实力，恐怕翻手间就能将他们魏家抹除。
然而林动仅仅只是瞥了他一眼，丝毫没有找他麻烦的意思，当实力强大到某种程度时，蝼蚁的挑衅，自然再难以让其有丝毫的动怒。
“呵呵，林动小哥，真是久仰大名了。”
那古家家主古统也是在此时回过神来，他压抑着心中的翻江倒海，冲着林动露出一抹笑容，他见到林动能够对古梦琪两女打招呼，心中不仅不恼，反而有着一种大喜涌上来，反正不管怎样，只要林动对他们古家不反感那就是最好的。
身为古家的家主，他非常的清楚，一名轮回境的巅峰强者，究竟意味着什么，虽然如今这武会岛上强者云集，几乎汇聚了天风海域十九八九的强者，但这在一位轮回境的巅峰强者眼中，却是丝毫算不得什么。
“这是我爷爷，古家家主，古统。”古梦琪轻声介绍道。
“见过古家主了。”林动冲着古统抱拳一笑，对于古家，他还算有点好感，毕竟当年也算是帮了他，虽然他不认识这古统。
古统闻言，顿时连忙回以一礼，他察觉到周围那些羡慕的目光，腰杆都是挺直了一些，能够结识这般强者，于他们古家而言，可是大有荣光。
“林动，你怎么来这里了？现在武会岛可是很麻烦的。”古嫣忍不住的问道，武会岛这情况，谁还敢主动跑来？
“受人之托，来保护塔中的那一位。”林动笑着，伸出手来指了指远处的那座洪荒塔。
众人皆是一愣，洪荒塔中的那一位？难道便是制造出这能量光罩的前辈吗？
古梦琪两女也是有些茫然，但也不好多问，只是微微点头，而后那漂亮眸子，突然看向了林动身后的应欢欢与青檀，两女皆是容颜出众，一个冰彻剔透，一个俏美可爱，而且那等气质，极为的不俗。
“这是应欢欢，我们道宗的小师妹，这是舍妹青檀。”林动见到她们的目光，也是笑着互相介绍了一下。
应欢欢与青檀冲着两女微微一笑，而后应欢欢那冰蓝色的美目便是扫了林动一眼，那眼神有些莫名，仿佛是在说，你这修行之旅，艳福也挺不浅的啊。
林动察觉到她的目光，干笑一声，然后将视线转向楼阁前的这些天风海域各方头头脑脑，道：“眼下这武会岛是什么情况，你们应该也都清楚，想要保命的话，那就听我之言，可有异议？”
眼下这武会岛之外异魔数量庞大，林动想要顾得周全，这些人的力量倒是可以动用一下，有了他们对付寻常异魔，也是能够省去他一些精力。
他目光扫视而开，那强大的气息压迫也是笼罩开来，顿时让得众人心中一凛，就那连三大洞天的洞主，都不敢心生反对。
“古家主，你清算一下岛上人数，而后各方势力率领各方人马组成阵法，一旦光罩被破，那便全力相迎。”林动凌厉的目光看向众人，淡淡的道：“你们无法对付的那些异魔，我自会来收拾，但其他的寻常角色，就得交给你们了。”
“现在大家都在同一条船上，我希望都能够连同一心，谁若是暗中耍手段，那我便只能先这些异魔一步，将他清除了。”
听得这最后一句话，众人身体都是一抖，这才明白过来，眼前的人，可也不是什么善茬啊。
“林动小哥说得是，此事关乎我们性命，自然是不敢不尽力。”那天风洞天的洞主连忙说道。
“欢欢，青檀，你们留在这，帮忙看着。”
林动见状，也是微微点头，旋即偏头对着应欢欢青檀两人说道，古家虽然是五大家之一，但毕竟不是天风海域最强的势力，若是没人帮衬，恐怕其他人也不服他们。
应欢欢两女点了点头，青檀更是美目淡淡一扫，她是黑暗之殿殿主，对于这般事情再熟悉不过。
林动说完，也就不再停留，身体之上银光一闪，便是出现在那洪荒塔之前，而后直接是冲进那洪荒塔最顶层之中。
众人见到他能够顺利的进入那洪荒塔最顶层，心中也是有些惊讶，但却是不敢多说什么。
“各位，麻烦都整顿下部下吧，这一盘散沙，实在太难看了一些。”
青檀见到林动进入洪荒塔，那俏美脸颊上的娇俏则是缓缓收敛，旋即她小手一握，巨大的黑暗圣镰闪现而出，镰身划而过，然后众人便是见到一道巨大的空间裂缝自天空上浮觌′出来，当即心中都是倒吸一口冷气，显然是没想到这看上去娇俏可爱的少女，竟然也是如此的恐怖。
“大家都快一点吧，我其实性子也不是很好的，到时候万一不小心把你们扫出了岛，林动哥会责备我的。”青檀抱着黑暗圣镰，懒洋洋的道。
“是，是。”
听得青檀这暗含威胁的声音，那些天风海域各方首脑顿时冷汗直流，急忙窜出去，开始整顿着自己的人马，生怕一不小心遭惹这随着林动一离开，便是瞬间变脸的小姑奶奶。
应欢欢望着岛上闹腾的一幕，冰蓝美目中倒是没太大的波动，只是望着那洪荒塔，喃喃自语：“洪荒之主么…”
当林动进入那洪荒塔最顶层时，印入眼中的，是一片紫金海洋，紫金色的能量，雄浑而狂暴，弥漫在这空间的每一个角落。
林动望着这如海般的紫金能量，倒是微微一笑，当年他来到这洪荒塔时，仅仅只是汲取了一些从这里泄露出去的紫金能量便是让得他受益匪浅，但以如今的眼光来看，却是少了太多的吸引力。
林动视线扫过这片紫金空间，而后身形一闪，迅速的对着那中心地带而去，在那里，他感受到一股古老而强大的气息在凝聚着。
林动闪烁的身形，在数分钟便是停了下来，然后他便是目光有些惊异的望向了前方，在那里，璀璨的紫金光芒溅射而开，而那紫金光芒中心处，一道约莫千丈庞大的巨人，正端然盘坐，那些如海般的紫金能量，正顺着他的呼吸，源源不断的钻进其身体之中。
巨人身体呈现紫金颜色，他的皮肤显得极为的粗糙，但那种粗糙之下，却是有着一种无法形容的强悍。
显然，这紫金巨人，便是那远古八主之一的洪荒之主，也是这世界上肉体最为强大的人！
“洪荒之主。”
林动身体悬浮在那紫金巨人前方，与其相比，他渺小得犹如蝼蚁一般，他能够隐约的感觉到后者肉体的强大，这令得他体内的洪荒龙骨，都是忍不住的发出嗡鸣之声，仿佛是要与其一较高下。
“嗡。”
磅礴浩瀚的紫金光芒，突然自那紫金巨人体内席卷而出，而后他那紧闭的巨目，也是缓缓的睁开，两道紫金光华，直接将林动笼罩而进。
随着他目光锁定林动，一股强大的压迫，也是笼罩而来。
吼！
惊天般的龙吟之声，陡然自林动体内咆哮而出，面对着那种压迫，只见得林动体内同样是有着耀眼的紫金光芒席卷出来，而后在其身后，化为千丈庞大的紫金巨龙。
“洪荒龙骨？”望着那紫金巨龙，巨人终是开口，有些讶异的声音犹如雷鸣，令得这片空间都是索索发抖。
“在下林动，见过洪荒之主。”林动冲着洪荒之主一抱拳，朗声道。
“你便是林动吗？炎主已经与我说过了，果然是有些不同之处。”洪荒之主一笑，他的面目看起来有些憨厚，但那紫金巨目中时不时划过的凌厉之色，却又是将这种憨厚破坏得淋漓尽致。
林动微微点头，他能够感觉到这洪荒之主的强悍，不过他气息有所滞涩，显然是还未彻底恢复的缘故。
“这一次，恐怕就要麻烦你了，我的伤势正处于最关键期，只要再需要五日时间，便是能够彻底的恢复，不过我想，魔狱那些家伙，不会让我轻易恢复的，所以，这五日内，我希望你能镇守这岛屿。”洪荒之主也是一抱拳，道。
“既然受了炎主委托，我自然会竭尽全力相护。”林动点点头，道。
“原本魔狱那些家伙是察觉不到我的，但随着我伤势逐渐恢复，气息也是越来越强大，这才令得他们探测到我沉睡之地。”
洪荒之主顿了顿，突然问道：“小师妹也在吧？我感受到了她的气息。”
林动略作犹豫，还是点了点头。
洪荒之主笑了笑，目光奇特的看了林动一眼，但却并未再多说什么，但显然的，他也是知道林动与应欢欢之间的事。
“这一次，必然会有着一位王殿现身，你能应付吗？”
林动微微一笑，轻轻点头，以他现在的实力，只要来的不是那种渡过堪比渡过三次轮回劫的王殿，他倒是不会再有丝毫的惧怕。
“既然如此，那此番便多谢相护了。”
洪荒之主盯着林动，旋即颇有深意的道：“我也正想看看，小师妹看重的人，究竟有着多少的能耐…”


第1263章 大战来临
在与洪荒之主略作交谈后，林动便是未曾在洪荒塔内过多停留，如今他正是恢复的关键时期，还是让得他能安静一些为好。
林动出了洪荒塔，然后便是见到岛屿上阵阵骚动，在那岛屿各处，人影犹如蝗虫般的掠过，时不时的有着各种斥喝声传出，不过在这众多喝声下，岛屿上原本乱作一团的局势，也是开始逐渐的显得有条有序，各方人马汇聚，比起之前那种一锅乱，已是好上了许多。
而在那天空上，青檀手持黑暗圣镰，懒洋洋的望着下方的动荡，她手中的镰刀，时不时的轻轻划过，然后便是有着一道恐怖无比的劲风呼啸而下，似是不经意的落在一些磨磨蹭蹭的人马周围，大地裂开，那些人马顿时骇得冷汗直流，再不敢有小动作，急忙动身，生怕天空上那小姑奶奶万一手滑将那死神般的镰刀落到他们头上来。
“这丫头。”
林动见到这一幕，也是暗感好笑，这丫头如今手段也是颇为厉害，那么大个黑暗之殿都是被她治理得井井有条，眼下这武会岛虽然混乱，但显然也难不住她。
“林动哥！”
青檀也是发现自洪荒塔中出来的林动，俏美的脸颊上顿时浮现一抹甜甜笑容，飞快的来到后者身旁，小手中的黑暗圣镰也是被她收起来，那巧笑焉熙的模样，哪还有先前半分冷厉这倒是看得那些被她整治得服服帖帖的诸多强者目瞪口呆。
林动冲着青檀笑了笑，然后两人对着那楼阁落去，楼阁中诸多天风海域首领见状连忙迎上来。
“见到洪荒之主了？”应欢欢优雅端坐，冰蓝色的美目望着林动，轻声问道。
“嗯。”林动点点头道：“恐怕我们要将这武会岛守上五日了。”
“洪荒之主？林动小哥，这是怎么回事？”众人听得茫然，那古家家主古统倒是忍不住的问道。
“洪荒塔内那位，便是远古八主之一的洪荒之主，只要我们将这里守上五天，他便是能够出关，到时候危机自解。”林动微笑道，洪荒之主很快就会出现，也没必要再掩饰。
“远古八主…”
众人闻言一些人面露茫然，而另外一些人则是满脸的震惊，显然是听说过这在远古时期的巅峰强者，只不过没想到，这等显赫今古的强者，竟然便是在这洪荒塔之中。
“诸位武会岛如今被围，这五日时间，我们便同在一条船，还望能够齐心协力，将其渡过，否则的话或许无人能逃。”林动目光一扫众人，缓缓的道。
“是！”
众人见到他那目光，心头微凛，旋即皆是略显敬畏的应道，他们被困在这里，本就是有些绝望，但眼下林动的出现，却是给予了他们希望，为了把握住这丝希望他们必然也会倾尽全力。
林动挥挥手，众人便是撤去，而随着众人逐渐的离去，楼阁上便是只有他们三人，而林动也是轻轻松了一口气，但那眼神却是略显凝重。
“怎么了？”应欢欢见到他这般神情，问道。
“这五日，恐怕不好过。”林动道，他有着预感，这次的事情恐怕不会那么顺利，洪荒之主马上便能恢复巅峰实力，一旦让其恢复过来，那就是极大的麻烦，魔狱费尽周折，方才将雷主，黑暗之主他们尽数的缠住，这种机会，恐怕不会那么容易放弃的。
“兵来将挡水来土掩，真要说来，我们也并不惧他们，他们固然能够缠住雷主他们，但也必然会为此付出最顶尖的战力。”应欢欢道。
林动微微点头，的确，雷主他们皆是全盛状态，即便那魔狱实力滔天，但想要缠住他们，也定然必须要有魔狱最为顶尖的强者出手才行，那样的话，他们也是无法腾出太多强者来对付他们。
林动抬起头，目光望着那笼罩着岛屿的深黄光罩，旋即他深深的吐了一口气，不管怎样，既然答应了他们，那自然是要护得洪荒之主周全，若是连这都做不到的话，也未免被这些远古之主小看了。
随着林动一行人来到武会岛，这岛上原本有些绝望的气氛倒是消减了不少，不管如何，对于轮回境的巅峰强者，很多人心中都是保持着一种敬畏，在他们看来，有着这种强者镇守在此，至少希望也是能够多上一分吧？
而在希望升腾间，岛屿上的所有人马都是被调动起来，然后略作整合，皆是形成战阵，操练一日，倒也是有了一些可堪一战的气象，至少比起之前那种一滩散沙好了许多。
在岛内士气凝聚时，那岛外异魔的攻势也是愈发的凶猛，滔天般的魔气冲击在那深黄色的能量光罩上，令得上面荡起激烈的涟漪，这一幕罾不少人满身冷汗，他们都知道，一旦光罩被破，那么接下来的，必然将会是一场极为残酷的血腥拼杀。
时间，在这种紧张的等待下，却是以一种相当缓慢的姿态流过，让得岛屿上的无数强者有着一种度日如年的感觉。
他们第一次觉得，这短短五日时间，会变得如此的难熬。
时间缓慢的流逝到第四日，岛屿之上的气氛也是愈发的紧绷，因为天空上那保护着他们的能量光罩，已是仅有薄薄的一层。
在岛屿中央的一座石峰上，林动静静盘坐，他望着天空上的光罩，漆黑眸子，也是变得凌厉了许多，接着视线转向那岛屿之外的重重魔云，在那之中，不知道会有着魔狱哪一尊王殿隐藏着。
在林动目光盯着那岛屿之外魔云时，身旁有着寒气夹杂着淡淡的幽香飘来，他微微偏头，然后便是见到应欢欢来到他身旁，然后在其身旁坐下来，纤细的小蛮腰让得人有种将其揽在怀中的冲动。
“还在担心啊？”应欢欢脸颊上浮现一抹微笑，道。
林动笑着摇了摇头，他的目光只是盯着应欢欢那张俏丽的脸颊，这让得后者忍不住嗔了他一眼，小手捧着脸颊，道：“干嘛这么看着我？”
林动想了想，道：“不知道为什么，我感觉…你现在有些像以前了。”
不知何时起，她那始终笼罩着寒气的脸颊，多上了一些笑颜，虽然没有当年那般的活泼，但却是有了一些温度。
“是吗？”应欢欢贝齿咬着嘴唇，看着他：“那你是喜欢我以前那样，还是现在这样？”
“不许说都喜欢！”应欢欢补充道。
林动笑了笑，伸出手掌摸着她那冰蓝色的长发，一种冰凉的感觉顺着掌心传来，他沉默了片刻，道：“我在这乱魔海经历生死苦苦的修炼着，那时候我想…让你能够一直笑得如同以往的那般开心。”
应欢欢怔怔的看着他，展颜一笑，那一笑，颇有些一笑百媚生的感觉，旋即她轻轻的将螓首靠在林动肩头上。
林动感受着靠在肩膀上的她，这种被依靠的感觉，犹如他是她的世界一般。
滴答。
在林动心中感叹间，突然有着冰滴落在他手臂上，他急忙低头，却是见到靠在他肩上的应欢欢眼眶微红，水花凝聚着，然后化为冰滴掉落下来。
“怎么了？”林动连忙问道。
应欢欢摇摇头，那冰彻的美目看着林动，轻声道：“如果，有一天我任性的做了什么事，你不要生气好不好？”
林动一愣，有些不明白她为什么会这么说。
“我逗你玩的。”不过还不待他询问，应欢欢却是微微一笑，旋即她伸展着那娇躯，露出一个极为动人的曲线弧度，而后她看着林动，道：“好久没有为你弹琴了，我弹给你听。”
说着，她玉手轻扬，只见得蓝光浮现间，一架寒冰所铸的古筝便是凝聚而出，她仲出那对近乎完美般的纤细玉手，指尖轻落。
抑扬顿挫的悠扬琴音，缓缓的自山峰上飘荡而开，最后荡漾在这岛屿上空，在这琴音之下，岛屿中原本紧绷的气氛也是悄然的松缓。
无数强者视线望向琴音传来处，只见得石峰之上，那美丽的女子素手轻扬，冰蓝色的长发轻轻飘舞，那一刻，犹如画卷之中绝美的一幕，令人忍不住的陶醉在其中。
林动望着此时的应欢欢，后者的脸颊上，有着动人的笑颜，只不过不知为何，却是令得他心中微微抽搐了一下。
“呼。”
他深深的吸了一口气，双掌却是缓缓的紧握着，若是保护不了想要保护的东西，那么这么辛苦的修炼，又有什么意义呢？
嗡嗡！
天空上，突然有着异声传来，打破了令人沉醉的琴音，无数道目光猛的转移而上，然后他们便是身体紧绷的见到，那天空上的能量光罩，竟然是在此时缓缓的蔓延出一道道裂纹。
林动也是站起身来，那漆黑眸子中，凌厉之色开始凝聚，那股恐怖的气息，也是在此时猛的荡漾开来。
终于是要来了啊…


第1264章 再遇七王殿
天空中，巨大的深黄色能量光罩上，一道道裂纹缓慢的蔓延出来，那种细微的咔嚓之声，令得整座武会岛都是在此时变得安静下来，无数人额头上皆是有着冷汗滑落而下。
林动站起，他眼神冰冷的望着这即将破碎的能量光罩，旋即深吸一口气，袖袍一挥，只见得一道光华掠出，而后在半空中化为一道光殿，正是玄天殿。
玄天殿一出现，便是爆发出嗡鸣之声，只见得铺天盖地的神物呼啸而出，而后对着岛屿之上那些无数强者飞去。
这些神物之上，覆盖着一层薄薄的温和白芒，这是祖石的力量，能够加深对异魔的伤害。
“诸位，若是此番危难渡过，这些神物，便尽属你们所有。”林动目光扫视着整座岛屿，朗声响彻而起。
哗。
他这话一落，顿时让得岛屿中爆发出惊天般的哗然声，旋即无数强者眼神火热，急忙将那掠来的神物抓住，眼中的炽热犹如是要将其融化一般。
这些神物虽然在林动的眼中不算什么，但对于他们而言，却是真正的宝贝，平日里费尽心机都难以得手，但眼下，却是被林动如此大量的赐予。
“我等必死守武会岛！”
手握神物，那无数强者心中底气仿佛都是变强了许多，目光狂热的望着林动，大喝之声，此起彼伏的响彻而起。
林动见到众人高昂的士气，也是微微点头，旋即他看向应欢欢道：“待会若有强敌，我会先行阻拦，你与青檀拦截其他的异魔强者。”
虽然知道现在应欢欢实力也是极为不弱，但那种力量，她能够少动用就尽量少用吧。
应欢欢螓首轻点她知道林动心中所想，自然不会有异议。
天空上，光罩颤巍巍的颤抖着，而岛屿的半空，也是有着无数强者悬浮着，强横的元力自他们体内涌出来，眼中原本的惧色，倒是在林动那平静的面色下逐渐的消退下去眼中取而代之的，是许些的凶芒，他们被这些异魔追杀得犹如丧家之犬般的逃到这里，心中的怒火，早已达到顶点。
嗡嗡。
光罩抖动的速度越来越快，而那裂纹也是蔓延到了极限，而后，一道细微的咔嚓之声响彻而起整片光罩，终是在那滔天魔气的冲击下，彻彻底底的崩溃而开！
就在光罩崩溃的霎那，这片天地，瞬间阴沉下来，那在岛外盘踞许久的滔天魔气也是在此时暴露出狰狞的本相，只听得凄厉尖啸声响彻，那魔云之中无数异魔犹如蝗虫般的掠出，对着武会岛冲击而来。
林动面色漠然的望着那等遮天蔽日的攻势，手掌一握，只见得天空雷云汇聚，无数道雷霆奔腾而下，狠狠的冲进那些魔云之中。
砰砰！
低沉的雷爆声响彻而起然后众人便是见到无数异魔惨叫出声，身体焦黑的带起道道黑烟不断的从天空上坠落而下。
“杀！”
众人见林动大展神威，精神也是一振，而后那惊天般的嘶吼声便是响彻而起，接着元力疯狂涌动，那如海般的攻势，也是席卷向那最前方的异魔。
轰隆隆！
双方正面交接，元力魔气席卷。
没有任何的预热，双方几乎是在一碰面下，便是直接进入红眼的拼杀之中，残肢断臂四处飞舞，那冲来的异魔，也是在瞬间被无数道元力冲击而中，当场爆成一团虚无。
不过好在如今的岛屿已算是固若金汤，那些异魔的冲击，依旧是被拦截在岛屿之外，并未能够冲进来。
林动望着这惨烈的局势，也是深吸一口气，然后将视线投向那魔云最深处，那里，开始有着一些极端恐怖的魔气涌出来。
应欢欢与青檀也是出现在林动身后，眸子望着魔云深处，与那些寻常异魔相比，接下来露面的这些，才是真正的棘手角色。
魔云滚动，不知何时，那些异魔的冲击突然间变缓了一些，紧接着，众人便是见到，恐怖的魔气在岛屿之外的上空凝聚成漩涡，漩涡旋转之间，仿佛连空间都是被撕裂而开。
魔气漩涡之中，隐隐的，似是有着沙沙的脚步声传来，数道黑影，仿佛由远至近，最后缓缓的自那魔气漩涡之中踏出。
而当他们出现在这片天地间时，所有人都是能够感觉到，弥漫天地的元力，都是在此时犹如受到刺激一般的沸腾了起来。
“呵呵，林动，还真是哪里都少不了你啊。”六道魔影脚踏魔气，那当先一人阴冷的双目却是看向了岛屿上空的林动，淡漠的笑声，缓缓的传出。
林动的视线，也是在第一时间望向那现身的六道魔影，那为首一人，模样极为的熟悉，赫然便是曾在北玄域与林动交过手的七王殿！
“我们还真是有些冤家路窄呢。”
林动望着这七王殿，也是淡淡一笑，当日他在这七王殿手中颇为的狼狈，虽说后来凭借着“太上感应诀”将其所伤，但最后也幸好雷主及时赶来，这才将这七王殿惊退，没想到，这一次前来对付洪荒之主的，也是这个家伙。
林动视线自七王殿身后扫过，那五道魔影同样是有着滔天魔气，那种程度的魔气，虽然比不上七王殿，但也堪比渡过一次轮回劫的巅峰强者。
这五人，皆是属于真王级别的异魔王！
一尊王殿，五尊真王。
这阵容，即便是林动，都是看得眼神有些凝聚，这魔狱，果然是肯下大手笔啊。
七王殿冷笑的盯着林动，目露讥讽之色：“上一次雷主救了你，这一次，你还想要谁来救你？那些远古之主，现在恐怕谁都脱不了身。”
林动一笑，悠悠的道：“自救。”
“就凭你？”七王殿嘲弄的道。
林动轻轻点头，那神色竟是显得格外的认真，他盯着七王殿，缓缓的道：“这一次，我会杀了你。”
七王殿舔了舔嘴唇，脸庞上有着残忍的嗜血之意流露出来，他盯着林动，眼神也是格外的森然：“我也很想说这句话呢。”
林动笑着，漆黑眸子盯着七王殿，浩瀚元力，却是逐渐的弥漫开来，引得风雷齐动，声势骇人。
“吞噬天尸能够对付一尊真王。”林动袖袍一挥，吞噬天尸闪现而出，凭借它的力量，要对付一尊真王并不算太难。
“我也能对付一尊。”青檀道，凭借着黑暗祖符以及黑暗圣镰，虽然她尚未晋入轮回境，但也是具备对付一尊真王的力量。
“其余三尊，交给我吧。”应欢欢微微一笑，她看着有些犹豫的林动，道：“不用担心，三尊真王而已。”
林动抿了抿嘴，最终只能点点头，眼下异魔来势汹汹，他仅仅只能拦住那七王殿，这五尊真王若是放任而去的话，恐怕下方的那些防御也将会迅速的被摧毁，到时候异魔冲进岛内，难免会干扰到洪荒之“那便…动手吧！”
林动眼神陡然冰寒下来，上一次在这七王殿手中吃的亏，这一次，那就要彻彻底底的找回来！
林动一步跨出，手掌一握，银色雷霆瞬间自其手中爆发而起，而后咆哮之间化为一道雷龙，张牙舞爪的对着那七王殿冲击而去。
“雕虫小技。”
七王殿淡漠一笑，屈指一弹，魔气喷薄而出，化为巨大的黑色魔链，瞬间便是缠绕上雷龙，旋即猛的一绞，直接是将雷龙绞爆而去。
“这一次，就让本殿宰了你！”
七王殿身形猛的暴掠而出，大手猛的一握，一道魔手便是从天而降，狠狠的对着林动爆轰而去。
林动身形掠上九天，一声长啸，璀璨的紫金光芒爆发而开，整片天空，都是被渲染成绚丽的紫金色彩，而后足足万道紫金龙纹，缠绕在其周身。
轰！
林动五指紧握，一拳轰出，万龙奔腾，惊天动地般的龙吟声响彻，那等拳风，犹如是穿透了空间，一拳便是将那魔手轰爆而去。
咚！
天空上，可怕的能量波动席卷而开，将那空间都是震得扭曲不堪起来。
“我们也动手。
应欢欢见到林动将七王殿牵扯而去，那冰蓝美目中，寒芒也是陡然掠过，旋即其娇躯闪掠而出，纤细五指探出，只见得寒气弥漫，瞬间化为一道万丈庞大的玄冰囚牢，直接是将三尊真王笼罩而进，然后她掠进囚牢，可怕的寒气弥漫开来，直接对着那三尊真王展开了极端凌厉的攻势。
青檀也是动手，手中黑暗圣镰挥舞，将那一尊真王死死缠住，而面对着黑暗圣镰的凌厉，那尊真王也是颇为忌惮，两人一时间，皆是陷入对恃之中。
吞噬天尸最为干脆，悍不畏死的冲出去，任由那尊真王攻势凶猛，他却是凭借着那拥有着吞噬之性的肉体，生生承受下来。
这武会岛天空地面，都是在此时陷入了激战之中，那等惨烈之气，冲上云霄，即便是千里之外，依旧是能够清晰可闻。
在那最为上层的天空，林动与七王殿对恃，彼此眼神皆是弥漫着狠辣，他们知道，不论下方多么的惨烈，他们这里，方才是最为重要的战场，只要他们能够解决掉对手，那么便是能够完成他们所接到的重要任务！
这一战，必将惨烈。


第1265章 血战
九天之上，浩瀚元力席卷而开，犹如海洋，无始无尽。
林动脚踏虚空，周身风雷相随，漆黑眸子噙着惊人的凌厉，紧紧的盯着那前方的七王殿，此时的后者，身后也是有着滔天魔气涌来。
七王殿感知惊人，一看林动这般阵仗便是察觉到了他如今实力，当即眼中掠过一抹惊异，这小子修炼速度怎么如此之快，短短数月时间不见而已，竟然便从转轮境晋入了轮回。
“我道是说你怎么有了与本殿叫板的勇气，原来是晋入轮回境。”七王殿眼中惊讶一闪而过，旋即便是冷笑一声，道。
林动闻言却是咧嘴一笑，只是那笑容显得有着几分狰狞，他并没有任何的答话，只是双手缓缓紧握，璀璨的紫金光芒从其体内爆发而出，他的身躯，仿佛都是在此时壮硕了一圈。
吼！
响彻天地般的龙吟传荡开来，只见得整整万道紫金龙纹在其周身呼啸而动，万龙奔腾间，那种可怕的力量，仿佛要撼动这天与地。
随着林动真正的晋入轮回境，青天化龙诀也终是真正的大成，那种强大的程度，就算是青雉在这里，都是得唯有点头赞叹。
林动的身体缓缓的膨胀，只见得他皮肤也是开始变成紫金颜色，隐约的，仿佛有着紫金色的龙鳞生长出来，现在的他，这具肉体倒是与洪荒之主有些接近。
林动大手探出，猛然一握，万道紫金龙纹在其掌心凝聚，而后紫金光芒大作，便是化为一柄十数丈巨大的紫金龙枪，龙枪之上布满着鳞片，那枪尖处，更是巨龙张嘴，锋利的龙牙探出来，其上寒光流溢，那等凌厉，足以撕裂空间。
林动脚踏虚空，手握紫金龙枪，他感受着体内那近乎无穷无尽的力量，忍不住的仰天长啸，啸声之中，惊雷滚滚，这是他在晋入轮回境之后第一次将力量施展到极限，那种充沛之感，犹如一拳之下，连这天空都是能够轰裂一般！
林动泛着紫金颜色的双目中，战意升腾，他炽热的盯着七王殿，现在的他，渴求着一场酣畅淋漓的战斗！
“哼。”
七王殿望着那战意升腾的林动，嘴中一声冷哼，只见得他双手缓缓的垂下，掌心魔气涌动，那对手臂竟是逐渐的伸长了一些，一对手臂变得漆黑如精铁，指甲长约三尺，犹如锋利的长剑，指甲轻轻摩擦间，刺耳的吱吱声传出。
“想跟本殿斗，再渡两次轮回劫吧！”
七王殿眼中寒芒一闪，身形一动，竟是如同鬼魅般的出现在林动后方，那锋利的指甲快若闪电般的撕裂空间，以一种无法形容的速度狠辣的刺向林动后背心。
“嗤！”
泛着魔气的凌厉指甲狠狠的洞穿林动的身体，但却并未有着丝毫的鲜血浮现，那道身影，反而是逐渐的变得淡化而去。
攻势落空，七王殿瞳孔也是微微一缩，旋即猛的转身，锋利指甲犹如利剑般暴刺而出。
叮！
紫金色的龙枪携带着磅礴力量而来，重重的与那锋利指甲硬憾在一起，肉眼可见的能量波动席卷而开，直接是将这片空间震得扭曲下来。
林动手持龙枪，皮肤表面的龙鳞闪烁着耀眼的光泽，他眼神冰冷的盯着七王殿，手臂一抖，只见得龙枪之上顿时爆发出璀璨光芒，龙枪震动，便是化为无数道枪影，疯狂的对着那七王殿爆轰而去。
那些枪影，每一道都是拥有着轰杀一名轮回境强者的可怕力量。
七王殿见到那漫天而来的狂暴枪影，身体之上魔气暴涌，而后那狰狞的魔皇甲便是显露而出。
叮叮！
狂暴枪影狠狠的落在那魔皇甲之上，七王殿的身形则是被震得倒飞而去，魔皇甲上，也是有着点点痕迹浮现出来。
“力量倒的确是长进了不少。”
七王殿望着魔皇甲上的那些痕迹，眉头微微一皱，上一次相遇，林动凭借着最大杀招方才对魔皇甲造成一些伤害，但没想到眼下，他的攻击，竟是能够穿透魔皇甲，令得他体内有些受震，虽然这种伤势于他而言根本不算什么。
“唰！”
林动却是没有任何答话的迹象，身体之上银光闪烁，直接是出现在七王殿头顶上空，手中龙枪狠狠的对着其天灵盖暴刺而下。
叮！
那覆盖着魔皇甲的漆黑手臂猛的探上，竟是犹如盾牌般将那龙枪生生抵挡而下，不过那上面涌动的可怕力量也是让得他的身体下沉了一些。
“哼！”
七王殿眼中寒芒一闪，那犹如魔剑般的锋利指甲划起刁钻弧度，也是趁此在林动胸前陡然划过。
嗤！
指甲划在龙鳞之上，溅射出一些火花，也是在林动胸前留下数道长长的血痕，鲜血则是顺着血痕流了下来。
胸膛处传来的痛感，却并未让得林动毫的变色，那漆黑眸子中，反而是有着嗜血的凶芒闪烁旋即他手掌一滑，竟是顺着龙抢掠下，两条手臂之上，奇异光芒闪烁，竟是尽数的化为液体之状。
液体手臂之中，仿佛是有着古老的符文浮沉，一种玄妙霊的力量，弥漫开来。
那七王殿见到林动这弥漫着祖符力量的液体双臂，目光却是一闪，身体一颤，竟是化为一道黑烟暴退而去，他知道祖符力量的厉害，自然也不会蠢得让林动顺利的击中他。
“躲得了吗？！”
然而林动见到他躲避，唇角却是勾起一抹冷笑，只见得其液体手臂上，银光陡然闪烁，那双臂竟是犹如伸进了空间之中，诡异的消失不见。
七王殿见到林动双臂凭空消失，瞳孔却是一缩，然而还不待他反应过来，只见得身前空间诡异裂开，两道液体之掌闪电般的窜出来，然后重重的印在其胸膛之上。
嘭！
低沉之声响彻而起，七王殿身体顿时倒飞而出，身体之上魔气都是爆发出阵阵紊乱，在其胸膛处的魔皇甲上，更是有着两道半指深的掌印。
“空间祖符？”七王殿遭受这般诡异攻击，面色终是一变，厉声道。
“恭喜你，答对了！”
林动眼中寒芒闪烁，下手却是丝毫不留情，只见得其祖符之掌凶悍轰出，直接是自空间中穿过，然后又是自七王殿周身的空间中诡异的窜出来，那般攻势，神出鬼没，令人防不胜防。
而面对着这般诡异攻势，即便是七王殿一时间都是有些措手不及，唯有着凭借着魔皇甲强大的防御力，将那些来自四面八方的攻势阻拦下来。
下方的岛屿，同样是在此时陷入激烈的厮杀中，不过不少人都是在偷偷的注意着最上一层的战圈，因为他们都很明白，那里的两人，才决定着这场攻防战的胜负。
而当他们在见到林动将七王殿逼入下风时，他们在感到惊喜的同时，心头也是暗感震惊，他们自然是能够察觉到那七王殿的强者，那般实力绝对不是寻常轮回境强者可以媲美，但没想到，林动依旧是有着与其抗衡的能力…
谁能想到，当年那仅仅只是代替着古家出战的年轻人，短短数年后，却是变态到了这般地步。
嘭！
又是一记祖符之拳穿透空间，狠狠的落在那七王殿身体之上，可怕的力量，直接是将其震得倒退数十步，那脚步落处，仿佛连空间都是有些颤抖。
七王殿脚掌重重一跺，强行将身体稳下来，他望着魔皇甲上的那些拳印，原本脸庞上的怒火，却是在此时一点点的收敛下去。
“不愧是拥有着三道祖符的人…”
七王殿抬头，他深吸一口气，那魔瞳逐渐的变得漠然，但那在漠然之下，林动却是能够感觉到一种暴戾到极致的情绪在涌动，显然，林动这一番狂轰猛打，让得这七王殿真正的动怒了。
“虽然很讨厌你这家伙，不过…现在，可真没时间陪你这种杂鱼玩啊！”
七王殿眼中的暴戾终于是涌了出来，而后他猛的一步跨出，可怕的魔气疯狂的席卷而开，这片天地，都是在此时变得索索发抖起来。
七王殿眼神阴厉的盯着林动，双手猛的变幻出道道诡异印法，旋即其一口黑血自其嘴中喷出。
黑血蠕动着，然后化为一道黑色骷髅符印，那骷髅之中，有着一种令人心惊胆寒的邪恶波动扩散出来，那种邪恶，甚至是超越了七王殿本身。
吼！
而随着这股邪恶至极的波动传出，那下方无数的异魔突然仰天嘶吼起来，那吼声之中，竟是有着近乎狂热的味道，一些异魔，更是直接跪拜而下，对着那骷髅符印朝拜而下。
林动见到这一幕，眼神也是微微一变。
七王殿冲着林动狰狞一笑，道：“原本是想用来对付洪荒之主的，不过你既然这么想死，那就先将你解决掉吧。”
七王殿双手结印，将那骷髅符印握在手中，而后，他的身体也是缓缓的跪拜而下，一股股粘稠的魔气自其体内暴涌而出，最后在其头顶上空，隐隐的，化为一道千丈庞大的模糊黑影。
随着那道黑影的出现，这片天地的元力，竟然都是在此时溃逃而去。
粘稠魔气弥漫，一道嘶哑的声音，则是在此时，缓缓的自那七王殿嘴中传出。
“魔皇，降临！”


第1266章 魔皇虚影
天地突然间昏暗下来，甚至连天空上的烈日，仿佛都是在此时颤抖起来，恐惧的躲入云彩之后。
无穷无尽般的魔气在天空上席卷而来，这片天地，仿佛都是在那一道巨大的魔影之下颤粟起来，海面之上，飞鱼绝迹，甚至连海风都是悄然的沉寂而去。
岛屿之上的惨烈拼杀，也是在这一刻诡异的凝固下来，无数道目光骇然抬起，他们望着那道魔影，一种无法形容的恐惧，自那心底深处犹如蔓藤般的缠绕而出，令得他们的面色因为恐惧而变得煞白。
那道魔影，仿佛是来自异世界的皇，它以一种蛮横的姿态，降临而来，这片世界，无人能够抵挡。
“异魔…皇？”林动也是震动的望着那道魔影，难道这便是那位只有传说中那位符祖方才能够抗衡的异魔王吗？
“这只是一道异魔皇虚影而已，那异魔皇当年被我的主人打出位面裂缝，连同裂缝一同封印，凭这家伙怎么可能召唤得出来，那道骷髅符印之内，只是有着一缕异魔皇的发丝！”岩的声音，在此时响起。
“异魔皇虚影？”
林动先是一怔，旋即又是在心中倒吸一口凉气，仅仅只是一缕发丝，竟然便是能够拥有着这般气势，那若是异魔皇真身的话，那究竟该有着多么的恐怖？
“桀桀，林动，这下子，就是你的死期了！”在那魔皇虚影之下，七王殿眼神狰狞，他冲着林动森然一笑，旋即手印猛的一变。
轰！
就在其手印变幻间，只见得那那道魔皇虚影，缓缓的抬手，而后对着林动轻拍而下。
魔手拍来，没有任何惊人的能量波动，但林动却是感觉到了一股致命的危险，他刚欲闪避，便是骇然的发现，周遭犹如被禁锢一般，根本动弹不得。
“该死的！”
林动暗骂一声，旋即其原本漆黑的双瞳，突然是在此时化为璀璨的银色，银芒也是疯狂的从其体内暴涌而出，空间之力，运转到极致。
呼。
魔手轻拍而过，但在那魔手过处，仿佛连天地元力都是被拍成了虚无，再接着，魔手便是在七王殿狰狞的目光中，落至林动身躯之上。
魔手飘过，那林动所立处，空无一人，这看得下方无数强者面色一变，难道林动也被拍成虚无了？
“空间祖符么，倒的确是有点麻烦呢。”
七王殿见状，眉头却是一皱，旋即阴寒的望着远处，那里空间扭曲着，林动的身形也是诡异的浮现出来，不过此时的后者，额头上也是有着一些冷汗浮现出来，先前若是再晚上一瞬，恐怕就得被拍中了。
虽然被拍中不至于将其轰杀，但也绝对会出现相当重的伤势。
“这次可不会让你再逃掉了。”七王殿冷笑，手印变幻，只见得那魔皇虚影再度抬手，那黑暗的手掌仿佛能够包揽日月，遮天蔽日的对着林动覆盖而去。
吼！
林动见状，眼中也是有着凶光爆发而起，旋即他仰天长啸，啸声如龙吟，无数道紫金龙纹铺天盖地的席卷而出。
紫金龙纹不断的凝聚，林动显然也是将体内元力催动到了极致，眨眼间，那紫金龙纹，便是达到了一万五千道恐怖的数量。
一万五千道龙纹缠绕在林动周身，万龙齐啸，声势惊人。
轰！
林动面色冰寒，手中紫金龙枪直接脱手飞出，一万五千道紫金龙纹随之飞去，最后直接是对着那魔皇虚影大手冲去。
“哈哈，在吾皇面前，你竟还敢反抗，不自量力的东西！”
七王殿见状，却是仰天大笑，在他大笑中，那魔皇虚影大手已是与那万道紫金龙纹相触，撞击中，没有任何的巨声传出，那大手面对着阻拦，仅仅只是顿了顿，然后众人便是见到那无数道紫金龙纹，直接是在顷刻间化为虚无消散而去。
这魔皇虚影，竟然恐怖到了这般地步！
岛屿中，古家所在的战圈中，古梦琪，古嫣她们见到这一幕，俏脸也是有些骇然，旋即她们望向那道削瘦的身影，玉手紧握。
天空上，林动见到如此强势的攻击都是毫无效果，那神色也是变得极端凝重起来，下一瞬，他深吸一口气，修长的手掌伸出，雷光顿时闪烁起来。
雷光流溢而开，充满着力量之感的雷霆之弓，便是闪现而出，而后林动手指拉动，一只黑箭，悄然的凝聚出来。
那七王殿见到林动终于是祭出这当初重创他的雷弓黑箭，眼瞳也是微微一缩，但旋即便是一声冷笑，他可不信，林动能够攻破魔皇虚影！
“本殿让你明白，什么才叫做真正的绝望与恐惧！”七王殿森然的盯着林动，眼神狰狞得令人心寒。
林动却未曾理会他的目光，弓成满月，黑箭微微的颤抖着，而后有着混沌之光自那神秘之处涌来，覆盖上黑色箭支。
他周身原本浩瀚的元力，都是在此刻突然的消失，此时的他，犹如普通之人。
不过外表虽然普通，但即便是岛屿之上的众人，都是能够隐隐的感觉到那种平静朴实之下所涌动的惊涛骇浪。
那种力量，必定连天地都为之颤抖。
所有人的手掌都是猛的紧握起来，谁都能够感觉到林动这般攻势的厉害，但这样，真的能够对付那恐怖得无法形容的魔皇虚影吗？
若是林动落败，岛屿之上，无数强者士气将会尽数低落，再也难以抵御异魔攻势，而这里，也将会真正的血流成河！
呼。
一口白气，轻轻的从林动嘴中吐出，那黑色的眸子中，一抹精光，陡然掠过，那道光芒，犹如烈日般的耀眼夺目。
林动紧扣着弓弦的手指，猛的松开。
咻！
黑箭暴掠而出，依旧是没有带起任何惊人的能量波动，但其速度却是快得无法形容，众人仅仅只能见到黑芒掠过，再接着，便是看见，那只黑箭，已是重重的射在了那魔皇虚影大手之上。
天地仿佛都是在此时凝固。
黑箭与大手僵持对恃，不过这一次，那魔皇大手却并未再能如之前那般摧枯拉朽，黑箭激烈的颤抖着，仿佛是有着一股看不见的可怕力量，将那来自魔皇大手之上的可怕力量抵消而下。
没有绚丽的爆炸，也没有可怕的能量冲击，但任谁都是能够感觉到，那种对恃之下的恐怖。
七王殿也是死死的盯着那对恃之局，他猛的一咬牙，眼中掠过一抹凶狠之色，旋即他手掌重重一拍胸膛，一口粘稠得甚至有些黑色结晶的鲜血被其喷出，而后这些黑色鲜血，便是掠进那魔皇虚影之中。
随着那些黑色精血入体，只见得那魔皇虚影仿佛是微微变得清晰了一点，那模糊的眼瞳处，似是有着微弱的红芒浮现。
“给我破！”
七王殿面庞狰狞，厉声大喝。
轰！
随着他喝声一落，只见得那魔皇大手，突然猛然握下，那支黑箭，也是在此时突然裂开一道道细微的裂纹，最后终于是在魔皇大手之下，彻彻底底的蹦碎而去。
就在那黑箭蹦碎时，下方无数人面色都是煞白下来，最终，还是输了吗？
噗嗤。
黑箭被毁，林动也是猛的一口鲜血喷出来，身体踉跄的退后了十数步，显然也是受到了牵连。
“哈哈！”
七王殿见状，顿时尖啸大笑起来，长发披散下来显得格外的可怖，他望着林动，狞笑道：“你最强大的攻势都是失败了，你还能如何？！”
然而，面对着狰狞大笑的七王殿，林动却只是安静的抹去嘴角的血迹，他缓缓抬头，看着七王殿，却是笑了笑：“那种力量，还不够啊…”
笑声落下，他却是再度缓缓的拉开雷弓，鲜血顺着贡献溢流而下，最后黑光凝聚间，那黑箭的颜色，都是变得暗沉了一些。
“还不死心吗？看来你也没招使了呢。”七王殿见到林动再度拉弓，却是讥讽一笑。
林动并未理会他的嘲讽，双目再度缓缓闭上，心神沉淀进入心底最深处，那种感应…他需要得更清楚一些…
那种神秘的力量…还不够呢。
“那种力量，出来吧…”
仿佛是最为深沉的喃喃声音，在林动心中悄然的传开，他的意识，却是在此时，悄悄的飘散在天地之间。
太上感应，究竟是在哪里呢…
意识散逸，不知不觉，林动的意识变得模糊起来，但在意识模糊间，他的心中，却是清明如镜，一种莫名的感悟，悄然的蔓延开来。
意识，突然的闯入了一片混沌之中，那种感觉，就犹如胎儿在母体之中，让人沉醉得不愿苏醒。
不过，那自身体之中传递而来的强烈意愿，却是生生的令得这股意识脱离了那种沉醉，而后，仿佛是有着一道声音，从他的意识中传出，最后进入那混沌之中。
“把你的力量，给我！”
混沌之中，仿佛是有着低沉的轰鸣声响起，林动的意识，则是在轰鸣中迅速消退…
紧闭的双目，在此时陡然睁开，黑色的眸子深处，掠过一抹混沌之光。
仿佛是有着一缕无人能够看见的混沌之光照射而来，落到了黑箭之上，而后黑箭颜色渐变，最后化为混沌之箭。
林动的唇角，在此时有着一抹笑容缓缓的勾起，然后，他冲着七王殿微微一笑，那拉着弓弦的手指猛的松开。
混沌之箭，携带着那神秘的力量，在那无数道紧张得近乎窒息般的目光注视下，再度划过天际。


第1267章 混沌之箭
一道混沌光虹，自天际一闪而过，那霎那间的光华，犹如世界初开时所诞生的光芒，有着一种极致般的神秘。
而在那道包裹着混沌光虹的长箭暴掠而出时，那七王殿的瞳孔也是陡然一缩，下一瞬，他眼中也是有着暴戾之色涌出来。
“给本殿死吧！”
七王殿面庞狰狞，只见得他双手相合，划出道道诡异印法，嘶喝之声，响彻而起：“灭世之手！”
天空上，那道魔皇虚影模糊的眼中，红光悄然的浓郁，而后那魔皇大手也是在此时，仿佛变得凝实一些，那掌心中，犹如有着邪恶到极点的魔气在涌动，那种魔气，只要是稍稍沾染，便足以将一名轮回境的强者彻底的抹杀。
“唰。”
魔皇大手猛的探出，然后天空蹦碎，那只邪恶到极点的大手，在下一个瞬间，已是在那无数道近乎屏息般的紧张目光中，重重的与那道并不起眼的混沌长箭，碰撞在一起。
嗡！
奇异的嗡鸣之声，在此时自天空之上传荡开来，一圈圈肉眼可见的能量涟漪源源不断的扩散出来，最后近乎囊括了这万丈天空。
而在涟漪的中心，魔皇大手与长箭不断的僵持着，不过在这种僵持中，那道混沌长箭，竟是纹丝不动，甚至连箭身都未曾有过丝毫的颤抖。
“吼！”
七王殿望着这一幕，眼中狰狞不由得更甚，一道嘶吼声，自其喉咙间传出，而后他眼中凶光闪烁，他那一条手臂上，竟是出现了一道道密密麻麻的裂纹，黑色鲜血，源源不断的从裂纹中渗透出来，转眼便是弥漫了整条手臂。
“桀桀，跟本殿玩，也不怕被玩死掉？！”
七王殿面庞扭曲，他冲着林动森然一笑，下一刻，他那布满着裂纹的手臂突然凭空的爆炸开来，黑色鲜血伴随着一些黑色结晶暴掠而出，最后冲进那与混沌之箭对恃的魔皇大手中。
而随着这些鲜血的灌注，那魔皇大手竟是变得清晰许多，甚至连大手上的一些诡异符文，都是能够隐约的可见。
下方岛屿上的无数强者见到这一幕，面色顿时煞白下来，这七王殿竟然如此之狠，为了搏杀林动，连一条手臂都是自爆了去…
这一次，那凝聚了林动最强力量的长箭，莫非又要溃败了？
古梦琪等人也是紧握着手，眼中满是紧张与担忧之色，若是林动再次溃败，那岛屿上的士气就真对骤降了。
“林动，加油啊…”古嫣捏着小拳头，喃喃道。
“吱吱。”
魔皇大手缓缓的握下，那混沌长箭也是微微颤抖了一下，阴影悄然的笼罩下来，犹如那异域的魔皇，伸出毁灭之手。
然而，面对着这般变故，林动的面色，却反而是逐渐的平静下来，下一刻，他深深的吸了一口气，轻声喃喃道：“太上，破魔！”
嗡！
混沌长箭，在此时猛然一颤，而后混沌之光竟是爆发而开，而在那种光芒的爆发下，正缓缓紧握下来的魔皇大手，却是被生生的阻拦下来，再也握下不得。
魔皇大手被阻，那七王殿的面色，也是在此时猛的一变，那望着那在魔皇大手中安稳如山般的混沌长箭，一抹不安之色，突然的涌上心这一支长箭，似乎与之前，有些不一样了。
唳！
就在七王殿心中不安间，那一直被魔皇大手压制的混沌长箭，突然发出一道清脆之声，而后那弥漫开来的混沌之光，突然尽数的缩进长箭之中！
嗡嗡！
混沌长箭，在此时变得晶莹剔透，一种神秘而古朴的力量弥漫开来，这种力量，与那魔皇大手上的邪恶截然相反，但却是有着一种无可匹敌之感！
长箭箭尾疯狂的振动着，旋即那箭尖之处，仿佛是有着一抹异光浮现，旋即长箭一震，猛的暴射而出！
嗤！
光华席卷而开，那魔皇大手之上突然暴涌出滔天魔气，试图阻挡突然爆发的长箭，但此时的长箭却是犹如神助，那混沌之光，竟是生生的撕裂了那重重粘稠得近乎实质般的魔气，在那无数道震惊的目光中，蛮横的穿透魔皇大手，接着顺着魔皇手臂一路势如破竹而上，最后径直的自那魔皇头颅之上，洞穿而出！
咻！
长箭自魔皇头颅上传出，最后冲上九天云霄，虹光犹如巨龙，散发着那无以伦比的傲意。
砰砰砰！
魔皇虚影体内，在此时传出道道低沉的爆炸之声，而后那庞大的身躯，便是在那无数异魔惊骇的目光中爆炸开来。
七王殿呆呆的望着那爆炸而开的魔皇虚影，手中的骷髅符印猛的炸裂而开，其中一缕黑色发丝，竟也是在此时无火自燃，悄然的化为灰烬飘散而去。
噗嗤。
一口浓浓的黑血，直接自七王殿嘴中喷射而出，他无法置信的望着这一幕，那魔皇虚影，竟然被林动摧毁了？！
“怎么可能？！”
他身体颤抖着，目光隐隐有些骇然的望着远处那手持雷弓，面色也是有些苍白的林动，这一幕，显然是出乎了他的意料。
岛屿上，那无数强者望着这一幕，先是一怔，旋即便是爆发出惊天般的欢呼声，随着那道恐怖的魔皇虚影被毁，那种可怕的压抑也是消散而去，一时间，岛上士气暴涨。
古梦琪与古嫣对视一眼，皆是从对方眼中看出浓浓的惊喜。
天空上，林动听得下方传来的惊天欢呼声，手中的雷弓也是缓缓的散去，还好如今的他晋入了轮回境，不然的话，这种恐怖的消耗，即便他拥有着吞噬祖符使劲的恢复，恐怕也是难以支撑下来。
“现在，你还有什么手段？！”
林动那狰狞的目光，同样是看向了七王殿，此时后者面色惨白，周身魔气紊乱，显然也是受到了极为严重的重创，这般时候，正是痛打落水狗的最好时机。
七王殿见到林动那目光瞳孔也是微微一缩，暗暗咬牙，这小子每一次见面都会更加的难缠，先前那一箭，如果不是有着魔皇虚影阻拦的话，他或许也将会极端的凶险。
“轰！”
而在他目光阴沉间，林动周身，已是再度有着万道紫金光芒暴射而出漫天龙吟响彻，他身形一动，空间扭曲间，已是出现在七王殿前方，而后再没有丝毫的留情，拳影呼啸道道龙拳夹杂着蹦碎山岳般的力量，狠狠的对着七王殿身上落去。
砰砰砰！
重创中的七王殿，气息不稳，魔气运转间，所形成的诸多防御直接是被林动这般蛮横攻势尽数摧毁而去，而后林动拳头犹如紫金巨龙腾飞，轰在其身上。
可怕的力量爆发开来，七王殿身体顿时被震得倒飞而去，那断臂处黑色鲜血不断的喷射出来，身体之上的魔皇甲，也是黯淡无光，深深的拳印显露出来。
此时的七王殿，仿佛彻底的没了还手之力。
七王殿吐出一口黑血，他面庞有些扭曲的死死盯着林动，那眼中的怨毒，犹如是要将后者焚烧一般。
“去陪九王殿吧！”
林动冲着他狰狞一笑，身形再度出现在了其前方，手掌一握，万道紫金巨龙凝聚，再度化为一柄龙枪，然后毫不留情的对着七王殿咽喉洞穿而去。
“原来是你杀了九王殿！”那七王殿听得此话，面色顿时一变。
林动却是不理，眼神狰狞，那龙枪已是逼近七王殿，然而就在那龙枪即将将刺中时，他却是见到，那七王殿的脸庞上，掀起了一抹诡异的笑容。
“你这小子…还真是难对付啊，不过，能够把你杀了，其实也算是勉强完成任务了吧…”
七王殿脸庞上诡异浮现出来，旋即他独臂伸出，竟是一把将那龙枪稳稳的抓住，那种可怕的力量，哪还有先前那般虚弱。
林动见到他这诡异变化，瞳孔也是一缩，一股不安猛的涌上心头，身体之上银光一闪，就欲暴退。
“想走？”
七王殿见状，却是咧嘴笑起来，旋即他的身体蠕动着，竟是有着两只手臂活生生的探出来，这两只手臂结成诡异印法，粘稠魔气涌动间，竟是快若闪电般的轰在了林动胸膛之上。
嘭！
魔气奔涌间，仿佛连天空都是颤抖了一下，然后众人便是惊骇的见到，那先前还取得上风的林动突然吐血倒飞而出，那胸膛甚至都是有些塌陷了下去，显然是受到了极为可怕的重创。
噗嗤。
一口鲜血自林动嘴中喷出，他急忙稳下身子，目光阴寒的望着那身体蠕动的七王殿，后者的身体之中，似乎隐藏着什么。
“呵呵，肉体还真强大呢，挨了我们三人联手一击，竟然还能活着？”
七王殿身体扭曲着，他的体内，突然有着嗡鸣的笑声传出，而后粘稠的魔气从他体内涌出来，最后在那无数道震惊的目光中，再度化为两道魔影矗立在他的身旁。
那两道魔影一个身材高大，一个略显单瘦，此时的他们，正面带笑容的盯着林动，只是那笑容中，有着无尽的杀意。
“自我介绍一下，在下魔狱八王殿。”那身材高大的魔影微笑道。
“我似乎见过你这小子呢，你便是获得吞噬之主传承的那家伙吧？”另外一道魔影扫了林动一眼，笑眯眯的道，他的模样颇为的熟悉，赫然便是当日在那破碎空间中出现过的十王殿！
这七王殿的身体中，竟然还隐藏着两尊王殿！
显然，这原本是对付洪荒之主的暗招，但眼下，却是被林动逼了出来！
岛屿上，原本的欢呼噶然而止，不少人身体微微颤抖着，仅仅只是一个七王殿就如此的难以对付了，眼下竟然又出现了两个不逊色七王殿的异魔？
这还怎么打？！
林动缓缓的抹去嘴角的血迹，眼神也是一片阴沉，这些异魔，还真是狡诈啊，这下子的话，就真是有些麻烦了。
以他现在的实力，对付一个七王殿已是极限，若是再来上两尊王殿，那真是…
“不过怎么样，想要去洪荒塔，还是得先从小爷身上过去。”林动咧嘴一笑，笑容狰狞无比，这般局面，也只能死拼了啊。
“呵呵。”
三尊王殿皆是嘲讽一笑，也不多说，身形一动，就欲同时出手，将林动这个大患斩杀！
然而，就在他们将要出手时，这片天地，陡然间变得冰寒下来，那种寒气，连涌动的魔气，都是逐渐的僵硬。
林动也是感受到这股寒气，面色当即一变，急忙将目光投下，只见得那万丈庞大的寒冰囚牢突然蹦碎而去，一道冰蓝色的倩影，缓缓的自其中走出，在她身后，三道冰雕，闪烁着耀眼的光泽。
她站在寒气上，冰蓝色的美目，毫无情感的望着那三尊王殿，冰冷的声音，在天空上传了开来。
“要杀他，我可不同意。”


第1268章 应欢欢出手
寒气弥漫开来，仿佛是在此时笼罩了这天地，而在那种寒气下，不论是涌动的邪恶魔气还是浩瀚元力，竟然都是逐渐的变得僵硬下来，那种模样，犹如是要被冻僵一般。
突然间的变故，令得双方所有人都是一惊，无数道目光望向那寒气弥漫处，只见得应欢欢轻移莲步，每伴随着她玉足的落下，寒气便是会在其脚下凝聚成一朵冰莲。
步步成莲，绚丽而神异。
在其后方，三具冰雕闪烁着寒芒，那是三尊堪比渡过一次涅盘劫的真王，然而现在，他们却是尽数化为冰雕。
“这…”
众多强者惊愕的望着这一幕，那从应欢欢体内弥漫出来的寒气，甚至是超越了林动，这令得他们眼皮急跳，这才猛的反应过来，原来这里最强的人，并不是林动，而是这个一直都是跟在林动身旁的冰冷－女孩。
“你…”
那七王殿三人也是因为这变故大吃一惊，他们目光停留在应欢欢身上，旋即似是想到了什么，瞳孔猛的一缩，那脸庞上陡然有着惊骇之色涌出来：“你是冰主？！”
冰主，这在远古时期，曾经仅仅只逊色于符祖的存在，无数异魔王在其手中化为冰雕粉末，甚至连魔狱最强的天王殿，也是在当年最后一场战斗中，被她所重创，不得不潜伏，从而结束纠缠许久的天地大战。
这个名字，对于异魔而言，同样是有着相当可怕的震慑力。
应欢欢那冰蓝色的美目看着七王殿三人，旋即其玉手轻握，后方那三具冰雕便是砰的一声爆碎开来，化为绚丽的冰屑漫天散落。
这看得七王殿三人眼皮急跳，谁也没想到，三尊真王，竟然在应欢欢手中如此的脆弱。
应欢欢最终来到了林动面前，她望着他那苍白的面色，这个男人，总是喜欢把最艰难的事情自己揽下，有时候有些莽撞，有些霸道，但就是这样，却是让得她那冰冷的心中，总是有着一缕淡淡的暖意在流淌着。
“欢欢？”林动望着那寒气惊人的应欢欢，眉头微微皱了皱。
应欢欢冲着他微微一笑，冰凉的玉手落到他的身体上，而后其身体上的那些伤痕便是有着冰屑浮现，一种寒冷的力量涌入他的体内，将震动的脏腑尽数的平复下来。
“他们，就交给我来吧。”
应欢欢冰凉的玉手摸了摸林动的脸庞，轻声道：“你总不能让我眼睁睁的看你被伤成这样却依旧无动于衷吧？这种想法，是有些自私的。”
林动微滞，旋即喃喃道：“我是很自私啊。”
“可我就喜欢你这种自私啊。”应欢欢展颜一笑，她美目看着林动，道：“不过这一次，就算你打我骂我，我也不会在一旁看着了。”
林动也是望着应欢欢，她脸颊上有着笑颜，但那笑容中，却是有着一抹不容改变的坚定，最后他只能轻叹了一声，道：“小心点。”
见到林动点头，应欢欢美丽脸颊上的笑容顿时变得明媚许多，她螓首轻点，然后缓缓的转过身子，那对眸子再度看向七王殿三人时，已是冰冷得足以冻结空气。
七王殿三人见到应欢欢这般眼神，眼皮皆是一跳，竟是退后了一步，毕竟在那远古，冰主的名头实在是太过响亮了。
“不用怕她，她并不是真正的冰主，只不过冰主轮回转世而已，现在力量还未完全恢复！”七王殿咬咬牙，厉声道。
听得此话，那八王殿与十王殿也是点了点头，目光盯着应欢欢，阴沉的道：“正好，趁她力量未恢复，直接将她给杀了！”
“就怕你们这三只小老鼠办不到呢。”应欢欢伸出纤细的玉指，寒气在其白玉般的指尖化为朵朵冰莲，她冰蓝美目看了一眼三人，淡淡的道。
“哼，那就来试试！”
七王殿三人对视一眼，眼中皆是有着阴寒之意涌起来，旋即同时一步跨出，三股恐怖的魔气便是席卷而出而三人中，七王殿气息最弱，显然是因为先前被林动重创的缘故。
不过即便是如此，三人气息齐齐爆发，那也是相当的惊人，三股魔气，犹如万丈狼烟般升腾而起，千里之外，都是清晰可见。
吼！
犹如野兽般的低吼声，猛的自三人嘴中传出，旋即眼中凶芒闪过，手印一变，只见得三道万丈魔气匹练已是呼啸而出，快若闪电般的对着应欢欢席卷而去。
那魔气匹练之中，粘稠如水，邪恶的波动令得人毛骨悚然。
然而面对着三人的联手攻势，应欢欢却只是美目轻抬，待得那魔气匹练即将轰中身体时，玉足方才轻轻一跺。
砰！
寒气疯狂的席卷开来，几乎瞬间，便是在她前方化为巨大无比的蓝色冰盾，那三道魔气匹练冲撞而来，力量尚还未喷薄而出，那霸道到极点的寒气，却已是弥漫了出来。
咔嚓！三道魔气匹练在此时凝固，魔气之上，冰层以一种惊人的速度蔓延出去，短短瞬间，三道可怕的魔气匹练，便是化为三道闪烁着寒芒的冰柱。
“该死的！”
七王殿三人见到魔气竟然被寒气冻结，面色都是有些难看，然而此次还不待他们再度出手，应欢欢玉手一握，只见得那三道万丈冰柱猛的暴射而出，狠狠的对着七王殿三人冲击而去。
“咚！”
三人见状，急忙全力相迎，覆盖着浓浓魔气的拳头重重的轰在那冰柱之上，两股可怕的力量冲撞在一起，顿时将冰柱震成了漫天冰屑。
哼。
不过虽说冰柱爆碎，但那八王殿以及十王殿皆是闷哼一声，本就被林动重创的七王殿更是面色一白，一口黑色鲜血便是忍不住的喷了出来。
“好厉害！”
岛屿上，那无数道目光望着那以一敌三竟然还能够占据上风的冰蓝色倩影，无不是被震撼得目瞪口呆下来，先前林动的表现已是让得他们感到震惊，然而谁能够想到，这个一直只是跟在林动身旁的女孩，竟然也是如此的不显山不露水。
应欢欢美目冰冷，她望着七王殿三人，莲步轻移，携带着漫天寒气，缓缓走向他们，在那寒气之中，仿佛是有着无尽的杀意在奔涌着。
“联手！”
七王殿见到应欢欢步步而来，心中也是忍不住的哆嗦了一下，在那远古时期，他们也曾与冰主交过手，不过却是险些被杀，眼下这般时候，若是再不联手，恐怕就真是要栽在这里了。
八王殿与十王殿闻言，也是咬牙点点头。
殿
三人手印瞬间变幻，然后猛的对轰在一起，只见得三人身体竟是在此时蠕动起来，一根根狰然的魔刺，缓缓的从他们体内伸探出来，此时的他们，犹如自那地狱深处爬出来的恶魔一般，令人心底发寒。
“嗤！”
黑色的鲜血，陡然自三人体内暴射而出，粘稠的血液，在他们上空飞快的凝聚，最后化为一道极为诡异的黑色符印。
黑色魔印，以一种惊人的速度膨胀着，短短数息间，便是化为万丈庞大，而且在那魔印周围，竟是有着无数魔臂伸出来，远远看去，犹如一尊从域外而来的邪恶天魔！
应欢欢抬头，美目望着那生长着无数魔臂的魔印，眸子深处，寒气愈发的惊人。
“万魔汇聚，阿鼻魔印！”七王殿三人猛的尖啸出声，凄厉的吼声，响彻天际。
轰！
那巨大无比的万臂魔印陡然一颤，而后猛然自天空上暴掠而下，万臂挥动间，仿佛连天地都是在此时被撕裂。
下方的大海，都是在此时被掀起万丈巨浪，轰隆隆的巨声，响彻而起。
林动望着这般恐怖攻势，眼中也满是凝重，这些魔狱的王殿，的确是相当厉害，这种攻势，恐怕足以秒杀任何渡过两次轮回劫的巅峰强者。
“也不知道欢欢能否接下来。”
林动有些担心的望着那道纤细的冰蓝色倩影，手掌紧握着，体内元力奔涌，打算一有不对，便是施展空间祖符将其救走。
应欢欢螓首轻抬，她美目注视着那呼啸而来的阿鼻魔印，脸颊上，却是没有丝毫的波动，她任由那种恐怖的劲风吹拂而来，衣裙猎猎作响，冰蓝色的长发，也是随风飘舞。
冰蓝色美目，倒映着恐怖的魔印，而后魔印急速的放大着，短短数息之后，那片巨大的阴影，已是在那无数道惊骇的目光中，将应欢欢笼罩在其下。
在那无数目光中，应欢欢也是缓缓的伸出纤细玉臂，湛蓝色的袖子滑落下来，露出犹如羊脂玉一般的皓腕，那修长的玉指展开，接着举起，最后终是与那万臂魔印，轻轻接触在了一起。
“永恒…冰冻。”
带着一些冰冷，但却悦耳得令人心旷神怡的轻声，在天空上悄然的传开，那携带着恐怖力量而来的魔印，却是在此时，陡然凝固。
咔嚓！
凝固的瞬间，无数人瞳孔猛的一缩，因为他们见到，一层厚厚的冰层，突然自应欢欢玉手处蔓延而出，然后以一种惊人的速度，把整个魔印包裹而进，并且将其化为巨大的冰块。
应欢欢美目轻抬，修长的玉指，轻轻弹在魔印之上。
嘭！
随着这一指弹下，只见得那凝聚了七王殿三人联手之力的魔印，竟是脆弱得犹如玻璃一般，在此时彻彻底底的爆炸开来。


第1269章 空间之主
冰屑铺天盖地的降落下来，犹如一场绚丽无比的冰暴，但在那绚丽之下，却又是蕴含着令人为之颤粟的可怕力量。
谁能想到，那来自七王殿三人的联手攻势，却是如此轻易的便是被她所破去。
“这便是冰主的力量吗？”
林动望着这一幕，双掌也是忍不住缓缓的紧握起来，这种力量，的确无以伦比，难怪她能够成为符祖之下的第一人。
“噗嗤！”
那天空上的七王殿三人，也是目瞪口呆的望着这一幕，旋即忍不住的一口黑血喷出来，身体颤抖间，周遭魔气飞速的萎靡下来，显然是受创不轻。
应欢欢美目毫无情感的望着七王殿三人，娇躯一动，快若鬼魅般的出现在三人之前，玉手之上，惊天般的寒气，席卷而开！
七王殿三人见到应欢欢丝毫不给他们喘息机会，面色也是剧变，三人身形一动，便是分开暴退。
“嗤！”
应欢欢美目中寒气凝聚，玉手一握，一柄修长的湛蓝冰枪便是自其手中延伸而出，旋即其身形一颤，竟是一分为三，三道冰寒身影追上那七王殿三人，冰枪瞬间洞穿虚空，狠狠的对着三人暴刺而去。
轰！
七王殿三人见到应欢欢穷追不舍，眼神也是剧变，一声低吼，滔滔魔气席卷而出，便是在面前化为巨大的魔气光幕，试图借此将应欢欢阻拦而下。
然而，面对着他们的防御，应欢欢眸子中寒气愈发的浓郁，手中冰枪没有丝毫犹豫，直接刺了出去，而后刺进那魔气之中！
咔嚓！
粘稠的魔气，几乎是顷刻间化为冰块，而那冰枪则是势如破竹般的洞穿而进，最后狠狠的刺在在三人身体之上。
吱吱。
强大无比的魔皇甲，在那冰枪之上扭曲变形，最终被寒气所洞穿，泛着无法形容寒气的冰枪便是这般生生的自三人身体之中洞穿而进。
噗嗤。
三人身体狼狈的倒飞而出，黑色的鲜血不要钱一般的狂喷出来，身体上寒气布满的蔓延出来，竟是在他们身体表面都是结成了薄薄的冰层。
呼。
林动望着转瞬间便是溃败的七王殿三人，轻吐了一口气，这种力量，还真是恐怖呢，如今的应欢欢还不算完全的觉醒，但其实力恐怕已是超越了寻常渡过两次轮回劫的巅峰强者，真不知道她若是完全觉醒的那一天，那种力量，又该达到何种恐怖的地步。
天空上，那七王殿三人再度被逼得汇聚在一起，他们体内不断有着魔气涌出来，抵抗着那种寒气的侵蚀，他们明白，如果当他们的身体化为冰雕时，应欢欢一指之下，他们就会化为漫天冰屑，当年一些比他们还要强大的异魔强者，便是这样的死在她那洁白如玉般的素手之下。
三人望着那再度将美目投向他们的应欢欢，眼中终是掠过一抹恐惧之色。
应欢欢那冰蓝色剔透的美目中，依旧是没有丝毫的情绪波动，此时的她，犹如真正的玄冰，任何情绪，都是被冰冻在那冰层之下。
“你们…该死。”
她看着七王殿三人，这样的喃喃自语，旋即其身形一动，直接出现在三人上空，玉手一种，一枚古老的寒冰符文浮现出来，而后直接是化为一道凝聚到极致的寒气光束，对着三人暴射而去。
嗤！
光束撕裂天空，仿佛连空间都是被那种寒气所冻结，然后七王殿三人便是骇然的感觉到，周遭的空间，已是在此时凝固，他们根本动弹不得，只能眼睁睁的看着那拥有着毁灭性力量的寒气光束，喷薄而来。
浓郁到极致的恐惧之色，猛的自他们眼中涌了出来。
不过，就在那寒气光束即将把他们笼罩时，三人前方的空间，突然诡异的扭曲起来，而后一道黑影便是凭空的闪现而出。
“呵呵，冰主，你这般身份，何必欺负这些小辈呢？！”
那道黑影一出现，便是一声轻笑，不过那轻笑中，倒是有着一些难掩的凝重与鸡蛋，而后他手掌探出，只见得其手中，竟是生有一枚诡异的邪恶眼睛。
他张开手掌，那邪恶眼睛也是猛的睁开，一道漆黑得令人心底发寒的粘稠光束，瞬间自那眼球中暴射而出，最后与那寒气光束，正面的轰击在一起。
两道毁灭性的光束狠狠的撞击在一起，顿时有着可怕的涟漪波动席卷开来，直接是将天空上的云层尽数的撕裂而开。
天空震荡，应欢欢娇躯终是在此时微微一颤，而后被震退数步，她美目泛着冰寒的望着那现身的黑影，道：“三王殿？终于肯现身了吗？”
漫天目光汇聚而去，只见得此时在那七王殿身前，一名黑衫男子微笑而立，他此时的手掌上，同样是被一层薄薄的冰层所覆盖，掌心那只邪恶的眼睛也是被封闭而上。
“三王殿？！”
林动听得此话，面色却是微微一变形一动，迅速的出现在应欢欢身后，从这三王殿身上，他感觉到了极端浓郁的危险波动。
显然，这家伙的实力，绝对堪比渡过三次轮回劫的巅峰强者！
这种家伙，就算是在那远古天地大战中，都是异魔中最为顶尖的战力，没想到这一次，连这种存在，都出现了。
那三王殿看了林动一眼，然后笑了笑，手臂微震，便是将那薄薄的冰层震裂而去，他微微抬起头来，林动这才发现在他的脸庞右侧，又着一条极为狰狞的伤痕，伤痕从嘴角蔓延到耳后，看上去几乎曾经被人将脑袋都是切了开来。
“有你在这里，我原本是不想露面的，毕竟这条伤痕，可是太让我心有余悸了呢。”三王殿微笑道，他手指划过脸庞上的狰狞伤痕，那脸庞上的笑容，却是愈发的温和。
应欢欢淡淡的看了他一眼玉手中，带着有着惊人的寒气凝聚而来：“那这一次，就把你冻成冰雕好了。”
“呵呵。”
三王殿轻笑一声，道：“现在的你，恐怕办不到吧。”
“那你就来试试。”应欢欢眸子依旧冰彻，玉手一漩，便是再度有着极端恐怖的寒气自其体内席卷而出，周遭的空间，都是在此时发出咔嚓的细微声音。
三王殿目光盯着应欢欢脸庞上的笑容愈发的浓郁：“呵呵，我很真的很想杀了你呢，当年竟然被你弄得那般的狼狈，让我在吾皇面前丢尽了颜面…”
随着他声音的落下，三王殿那温和的面庞却是逐渐的扭曲起来那眼瞳中，有着无尽的怨毒涌出来。
“今日我就真要把你给杀了！”
三王殿猛的一步踏出那等魔气，冲天而起，直接是笼罩了这千里天地，甚至连天空上的烈日，都是在此时被魔气掩盖，再也不敢露面。
这般气势天地颤抖。
林动见状，眼神也是一变，手中再度有着雷弓凝聚出来，一只手掌也是拉上了应欢欢那冰凉的玉手。
应欢欢擦觉到他的举动，娇躯微微僵了僵然后逐渐的放松下来，偏头冲着林动露出一个放心的笑容。
“他出不了手的。”
听得此话，林动一怔，显然是有些不明白。
“会有人拦住他的。”应欢欢微微一笑，旋即眸子看向某处空间：“你还是喜欢这么躲来躲去？”
“呵呵，小师妹的感知还是这么敏锐。”
她的声音刚落，只见得那里的空间竟然是逐渐的扭曲起来，而后一道轻笑声传出，空间扭曲间，一道身着银袍的人影，缓缓的走出。
那道人影，苍白的头发披散下来，他身形颇为的干瘦，而最奇异的是他的双眼，那对双瞳，璀璨如银，其中仿佛是渗透着极为神秘的波动。
林动见到这道人影，眼中却是掠过一抹惊愕之色，因为此人，竟然是他在天妖貂族天洞最深处见到的那神秘之人！
此人，竟然也是远古八主之一？
“这位便是林动了吧？”银瞳男子微笑着看向林动，然后瞥了一眼他与应欢欢拉在一起的手掌，嘴角微微勾了一下，道：“果然如此呢。”
“你便是…空间之主吧？”林动望着这银瞳男子，在他出现时，他能够感觉到体内的空间祖符震动了一下，当即也是明白了他的身份。
银瞳男子笑着点了点头，道：“抱歉了，之前被魔狱施展手段困住了一些时间，不过小师妹，我可没偷懒啊，你看，我一脱困就立即来了，这速度至少比雷主快吧？”
应欢欢冰蓝美目淡淡的看了他一眼，空间之主声音这才一顿，无奈的摇摇头：“怎么还是这样啊…”
说着，他也是转过身来，笑望着那三王殿，道：“哟，真是老熟人了呢，这些年，你还好吗？你当年不是脸被小师妹撕烂了吗？咦，好了啊？”
林动有点愕然，怎么感觉这空间之主有些话唠呢，先前出场的气度风范在他这唠叨下，顿时打了折扣。
三王殿也是眉头微皱的望着这赶来的空间之主，心中无奈的叹了一口气，这家伙来得还真快啊。
“看来今日是杀不了她了啊。”
三王殿仲了一个懒腰，旋即唇角有着一抹诡异笑容勾起来：“不过，我们的目的，也算是达到了呢。”
应欢欢与空间之主闻言，眼神都是猛的一凝。
而也就是在这一霎，林动心头猛的一惊，急忙转头望向遥远的海域，在那极远极远的地方，他突然感受到了一股极端暴戾的魔气出现。
应欢欢轻吐了一口气，眸子却是彻底的冰寒下来。
“大天邪魔王…逃出来了。”


第1270章 诸强汇聚
“大天邪魔王？”
林动闻言，面色顿时剧变下来，当日炎主便是说过，他所镇压的镇魔狱，主要是为了镇压那大天邪魔王，为了此次的变故，他还特地将分身力量都是收了回去，没想到，即便是这样，都未能将那大天邪魔王压制住？
一旁空间之主脸庞上的笑容也是在此时缓缓的收敛起来，他那银瞳闪烁着寒芒的盯着三王殿，冷声道：“原来你们这么大费周章，并不是要对洪荒之主出手，而是想要将大天邪魔王救出来。”
“呵呵，其实也不尽然，如果又能够将大天邪魔王救出来，又能够趁机解决掉洪荒之主，那自然是最好的。”三王殿微微一笑，道。
“既然如此，那就只要将你留在这里了做抵偿了。”
空间之主脸庞上也是有着一抹笑容浮现出来，只是那笑容中却满是冰寒之色，旋即其手掌缓缓自袖中伸出来，他的手掌颇为的干瘦，那在其指尖却是有着银光闪烁，那种力量林动极为熟悉，那是空间祖符之力。
“是么？”
三王殿双目也是微眯，那掌心中的诡异眼睛，也是再度微微睁开一丝。
“当年就看你这货不顺眼了，今天说什么都得甩你两耳挂！”空间之主笑着说道。
“呵呵。”三王殿也是微笑，下一瞬，他瞳孔微微一缩，那有着一只诡异眼睛的手掌猛的朝后方怒拍而去，滔天魔气席卷开来。
嘭！
魔掌掠过，那片空间突然扭曲下来，一只闪烁着银光的拳头轰出，与其硬憾在一起，狂暴的波动，在天空上传出惊雷般的声响。
“这么多年不见，还是喜欢用这般手段。”三王殿偏过头望着那自他身后缓缓出现的银瞳人影，冷笑道。
林动见状，眼中也是掠过一抹惊色，目光瞥了一眼先前空间之主所立的方向，只见得那里的“空间之主”，正在缓缓的消失而去，显然，在先前他未曾察觉间，后者竟已是出手了。
“不愧是空间之主。”林动心中暗赞了一声，他方才获得空间祖符没多久，要论起对空间之力的掌控，显然是不及这空间之主。
“呵呵，你也不赖啊，躲了这么多年，也没躲傻掉呢。”空间之主含笑，掌心银光再闪，只见得这片空间猛的扭曲起来，两道银色的空间之刃，犹如剪刀一般，闪电般的对着三王殿延伸而去。
三王殿体内魔气喷涌出来，化为魔气光柱将其护在其中，同时也是将那空间之刃抵挡了下来。
双方出手都是快若闪电，但其中所蕴含的可怕之力，林动却是能够清晰的感觉到，到了这种层次，举手投足间，那种力量，都是凝聚到了极点。
“他们的实力，似乎相差不多。”
林动见到两人交手，轻声说道，空间之主已那三王殿虽然脸庞上都是笑容和善，但那下手全部都是狠辣手段，只不过两人实力的确有些不分伯仲，这般战下来，都是难以取得上风。
“嗯，他们都算是渡过三次轮回劫的实力，若只是这样斗的话，结果很难分出来。”应欢欢螓首轻点，道：“而且他们现在，都没必杀对方的心思，这种战斗，持久不了的。”
林动也是点头，他们两人都清楚这样杀不了对方，所以那等杀意，反而是比较的淡薄，不过林动明白，会有一天，他们将会真正的以命搏杀，那时候，恐怕就真是有些恐怖了。
砰！
天空上，银光黑芒交错而过，两道身影也是微微一震，而后便是稳下了身形。
“空间之主，你也杀不了我，何必在这里浪费时间，你是在等什么吗？”三王殿冷笑道。
“你不也是在等什么吗？”空间之主笑吟吟的道。
林动见到同样是笑面虎的两人，无奈的摇了摇头，眼下这事，真是越来越麻烦了啊。
“将下面那些异魔隔开吧。”林动看了一眼依旧有些混乱的武会岛，然后偏头对着应欢欢道。
“嗯。”
应欢欢点点头，玉手一扬，只见得那空气便是凝固成冰，而后坚冰犹如冰墙般蔓延而下，很快的便是将整个武会岛都是隔离在了其中，而那些异魔，则是在冰墙之外，再也进去不得。
林动见到她这手，便是忍不住的摸摸鼻子，有点郁闷。
似是察觉到他的郁闷，应欢欢冰凉的玉手不由得反握住他的大手，那有些冰冷的美丽容颜上，浮现出一抹浅笑，那霎那间的笑容，犹如冰山上盛开的雪莲。
“哈！小师妹，你笑了！”不过林动尚还没来得及感觉这霎那间的惊艳，那空间之主便是猛的睁大眼睛，手指指着应欢欢，大声道。
林动望着这出场时极有风范，但现在却总是一惊一乍的空间之主，忍不住的有些哭笑不得，其他的远古之主，不是沉稳便是深沉，而这家伙，在与三王殿交手时看上去还很让人折服，但没想到一说话，就…显得如此的与众不同。
“小师妹，没想到你笑起来这么好看，以后可要多笑笑啊，这样师兄们除魔起来也能多有些力气。”之主凑近过来，笑眯眯的道。
“闭嘴！”应欢欢轻咬银牙，狠狠的盯了空间之主一眼。
空间之主干笑了两声，然后拍了拍林动肩膀，竖起大拇指：“哥们，你真牛，连这种冰山都能搞定，我很…唉，才苏醒过来没多久，脑子有点不太好使，我说的话，你可千万不要往心里面去。”
空间之主话还未说话，面色便是陡然间郑重起来，对着林动很认真地说道。
林动也是回以干笑，他可是看见一柄冰剑从应欢欢玉手中延伸了出去，然后抵在了空间之主腰间。
空间之主作怪了一番，那面色却是逐渐凝重了下来，轻声道：“来了。”
林动微微点头，他也是感觉到，这大海之下，有着一股极端恐怖的波动，正在飞快的接近。
应欢欢玉手上，寒气悄然的凝聚，冰蓝美目，扫向下方的海面。
砰！
而就在空间之主声音落下后不久，那下方的海面，突然有着一个万丈漩涡成形，只见得滔滔魔气自那漩涡中席卷而出，最后，一道万丈黑影，猛的自其中暴射而出。
黑影一出现，这天地都是昏暗了下来，林动目光望去，瞳孔便是微微一缩，只见得那道魔影，足有万丈巨大，它身体漆黑如精铁，在其身体上，布满着狰狞的伤痕，它没有眼目，只有着一张布满黑色利齿的巨嘴，而且在其庞大的身躯上，有着八只巨大魔臂伸展。
“大天邪魔王？”
林动一见到这般模样，便是将其认了出来，当日元门三巨头寄生的大天邪魔与其一般无二，只不过那普通的大天邪魔，只有四只魔臂，而这家伙，却是有着八只。
另外，这大天邪魔王的气息，也是达到了一种极端恐怖的程度，虽然他如今气息起伏不定，显然是重创未愈，但林动却是隐隐的感觉到，恐怕这家伙全盛时期的实力，比起这三王殿还要强上一些。
难怪当日炎主说，这大天邪魔王的实力，在魔狱之中，仅仅只比那最强的天王殿要弱。
那大天邪魔王出现在天空，而后它竟是转向了应欢欢，一道嗡鸣的嘶吼声，犹如无数野兽咆哮般的传出：“冰主？你果然还活着！”
这大天邪魔王，显然也是拥有着极高的灵智，虽无眼目，但却是能够看见应欢欢的存在。
“没想到还真被你这孽畜逃了出来。”应欢欢美目冰寒的望着那大天邪魔王，道。
“呵呵，为了救他出来，我们可真是费了好大的力气呢。”
大天邪魔王肩膀处，突然有着笑声传来，两道魔影缓缓的凝聚出来，然后他们对着应欢欢微微弯身：“冰主，万载不见，别来无恙啊。”
“哟，原来是四王殿与五王殿呢…你们人来得可真是齐啊。”空间之主望着那两人，眼神微凝，旋即笑道。
林动手掌紧握着雷弓，眼中满是戒备之色，眼前这阵容实在是太过恐怖了，魔狱排名前五的王殿，足足来了三位，若是再加上那大天邪魔王的话，恐怕得算是四位堪比渡过三次轮回劫的巅峰强者。
“没办法，炎主不好对付呢，不然怎么能救出大天邪魔王来。”四王殿微微一笑，道。
“你们说的那家伙追过来了。”空间之主回以微笑，而在其话音落下间，那天际之边，突然云彩都是燃烧起来，一道火光席卷而来，下一霎那，已是出现在了这岛屿上空，火光散去，正是炎主。
不过此时的炎主，头发略微有些凌乱，那面色也是不太好看，显然之前是经历了一场相当惨烈的大战，他露出身来，先是看了一眼空间之主与林动，然后便是将目光投向了应欢欢，有些无奈的道：“吃了点暗招，被他们把大天邪魔王救出来了。”
应欢欢微微点头，但却并未多说什么。
“哎呀呀，好多年没这么热闹了…”
三王殿伸着懒腰，笑眯眯的望着应欢欢等人，然后伸出手指数了数，道：“好像是我们人数占优一点啊，要不就在这里把你们解决掉好了？”
“哈哈，人数占优又如何？这种时候，怎么能够少得了我。”
不过这三王殿声音刚落，突然间有着轰鸣般的大笑声在这天地间响彻而起，然后林动心头微动，偏过头来，便是见到那洪荒塔中，爆发出了万丈紫金光芒。
“真是好玩啊…”
林动见状，不由得轻吐了一口气，这洪荒之主也出来了么？看这模样，难道是要在这里直接开启天地大战了？


第1271章 巅峰对恃
紫金光芒铺天盖地的自那洪荒塔尖处暴射而出，直接是将这千里之内的海洋都是着照耀得闪闪发光，而天空上那原本因为大天邪魔王带来的魔气，则是在这种光芒的照耀下，迅速的被逼退。
天空上，顿时再度有着阳光倾泻下来。
“洪荒之主。”那三王殿等人眼神也是在此时微微一凝。
洪荒塔尖，那紫金光芒中，一道巨大无比的身影也是缓缓的出现，那道身影以一种惊人的速度膨胀着，最后也是达到万丈，那般体积，丝毫不比大天邪魔王小。
“哈哈，一出来就遇见这么热阄的事，看来我可真是好运气啊。”那紫金巨人仰天大笑，笑声如雷，在那海面上掀起滔天巨浪，天地间，甚至有着飓风因其而成形。
“哼！”
那大天邪魔王狰狞巨嘴中也是爆发出一道低沉哼声，魔气涌动间，将那万丈紫金光芒抵御而下。
“你这孽畜，还真是命硬，这样都死不了。”洪荒之主望着大天邪魔王，也是忍不住的摇了摇头，言语间颇为的遗憾，当年为了重创这大天邪魔王，也是费了他们不少功夫，这才将其镇压封印，没想到，还是被他逃了出来。
“当年不过是你们使诈，不然的话，谁输还不一定！”大天邪魔王阴沉的声音，在天空上回荡着。
“不服气的话，现在再打一场也可以啊。”洪荒之主巨目望向大天邪魔王，庞大的身体上，倒是有着惊人的战意升腾起来。
“呵呵，莫急莫急，总归会有机会的，到时候，我们可不打算放过你们。”三王殿笑吟吟的摆了摆手，道。
虽然眼下他们人数略微占优，但大天邪魔王刚刚脱离镇压，实力尚未彻底恢复，若是动起手来也是难分输赢，所以…现在可不是动手的好时机啊。
“到了那般时候，恐怕你们就没第二次再度潜伏的机会了。”炎主缓缓的道。
“炎主，现在把话说得这么满可不好。”
四王殿也是一笑，只是那眼目中，却是充斥着阴森之极的杀意：“这一次的话，恐怕输的该会是你们了，而且，我们可不会再让你们有任何翻身的余地，这片位面，总归会沦为我异魔族的掌控！”
“真是很想试试啊。”空间之主也是微笑道，那银瞳中，银芒闪烁着冰寒的光泽。
双方对恃，庞大的气息泾渭分明的涌动着，他们几乎算是这片天地间最为顶尖的强者，而这片位面无数生灵的生死，也是掌控在他们的手中。
“会有机会的，不过现在么，还是都先等等吧，你们想等冰主彻底觉醒，我们也想等天王殿彻底恢复，到时候，远古的恩怨，再来了结清楚吧。”
三王殿看了应欢欢一眼，然后轻笑道：“这一次我们的目的已经达到，就先行告辞了。”
说着，他顿了顿，接着道：“另外，第二场大战，现在应该算是开始了，只不过这一次，输的人，将会是你们，哈哈。”
笑声落下，三王殿袖袍一挥，滔天魔气席卷而开，直接是在天空上化为巨大的魔气漩涡，漩涡旋转间，他们的身形尽数的没入其中，下方那铺天盖地的异魔，也是如同蝗虫般的冲了进去。
魔气漩涡急速的旋转，最后迅速的消散而去，而随着这些家伙的离去，那笼罩着这片天地的魔气也是迅速的消退，温暖的阳光，再度自天空上倾洒下来，照耀在岛屿之上。
林动望着再度恢复平静的天空，也是微微松了一口气，他知道今日这大战打不起来，魔狱那天王殿以及二王殿皆是未曾现身，那大天邪魔王也是未能恢复战力，魔狱自然是不会在这种时候选择开战，不过，他有着预感，恐怕那一天，不会太远了。
洪荒之主那巨人身躯也是在此时迅速的缩小下来，然后化为一道身躯约莫两丈高大的铁塔身影，他的皮肤闪烁着紫金色光芒，一种无法言语的强大之感散发出来，要光论肉体的话，恐怕这洪荒之主算是当世第一人。
“呵呵，这次多谢相护了。”洪荒之主望向林动，笑着拱了拱林动苦笑着摇了摇头，道：“我可没帮到多大的忙。”
他如今的实力，仅仅只能与七王殿那种层次的魔狱强者相斗，再上一层，类似三王殿这些堪比渡过三次轮回劫的强者，他便是有些无能无力了。
先前那种对恃，他显然是有些难以插进手的，这种情况，无疑是让得他暗叹了一声，实力，果然还是不够啊，虽然他也已经很努力了。
似是感觉到他神色中的沉重，应欢欢握住他的手掌不由得微微用力了一下，林动有所察觉，回以一笑，不管再艰难，他都不会轻易放弃的。
一旁的炎主，空间之主，洪荒之主三人见状，目光交织了一下，神色都是有些奇特，也不知是在想着什么。
“待会或许雷主与黑暗之主也会赶来了。”炎主看着应欢欢，说道。
林动心头微震，若是连雷主与黑暗之主都是来了的话，远古八主，除了吞噬之主以及生死之主外，其余的人，基本都算是来齐了。
“先去岛上吧。”林动开口道。
炎主几人闻言目光都是看向应欢欢，后者螓首微点，然后拉着林动对着岛屿之上掠去。
炎主三人在后面望着两人的身形，目光皆是闪烁了一下。
“炎主，这是个什么情况？”空间之主犹豫了一下，问道，看应欢欢这模样，显然对林动并不是什么浅情啊。
炎主无奈的摇了摇头眼神有些复杂，道：“她自己明白的。”
说完，他就不再多说，然后掠出，空间之主与洪荒之主面面相觑一眼也是苦笑一声，迅速的跟了上去。
武会岛上，因为之前的大战，皆是一片混乱，林动数人直接是进入楼阁，接着青檀以及那些天风海域的首领也是跟来，只不过除了青檀外其余人都是眼神敬畏的望着楼阁中的一群人，竟是不敢怎么靠近过来。
显然，他们之前也是见到了洪荒之主的出世而其他几位那般气度，丝毫不比洪荒之主弱，这样一思量，他们也是隐约的猜出了一些端倪，那心中顿时是有些翻江倒海，毕竟这些，可都是显赫今古的巅峰强者啊…
几人在楼中闲坐，其他人不敢靠近，林动只得吩咐青檀为众人上了茶，然后楼中便是变得有些安静下来。
应欢欢坐在林动身旁，冰蓝色的长发顺着纤细的腰肢垂落下来她美目盯着地面，玉指间夹着一缕发丝，轻轻磨挲，也不怎么说话。
炎主三人也不知是在想什么，也是保持着安静，林动见状，心中苦笑一声，也只好如此。
这般安静，足足持续了一个时辰应欢欢方才微微抬起脸颊，美目望向远处，道：“来了。”
随着她声音落下，远处突然有着细微的雷鸣声响起，旋即一道雷光掠过天际，下一霎，已是出现在了楼中，那般模样，正是雷主。
而在雷主出现后没多久，原本明亮的天空则是突然的黑暗了一瞬，青檀嘴中不由得发出一道细微的轻咦声。
黑暗仅仅持续了短暂的一个呼吸，然后林动等人便是将目光看向了楼中一个空出来的座位上，那里，不知何时，一道身着黑色软甲的倩影正慵懒而坐，她抬起那深邃异常的黑色美目，扫了众人一眼，然后停留在应欢欢身上，脸颊上这才有着笑容浮现出来：“小师妹，好久不见啊，想师姐没？”
应欢欢看着黑暗之主，那冰蓝色的美目中也是泛起一抹波动，旋即轻声道：“师姐没事就好。”
“我又不是他们这些蠢货，怎么会有事。”黑暗之主笑吟吟的道，旋即她看了一旁的林动一眼，眸子中倒是划过一抹诧异之色：“小子不错啊，竟然到轮回境了，不过这跟小师妹还有着很大的差距啊，当初你可是信誓旦旦的说¨”
“师姐。”
应欢欢弯月般的柳眉微微蹙了蹙，声音提高了一些。
“好吧，我不说了。”黑暗之主优雅的摊摊手，旋即看向炎主：“大天邪魔王被救走了？”
炎主点点头。
“有心算无心，我们棋差一着啊。”空间之主道。
“这些家伙，还是一如既往的狡诈啊，我原本是想探测出他们所在的位置，但却反而被他们利用，设计将我困住，这才抓住破绽，劫走了大天邪魔王。”雷主咬了咬牙，道。
“你总是这样一头乱冲，不出事才怪了。”黑暗之主白了雷主一眼，道。
雷主闻言，尴尬的笑了笑。
“如今他们将大天邪魔王救出，接下来便是要全力相助天王殿彻底恢复了。”炎主缓缓的道：“若是等天王殿彻底的恢复，我想，应该便是第二次天地大阵彻底开启的时候了。”
听得天王殿这这个名字，楼中众人都是静了静，眼中都是掠过一抹忌惮之色，再接着，他们的目光，突然看向了应欢欢。
应欢欢玉手握着碧绿玉杯，她望着茶水中倒映出来的美丽容颜，最后轻轻一叹，幽幽的声音，在楼中回荡开来。
“开启祖宫阙吧。”


第1272章 祖宫阙
“祖宫阙？”
当这三个字在楼中传开时，黑暗之主他们的神色都是微微凝了凝，旋即目光停留在应欢欢的身上，犹豫了一下，道：“你确定？”
应欢欢微微点头。
“祖宫阙是什么？”林动见到他们的面色，心中有些不安，问道。
应欢欢抬起俏脸，那冰蓝色的美目望着林动，微笑道：“是师傅留下来的一座宫阙，你如今的精神力，应该是在大符宗境吧？还没有凝聚神宫吧？”
林动点点头，精神力达到这种程度，若是再进一步，那便是能够将泥丸宫修炼成神宫，而一般来说，这般境界，也算是精神力的巅峰层次，名为神宫境，不过自古到今，能够将精神力修炼到这一步的人，屈指可数。
虽然林动精神力天赋极为的优秀，但却始终未能成功将泥丸宫修炼成神宫，他也明白，那一步，不是那么容易跨入的。
“你想让他进祖宫阙凝聚神宫？”黑暗之主看了林动一眼，道：“这一步可不是那么容易晋入的，我们之中，也就你修炼出了神宫，我们几人，精神力顶多只能算做半步神宫境。”
“他应该可以的。”应欢欢道。
“那祖宫阙对你，有什么作用？”林动盯着应欢欢，他能够感觉到她似乎是有些顾左言右，似乎是在闪避着什么。
“祖宫阙有着助人修炼的神效，待得时机成熟我们会将其开放，让这天地间达到转轮境的强者进入其中，一般说来，会有着三成的几率令得他们成功突破。”应欢欢继续说道。
林动微皱着眉头盯着应欢欢，后者也是用那冰蓝美目与他对视着不知为何，两人间气氛微微紧绷了一些。
“咳。”
黑暗之主见到两人这番模样，不由得轻咳了一声，对着林动有些意味深长的道：“林动，有些东西，只有当你拥有了足够的实力，才能够去改变但现在的你，还不具备改变它的力量，而进入祖宫阙能够让你拥有这种资格。”
“真是这样吗？”林动也不看黑暗之主，只是看着应欢欢。
应欢欢轻轻点头，道：“相信我，好吗？进入祖宫阙，对你会有好处的。”
林动缓缓的收回目光，这种时候他还能再说什么吗？
“岩，你也别一直躲着了，这么多老朋友在这里，你就不出来见见吗？”空间之主盯着林动的身体，突然说道。
他声音一落，林动体内便是有着温和的光芒散发而出岩闪现出来，他望着眼前这么多熟面孔，也是有些感叹与无奈的摇了摇头。
“岩，开启祖宫阙的事，或许还需要你的帮忙。”应欢欢看着岩，说道。
岩望着应欢欢，嘴巴动了动，但最后还是苦笑一声，微微点头。
炎主见状也是轻咳一声，道：“另外，那大天邪魔王在逃出来的时候，还有着不少被镇压的异魔也是逃了出来，如今的乱魔海，相当的混乱，而且魔狱也没有丝毫约束的想法，看来真如他们所说，天地大战，在这个时候，已经算是拉开序幕了。”
“他们这是想用这些混乱来牵绊住我们呢。”空间之主磨挲着下巴，银瞳之中掠过一抹沉思之色：“我总感觉这些家伙，似乎是在预谋着什么。”
炎主等人也是微微点头，那些异魔数量庞大，实力极强，乱魔海中，根本没有任何单一的势力能够阻挡，若是任由他们横行的话，乱魔海中，恐怕会造成相当大的伤亡。
“让乱魔海组成一个联盟吧。”林动想了想，道，这种时候，这乱魔海如果不将众多势力汇聚在一起，恐怕只会被那些异魔分别吞噬而掉。
“联盟么，倒也是可行…不过操作起来有些麻烦。”洪荒之主点点头，要组建成那等庞大的联盟，必然要消耗太多的精力，他们显然不够。
“不用我们亲自组建，乱魔海中，有着一方顶尖势力，名为炎神殿，其殿主正是火焰祖符的掌控者，而且炎神殿在乱魔海也是颇有号召力，再加上如今异魔为患，只要稍加引导，要组建联盟倒是不难。”林动接着说道。
至于联盟组建后那庞杂的运作，以炎神殿唐心莲的本事，想来没人会比她做得更好了。
“当然，这之中，或许还需要几位出面，在这乱魔海中，毕竟还有着一些底蕴极为悠长的势力以及种族。”
在乱魔海中，最为强大的，并非是人类势力，而是海妖一族，要让他们也是加入到联盟里面来，倒是有点不容易。
“海妖一族么…那我去一趟便可，那些海妖一族中的大族，我与他们先祖都算是有些渊源。”洪荒之主道。
“洪荒之主本身便是海妖中的海巨人一族，他出面的话，想来要让海妖一族加入联盟不是难事。”应欢欢轻声道。
林动看了她一眼，也不说话，只是点点头。“既然如此，那我们明日便动身去炎神殿吧，这一两月异魔肆虐，想来乱魔海中各大势力都是吃了极大的亏，此时号召的话，能够有事半功倍的效果。”林动说道。
众人闻言，倒是皆无异议，而后他们目光看了一眼应欢欢与林动二人，然后便是各自出去，青檀见状，犹豫了一下，也是跟了出去。
喏大的楼中，眨眼便是仅有两人坐于其中，林动望着那垂目不说话的应欢欢，也是无奈的摇了摇头，猛的站起身来，似便是要这般离开。
察觉到他的举动，应欢欢贝齿轻咬着嘴唇，素手紧握，心中有着一抹难受的感觉涌出来。
不过林动最终并未就这样离去，他来到应欢欢身前，伸手握住她那冰凉如玉的纤细小手，微微一用力，便是将其拖入怀中，手臂搂住那柔软的蛮腰，将头轻轻的埋在她那清凉的冰蓝色长发中。
“你的记忆是不是恢复了？”林动轻声问道，今天她的表现，显然是有些不太像平常的她。
“有着一些…所以我认识他们，也记得一些东西。”应欢欢微微犹豫，道。
林动点点头，难怪这些远古的事，她能够清楚的知晓。
“你放心吧，我现在还是应欢欢呢。”应欢欢微微一笑，她显然是知道林动在担心着什么。
“不管怎么样，不要骗我，行吗？”林动点点头，他嗅着传入鼻中的那淡淡幽香，手臂紧紧的环住怀中的人儿，声音略微有些沙哑的道。
应欢欢也是将脸颊轻靠在他的肩膀上，呼吸扑打着林动的脖子，轻轻嗯了一声，旋即又是用仅有自己听见的声音喃喃道：“我也不想骗你啊。”
许久后，林动方才离去，应欢欢则是来到楼阁前，美目望着黑暗下来的海面，海风吹拂而来，将其长发吹得轻轻飘动。
“你真要让他进祖宫阙？”突然有着声音从后方传来，应欢欢偏过头，然后便是见到黑暗之主斜靠着柱子，纤细手臂抱在胸前，黑溜溜的美目望着她。
“他需要凝聚出神宫啊。”应欢欢道。
“想要开启祖宫阙，至少需要六位远古之主联手才行，所以你也必须动用那被你一直压制的力量，而那种力量，被你动用了，想要再压制，恐怕就不是那么容易了。”黑暗之主道。
“你以前的力量可怕是可怕，但却冰冻了你所有的情绪，你确定如果力量复苏，还能够像现在这样的对待他？”
应欢欢在长椅上坐下，她蜷缩着修长的双腿，冰蓝色的长发倾泻下来，令得此时的她看上去略微有些柔弱：“我都知道…可世界上哪有那么多两全其美的事情，魔狱显然是在酝酿着什么，我能够感觉到，那天王殿似乎快要彻底恢复了…”
黑暗之主来到她的身边，轻轻一叹，搂着她冰凉的身子，心中也是有些心疼，从那远古便是如此，那种连他们谁都扛不起的重担，要落到她那柔弱的肩上，她犹自还记得很多年前，第一次看见她时，她还是个活泼可爱的小姑娘，然而后来，她脸颊上的笑容越来越少，特别是在当师傅告诉她，他会陨落，而这天地无数生灵就要依靠她来守护时，他们都是能够感觉到，她开始将所有的情绪冰冻，只为了掌控那种力量…
“其实，我们也挺感谢那个小子的，至少，能让你开心让你忧愁…”黑暗之主喃喃道。
“这件事情，不要告诉他。”应欢欢道：“不然以他的性子，绝不会去祖宫阙的。”
“都这时候了，还为他想着。”黑暗之主无奈的道。
应欢欢靠在黑暗之主怀中，道：“其实，我知道的，他一直心想着超越我，那样的话，就能理直气壮的让我待在他身后。”
“异想天开啊。”黑暗之主撇撇嘴，若这个小师妹真这么容易超越的话，那师傅会将那般重任交给她来么？
应欢欢却是轻笑了一下，她低声说道：“我相信他的。”
黑暗之主一愣，低头看着她，此时的应欢欢，唇角仿佛是有着一抹充满着温柔的弧线微微勾起来，那对美目，明亮异常。
“他会做到的。”
楼阁中，女孩轻轻的声音中，有着一抹不容置疑的坚定。


第1273章 联盟
翌日清晨，林动一行人便是准备出发，而当他们出了楼阁时，却是发现那些天风海域的首领皆是眼巴巴的等在外面，见到他们一出来，顿时围了过来。
“林动小哥，如今乱魔海依旧混乱不堪，据说已经有着一些海域被异魔血洗，你们这样走了，我们天风海域可怎么办啊？”那古统显然是被推出来当代表，面对着眼前这群显赫今古般的存在，显然即便是这些天风海域的诸强也是极有压力，因此都不敢与他们说话，只敢对着相对而言比较熟悉的林动来说。
林动见状，却是忍不住的一笑，看来这些家伙都是被异魔吓怕了啊，这样也好，这种情绪蔓延出去，对于联盟的形成极有好处。
“古家主，放心吧，这片海域不会再有异魔敢过来，另外我们此行前去炎神殿，便是要组建乱魔海联盟，到时候联盟组成，也再不用惧怕异魔。”林动道。
“联盟？”众人听得眼前一亮，如今这天风海域岌岌可危，他们显然没了自保的能力，若是能够联盟的话，于他们而言显然是有着极大的好处。
“等联盟消息出来，你们便派人来炎神殿，到时候各大海域构建挪移阵法，一旦有难，便会有人来救。”林动冲着众人一笑，而后身体之上便是有着银光浮现出来，然后将他们一行人包裹而进。
“所以诸位就等着好消息吧。”
银光逐渐的绽放开来，随着林动最后一句话的落下，他们一行人的身形，也是凭空消失而去，留下那一群眼巴巴的天风海域各方首领。
炎神殿。
一座大殿之中，一道火红的倩影坐于首位此时的大殿中人影绰绰，不断的有着人往来，道道情报飞快的上报而去。
唐心莲优雅而坐，此时的她，或许是因为并未统兵的缘故，只是身着红色长裙，看上去多了一分柔软，少了一分飒爽的英姿。
而她听得那一道道传来的急报声，柳眉也是微微蹙着最近乱魔海的混乱，也是波及到了炎神殿，在炎神殿所在的海域，同样是遭受到了大量异魔的攻击，不过如今的炎神殿随着青雉常驻在这里，也是吸引了不少顶尖强者而来，这也是导致炎神殿实力大涨再加上她的统率之能，那些袭来的异魔仅仅在第一天造成了一些混乱，然后便是被尽数的挡在了炎神殿统治的海域之外。
当然，虽然抵挡是抵挡了下来，但那繁杂的军事，也是令得唐心莲变得繁忙了许多。
大殿中一道道命令飞快的自唐心莲那红润小口中传出，然后大殿中的一些长老，便是领命而去，人来人往，但却是丝毫不显紊乱。
“凌长老，北海城有异魔出没，按照情报来看数量应该在五百左右有一尊异魔将率领，你领一千炎神卫前去将之剿杀，北海城护卫，随你调遣。”
最后一道命令从唐心莲嘴中传出她玉指一弹，便是有着一道火红令牌落到了一名青衣老者手中，后者接过，也是立即而去。
呼。
将这么多事情处理完毕唐心莲也是轻轻松了一口气。
“嘿嘿，小唐唐可真是厉害炎神殿交给你，师傅我真是放一百个心啊。”在那大殿的右侧太师椅上，摩罗端着茶杯笑眯眯的看着唐心莲将这些事情处理得井井有条，脸上笑容就跟一朵花一样。
在他一旁，青雉也是笑着摇了摇头，这家伙，所幸找到一个这么出色的弟子，不然这炎神殿恐怕还真会被他搞垮掉。
唐心莲淡淡的瞥了摩罗一眼，却是未曾理会。
“咳，算了，老青，我们出去下下棋吧。”见到吃了一记白眼，摩罗也是尴尬的笑了笑，然后招呼着青雉。
“师傅，您老人家会不会太悠闲了？您若是觉得没事情做，我可以给您派分点！”唐心莲咬着银牙，冷冷的道。
“不是说有大事再让为师出马吗？”摩罗干笑道。
唐心莲冷哼一声，随着如今炎神殿的扩大，摩罗这甩手掌柜也是越来越干脆了。
摩罗见到唐心莲怨气直线上涨，也不敢在这时候去招惹他，对着青雉使了个眼色，就打算溜之大吉。
青雉见状，也是一笑，刚欲起身，其神色突然一动，笑道：“算了，今天炎神殿有贵客来临，你还是老老实实接客吧。”
“贵客？”
听得此话，摩罗与唐心莲都是一怔。
而就在青雉话音落下间，只见得这炎神殿内，空间突然扭曲起来，璀璨的银光爆发而开，待得银光散去时，数道身影便是出现在了大殿内，那当下的青年，冲着摩罗微微一笑。
“你…林。”
摩罗见到那青年，顿时一愣，还不待他说话，大殿首位上的唐心莲脸颊上却是有着欣喜笑容涌出来：“林动？！”
“青雉前辈，摩罗前辈，心莲姑娘，好久不见。”林动望着大殿中的三人，脸庞上也是有着笑容浮现出来，抱拳道。
青雉与摩罗笑了笑，然后他们那有些凝重的目光便是望向了林动身后的炎主等人。
“若是我所料不差的话，这几位，应该便是传闻之中的远古之主吧？”青雉微笑道，虽然在那远古时，他根本没资格与炎主他们对话，但对于他们，显然还是有些熟悉的，而且这种气息，放眼天地间，除了那几位，恐怕也寻不出其他人了。
“远古之主？”
摩罗也是一惊，如今的他，也算是晋入了轮回境，但他依旧无法感觉到炎主等人的深浅，旋即忍不住的咂咂嘴，不愧是那位符祖大人的亲传弟子啊。
炎主等人也是冲着青雉二人一笑，态度倒是不倨不傲，丝毫没有远古之主的架子。
“你这小子竟然也轮回境了？”摩罗突然注意到林动，面色猛的一变，惊声道。
“前不久刚好突破。”林动微笑道。
“啧啧，还是这么变态…”摩罗啧啧出声，满脸的惊叹，他修炼这么多年，方才在去年晋入轮回境，但哪料到，当初离开时不过死玄境的林动，竟然也是达到了这一步。
“你就别去与他比了，会自卑的。”青雉笑吟吟的道，他目光扫视了林动一圈，旋即微微点头，道：“看来你已经获得洪荒龙骨了。”
“多亏了青雉前辈介绍。”林动道，如果不是青雉让他去龙族的话，恐怕也难以获得这洪荒龙骨。
“这都是你自己的本事，洪荒龙骨我也觊觎了很久，可惜最后没能成功。”青雉摇了摇头，道。
林动一笑，他现在已经能够感觉到青雉的实力，应该是处于两次轮回劫左右，这比起当初所见似乎强悍了许多，看来在这一两年中，青雉实力也是有所进步。
“林动你真的到轮回境了啊？”唐心莲也是从那首座下小跑了下来，她来到林动面前，上下的打量着他，美目中有着难掩的惊喜之色。
一旁的应欢欢看了一眼身着火红长裙，飒爽而妩媚的唐心莲，旋即似笑非笑的看向林动。
林动察觉到她的目光，不由得干笑了一声，冲着唐心莲点点头，然后将身后诸人皆是介绍了一番。
“呵呵，不知道诸位大驾炎神殿是有何事？”摩罗忍不住好奇的道，这般阵容，连他都是感到有点震撼。
“应该是因为最近乱魔海中异魔之乱的事吧。”
唐心莲微微一笑，笑容明艳，她看了一眼林动，道：“若是我所料不错的话，你们应该是打算让我们炎神殿号召乱魔海诸多势力联合对抗那些异魔吧？”
“聪明。”林动竖起拇指：“那些异魔太过厉害，单一的势力根本无法阻拦，若是任由他们肆虐下去，乱魔海损失将为极其惨重，所以我们想要让炎神殿号召联盟，抵挡乱魔海中的异魔。”
“联盟？”摩罗愣了愣，旋即磨挲着下巴，道：“我们炎神殿虽然有些号召力，但也不是所有人都买我们的帐啊，这乱魔海中，还有着不少强横势力的。”
“呵呵，那些势力，自会有他们去帮忙说服。”林动指了指身后炎主等人，道。
摩罗微微犹豫了一下，然后看向唐心莲，道：“你还是问这丫头吧，现在炎神殿殿主是她。”
林动无言，这家伙甩担子的功力越来越厉害了，竟然连殿主之位都已经教给唐心莲了。
林动目光转向唐心莲，后者也是将他给看着，那明亮而妩媚的漂亮眼睛让得他心虚了一下，旋即笑道：“心莲姑娘认为如何？”
“联盟的话，倒的确是能够阻止乱魔海中的动乱，不过我们炎神殿似乎没什么好处啊，我们现在倒是能够守住来着。”唐心莲玉手托着一只纤细的手肘，然后抵着雪白而带着一些尖尖弧度的下巴，微笑道。
林动愕然，这种时候还想着自家的事，似乎有点不妥吧，以唐心莲的精明，不可能会不知道任由异魔肆虐下去，会造成多大的灾难吧？
“唐姑娘与你说笑呢。”应欢欢道。
林动闻言，这才察觉到唐心莲嘴角的戏谑，不由得无奈的摇了摇“你有这么不了解我吗？”
唐心莲看了林动一眼，然后便是将那漂亮的眸子转向了应欢欢，冲着她伸出纤细玉如的小手，似是很随意的笑道：“你是应欢欢吗？早就听林动说起过你呢，有一次我还问起过，我与你谁更漂亮呢。”
林动的身体瞬间便是僵硬下来，目瞪口呆的望着眼中有着狡黠之色掠过的唐心莲。
“哦？”
应欢欢也是浅浅一笑，道：“那他怎么说的？”


第1274章 大会
大殿中，两个同样美丽得耀眼的女孩微笑对视，这本该是令人赏心悦目的一幕，但林动是满脸冷汗，显然是有点搞不明白这局势是个什么情况。
众人眼看着这一幕，先是一怔，旋即也是饶有兴致起来，特别是那黑暗之主几人，更是目光奇特的看着应欢欢，想来对于后者这般反应，极端的有兴趣。
“咳。”
林动干咳了一声，这个时候，总归还是得拿出点男人的威严才行，所以他直接插进两女之间，然后对着唐心莲无奈的道：“我记得从没说过这句话吧？”
“是吗？”
唐心莲眨了眨凤目，红唇微掀，漫不经心的道：“那可能是我记错了吧。”
林动无言，女人啊，真是可怕的生物。
唐心莲笑容明媚，望向应欢欢，道：“你不会介意的吧？”
应欢欢浅笑着微微摇头，但在偏头间，却是对着林动投去一个寒意十足的目光，她可算是好好领教了一下林动这所谓乱魔海的历练是有着多么深厚的艳福了…
“好了好了，说正事吧。”林动无奈，旋即只能板起脸，两女见状，这才相识一笑，各自收敛下来。
“联盟的事，的确是眼下唯一的办法。”唐心莲螓首轻点，道：“根据我们所获得的情报，现在乱魔海中，已经有着一些海域被异魔血洗，不少没有实力的势力，已经开始在往一些大型海域撤退。”
“如果此时我们号召联盟抗魔的话，效果会很不错，不过，到时候众多势力蜂拥而来，那所组成的联盟必定极为的庞杂，能否真正运转起来都是难事，更何况抵御异魔？”
林动也是点了点头，不是什么时候都是人多就好的，这种情况，万一混杂在一起控制不好，恐怕反而将会更糟。
他虽然在妖域整合过四象宫，但他却是明白，与乱魔海这种整体地域的联盟相比，四象宫简直就是小儿科，而且光是四象宫那样，就已经让得他费尽周折，若再让得他来弄乱魔海的联盟，恐怕把他杀了都完不成。
“虽然很麻烦，不过以心莲姑娘的能力，想来不会是难事的吧？”林动一笑，他又不是傻子，这种明知道做不了的事情自然不会往头上揽，而且现在这里，可还有着一个最为完美的人选。
“我已经很忙了，你就能别再为我找事了行吗？”唐心莲白了林动一眼，在见到后者悻悻神色后，又是无奈的叹了一口气，道：“一来就给我带个这么大的麻烦。”
“这种联盟，还需要一些乱魔海真正的大势力加入才能造成声势，人类势力，那万海天宫必不可少，他们是乱魔海中的老牌势力，声名极强，而海妖一族，不死圣鲸族必不可少，你们能搞定这两方么？”唐心莲看着林动等人，问道。
“万海天宫么？他们的创建者，是不是万海老人？如今还活着吗？”炎主想了想，问道。
“嗯，不过万海老人很多年前便是因为渡第二次轮回劫失败从而陨落，现任掌教，是他的孙子，实力应该是渡过了一次轮回劫，也算是乱魔海中顶尖的强者之一了。”
“那应该没问题，万海老人曾经受过我的指点。”炎主淡笑道，这说出来的话，连林动都是滞了滞，旋即无奈摇头，这些老古董，还真是小觑不得啊。
“不死圣鲸族我去吧。”洪荒之主笑道，在海妖一族中，他算是个传奇人物，因为他本身便算是这其中一族，所以要说服海妖一族加盟，倒不是难事。
“既然如此，那我们今日便将消息发布出去，我想，或许很快的，乱魔海那众多实力便是会派人蜂拥而来。”唐心莲点点头，道。
“好。”
众人对视一眼，然后皆是点点头，这般震动乱魔海的大事，便是定了下来。
而随着这般决议定下，整个炎神殿也是彻底的忙碌起来，无数人影掠出，然后开始将消息以一种惊人的速度散播出去，按照这种速度，或许短短两三日内，炎神殿便将会成为乱魔海中最为繁忙的地方。
不过虽然有所准备，但那种联盟所引发的震动，却是有些出乎林动意料，直接是在短短两日之间，便是反馈到了炎神殿中，再然后…炎神殿便是开始被那些犹如蝗虫般从乱魔海各处地域而来的强者所淹没，那种阵仗，比起当年那场因为雷霆祖符而引起的争夺比试，更为的猛烈。
在炎神殿一座楼阁上，林动望着那犹如蝗虫一般，从远处天际掠来，最后涌至城市的人影，也是忍不住的吐了一口气。
“不愧是乱魔海啊。”
林动感叹的道，乱魔海的面积真要说起来，恐怕超越了妖域以及四大玄域，而这里所存在的大大小小势力，也是如同繁星般难以数尽，平常时候或许还不怎么觉得，可一旦遇见这种特殊情况，那种恐怖的数量便是凸显了出来。
当然，这也是从某种角度凸显出如今这乱魔海的局势有着多么的凶险，毕竟乱魔海本就竞争激烈，若不是真到了那种无路可走的情况下，或许谁都不会想到所谓联盟之事。
“我们也去吧。”应欢欢在其身后说道，炎神殿虽然在乱魔海中也算是顶尖势力，但毕竟无法彻底的服众，想要完成联盟，还得需要他们坐镇震慑。
林动点点头，旋即袖袍一挥，银光将两人包裹，待得再度出现时，已在一座巨大无比的广场之前，在那广场中，隐约可见蔓延到视线的人山人海，而且那些气息，每一道都是相当的惊人，显然，来到这里的人，大多都是乱魔海各方势力中相当有实力的代表。
“倒还真是隆重啊。”林动见状，微微一笑，刚欲与应欢欢进场，突然一道清脆欣喜的声音从后方大声的传来“林动大哥！”
林动听得喊声，愣了一下，然后转过身来便是见到一道白色的身影从远处扑来，最后直接撞进了他怀中，那股恐怖的大力，令得他身体都是微微一颤。
林动急忙将怀中的人接住，目光看去，然后一张精致可爱的脸颊便是出现在了眼中：“灵珊？”
林动愕然的望着怀中的少女，这赫然便是当初在乱魔海遇见的慕灵珊，只不过当初的她尚还是一个留着可爱羊角辫的小女孩，而现在，那羊角辫变成了乌黑活泼的马尾，娇小的身躯也是逐渐的玲珑有致起来，再不复当初的那种稚嫩。
“嘻嘻，林动大哥，果然是你呢！”慕灵珊笑嘻嘻的拉着林动，那小脸洋溢着欢快的笑容，显然能够在这里见到林动让得她极为的开心“你怎么来这里了？”林动见到她，心头也是有些欣喜，他笑着摸了摸慕灵珊那泛着青春活力般的乌黑马尾，心中感叹，当年的小女孩，也是长大了啊。
“我和爷爷他们来的。”慕灵珊素白的小手指着后方，只见得在那里，有着两道人影而立，其中一人，身子欣长，模样英俊，赫然便是慕灵珊三爷爷，慕岚。
而在慕岚身侧，则是一名发须皆白的老人，老人身躯单薄，正抚着胡须，含笑的望着他们。
“慕岚前辈。”
林动冲着慕岚一抱拳，笑道，当年慕岚帮过他不少，他心中也一直是记挂着。
“哈哈，林动，现在你的实力连我都是赶不上，这声前辈，我怕是承受不起了。”慕岚笑着走过来，那看着林动的目光中，却是有着一丝震动，当初与林动分别时，后者才不过死玄境的实力，然而现在，竟然连他都是感觉到有些压抑。
“这是我大哥，慕天。”
“见过慕天前辈。”林动笑着抱了抱拳，想来这位老人便是慕灵珊的那位大爷爷了，他身体上的波动，让得林动有些惊讶，看来这位老人家，也是一位渡过一次轮回劫的巅峰强者啊。
“呵呵，你便是林动小友吧，我家这丫头回来后经常提起你，老夫这耳朵都听出茧来了。”慕天温和的笑道。
林动笑着拍了拍慕灵珊小脑袋，道：“慕天前辈也是来参加联盟大会的吧？”
“嗯，洪荒前辈亲自来说，我不死圣鲸族哪敢不给面子。”慕天笑着点点头，道。
“既然如此，那便先里面请吧。”
林动看了下这地方也不是说地方的地，便是一笑，然后将他们一行人引入场中，而在进场时，他目光一扫，只见得无数群强云集，那视线看去，几乎是看不见尽头，只能够感觉到无数道气息在这片天空升腾凝聚，甚至连天空上的云层，都是被生生的冲散而去。
“慕天前辈，请先入位吧。”
他们进来后，很快便是有着炎神殿的人前来指引，将慕天三人带入了广场最前方的席位，在这里，显然是实力为尊，那能够坐在那一排的，几乎个个都是乱魔海中顶尖的强者，声名赫赫。
而在那最前一排的更前方，却仅仅只有数个席位，而此时，炎主等人正闭目而坐，无数道目光扫到他们的身上，都是充满着一种狂热的色彩。
“走吧。”
应欢欢径直走向那最引人瞩目的中心位置，并且玉手还拉着林动。
“咳，我去一边就好了。”
林动见到那场中最瞩目的位置，顿时干咳了一声。
然而应欢欢却是不理他，身形一动，直接是掠进那排主位之上，然后脸颊平静入座，同时还将林动也拉扯着坐了下来。
这排主位，算是整个会场最为耀眼的地方，甚至连那些慕天这些乱魔海顶尖的强者都没资格坐上去，但眼下林动这陌生的面孔一上去，无疑是瞬间将所有的目光都是吸引了过去，紧接着，一道道惊愕的窃窃私语声，便是爆发而开。
“那小子是谁啊？远古八主有这位么？”
“怎么看上去有点眼熟啊？”
“呃…好像是当年在炎神殿那场争霸赛上夺魁的家伙啊，是叫林动吧？”
“是那个小辈么？真是不识大体，他怎么敢坐那个位置！”
一旁的炎主他们倒是微微睁开眼，眼带着一丝笑意的望着受窘中的林动，似是想要看他如何收场一般。
林动差距也到他们的目光，再听得那漫天不满的声音，倒也没有着恼，只是微微一笑，手掌轻轻的在那石桌之上一拍，紧接着，一股恐怖的气息便是陡然席卷而开，瞬间笼罩了整个会场。
在那种恐怖气息的笼罩下，所有的不满之声几乎是瞬间噶然而止，甚至连那最前方的慕天等诸多乱魔海顶尖强者，眼神中也是掠过一抹浓浓的震惊之色。
在那种气息下，即便是他们，都是察觉到了一种危险到极致的味道。
“这个年轻人，不简单啊…”
他们暗中对视一眼，心中皆是掠过这般想法，同时也是将心中的一些质疑，尽数的收敛而起。


第1275章 大会
云集了乱魔海众多顶尖强者的会场，此时也是在林动那陡然间爆发出来的气息下安静了一瞬，不少人眼神变幻，最后凝聚在凝重之上，再度看向林动的目光时，已是不敢再有丝毫的质疑。
能够来到这里的人，大多都算是乱魔海中有名之人，手中实力自然是不弱，因此也是能够凭借着这股气息，猜测出林动的实力，那种程度，必然是踏入了轮回境。
当然，至于轮回境的强弱，就唯有那会场最前方一排的乱魔海顶尖强者方才能够感觉到，他们都算是渡过一次轮回劫的人，但即便是如此，依旧是在林动那股气息下感到一种危险的味道，这让得他们明白，眼前的青年，虽然看上去只是寻常轮回境，但若真要动起手来，恐怕他们根本就不是对手。
“这个林动小友，厉害啊。”慕天抚着胡须，对着身旁的慕岚道。
“是很恐怖。”慕岚也是感叹的点了点头，他实在是有点无法将当年那弱小青年与眼前这般面对着乱魔海诸多强者而气度依旧沉稳的人联系到一起。
“那当然，林动大哥肯定比大爷爷还厉害。”一旁的慕灵珊骄傲的道。
慕天见到这孙女胳膊拐得厉害，也只能无奈的摇了摇头，然后他偏过头，与不远处的一名身着蓝袍的老人互相笑了笑。
那是万海天宫的现任宫主，万玄，没想到连他也来了。
“这大会的规格，还真是有些恐怖啊。”慕天环顾一场，然后便是发现乱魔海十之七八的顶尖强者，都是到场，这种情况，几乎算是乱魔海千载之内首次所见。
“如今乱魔海局势动荡，异魔横扫而来，我们不死圣鲸族暂时避开了锋芒，但也不是长久之计。”慕岚道。
“是啊。”
慕天点了点头，旋即他望着这会场中无数群强，即便是他老迈的心中都是涌起一股豪情，若是能够促成联盟，又何惧那些异魔？
烈日高悬，一道钟吟之声，也是在庞大无比的会场之中响起，而后满场开始寂静，一道道目光望向场中，在那里，一道火红妖娆的倩影亭亭玉立，正是炎神殿唐心莲。
“诸位，想来大家都已明白炎神殿招集大会的原因，如今异魔横行，远古天地大战已是隐有端倪，异魔之强，或许在座不少人都已经领教过。”
唐心莲美目扫视，清澈的声音回荡在全场。
“从我炎神殿得来的情报之中，乱魔海已有八处海域被异魔血洗，其中一百二十方势力被抹除，他们所过之处，尸横遍野，血流成河，我想，恐怕根本没有任何单一的势力，有着资格说能与他们抗衡吧？”
无人回答此话，所有人面色都是有些沉重，不少人还隐有悲戚，显然他们已是在异魔的进攻中，付出了极大的代价。
“不知道唐姑娘有何想法？”那身着蓝袍的老人，微微沉吟，道。
“说难也不难，简单说来，无非是联盟而战。”唐心莲明媚一笑，旋即道：“我炎神殿受五位远古之主所托，在此请诸位组成联盟，共抗异魔！”
庞大无比的会场中，发出一些嗡鸣之声，不少强者面面相觑，他们来时就隐约的猜到一些，不过当亲耳听见时，还是有点震撼，这种囊括了整个乱魔海的超级联盟，可实在是前所未有。
嗡鸣声在场中传开，不少强者都是有些意动，但却暂时都还未说话，虽然现在被异魔逼得有些惨，但如今的势力都是辛苦创建，若是联盟一成，还能有他们掌控的余地吗？
“诸位，所谓联盟，仅仅只是在抗魔一事之上，而联盟一旦形成，便是共同进守，到时候为了防止专权，我们可以推荐出一些有名望的势力组成决议团，联盟大事，都得经过决议团，另外，在决议团之上，设元老席，由五位远古之主以及林动担任，行使监督之责。”
“而到时，各方派出精锐，组成除魔军，而且空间之主也会出手，将各大海域设置挪移阵法连接，那时候只要任何一方海域遭受异魔攻击，除魔大军便是能够即时赶到！”
唐心莲傲然而立，清澈之声传开，原本会显得庞杂的联盟运作，却是在其手中丝毫不乱，反而最大化的将其运转起来，而且权利分化，也是绝了一些势力怀疑炎神殿试图一家独大的想法。
庞大的场中，鸦雀无声，无数人眼神闪烁，心中却是有着一些激荡在涌动，这种联盟的话，倒的确是可行的。
对于五位远古之主形成的元老席，倒是无人反对，以他们那般地位声望，足以胜任，只不过他们看向林动的目光有些奇特，毕竟这里，即便是慕天这种乱魔海顶尖强者都没资格进入元老席，而他却是能够凭借这般年龄进去，真是让人不得不为之震撼。
林动听得同样暗暗点头，一旁的炎主等人眼中也是掠过一抹细微的赞色，这唐心莲统率之能的确相当厉害，想来以她的能力，一旦联盟组成，这乱魔海也将会爆发出相当恐怖的力量。
“诸位，我们皆是赞成此法，你‘为如何？”炎主缓缓的开口，他那略显低沉的声音，让得场都是安静了下来。
“我们之前已是与魔狱交过手，或许乱魔海如今的动荡，仅仅只是第一波而已，很快的，异魔就会席卷整个天地，到时候，便是第二场天地大战真正拉开帷幕的时候。”
“倾巢之下，焉有完卵？此时再不联盟，这片天地，无数生灵，必遭涂炭。”
听得炎主那最后的低沉之声无数强者心头都是一跳，一种浓浓的不安涌上心头，若真是天地大战开启的话，恐怕谁都避免不了。
“另外，为了提升抵御异魔的力量我们将会开启师傅当年所留下的“祖宫阙”，到时候会让达到转轮境的强者进入其中，若是有所机缘的话，应该能助你们突破到轮回境。”
炎主这句话，瞬间让得场中暴动了起来，无数停留在转轮境的超级强者霍然起身，目光泛着狂热的盯着炎主。
场中的气氛达到顶点，这对于他们的诱惑，实在是太大太大了。
“炎主大人说得不错我万海天宫，愿意联盟。”那蓝袍老人见时机成熟，率先起身，抱拳恭声道。
“我不死圣鲸族，也愿联盟，共抗异魔！”慕天也是起身沉声道。
“我天海崖同意联盟！”又是有着一名乱魔海顶尖强者起身。
“海剑阁愿意…”
“我们也愿意联盟！”
庞大无比的会场中，一道道身影猛的站起，低喝之声此起彼伏的响彻而起，最后那些声音犹如是汇聚在一起，冲上云霄，一种凛然战意悄然凝结。
林动望着眼前这有些令人热血沸腾的一幕，也是忍不住的轻吐了一口气，联盟之事，有着炎主他们坐镇，再加上来自异魔的压迫，倒是这般不费多少力气的促成成功，不过，想要真正的将这个联盟发挥出力量，还是得看唐心莲的了…
联盟之事成功促成，而接下来，便是极为繁琐的整合之事，不过这种整合与四象宫那种不同，这种超级联盟，显然是更为的复杂，但好在的是，不用林动来操心。
这些事，都是落到了唐心莲的身上，而她也是展现出了让人惊讶的能力，短短五日时间，联盟已是初具雏形。
在这几天中，林动与空间之主等人同时出手，前往各大海域构建挪移阵法，彼此相连，这样只要待得联盟除魔大军招集成功，便是能够开始真正的抵御下异魔肆虐。
而这般高强度的工作，即便是林动如今晋入了轮回境，依旧是有些累得够呛，待得他回到炎神殿时，脸色都是有点发白。
大殿内，正在忙碌着联盟事宜的唐心莲见到林动，也是一笑，然后亲自送上香茶，道：“辛苦了。”
“给你带了这么大的麻烦来，这些辛苦算什么。”林动接过香茶，摇了摇头，道。
“这可不算是麻烦，这种事说来又不是你一个人的，异魔在乱魔海肆虐，迟早会波及我们炎神殿。”唐心莲笑道：“倒是你，又不是远古之主，还来操心这些事。”
“我倒的确没他们那种大义。”林动苦笑一声。
“他们的确很令人敬佩，不过你也不赖的啊。”唐心莲微微一笑，那狭长的眸子透着一股妩媚，此时的她倒没了平常的飒爽英气，脸颊上的笑容，显得柔美了许多。
她盯着林动目光，显得有些温柔，这很难将那在乱魔海无数豪强面前自信得将他们尽数说服的骄傲女人联系到一起。
“对了，心晴怎么样了？你没欺负她吧？”唐心莲突然想起当初陪林动去妖域的心晴，道。
林动无奈的摇了摇头，然后将九尾族的事说了一番。
“没想到这小丫头还有这番际遇，等她出关的时候，恐怕也是很了不得了。”唐心莲有些讶异的道。
林动笑着点了点头，刚欲说话，大殿中再度有着空间波动散发出来，而后空间之主，炎主，应欢欢等人也是闪现而出。
“挪移阵法基本都布置得差不多了。”空间之主冲着两人一笑，道。
“麻烦几位了。”唐心莲连忙道。
应欢欢优雅的坐在椅子上，纤细玉手握着翡翠般的玉杯，沉默了一下，然后冰蓝眸子盯着林动，道：“明天我们要开启祖宫阙了。”
林动感觉到大殿中突然静了一下的气氛，再看着应欢欢带着浅笑的脸颊，心中微堵，但最终还是轻轻点头。


第1276章 开启祖宫阙
翌日，炎神殿半空，数道身影凌空而立，而在四周，无数强者远远的看着，他们的眼中，弥漫着一种难掩的激动之色，显然，经过一些渠道，他们已是知道，就在今天，那六位远古之主便是会联手开启由传说之中那位符祖大人所留下来的“祖宫阙”。
据说，在那“祖宫阙”之中修炼，能够有着极大的几率突破，特别是转轮境的强者，更是能够借其迈出那最为重要的一步，晋入轮回之境！
这种近乎恐怖的能耐，足以让得无数人眼睛发红，恐怕也唯有符祖那等传说中的人物，才能够留下如此神奇的东西。
天空中，应欢欢六人静静而立，虽然没有太过惊天般的声势，但任谁都是能够感觉到那平静之下所涌动的惊涛骇浪。
林动站在距他们不远处的地方，在其身后，青雉，青檀，摩罗，唐心莲也是将目光投射而去。
“开始吧。”应欢欢看了五人一眼，轻声道。
黑暗之主五人闻言，对视了一眼，而后眼神中掠过一抹复杂之意，微微点头。
应欢欢玉手一招，便是有着一道温和光芒自林动体内掠出，最后化为一枚祖石悬浮，在那祖石之外，岩的身形也是浮现了出来。
“你真要开启祖宫阙了？”岩看了看应欢欢，犹豫了一下，问道。
“都这时候了，还能有假不成？”应欢欢淡淡的道。
岩苦笑一声，无奈的点点头，身形一动，便是钻进祖石中，而祖石也是落到了应欢欢手中。
应欢欢把玩着温润而古老的祖石，她那冰蓝色的美目中掠过一抹微茫之色，旋即偏过头，冲着那不远处的林动微微一笑。
林动见到她的笑容，却不仅没有欣喜，心中那抹不安反而是更为浓烈了，察觉到这抹不安，他突然掠出，出现在应欢欢身旁，一把抓住她的皓腕，道：“现在可以终止吗？我自己可以修炼的。”
应欢欢美目微垂，道：“祖宫阙是师傅留下来帮助我们对抗异魔的宝贝，它能够让我们这个阵营的战斗力提升，所以，不仅仅是为你，开启祖宫阙，也是为了让我们赢得这第二场天地大战。”
林动微微咬牙。
“放心吧，我不会有事的。”应欢欢微笑，旋即她伸出纤细双臂，轻轻的将林动环抱了一下，然后玉手一扬，手中的祖石便是暴掠而出。
“开始。”
祖石之上，突然有着强烈的光芒爆发而出，这些光芒在天空上交织，隐隐的，似乎是化为了一座庞大无比的巨阵。
“该你们了！”岩的声音自祖石中传出。
“火焰！”
炎主率先点头，他面色凝重，脚掌轻跺，只见得滔天般的火焰自其体内席卷而出，那火焰乃是由元力所化，但却是有着极高的温度，其中弥漫着火焰祖符的力量。
他袖袍一挥，只见得那恐怖的火焰元力便是直接冲进了庞大的阵法之中，顿时令得阵法变得凝实了一些。
“空间！”
空间之中也是大喝出声，银色元力，喷薄而出。
“黑暗！”
“洪荒！”
“雷霆！”
又是三道同样恐怖的能量呼啸而出，最后互相交织，在那等可怕的能量下，甚至连天地都是出现了颤抖。
五道恐怖的能量灌注进入光阵之中，只见得那光阵顿时爆发出了璀璨强光，在那强光中，隐约可见一座古老宫阙。
炎主五人出手完毕，那目光也是尽数的投向了应欢欢，她银牙轻咬，玉手之中，一枚古老的寒冰符文浮现而出，旋即她那本就冰蓝的美目，更是在此时变得寒冷许多。
“寒冰！”
冰彻得仿佛足以令空气都凝固的声音，自应欢欢嘴中传出，旋即她玉手一扬，铺天盖地的雪白寒气弥漫而出，竟是化为一头寒冰巨凤振翅掠出，而后冲进阵法之内。
随着那寒冰巨凤的冲进，那阵法之中原本的五道能量，竟是被逼退了一些，不过很快的，这六道恐怖的能量便是在六人的操控下汇合在一起，最后彻彻底底的与阵法相融。
轰隆！
而就在那六道能量相融时，只见得那庞大无比的阵法无法剧烈的颤抖起来，光芒四射间，那模糊的古老宫阙，也是开始越来越明显。
应欢欢六人，持续着恐怖的能量灌注，直接是令得天地间的元力都是疯狂的沸腾起来。
“凝！”
突然，应欢欢六人眼神一凝，双手结出一道奇妙－印法，一股奇特的波动，也是随之传荡而出。
嗡嗡！
随着那股波动传进庞大阵法之中，只见得那阵法开始剧烈的颤抖起来，光芒逐渐的散去，而后白雾升腾间，一座庞大的古老宫阙，竟然便是这般凭空的出现在了那庞大阵法之上！
古老宫阙相当的古朴，没有丝毫奢华，在那宫阙周围，布满着玄奥而晦涩的符文了种无法形容的奇异波动，荡漾开来，隐约间，仿佛是有着仙齐鸣。
“这便是祖宫阙吗？”
无数道目光狂热的望着那出现在阵法之中的古老宫阙，这便是那位传说之中的符祖大人所留下的吗？
在宫阙之外，光阵笼罩，形成天然的阻碍，将所有人都是阻拦在其外，显然，这种地方，也不是什么人都可以进入的。
“祖宫阙。”
林动也是抬头盯着那古老宫阙，然后转头看向前面的应欢欢，快步上前，有些担心的问道：“你没事吧？”
应欢欢俏脸略微有点苍白，她冲林动笑着摇了摇头，纤细小手不经意的收入袖中，然后道：“你跟我进去吧。”
说完，她便已是对着祖宫阙掠去，林动见状，微微犹豫，然后也是紧跟了上去。
应欢欢靠近宫阙，玉手中的寒冰符文闪烁了两下，只见得那阵法所形成的光幕，便是缓缓的撕裂而开，应欢欢身形一动，便是掠入其中。
林动紧随着进入，而当其脚掌刚刚落到那祖宫阙中时，便是感觉到身体仿佛都是变得沉重许多，他惊讶的抬起头，这才发现，在这祖宫阙中，竟是有着无数细小的雨滴降落下来，这些雨滴犹如能够漂浮一般，并不落地，林动仲手接过一滴，然后心头便是一震，因为他感觉到，这些雨滴中，竟然蕴藏着轮回波动！
“好可怕的手笔。”
林动面色震动，难怪说在这祖宫阙中修炼能够加大晋入轮回境的成功率，原来是有着这等奥妙。
“这只是祖宫阙的外围。”
应欢欢轻声道，步伐却是不停，直往祖宫阙深处而去，林动也是步步紧随。
两人穿梭过重重庭院，最后来到祖宫阙深处，这里是一片碎石广场，广场四周，布满着葱郁树木，而且那些树木之上，也是有着极端雄浑的轮回波动散发出来，这让得他震惊不已，真不愧是符祖留下来的东西啊。
林动视线扫过四周，然后便是停在了碎石广场中央，只见得那里，有着一尊石像，石像似乎手持着石杖，不过不知为何，林动看去，却是无法看清他的模样，只觉得在其脸庞处，有着无比玄奥的光芒涌动，阻绝了任何人的探测。
而就在林动注视着这神秘石像时，却是突然发现体内的三道祖符都是发出了嗡鸣之声，那是一种尊崇的声音，这让得他心中震惊更浓，能够让得祖符都是表露出这种情绪，这天地间，除了那一位之外，还能有何人？！
“这是…符祖大人？”林动小心翼翼的问道。
应欢欢微微点头，然后她来到石像之前，轻轻跪拜而下，而林动见状，也是在其身旁跪拜而下，不提符祖那等实力，就是他守护这片天地的那份心念，就足以让得林动诚心跪拜。
“师傅。”
应欢欢望着石像，眼中有着悲痛以及怀念之色涌动着，许久后，她方才对着石像磕了三个头，这才盈盈起身。
“石像内，有一道师傅开辟的修炼之所，待会我便将你送入其中，除非成功凝聚神宫，不然你就出来不得。”应欢欢道。
“呃？”林动一愣，苦笑道：“这也太狠了吧？”
神宫可不是那么好凝聚的，不然的话，也不会连远古八主中，也就只有冰主成功凝聚。
“这么没有信心吗？”应欢欢冰蓝美目盯着林动，道。
林动深吸一口气，旋即缓缓点头，伸出手臂将应欢欢揽进怀中，轻声道：“放心吧，我说过，一定会超越她的。”
应欢欢将脸颊轻轻的靠在林动肩膀上，道：“在石像中的修炼之所中，时间是外界的十倍，你之前实力精进太快，可以借助于此将实力稳固。”
林动点点头，这符祖大人还真是手段通天，竟然连这种扭曲时间的逆天之事都能够办到。
“进去吧。”
应欢欢玉手轻扬，只见得那石像突然蠕动起来，然后一个光圈，浮现出来，在那光圈之内，仿佛是无尽虚空。
林动见状，也是重重的点了点头，也不再有丝毫的拖沓，转身大踏步的走出，然后便是在应欢欢的注视下，走进光圈。
嗡。
光圈震动着，而后悄然的散去，应欢欢见状，这才深深的吐了一口气，接着她将小手自袖中仲出，她望着这变化的右手，美目中，也是掠过一抹复杂之色。


第1277章 修炼之路
应欢欢低头看着自己的右手，此时，那原本白皙如玉的小手，已是变得晶莹剔透，看上去就犹如一块万载玄冰一般，而且在其上，还有着一道道犹如花一般的冰纹铭刻着，她手掌微微握了握，感受着那右手中涌动的可怕力量，她眼神却是愈发的复杂。
真是很熟悉的力量啊。
“唉。”
一声叹息自她身后响起，应欢欢微偏过头，然后便是见到站在后面的黑暗之主，她望着应欢欢那晶莹如冰般的小手，苦笑一声，也不知道是该遗憾还是欣喜。
“还好，只是一只手臂变了点。”应欢欢冲着黑暗之主微微一笑，道。
黑暗之主走近过来，纤细玉手握着应欢欢那犹如玄冰般的剔透的小手，一种可怕的寒气顿时顺着掌心传来，即便是她的实力，都是感觉到掌心刺痛。
“这种力量，一旦松开压制，便是会逐渐的弥漫出来，最后扩散到你的全身，到时候…你就又得跟以前一样了。”黑暗之主喃喃道。
应欢欢沉默了一下，道：“也不用这么担心，说不定就算那股力量恢复，我依然能够保持着心性啊。”
黑暗之主脸颊上笑容有些苦涩，她见识过当年应欢欢在使用那种力量后是什么结果，她的情绪似乎完全被冰冻，冰冰冷冷，让人看着就觉得有些身体发寒。
“他出来后，见到你这样…”黑暗之主看了一眼石像，叹道。
应欢欢那冰蓝美目也是盯着石像，脸颊上浮现一抹浅笑，道：“这个傻子，总想着超越我…不过，真当我有这么容易超越啊。”
话到最后，她唇角的笑容，已是分不出是苦涩还是什么，原来，有时候太出色了，也会让人这般的无奈。
黑暗之主也是深有同感的点点头，超越小师妹？这真的很困难，他们八人中，冰主是符祖最晚收的弟子，但成就却是最大的，同时也是符祖唯一开口说，她或许有着资格达到他那一步的人。
超越这种连说是天才都是不及的天大妖孽，谈何容易？
“不过希望这家伙能够凝聚神宫吧，不然的话，那就真是没一点机会了。”黑暗之主道。
“他可以的。”应欢欢微微一笑，旋即她取出一个纤细的雪白色长袖手套，将她那有些异变的小手藏了进去，而后指尖轻划，一道寒冰符文便是落到手套上，然后隐匿进去。
而随着那符文的隐匿，那自应欢欢玉手中散发而出恐怖寒气，则是逐渐的收敛起来。
黑暗之主见状，只是摇摇头，也不好再多说什么。
“接下来怎么办？”有着雄浑的声音响起，只见得炎主几人也是进到此处，然后对着应欢欢问道。
“既然乱魔海联盟成形，想来依靠他们的力量对抗那些异魔不成问题，我们…就去找那些家伙吧。”应欢欢挽起冰蓝色的长发，那美目之中，却是有着许些寒意涌动。
“我感觉他们应该是在预谋什么，所以还是将他们找出来为好，他们放纵异魔肆虐，无非是想要打乱我们的视线，并且让我们无暇多顾。”
炎主几人闻言，皆是点了点头。
“另外，这祖宫阙，也挑选一些有潜力的转轮境强者进来修炼吧，天地大战再启的话，我们也需要尽可能的提升整体实力。”应欢欢想了想，微偏着头，道：“让青檀进里宫修炼。”
那里宫算是祖宫阙比较重要的地方，虽然不及这石像中能够逆转时间的修炼场，但也远非其他地方可比，青檀与林动是兄妹，她自然是要照顾一些。
“走吧，我们也该动身了。”
说完这些，应欢欢也是率先走出，而要在转角时，她再度转过头，深深的看了一眼那石像，带着雪白手套的小手，轻轻紧握。
“林动，你一定会成功的…我等着你。”
当林动在踏入石像之中时，他能够清晰的感觉到天地倒转，斗转星移，淡淡的眩晕之感涌上脑来，而当他将这丝眩晕驱逐而去时，却是发现眼前已是大变了模样。
出现在林动前方的，是一座古老的祭坛，祭坛四周，有着高耸入云般的石梯，在那石梯的最顶部，似乎是有着一道破旧的蒲团。
在祭坛的周围，则是无尽的虚空，视线看去，仅仅只能看见如墨般的黑暗，祭坛在这虚空中，犹如飘荡的蜉蝣，显得格外的神秘。
林动走上石梯，然后一步步的登上祭坛，来到那顶部位置，那道破旧蒲团犹如杂草编制，很是不起眼，但林动却是在这上面察觉到一股仿佛凌驾于天地般的残留气息。
那是…符祖所留！
林动面色郑重，这天地间，从古至今，能够拥有着这般气息的，除了那位符祖大人之外，恐怕就真找不出第二位了。
这里，应该是那位符祖大人曾经的修炼场。
“要在这里修炼出神宫么…”
林动捎了捎头，感到有点压力，虽然如今的他精神力晋入大符宗境，但他却是明白，这一境界与凝聚神宫之间，究竟有着多么恐怖的差距，“不过…再困难也得完成啊。
林动抿着嘴，在应欢欢开启祖宫阙后，他隐隐的感觉到一点不对劲，如果他没料错的话，这样做对她必然是有着某种的损伤，只不过她是故意的在瞒着。
“我知道超越你很困难，不过…这样就放弃，可不是我的性子啊。”
林动笑着，旋即他便是自那破旧的蒲团上缓缓的盘坐而下，双目微闭，心神逐渐的沉入泥丸宫中，所谓凝聚神宫，其实简单说来，便是将这泥丸宫改造成神宫，只不过这一步，从某种角度来看，基本是不可能的事情而已。
泥丸宫乃是天生，而且也极为的奇特，它不似丹田那般能够清晰触及，泥丸宫一般处于游荡状，它或许只是一个极为细微的光点，但那之中，却是蕴含着一个人的精气神。
丹田被破，或许还有活路，可一旦精神力被人抹除，那么就将会成为一具仅有身体，没有意识的植物人，而凝聚神宫，就很有可能会造成泥丸宫损伤，从而伤及精气神，到时候，神宫未凝聚，其本身就已变成了植物人。
这种情况，古往今来，并不在少数，这也是很多精神力达到大符宗的巅峰强者，不敢去触及那一步，因为那一步，一般都是九死一生。
不过，那种惧意，显然无法动摇林动的决心，世间力量都是伴随着风险，只是若连尝试接触的勇气都没有的话，那么在强者道路上，他也注定无法走远，更何况，想要去超越冰主…
林动静静的盘坐，心神却是进入了另外一番天地，在那里，雷霆凝聚，飓风吹拂，冰雹如雨，犹如洪荒时期那残酷情形一般。
这便是林动的泥丸宫。
他的心神漂浮在这狂暴的泥丸宫中，然后心神微动间，便是有着精神力凝聚而来，最后化为一道精神体，那番模样，与林动一般无二。
林动的精神体打量着这狂暴的泥丸宫，旋即淡淡一笑，他能够感觉到这里涌荡的浩瀚精神力，但想要凝聚神宫，显然这些精神力还少了一些凝实以及韧性。
以往他注重元力，精神力的修炼倒是略微有些放松，如今想要凝聚神宫，自然是必须将这一步的修炼补回来。
这需要一个不短的时间，不过幸运的是，现在的他，时间还算充裕，这修炼场中，时间比外界缓慢十倍，这里十年，外界仅仅一年，所以，林动可以用这些时间，来好好将以前所落下的精神力课程，尽数的修炼。
呼。
林动的精神体吐出一团白气，然后也是盘坐而下，在其四周，狂暴的雷霆以及飓风不断的呼啸而来，但却始终未曾落到他的身体上。
随着入定，泥丸宫内，那浩瀚的精神力，突然凝聚成一丝一缕，然后犹如白龙一般，钻进林动的鼻息之中，半晌后，再度钻出，不过在钻出来时，那缕精神力，则是变得比先前更为的凝实，其上淡淡的光芒闪烁。
伴随着林动这般吞吸之间，越来越多的精神力源源不断的涌来，最后仿佛一团浓浓的白雾，将林动的身体尽数的包裹。
而林动则是静静的盘坐，身形犹如磐石一般。
想要将这里的所有精神力淬炼一番，显然是需要一个相当漫长的时间，而林动也是心知肚明，心中没有丝毫的焦虑，他彻底的沉下心神，心境通明，有时候，他甚至会忘记所处的位置，只是惯性般的将精神力吞纳，而后淬炼…
狂暴的泥丸宫内，时间悄然的流逝，那道身影依旧纹丝不动，他的身体上，仿佛是布满了尘埃，转眼间，已是一年时间过去。


第1278章 动荡
泥丸宫内，当林动缓缓的睁开紧闭了一年时间的双眼时，那狂暴无比的天地，仿佛都是在此时安静了下来。
漫天的雷霆，不知何时化为道道雷河，横贯天空，飓风也是柔和下来，轻轻的吹拂着，冰雹化为雪花，清澈而动人。
整个泥丸宫，就犹如林动此时那清静的心境一般，宁和得令人感到心旷神怡。
林动注视着宁静的泥丸宫，旋即微微一笑，身体之上布满的尘埃尽数的抖落而下，尘埃落地时，也是化为虚无散去。
他那漆黑的眼眸中，仿佛变得深邃了许多，令人有种沉迷其中的感觉。
他缓缓的仲出修长的手掌，指尖轻漩，旋即一道由精神力所化的雷河便是呼啸而下，缠绕在身边，雷河奔腾间，只见得在那其中，竟是有着无数鱼儿跳跃。
这些鱼儿，全都是精神力所化，但却栩栩如生，犹如活物。
雪花也是飘落而来，在林动前方堆积成一道身影，那模样与林动相同，雪人林动抖了抖身体上的雪花，然后冲着林动笑了笑，对着他伸出手掌。
林动也是伸手，与其相握，而后雪人悄然的化去，留下一手的冰凉。
林动脸庞上的笑容，却是显得愈发的清静宁和，他这一次，在不知不觉间，竟是修炼了将近一年时间，不过这一年，于他而言，却是好处巨大。
如今泥丸宫内的精神力，再没有以往那种狂暴，而是随着林动心念转动间变幻，林动若是心静，这里便是如臂指挥，心念所到处，精神力滚滚而至。
而且，如今泥丸宫内的精神力，比起以往，显然是凝炼了无数，若是说大符宗之内也是有着境界之分的话，那么现在的林动，应该也算是处于大符宗顶尖的层次。
不过，即便是如此，林动对于修炼出神宫，依旧是没有太大的信心，因为那一步，着实是有些虚无缥缈了一些。
当然，再困难，也总是得拥有着尝试的勇气。
呼。
这般想着，林动也是轻轻的吐出一团白气，白气中，仿佛是有着雷芒闪烁，旋即这片大地突然的颤抖起来，他所盘坐之处，竟是开始缓缓的上升，那就犹如擎天之柱破出地面一般，最后高耸入云。
而林动便是盘坐在这泥丸宫最顶处的位置，俯览着雷河，雪花以及大地，他并没有立即着手，而是静静的看着这片泥丸宫。
而这一看，便是将近一月时间。
他安静的看着雷河流转，冰雪飞舞，轻风拂动，仿佛旁观者一般的注视着这属于他的泥丸宫，这里是他精神力诞生的地方。
待得一月时间抵达时，林动那静如深潭般的眸子，方才轻轻波动了一下，然后缓缓闭目，心神尽数的融入这泥丸宫中。
轰！
平静的泥丸宫，在此时瞬间暴动，只见得那先前还宁静的雷河突然愤怒咆哮，犹如怒龙一般在天地间呼啸着，而飓风冰雹再度成形，疯狂的飞舞着。
而林动便是安静的坐于那等狂暴之中，许久后，其心神运转，只见得那雷龙，飓风，冰雹，竟是被他强行的糅合在一起。
轰隆！
狂暴的精神力疯狂的蔓延出来，浩瀚的精神力糅合在一起，然后不断的变幻着形状，隐隐的，仿佛是有着一座宫殿雏形成形一般。
砰！
不过，就在那宫殿雏形隐隐浮现时，那等狂暴的精神力终是不堪压制，彻彻底底的爆炸开来，那神宫雏形，也是瞬间烟消云散。
林动的身体，也是在此时猛的一震，但他却是没有丝毫的犹豫，心神运转，再度催动精神力，尝试凝聚。
砰！
不出意外，精神力再度爆炸。
继续。
砰！
失败！
砰！
砰！砰！砰！
泥丸宫内，林动仿佛不知疲倦的催动着精神力，不断的尝试着，但却尽数失败，显然，这凝聚神宫的困难程度，的确是远远的超出了他意料。
不过，这重重失败，到也是激起了林动心中的执拗，他在那一次又一次的失败中，寻找着失败的原因，而后尽可能的将其弥补，偶尔会有停滞，但隐隐的，却是比之前那一次，要稍微显得完善一些。
这种在失败中寻找破绽，虽然算是一个相当笨的办法，但不得不说，在林动完全不知道如何来凝聚神宫的情况下，却是最为妥善之法。
只不过，这似乎同样需要相当漫长的摸索时间。
而林动却是将这种顾虑尽数的抛弃，如今的他，只是将所有的心神都是沉侵在如何凝聚神宫之上，这里的时间，足够他挥霍。
泥丸宫内，精神力的爆炸声，一直是在持续着，谁也不知道何时才是尽头，那道盘坐在最顶处的身影，犹如入魔一般，沉醉那精神力的糅合与分裂之中，不知时间。
光阴悄然流逝，那道身影，依旧未曾停歇。
修炼场中，时间流逝，而在那外界，时间相对而言，却是略显缓慢，不过，在这缓慢中，天地间，气氛却是有些动荡而起。
随着乱魔海联盟的成功组建，肆虐的异魔，终于开始被逐渐的抵御下来，那由无数势力之中顶尖强者汇聚而成的除魔大军，算是成为了乱魔海中最为恐怖的一股战力。
这股战力，在唐心莲的手中，更是发挥出了无以伦比的威力，异魔的肆虐，再也难以取得之前那般势如破竹般的效果，双方之间，逐渐的在乱魔海形成僵持之势，谁也无法取得绝对的上风。
而应欢欢等人在见到乱魔海局势能够依靠联盟独自支撑后，也是开始动身，他们的目标，是隐匿在这天地间的魔狱最高层，只有将他们解决了，才能真正的避免或者结束那第二次的天地大战。
不过，让得他们有些奇怪的是，魔狱高层，随着那次在乱魔海露面后，竟是彻彻底底的消失而去，任由他们如何的探测，都是未能寻出踪迹。
而这种平静，却并没有让得应欢欢他们有着丝毫的松气，他们与异魔打过太多的交道，很清楚这些家伙的狡诈凶残，他们之所以不露面，必然是在筹划着什么。
这无疑是让人有点不安。
东玄域，一座山峰上，应欢欢站于山巅上，她那冰蓝色的美目望着远处，在那山脉之中，一座巨大的宗派矗立着，隐隐能够看见无数身影闪烁。
那是道宗。
自从离开乱魔海后，她一直是在与炎主他们探测魔狱的踪迹，回道宗的时间变得少了很多，对于这种情况，她隐隐的有些歉疚，偶尔神情恍惚间，能够想到在那多年前，那时候，林动也还在道宗，那段时日，是她这些年最为开心的时候。
只不过，现在，似乎是有点物是人非的感觉。
“嗡。”
在她眺望着道宗时，她身后的空间突然闪烁了一下，银光浮现，空间之主便是浮现出来，他望着应欢欢，沉声道：“出事了。”
“什么？”应欢欢偏过头，问道。
“妖域也出现了大量的异魔以及被他们操控的魔尸，现在整个妖域都被波及。”空间之主道。
应欢欢柳眉微微一簇，道：“他们这是想要不断制造麻烦拖延住我们。”
“嗯。”空间之主点点头，无奈的道：“但我们又的确不能坐视不管。”
“为今之计，只能让妖域也效仿乱魔海，自成联盟，抵御异魔。”应欢欢沉吟了一下，道。
“嗯…不过，这些异魔的数量，似乎是有些庞大了一些，他们这些年，繁衍得有这么厉害吗？”空间之主犹豫的道。
应欢欢闻言也是微微一怔，旋即似是猛的想起什么，俏脸忍不住的有些变化。
“怎么了？”空间之主见状，忙问道。
应欢欢眸子之中满是冰寒，缓缓的道：“我想…他们恐怕是找到第三座镇魔狱了。”
“第三座镇魔狱？”空间之主面色也是微变，那一座镇魔狱，当年是由生死之主镇压，但后来生死之主晋入轮回，从而也是令得镇魔狱的封印地点失去，这连他们都不曾知晓，难道被魔狱找到了？
“那现在怎么办？”空间之主皱眉道，那第三座镇魔狱中虽然没有类似大天邪魔王这种厉害角色，但数量却是最为的庞大，如果这被魔狱掌控，那真是一个大麻烦。
“生死之主尚无音讯，最好是让她自然觉醒，若是强制召唤的话，难免对她有些损伤。”应欢欢轻摇螓首，道：“先将妖域的麻烦解决下来，到时候，我们联手开启天眼，探测这天地。”
听得要开启天眼，空间之主神色也是一凝，旋即重重的点了点“那我们现在就动身去妖域吧。”
“先等等。”应欢欢犹豫了一下，美目却是看向北方，那里似乎是大炎王朝所在的方向。
“我先去一趟大炎王朝…那个，你们都陪我去。”
她踌躇着，然后脸颊破天荒的红了一下，这样说着。


第1279章 凝聚神宫
古老的祭坛，静静的矗立在虚无之中，在那祭坛的最顶部，一道削瘦的身影犹如磐石般的静静盘坐，他的身体略显冰凉，甚至连鼻息都是弱不可闻，若不是隐隐的能够察觉到一丝气机存在的话，恐怕任谁都会认为，眼前的他，仅仅只是一具没有气息的尸体而已。
这片奇特的地方，没有任何的声音，寂静得犹如是在时间长河中的偏僻一隅，无人能够察觉，无人能够到达。
祭坛之上，那道削瘦的身影突然微微一颤，那紧闭的双目，则是缓缓的睁开，黑色眸子漆黑如夜，他眉头轻轻的皱着，眉宇间显得有些无奈与不解。
“一直都在失败啊。”
林动喃喃自语，这应该是来到这里的第四个年头了吧，也就是说，他在这里修炼了四年，而这四年，除了刚开始淬炼精神力的第一年，其余的三年，他都是在尝试着凝聚神宫，还毫无意外的，他全部失败了。
虽然他不断的在寻找失败的原因，甚至这种自我找寻到了现在，已算是相当的完美，但不知为何，凝聚而出的神宫，总是徒有其形，丝毫没有让得林动感觉到似乎晋入了某种层次的特殊感觉。
“为什么会失败…我的精神力已是极为凝炼，为何不能在泥丸宫内将神宫凝聚出来。”
林动紧皱着眉头，眼芒不断的闪烁着，眼中却是迷茫与疑惑，这般思索许久后，依旧没有多少头绪，他只能叹了一口气，微闭着眼，准备再度尝试。
“嗯？”
不过，就在他想要继续尝试时，突然微微怔了怔，而后那目光便是看向眼前的石板，那里有着一些交错的痕迹，看样子应该是谁随手所划。
那划痕颇为的粗糙简单，仅仅只是一个痕迹斑点，然后有着数到深深的痕迹直接从上面切割而开，仿佛是将这斑点彻彻底底的破坏了一般。
林动望着这些随手痕迹，失望的摇了摇头，再度闭目，不过片刻后，他猛的睁开双眼，再度看向那些随手痕迹，眼中掠过一些惊疑之色。
这斑点，看上去有些类似泥丸宫，这交叉的痕迹将其破坏，难道是说，神宫并非是由泥丸宫改造…而是要将其彻彻底底的破坏，重生铸造不成？
这是…破后而立？
林动心脏猛的剧烈跳动起来，先破掉泥丸宫？开什么玩笑，泥丸宫是天生的，一旦破坏，便将会让得人精气神流逝，那种后果，即便是他都承受不起。
万一到时候泥丸宫破了，他又凝聚不出神宫，那不仅精神力修为荡然无存，甚至还会有损他的神智，变成白痴都不是不可能的事…
这代价，就连林动的心性，都是迟疑了一些，但隐隐的，他又感觉到这或许的确是通向神宫境的真正大道。
破，还是不破？
林动眼神急速的变幻着，心中显然是有些挣扎，如此好半晌后，他终于是轻轻吸了一口气，手掌猛的紧握，漆黑眸子中，掠过一抹坚定之色。
若无大毅力，如何成就大能力，更如何去超越冰主？
心中下了决定，林动再无丝毫犹豫，双目紧闭，心神一动，便是来到了泥丸宫之前，此时的泥丸宫，犹如一个细小的光点，轻轻飘荡，有着浩瀚的精神力波动散发出来。
林动心神注视着那道光点，心神一动，泥丸宫内的浩瀚精神力竟然是在此时以一种极端惊人的速度收缩起来，短短数息时间，那所有的精神力，都是在泥丸宫内，化为一颗仅有人头大小的精神力圆球，这圆球犹如实质，表面光华如镜，但其中那种恐怖的波动，就算是渡过两次轮回劫的巅峰强者，都足以感到心惊胆颤。
而当这种压缩，达到极致的时候，那精神力圆球突然急促的颤抖起来，旋即圆球表面开始破碎，裂纹蔓延出来，一道道毁灭性的光束，悄然的射出来。
当那种光束强盛到极点的时候，仿佛一轮耀日，自泥丸宫升腾而起，再接着，毁灭波动，席卷而开，泥丸宫内，彻底的爆炸。
砰！
林动的身体，都是在此时疯狂的颤抖起来，泥丸宫爆炸，其中的精神力，也是犹如潮水一般疯狂的涌出来。
林动强忍着脑海中传来的剧痛，急忙收揽着那些失去泥丸宫的约束而变得狂暴无比的精神力，此时若是让如此恐怖的精神在其体内肆虐开来，就算他有着洪荒龙骨守护，恐怕都得支离破碎。
璀璨的紫金光芒席卷而开，林动体内的洪荒龙骨也是察觉到危险，竟是化为低沉的龙吟之声，紫金光芒化为无数道紫金巨龙，犹如龙罩一般，将那些肆虐的精神力尽数的围困住。
三道祖符也是在此时出现，将精神力包围得水泄不通，光芒闪烁间，总算是没让得精神力失去控制。
林动此时心中也是紧张无比，泥丸宫如今已经破碎，他不可能一直这样约束着精神力，如果无法再给它们制造出一个“泥丸宫”，那么精神力就会失控从而携带着他的精气神消散，而他也将会遭受到无以伦比的打击。
不过虽然现在情况紧迫，但林动毕竟经历了太多生生死死，所以也并没有丝毫的慌乱，心神一动间，便是如同之前在泥丸宫内操控精神力凝聚神宫那般，开始将其凝聚在一起。
浩瀚的精神力涌荡着，然后缓缓的汇聚，此时精神力犹如液体一般，流淌而动，勾勒之间，仿佛是有着一座神宫的雏形在悄然的成型。
精神力在林动入微的操纵下，开始一点点的完善着神宫的无数步骤，不过这种工程显然是极端的浩大，即便是以林动的能力，都得消耗许久的时间，但好在如今林动本就是处于静修中，在这种地方，也不会有任何的人来打扰他，不然的话，一旦精神力失控，那着实是有些严重。
时间缓慢的流逝，眨眼间，便是两月时间过去，而那神宫也是逐渐的初成规模，神宫巍峨，浩荡大气，气象万千，仿佛神之居所。
林动的心神注视着这成形的神宫，心中也是忍不住的有着狂喜涌动，旋即其心神一动，那浩瀚的精神力便是如同潮水一般的涌入神宫之中。
然而，就在精神力涌入其中后，林动心中的狂喜却是骤然间冰凉下来，因为他猛的发现，精神力在这神宫中，根本就没有半点形成循环，自发衍生的迹象。
以往的泥丸宫，只要精神力进入其中，便是犹如回巢之鸟，自会有一种安宁之感，然而现在，那种感觉，却是荡然无存。
显然，这神宫，根本就未取到代替泥丸宫的效果！
这样一来，林动就必须死死的牵制住这些精神力，使得它们不会消散，但这样一来，他就将永远没有分心做其他事的力量，也就是说，他被自己的精神力牵绊住了。
林动冷汗直流，他盯着那神宫，究竟是还少着什么吗？
为什么凝聚出来的神宫，无法取代泥丸宫呢？
林动心念急速的转动着，他隐约感觉到神宫似乎是少了什么，但却无法明白那少的，究竟是什么…
时间缓缓的流逝着，但林动却依旧毫无头绪，其心头也是忍不住的微沉，这僵局，难道就无法打破吗？
祭坛之上，林动紧闭双目，满身的冷汗，冷汗顺着身体滴落下来，落到那破旧的蒲团上，隐隐的，蒲团似是微闪了一下毫芒。
而就在那毫芒闪烁的瞬间，林动的心中，仿佛是响起了一道极为悠扬的古老声音：“太上。”
突如其来的声音，令得林动身体猛的一僵，不过他却来不及思虑这道声音，一道灵光，猛的闪过脑中：“太上感应诀？！”
“是少了那种力量吗？”
林动心神一动，然后不再迟疑，迅速的沉侵进入太伤感应之中，意识飘荡，许久后，一缕缕混沌之光，终于是射来。
混沌之光，落到那神宫之上，而后神宫竟是爆发出一阵清澈的鸣叫之声，就在这一霎那，犹如死物般的神宫，竟然是犹如具备了生命一般，神宫表面，光芒流溢，一种生机，蔓延而出。
轰隆隆。
精神力涌荡在神宫之内，隐隐的，似乎是逐渐的构成了一种最初步的循环，而且林动能够感觉到，就在神宫成形时，似乎也是与那冥冥之中极为神秘的太上，建立了一种玄妙的联系，一缕缕的混沌之光不断的涌来。
精神力翻涌，那一丝丝的混沌之光，开始侵入精神力之中，再接着，林动便是感觉到那些融入了混沌之光的精神力，开始了翻天地覆的变化。
那是一种真正的蜕变。
林动心有所悟，心神渐渐的平静，他静静的守护着神宫，任由精神力在其中翻涌，他能够感觉到，等到精神力尽数蜕变的时候，或许，神宫就能够真正的取代泥丸宫！
只不过…那似乎需要一个相当漫长的时间，而幸运的也是，现在的林动，并不缺少这个时间。
他只需，静静的等待着涅盘之日。


第1280章 三大联盟
光阴流逝，岁月如梭。
不知不觉，修炼场之外的世界，已是一年过去，而在这一年中，其间大事发生了太多，先是乱魔海之乱，待得乱魔海联盟刚刚组成，与那肆虐的异魔形成对恃时，妖域也是陡然传来了异魔进攻的消息，而在最为开始的时候，妖域的一些势力自恃能力，倒是妄图解决掉那些异魔，但可惜的是，异魔过处，尸横遍野，血流成河，任何的阻拦者，都是被他们彻底的抹杀而去。
鲜血仿佛都是染红了妖域的天空。
当妖域西北的地域，所有势力被魔狱抹除后，妖域终于开始颤抖，那些各大种族与势力开始明白过来他们此次所面对的对手究竟有着多么的恐怖。
恐惧蔓延，他们开始知道，在这个世界上，并没有任何单一的势力能够抵挡魔狱，即便是妖域之中的那些霸族，也是无法做到。
不过所幸，就在魔狱要在妖域肆虐时，远古之主及时赶来，而后四大霸族，八大王族在诸位远古之主的撮合下，组建妖域联盟。
而妖域联盟一形成，妖域无数势力纷纷涌来，这种时候，若是再不抱做一团，恐怕便是只有灭亡一途。
妖域联盟，由四大霸族联手掌控，以他们在妖域的声望，倒是没有任何人对此表达不满，不过在妖域联盟中，显然并没有类似唐心莲那般具备着完美统率之能的人，四大霸族都是心气高傲，不想弱于对方，因此联盟的权利也是各有纷争，这也是导致在联盟初成的第一月中，妖域联盟依旧是在魔狱的进攻下，节节败退。
这一情况，无疑是令得诸位远古之主颇为的震怒，直接是将四大霸族族长召来，经过一番呵斥以及强行镇压，四大霸族不得不挑选一个真正能够指挥联盟力量的人，而这人选，选来选去，最后落到了天妖貂族头上。
而天妖貂族长则是将这个重任，落到了小貂身上，而出奇的是，诸位远古之主对这般选择也是未有异议，于是人选便是定下，小貂成为了妖域联盟唯一的盟主。
联盟内部纷争被平息而下，终于是能够将所有的力量动用而出，而小貂同样也是展现出了极为不菲的能力，虽说或许比起唐心莲那种能够迅速的让乱魔海联盟将魔狱步伐挡下来有点差距，但至少，妖域能够开始与魔狱形成对恃，未曾再出现类似之前的那种重大损伤。
这种对恃，持续了半年左右，然后却是因为一些突发之事，令得妖域联盟实力上涨，这些事，一是一些被挑选去“祖宫阙”修炼的超级强者成功突破至轮回境，从而强势归来，另外一件，便是这妖域，突然出现了第五个霸族。
这个霸族，名为九尾族，曾经的霸族之一，不过后来逐渐的没落，但谁也未想到，这个之前消失了数年时间的九尾族，竟然会在这个时候重新出现，而且，当她们出现时，那所展现出来的实力，却是令得整个妖域为之震惊。
那九尾族中，其族长似乎仅仅只是一个妖媚得令人心颤的少女，可这少女的实力，却是恐怖得连那些踏入轮回境的巅峰都是都有些心惊，因为，这少女不仅踏入了轮回境，而且还生生的渡过了一次轮回劫！
而且除她之外，九尾族的整体实力，比起其他霸族，也并弱不了多少，这怎能不让其他的势力感到震撼。
九尾族横空出世后，妖域联盟迅速与她们联系，之后由现任妖域联盟除魔统帅林炎出马，却是并未沟通多久，九尾族便是答应加入妖域联盟，并且她们也并没有太多争权夺利之心，对于小貂的统率，也并没有多少的反对。
这一支生力军的加入，无疑是令得妖域联盟势力上涨，终是能够在与肆虐妖域的魔狱部队的对恃中渐渐的占据一些细微的上风。
而妖域的局势，也是因此而出现了僵持。
不过，魔狱的高层，显然并不希望这片天地能够安宁，因此，就在妖域局势渐被平稳下来后，那四大玄域，也终于开始被他们的魔掌所触及。
有了之前两次的经验，这一次魔狱显得雷厉风行的许多，甚至不待多少人回过神来，四大玄域之中的西玄域，便是被血洗近半。
惨况很快的传播开来，诸位远古之主最先赶到，这才阻下魔狱脚步，而后迅速命令四大玄域组成联盟，这次的联盟，完成得极为的迅速，这之中黑暗之殿居功至伟，而没有多少意外的，四玄域联盟盟主之位，由青檀所掌控。
从祖宫阙出来后，青檀已是真正的晋入轮回境，再凭借着黑暗租符以及手中的黑暗圣镰，即便是面对着渡过两次涅盘劫的强者都能一战，凭借她黑暗之殿殿主的身份，四玄域中，倒的确是无人能够与她争夺盟主之位，这也是让得四玄域多了一些时间组建联盟大军，最后经过数月的血斗，虽然将整个西玄域丢弃，但至少，其他三域被成功的保护了下来。
至此，这天地之间，三大联盟正式成形，各自镇守乱魔海，妖域，四玄域，无数势力，宗派，种族汇聚在一起，在魔狱那残酷的压迫下，曾经的隔阂之间的消除，那一股股力量，终是彻底的凝聚在一起，他们都很明白，若是不将这魔狱除去，那么这世间无数生灵，都将不会有丝毫的善果。
那时，生灵涂炭，所有人都将沦为异魔的掌控，犹如最为悲哀的奴隶。
三大联盟成形，终于是逐渐的缓住了局势，魔狱的肆虐也是被终止，而此时，平定了局面的应欢欢等远古之主，也终于是彻底的腾出手来。
东玄域。
在那道宗之外的天空上应欢欢，炎主，黑暗之主等人凌空悬浮。
应欢欢看了一眼四周，然后淡淡的道：“我打算再开一次天眼。
她的声音，冰彻入骨仅仅只是传开，便是见到空气荡漾着，竟是有着凝聚成冰的迹象，她似也是察觉到这种情况，那柳眉不由得微微一簇，旋即她低头，只见得如今的她，两只修长的玉臂都是被包裹在了纤细的长袖白色手套之中。
隐隐的，还有着一种惊人的寒气，自手套中传出来。
黑暗之主五人闻言，倒是一愣，旋即道：“我们之前不是已经开过一次了吗？”
在一个月之前，他们便已开过天眼，试图将魔狱那些高层找寻出来，但最终却是以失败而告终那些家伙，显然是早有准备。
“这次不用你们出手。”应欢欢摇了摇头，然后看向不远处的山峰上，那里有着一道修长的倩影而立，她身着白色衣裙，容颜绝美清冷的气质，犹如那空谷幽兰，令人感到惊艳。
这道倩影赫然便是绫清竹。
她察觉到应欢欢的目光，娇躯一动，便是掠来。
“她是谁？”空间之主等人有些讶异的看着绫清竹，后者的实力似乎并不是特别的强，但不知道为何，他们却是从他的身上察觉到一股极为熟悉的波动。
“小师妹的情敌。”黑暗之主似乎是知道一些，偏过头低声道。
听得此话，空间之主他们面色顿时古怪起来。
“她感应了太上，而且感应程度恐怕还胜于你们，我与她联手，应该能够将魔狱那些家伙找出来。”应欢欢淡声道。
“什么？”此话一出，炎主等人面色顿时一变，眼神略微有些震惊的望着清冷的绫清竹，这天地间，除了他们之外，竟然还有其他人能够感应太上？
“我们一起出手吧。”
应欢欢走近绫清竹，冰蓝美目看着她，不过那冰彻的声音，却是让得绫清竹微微一怔，道：“你变了许多。”
女人的直觉，总是敏锐得令人可怕。
应欢欢娇躯微凝。
“他知道吗？”绫清竹再度道。
应欢欢摇了摇头。
绫清竹则是默然。
“动手吧，我们需要在天王殿未曾彻底恢复前将他找出来，只有那样，才能提前结束这场劫难。”应欢欢似是轻叹了一声，道。
绫清竹螓首轻点，旋即两女便是凌空盘坐而下，美目微闭，周身的能量波动，竟是尽数的收敛而起，甚至连这片天地的风都是彻底的静止下来。
两人的静坐，持续了约莫半个时辰，然后炎主他们便是察觉到天色逐渐的暗沉下来，一缕缕混沌之光，从神秘之地而来，最后在两女的上空，迅速凝聚。
一股奇特的威压，弥漫开来，令得炎主他们眼神都是微微一凝，这种威压，来自于那神秘的太上。
混沌之光凝聚着，最后逐渐的化为一只混沌眼球，那眼球约莫丈许大小，其中弥漫着混沌之色，其中反射着天地之景，极为的神奇。
那混沌之眼一出现，便是缓缓的眨动，开始扭转着方向，似乎是在探测这天地之间。
眼球之中，景物飞快的掠过，但它始终未曾停歇下来，而炎主等人，便是这般安静的等待着。
这种安静，持续了约莫将近小半日的时间，不过就在炎主他们以为这次探测依旧失败而心生失望时，那混沌之眼中，突然暴射出强烈的光芒，光芒在半空中凝聚，最后化为一片镜幕。
在那镜慕之中，是一片黑暗之地，那里似乎是流淌着黑色的河水，而此时，在那河水的中央，一具黑色棺材，静静的悬浮。
镜幕移至棺材上方，破去魔气，而后一道身影，便是出现在了镜幕之中。
那道身影，身着黑袍，黑袍边缘，有着金色的纹路，他的模样极为的英俊，而此时，他紧闭着双目，但即便是如此，那脸庞上，依旧是有着一种无法形容的邪恶之气在流转。
应欢欢那冰蓝美目，几乎是在此时瞬间冰冷了下来，覆盖着纤细手套的小手，缓缓紧握。
“天王殿，终于找到你了。”


第1281章 天王殿
镜幕之上，光芒流溢，将那黑色棺材之中的人影，尽数的展示出来，虽然此时的他紧闭着眼睛，但任谁都是能够感觉到那种凌厉到极致的侵略性。
“天王殿！”
炎主等人也是在此时眼神一沉，脸庞上掠过一抹浓浓的杀意，终于是将这家伙给找到了！
应欢欢美目盯着那镜幕中的黑衣男子，旋即她眼神一凝，后者紧闭的眼睛似乎是微微颤了一颤，而后双目，竟是在此时缓缓的睁开。
轰！
就在他双眼睁开的瞬间，仿佛是有着一种足以让人堕落的邪恶暴涌而出，那种邪恶，若是心智不坚者看上一眼，怕就是会迷失掉自己的心智。
应欢欢等人则是眼神冰冷的注视着睁开眼的天王殿，而后者也是这样的看着镜幕，他仿佛是察觉到了极为遥远之外的那种探测。
“呵呵，是冰主吗？”
棺材中的天王殿缓缓的撑起身子，他望着镜幕淡淡一笑，神色略显慵懒的道：“真是有些手段呢，这样都能被你们探测出来。”
没有人回答，因为回答了也听不见，天眼能够探测到的天王殿，但却无法让得双方沟通，此时的天王殿，也只是在对着虚空说话，只是他知道，他所说的话，应欢欢他们必然看得见。
“既然探测到了我的位置，那我便等着你们吧，等你们来到，我想，我们的恩怨，也该彻彻底底的了结了。”天王殿微笑道。
“不过…”
天王殿嘴角挑起一抹邪恶以及残酷的笑容，他手掌对着镜幕缓缓的紧握，眼中掠过一抹奇异光芒：“这一次，你们必输无疑。”
镜幕微微的颤抖着，而后景象开始变得模糊，那天王殿的身形也是越来越朦胧，最后镜幕一颤，终于是彻彻底底的消散而去，天空上那混沌之眼，也是在此时化为虚无。
应欢欢站起身来，那美目中满是冰寒之色，道：“天王殿应该恢复得差不多了。”
“现在怎么办？”炎主沉声道，天王殿恢复过来，那魔狱的战斗力也将会达到顶点，这对他们而言，显然不算什么好消息。
“他们的位置，应该是在西玄域，那里便是他们的大本营所在。”应欢欢道。
“通知三大联盟，尽快解决与他们僵持的异魔，然后将所有力量调往西玄域，这一次，我们要将他们清除得干干净净！”
应欢欢美目之中，有着一抹名为凌厉的光芒在涌动，这些异魔太过难缠，若是再如同上一次那般只是将他们逼得潜伏，恐怕始终不算彻底的解决，不过所幸这一次，他们占据着一个天大的优势，那便是这是他们的主场，而如今这天地间的异魔，都算是被囚禁在这里，没有任何后援的他们，终归是会失败！
“嗯。”
炎主等人面色凝重的点点头，看来，这场天地大战，果然是要再度开始了，不过这一次，可不能再给这些家伙丝毫的机会了！
“我们准备招引祭，等三大联盟大军齐聚，便动手召唤大师姐。”应欢欢想了想，再度道。
“强行召唤生死吗？”炎主等人看向应欢欢。
“我能隐约感觉到大师姐的波动，她应该也差不多了，召唤的话，不会有大碍。”应欢欢道。
炎主他们缓缓的点了点头，再度犹豫了一下，道：“但你若是要与天王殿交手的话，恐怕就需要动用所有的力量了。”
应欢欢终是沉默下来，她那带着修长雪白手套的小手缓缓的紧握着，旋即她抬起头，美目遥遥的望着乱魔海的方向，良久后，方才道：“我知道，放心吧，天王殿交给我便好，这一次，他不会再有逃生的机会。”
正好，他现在不在，便趁着这些时间，擅自的完成这些吧，不然，在他的注视下，自己真的能够下定这种决心吗？
炎主他们对视一眼，旋即轻叹着点了点头，一旦将力量全面苏醒的话，或许，她又将会变回远古时候的那般模样了吧？
“准备吧。”应欢欢似是有些疲倦的挥了挥玉手，炎主等人也知道她的心情，也就不再多说，身形一动，开始去将消息传给三大联盟。
见到他们散去，应欢欢贝齿也是忍不住的轻咬着红唇，望着乱魔海方向的美目，却是迟迟不肯移动。
“为什么不等他出关？我们应该还有时间的。”绫清竹清眸望着应欢欢，问道。
“我怕那时候就狠不下这心了。”
应欢欢微微一笑，道：“在他面前，我可没办法再保持那冰一般的心境，而且，这种责任，总归需要人来担当的，他很出色，但距此，还有点距离。”
“这种责任也太重了，让他来担的话，我也心疼，所以，还是我自己来吧。”
绫清竹盯着应欢欢，道：“可他怕是接受不了这个结果，你这样，未免也不是一种逃避。”
“可世界上，哪有这么多的选择呢。”应欢欢幽幽的道：“而且，也只有他一直像傻子一样固执的坚持着，他总想着，有一天，能够担负这些的。”
应欢欢唇角的笑容变得苦涩与无奈了一些：“可其实…这又怎么可能呢。”
绫清竹还欲再言，应欢欢却是走近，冲着她微笑道：“如果以后出了什么变故的话，请帮我好好照顾他，好吗？”
绫清竹盯着面前女孩那俏美中含着一些冰寒的脸颊，轻声道：“我从你们所说中，能够感觉到你若是彻底苏醒力量会有什么变化，不过…相信我，即便是那样的你，面对着他，依旧会有些不同的。”
“有种情感，不是纯粹的力量能够冰冻的。”
应欢欢一怔，旋即喃喃自语：“我．也希望…真是那样啊。”
只不过，那时候的我，连我自己都讨厌，你…还会喜欢吗？
黑暗的河流上，棺材中的男子缓缓的站起，他微微摇了摇头，将脑海中的细微眩晕感消除而去，然后抬头望着先前窥探他的虚无处，淡淡一笑：“冰主么？真是老冤家了呢，没想到，你还真的成功轮回转世了啊。”
“轰。”
黑暗的河流突然爆发出巨响，一道万丈庞大的黑影迈步而来，八只巨大的魔臂挥舞间，连空间都是剧烈的扭曲下来。
“天王殿，你总算是全部恢复了吗？”大天邪魔王嗡鸣的声音，在这黑暗河流上回荡着，引起阵阵涛浪翻滚。
在周围的河流上，一道道黑色的水柱涌起，而后有着八道身影，自水柱顶端浮现出来，仔细看去，赫然便是三王殿等魔狱真正的掌控者。
“我们应该已经被发现了。”在那最右侧的水柱上，一名清瘦的少年轻声道，他模样清秀，看上去有着一种稚嫩之感，不过那黑色眸子中，却是有着一种令人心悸的阴森缠绕。
“是啊，真是小看了冰主呢，原本还想再准备一些时间的。”天王殿淡淡的笑道。
“现在怎么办？直接与他们正面开战？”三王殿开口问道。
“放心吧，他们必然会先对付散布在外面的异魔，通知下去，命令他们展开死攻，只要他们三大联盟军队未曾齐聚，想来冰主他们也不敢独自前来。”天王殿漫不经心的道。
“三大联盟如今已经稳固，光凭那些部队，恐怕会挡不住。”三王殿皱眉道。
“那就全死了吧。”天王殿笑着道，露出森白的牙齿：“这是他们的荣耀。”
他虽然笑着，但那眼中，却满是无情之色，仿佛那被他抛弃的无数异魔战士，并没有令得他有丝毫的情绪波动。
“这一次，不会再有人能够阻拦下我们的脚步，这片位面，终归会沦为我们异魔族的掌控。”
天王殿咧嘴一笑，那笑容中，竟是有着无尽的狂热：“皇当初未完成的大业，我来为他完成！”
“冰主…你等着吧，这一次，我们可不会再失败了啊，真是期待你们那绝望的脸庞呢，哈哈。”
略显低沉的笑声，在这黑暗中远远的传开，河流涌动，隐隐的，似乎是有着无数邪恶的眼睛，在那黑暗中睁开。
在外界因为异魔而彻彻底底沸腾间，那仿佛自时间长河中开辟出来的修炼场，却依旧是寂静无声，那种宁静，弥漫着难言的孤寂。
在那的祭坛顶部，那道身影，已是静坐了十年时间，十年时间，令得他身体上的衣衫逐渐被岁月侵蚀得破碎，只是他的容貌，却是并没有丝毫的变化，只是那脸庞上的坚毅，仿佛被勾勒得愈发的明显。
在他的体内，没有一丝一毫的元力以及精神力散发出来，他就犹如那最为普通的普通人一般，仿佛弱小得能够让任何人将其无视。
唯有着那种感知敏锐到极点的人，方才能够隐隐的感觉到，那朴实之下，孕育着何等锋芒。
这般安静，不知道持续了多久，那道静坐十年的身影，突然微微一颤，身体上的衣衫顿时化为粉末飘散而下。
而就在衣衫化为粉末时，那道身影紧闭十年的双瞳，也终是在此时，轻轻的睁开。
那双眼瞳，漆黑如夜，其中没有任何的波动，但就是这般的平静中，这片虚无天地，仿佛都是在那对眼瞳的注视下，轻轻的颤抖起来。
那眼，犹如神之注视，天地皆惧。


第1282章 一瞬十年
祭坛顶部，林动睁开双瞳，那漆黑眸子中，仿佛包揽了世界，玄妙霊得令人忍不住的沉醉于其中。
从那漫长的修炼中苏醒过来，他先是茫然了一瞬，而后眼中开始恢复清明，他低头望着自己那修长的双掌，嘴角却是有着一抹淡淡的笑容浮现出来。
“这就是…神宫境吗？”
林动喃喃自语，他能够感觉到，此时的他，似乎是晋入了一种相当恐怖的境界，那种境界，是他以往从未触及过的。
他有种感觉，现在的他，若是再遇见那魔狱七王殿时，恐怕翻手间就能将其重创，甚至，面对着那些堪比三次轮回劫强者的王殿，他都不会再有丝毫的忌惮。
这，就是神宫境所带来的力量。
林动微微闭目，心神一动，便是见到，在其眉心处，一座闪烁着万丈神光的神宫静静的悬浮，在那神宫之内，无穷无尽的精神力在呼啸舞动，如今这种精神力，呈现淡淡的混沌光芒，那是融合了那种神秘的太上力量所产生的变化，正是这种细微的变化，让得精神力犹如蜕变一般。
睁开双眼，林动缓缓的站起身来，身体上光芒闪烁，便是化为一件黑色衣衫，他手掌握了握，这十年时间，他虽然绝大部分的心神都是放在凝聚神宫之上，但体内的元力，也是有了极为显著的精进，只不过到了这种层次，想要考虑的，则不再是如何突破而是该如何渡过那恐怖的轮回劫。
“十年，外面也过去一年了吧，不知道情况究竟如何了。”
林动自语道，退出修炼状态后，他心中也是涌起了一些忧色毕竟在闭关之前，外面的情况就已十分不乐观，也不清楚现在又变成怎样了？
“还是先出去吧。”
林动这般想着，也就不再迟疑，他转过身，对着祭坛上那破旧的蒲团弯身拜下，这一次能够成功的凝聚神宫，那冥冥之中的提醒颇为的重要不然的话，恐怕他怎么样都是想不到，凝聚神宫竟然是需要那神秘太上的力量，这也多亏他从绫清竹那里将太上感应诀修炼成功，不然的话，恐怕这一步，他终身都是无法跨进。
当然，想要晋入神宫境也并不仅仅只是需要那种神秘的力量，在这凝聚神宫的数年中，绕是以林动的谨慎，都数次失手，差点便是令得神宫崩溃，所幸最后被他生生的挽回了回来不然的话，恐怕依旧还是得以失败而告终。
嗡嗡。
而就在林动对那蒲团弯身下拜时，那道蒲团竟是发出了细微的嗡鸣之声一种奇异的光亮缓缓的浮现出来。
嗤。
光芒在蒲团之上凝聚着，隐隐的，似乎是化为一道迷你型的玄奥光阵，林动望着那光阵，却是陡然愣了下来，这道光阵竟是与他体内的乾坤古阵，一模一样！
而就在这道光阵出现时林动体内的乾坤古阵，竟然也是在此时发出震动，竟是不受林动控制的掠出，然后直接与那光阵融合在了一起。
两者相融，顿时那种晦涩玄奥的程度，再度变得强烈，那种古老的波动，也是浓郁了许多。
“这才是乾坤古阵的阵灵吗？”林动望着那融合在一起的乾坤古阵，心中也是掠过一抹明悟，他所获得的乾坤古阵，一直都是缺少了什么，而从魔狱那些家伙嘴中，他也是知道，乾坤古阵乃是符祖所创，远古时期不知道多少异魔被其诛灭，只不过如今他手中的乾坤古阵，并不算完美。
眼前这一幕，显然是让得林动明白，乾坤古阵一直所缺陷的东西，正是隐藏在这破旧的蒲团之中！
光阵开始回缩，光芒尽数的收回那蒲团之中，而后只见得破旧蒲团竟开始融化，火焰升腾间，一团异光缓缓的升腾而起。
那团异光，似是一个约莫巴掌大小的黑白石盘，石盘充满着洪荒般的味道，在那上面，还有着无数神异的纹路。
林动仲出手掌，那黑白石盘便是落入他的手中，入手冰凉，他有些好奇的将其来回看了看，旋即心神一动，那黑白石盘便是化为一道异光，钻进他的身体之中。
至此，这由符祖大人所创造而出的乾坤古阵，也终于是彻彻底底的完整！
而获得完整的乾坤古阵，林动眼中也是忍不住的掠过一抹欣喜之色，没想到在这里竟然还能够遇见这等机缘。
“多谢符祖。”
林动对着这虚空抱拳相谢，虽然如今符祖早已不在，但却不妨碍他的一腔感谢之意。
林动行完谢礼，也就不再停留，心神一动，祭坛前方的虚无便是有着光圈缓缓的成形，他终是不再迟疑，一步跨入光圈，而后逐渐的消失而随着他的消失，这片能够逆转时间的虚无空间，终是再度的变得孤寂而下，唯有着祭坛顶部的那破旧蒲团，微微的闪烁着毫光，似是在送别着林动一般。
炎神殿。
这里如今已是乱魔海联盟的大本营，天空上，无数光影飞掠，整片天地，都是显得热闹之极，那等人气，比起一年之前，不知道强盛了多少。
当然，这片区域，最为令人眼热的，无疑还是那矗立在天空之上的“祖宫阙”，在那宫阙大门处，有着数道身影笔直而立，惊人的气息自他们体内散发出来，竟然都是踏入转轮境的超级强者。
在那下方，有着无数道羡慕的目光望着那些偶尔经过审核能够进入祖宫阙的强者，这一年来，可是有着不少转轮境的强者在其中获得突破，最后晋入那让无数人敬畏的轮回之境，也正是因为如此，他们联盟的实力方才会越来越强，并且反制异魔。
而此时，在那祖宫阙最深处，碎石广场中，那道古老的石像突然闪烁起光华，而后一道身着黑衫的削瘦身影，缓缓的自其中走出。
林动走出石像，他望着眼前这熟悉的地方，那因为十年苦修，而出现一些孤寂的黑色眸子中，也是再度有着熟悉的神彩涌出来。
他深吸一口气，旋即微微一笑，缓步走出，而在走出祖宫阙的路上，他偶尔能够遇见一些在其中苦修的强者，这些强者实力极为不弱，他们见到那缓步走来的林动，倒是有些发愣，甚至待得他走过身旁后，方才猛的回过神来。
“那是…林动元老？！”
在这乱魔海联盟中，林动身兼那元老之位，与应欢欢他们并列，那等地位自然是尊崇无比，无怪这些联盟的人见到他会如此的震惊。
“原来林动元老也是在祖宫阙中修炼…”众人窃窃私语，而后心中有些震动，先前林动与他们搽身而过时，他们甚至是没有感觉到他体内有任何的元力甚至精神力波动，这显然不是林动实力全失的原因，假如如此，那就只能是如今的林动，已经达到了一个他们无法想象的地步。
林动倒是并未理会后方的一些骚动，径直来到祖宫阙大门，刚欲出去，却是被一道人影拦住：“这位兄弟，你刚刚完成修炼吗？那就来登记一下，你来前是哪方联盟的？”
林动愕然的望着眼前那拿着玉盘对他审问的中年男子，然后捎了捎头，刚欲说话，那一旁却是猛的有一人一把将先前那人扯到一旁，原本不言苟笑的脸庞上却是堆满了敬畏的笑容：“林动元老，这家伙是刚来的，不认识您老人家，多有得罪，多有得罪。”
“林动元老？”先前那审查之人听得这个名字，脸庞顿时跳了跳，旋即有些狂喜的望着林动。
“这怎么回事？”林动有点哭笑不得的问道。
“呵呵，自从一年前诸位远古之主开启祖宫阙后，便是选拔了很多晋入转轮境的强者进入其中修炼，同时也为我们三大联盟输送强者。”那壮汉恭声回道。
“三大联盟？”林动一愣。
那壮汉见他这模样，也不奇怪，毕竟这些事都是在林动闭关后发生的，当即他便是将这一年的事情尽数的说了一遍。
“竟然连妖域与四玄域都是被波及了…”
林动听完，心中也是颇感震动，没想到局势竟然发展到了这种地步，看来那魔狱，真的是打算掀起第二场天地大战了。
“前些时候远古之主传来消息，命三大联盟全力清除魔狱部队，然后汇聚所有力量，攻向西玄域，彻底清除魔狱。”
“如今我们乱魔海联盟，所有大军已是在心莲盟主的率领下与魔狱部队展开决战，不过战场似乎是有些胶着。”那壮汉说道。
林动微微点头，也并未询问战场地点，心神一动，便是感应到了遥远地方的冲天煞气。
“我去一趟吧。”林动说道。
“有林动元老出手，那魔狱部队，必定一触即溃！”那壮汉闻言，顿时大喜。
林动笑了笑，摆了摆手，他的身体便是在那壮汉两人惊异的注视中，缓缓的变得虚幻起来，片刻后，彻彻底底的消失不见。
那壮汉二人见到林动这般近乎融入天地间的手段，那眼中也是有着浓浓的震惊以及狂喜涌出来。


第1283章 青雉战魔
乱魔海，北海域。
这片浩瀚的海域，在这一年，几乎成为了魔狱与乱魔海联盟的最前线战域，双方在这里开战数十次，每一次，都是惊天动地，鲜血染红海洋，魔气遮掩了苍穹。
而现在，这片海域，依旧是整个乱魔海最为血腥的地方。
因为这一次，双方已是在展开决战。
乱魔海联盟为了能够腾出力量，然后联合其他两大联盟尽数反攻，已是打算尽全力的将乱魔海的战争平息，而至于魔狱这边，也是下了最残酷的命令，不惜一切，阻拦乱魔海联盟。
于是，这一年来，最为惨烈的大战，再度爆发。
轰轰！
魔气铺天盖地的涌动着，无数道强横的元力也是爆发而开，放眼望去，只见得那望不见尽头的光影，夹杂着滔天的杀伐，对着那些魔狱军队冲杀而去。
两股庞大的洪流冲撞在一起，厮杀声响彻天地，海面上，万丈涛浪滚滚而起，轰隆隆的巨声，却是掩盖不住那种充满着的血腥厮杀之声。
在那无穷无尽的联盟大军后方，唐心莲凌空而立，她身着紧身的红色软甲，勾勒出玲珑有致，凹凸起伏的动人曲线，而此时，她那张俏丽脸颊，却是紧紧的望着这片战场，清澈的冷喝之声，不断在元力的包裹下远远的传开，将联盟大军，尽数的指挥着。
“盟主那些异魔此次如同疯了一般，战况太过胶着了。”在唐心莲身旁，一名老者面色凝重的道。
唐心莲凤目中寒芒一闪，道：“发出死令，这一次一定要解决掉乱魔海的魔狱部队，如今魔狱大本营已是暴露，这些部队突然誓死反抗，必然是收到了大本营的命令，他们是想要舍弃这些部队来拖延我们联盟大军的脚步。”
“能够用这么大的代价来做这些事，魔狱必然是在借机准备着什么，我们必须尽快！”
那老者闻言心头也是一凛，急忙应道，虽然他也是乱魔海中顶尖的强者论起辈分恐怕还要超摩罗一些，但这一年来，唐心莲掌管乱魔海联盟以来，原本的局势在其手中一点点的被逆转，这种能力，已是取得所有人的认可因此对于她的命令，整个乱魔海中，竟是无人不一道道命令，飞快的传遍联盟大军，那无数强者眼中也是涌起猩红之意，这一年他们有着太多太多的人死在魔狱手中，那种仇恨，已是无法洗净，既然仇恨不能解除，那就将他们彻彻底底的杀光吧！
“杀啊！”
惊天动地般的嘶吼声响彻而起，无数强者犹如洪流般的冲出，磅礴元力将那前方的异魔尽数的撕碎而去。
整个天地，一片猩红。
唐心莲俏脸冷厉的望着战场，旋即其美目望向天空上那里是双方的高层战斗，虽然人数不多，但却是能够对战争结果造成极大的影响。
天空上的那些战圈，个个都是踏入了轮回境的强者，而他们的对手，也全部都是异魔王级别的存在，其中甚至还有着不少真王级别的异魔，不过所幸，那些棘手的家伙，都是被联盟这边的顶尖强者拦截了下来。
“也不知道青雉前辈那里能否拦住。”
唐心莲玉手微握的望向这片战场的最上空，那里有着恐怖的波动在散发出来，在那云层中，有着一道青衣身影，而在他的对面，则是一具数千丈庞大的巨大异魔，这异魔极为的狰狞，庞大的身躯上，竟是有着十数只巨大的眼球，一种狂暴无比的凶残波动，弥漫开来。
这只丑陋的异魔，乃是由魔狱十名真王融合而成，这种融合，令得这新生的怪异异魔实力极端的恐怖，甚至堪比渡过两次轮回劫的巅峰强者，这种存在，在乱魔海联盟中，也唯有青雉能够阻拦。
显然，为了能够抵挡下乱魔海联盟的剿灭，这魔狱部队，也是拼尽了所有的力量。
“吼！”
青雉的面色，也是格外的凝重，他的体内，有着铺天盖地的青光席卷而出，而且他的身体，竟然也是在此时疯狂的膨胀起来。
衣衫破裂，短短数息间，青雉便是化为一道数千丈庞大的龙人，青色的鳞片，布满着他的身体，闪烁着凌厉的寒芒，龙臂之上，青筋如同虬龙般耸动着，上万道青龙光纹，在其周身缠绕低吼。
青天化龙诀乃是青雉所创，如今这等武学，由他施展出来，那等凌厉强横程度，足以让任何人都为之惊叹。
“吼！”
青雉脚踏虚空，空间蹦碎，他已是化为一道青光，暴冲向那狰狞巨魔，龙拳呼啸间，数万道龙纹随之奔涌，快若闪电般的落到那巨魔身体之上。
砰砰砰！
面对着青雉这般尽数凶残的攻势，那巨魔也是仰天嘶吼，锋利的魔掌狠狠的轰出，竟是没有丝毫的退避，直接是与青雉正面的憾在一起。
两道庞然大物，在天空上交缠，没有太过精妙的招式，有的只是那种足以轰爆空间的恐怖力量，拳拳到肉的低沉闷声，甚至是盖过了漫天的杀伐之声。
“嗤！”
青雉锋利的龙爪狠狠的自巨魔身体上撕裂而下，一大块泛着黑色的血肉被他生生的撕下来，黑色射出间，那巨魔也是咆哮着，一拳狠狠的轰在青雉身体上，那股力量，甚至是将其身体上的青色龙鳞都是震碎而去，也是将其轰飞了数千丈。
“噗。”
青雉龙目之中血丝攀爬，后背一震，数千丈庞大的青龙巨翼伸展而出，龙翼一震间，便是再度冲出，与那巨魔轰撞。
唐心莲望着天空上那种可怕的厮杀，美目也是一凝，冷声喝道：“第三部队，动手。”
就在她喝声刚落时，只见得那异魔军队的遥远后方，突然也是爆发出惊天般的杀意，而后只见得铺天盖地的人影自后方犹如尖刀般的冲来，直接是将异魔军队一分为二。
突如其来的生力军，也是令得联盟大军精神一振，一时间，竟是逐渐的取得上风。
“嗷！”
而那些异魔军队似乎也是察觉到局势不对，突然有着惊天动地般的嘶吼声响彻而起，那吼声中，充满着暴戾与疯狂。
咻咻咻！
吼声传荡，只见得天空上那些异魔强者身体突然尽数的爆炸而开，而后化为一道道黑色血光，冲进了最上空那正在与青雉激烈僵持的巨魔体内。
吼！
突然接受到这般恐怖的力量，那将近万丈的巨魔身躯竟然是再度膨胀起来，身体连肉体表面，都是裂出巨大的血痕，显然是有些承受不住那种力量。
嘭！
巨魔凄厉嘶吼，一拳轰出，洞穿空间，狠狠的落到青雉身躯之上，一拳便是将其轰飞而去，后者身体上龙鳞尽数的炸开，鲜血直流。
显然，这些异魔部队，已是开始疯狂，融合了这么多异魔的巨魔，实力已是无限的接近渡过三次轮回劫的巅峰强者，不过这也已经抵达极限，若是再融合的话，恐怕这异魔便是会承受不住的自爆而去。
而且，即便他们渡过这场战斗，也无法再活命，他们这完全是在用命抵挡着联盟大军的脚步。
“吼！”
青雉见状，眼中也是杀意暴涌，龙嘴之中传出一道咆哮，青光席卷天地，竟是再度冲出，数万道龙纹，化为一条万丈青龙，咆哮着冲向那疯狂的巨魔。
这般时候，他根本退不得！一旦退后，让这巨魔冲入联盟大军，那死伤就将会极为的惨重。
“嗷！”
巨魔也是疯狂的咆哮着，他望着冲来的青雉，巨嘴巨掌，只见得无数黑色鲜血汇聚而来，砰的一声，直接是化为一道黑色光束，快若闪电般的轰在了青雉身体之上。
砰！
鳞片爆裂，青雉庞大的身躯顿时在联盟军那无数道惊骇的目光中倒飞而出，在其肩膀处，出现了一个巨大的血洞。
“死吧！”
弥漫着杀意与暴戾的咆哮声自巨魔嘴中传出，他锋利无比的魔掌直接呼啸而出，狠狠的对着青雉怒拍而去，那般架势，显然是想要彻底的轰杀青雉。
“帮忙！”
那些联盟中的顶尖强者也是急急怒吼出声，滔天般的元力匹练轰出，试图将那巨魔阻拦，但后者却是不闻不顾，任由那些攻击落到他的身躯上，将他震得不断颤抖。
呼！
魔掌狠狠的拍下，最终直接是狠狠的落向青雉，然后，就在众人眼睁睁的看着青雉即将被重创时，天空上，突然有着一道低沉之声响起，那呼啸而下的魔掌，突然间凝固。
“那是？！”
无数目光猛的一惊，急忙看向青雉上方，只见得那里，有着一道人影凌空而立，他似是仲出右手，轻轻的将那巨魔的魔掌抵挡了下来。
两者体积完全不成比例，然后在他那单薄的手掌下，那巨魔，竟是再也难以有丝毫的寸进！
唐心莲望着那道熟悉的削瘦身影，先是一怔，旋即俏脸上，有着惊喜之色蔓延了出来！
“林动？！”


第1284章 平定乱魔海
天空之上，巨大的魔掌下，一道身影凌空而立，看似单薄的身躯，却是将那蕴含着极端恐怖力量的魔掌轻松的抵挡了下来。
“那是…林动元老？”
那乱魔海联盟大军中也是在此时爆发出一些惊呼声，自从一年前联盟建立后，林动便是消失了踪迹，据说是闭关修炼，没想到在这关键时刻竟然出现了！
“这家伙，总算来得及时。”
唐心莲俏脸也布满着欣喜之色，如今这天地间处处都是战场，不过总体说来，三处主战场中，都并没有双方最为巅峰的强者出手，因为现在魔域的最厉害的那些家伙，都被远古之主他们钉在西玄域，不过相应的，远古之主他们也再无法给这外面的战场多大的帮助，这种时候，林动的出现，无疑是能够取到极为重大的作用。
“青雉前辈，没事吧？”林动挡住那魔掌，然后偏头看向身后的青雉，问道。
青雉身体上青光闪烁，身形则是逐渐的变回人形，他抹去嘴角的血迹，冲着林动苦笑了一声，道：“出关了啊？”
林动点点头，笑道：“这大家伙交给我来吧。”
青雉倒是没有异议，如今的林动，身体上似乎并没有任何的波动散发出来，但隐隐的，他却是能够感到一种极端危险的气息，显然，这一年的闭关，林动的实力似乎提升到了一个他看不透的地步。
这种变化，让得他有些嘘唏与欣慰，遥想当年第一次见到林动时，后者才只是一个为了从那小小的百朝大战中脱颖而出的少年而已，那时候他看后者似乎有些天赋，这才将青天化龙诀教给了他，但谁能想到，在这多年后，当年那个曾经的少年，竟是已将他都是超越了去。
“小心一些，这巨魔融合了十多位真王，虽然恐怕他们活不了多久，但这时候极为的狂暴。”青雉提醒了一声，然后便是落下身形，由众多联盟强者将其护在其中。
“融合异魔吗？”
林动倒是并不意外，这与那元门三巨头采取的方式倒是有些雷同，不过这些家伙显然更为的凶残，但这应该也是他们融合的极限了，再强的话，恐怕便是会自爆了。
“吼！”
那巨魔也是抽回魔掌，暴戾的魔目盯着林动，一声愤怒咆哮，只见得其庞大的身躯上，竟是有着犹如泉水般的黑色血液暴射而出，这些血液蠕动着，有着邪恶到极点的波动散发出来。
“魔血之界！”
巨魔狰狞巨嘴中，有着嗡鸣的怒吼声传出，旋即那黑色血液竟是猛的扩散而开，犹如光球一般，将林动笼罩而进，然后血球瞬间收缩，随着血球的收缩，一种无法形容的可怕波动，也是飞快的凝聚而起。
咻。
那血球眨眼间，便仅仅数丈大小，其中林动的身形已被掩盖，那巨魔魔掌陡然握下，就欲捏爆血球，彻彻底底的轰杀林动。
嗤！
不过，就在他魔掌即将握下的霎那，一只修长的手掌，却是噗通一声，直接是从那血球内部洞穿而出，然后犹如撕碎薄纸一般，将那凝聚了恐怖能量的血球，生生的撕成两半。
林动微微一笑，手掌一翻，那破碎的血球便是被其吸入掌心，滴溜溜的旋转间，化为一颗仅有拳头大小的黑色珠子。
“去。”
林动屈指一弹，那黑色珠子的表面，仿佛是掠过一抹混沌之色，而后珠子便是化为一道黑光，掠过天际，直接是射在了那巨魔胸膛之上。
嗤嗤。
那黑光上似乎并不具备多大的力量，然而在射中那巨魔身体时，后者那足以抵挡青雉全力攻击的魔身，竟直接是被那颗细小的魔珠洞穿而去，最后深入他的身体。
“蠢货，用我们的魔气来杀我们吗？！”
那魔珠冲进巨魔体内，后者却是发出刺耳的狂笑声，那魔珠入体，不仅没有对他们造成丝毫伤害，反而还令得他们体内魔气暴涨。
林动闻言，却是一声轻笑，手掌轻轻握下：“是吗？”
嘭！
就在林动手掌握下的霎那，那巨魔狂笑声陡然凝固，他那庞大的身躯也是激烈的颤抖起来，只见得在其体内，竟是有着一道道混沌色泽的光束暴射而出。
啊！
在那种混沌光束下，巨魔体内无穷无尽般的魔气，竟是以一种极端惊人的速度被化为虚无，那巨魔的身体更是在此时分裂而开，最后化为十数道魔影倒射而出，黑血狂喷。
那巨魔，竟是被林动将那些融合在一起的真王，生生的轰了出来。
那些真王被轰出，个个都是面色惨白，望向林动的眼中有着浓浓的惊惧之色，不过还不待他们有什么举措，林动已是面色漠然的出手“轰隆！”
一道雷龙划过天际，这雷龙栩栩如生，庞大的身躯之上，雷霆闪烁，比起以往凝聚的雷龙，如今的这一条，却是犹如真的活物一般，那种气势，相当的骇人。
咻。
雷龙暴冲而下，沿途过处，那一尊尊真王级别的异魔，竟直接是在那凄厉惨叫声中，彻彻底底的爆炸而开，雷霆之力弥漫，将魔气尽数的消除。
那些真王见到林动这般凶悍，顿时骇得急忙逃窜。
林动见状，袖袍一挥，只见得在那些真王后方，一道道千丈庞大的黑洞蔓延而出，黑洞旋转，犹如巨嘴一般，直接是将他们生生的吞噬而进。
短短十数息的时间，那十来尊真王级别的异魔王，竟然便是被林动清除得干干净净。
这一幕，却是看得下方战场中无数人有些目瞪口呆。
林动解决掉那些真王，然后那漠然的漆黑眸子，便是望向了下方的战场，这是他第一次看见如此浩大的战场，这战场几乎弥漫了整个北海域，目光所及之处，全都是魔气与元力之间的对碰。
这种数量的异魔，怕起码都是数百万吧，而联盟大军这边的数量，也丝毫不比其少，而且这些大军可不是寻常的军队，其中每一个人，都是拥有着相当不弱的实力。
林动心神一动，天空上，无数雷云汇聚而来，轰隆隆的巨声响彻天地，竟是将那滔天般的厮杀声都是遮掩了下去。
“这边天地，不是你们该来的地方。”
雷霆涌动，林动脚踏虚空，他淡淡的望着那数不尽的异魔，眼中也是有着冰冷之色涌起来：“所以，被净化吧，或许，还能魂归你们该去的地方。”
轰隆！
天地在此时颤抖，只见得雷云膨胀，无数道蕴含着恐怖力量的雷霆，在此刻疯狂的降临而下，雷霆横扫过处，无数异魔连惨叫声都是未曾发出来便是被生生的蒸发而去。
突然遭受到如此恐怖的攻击，那异魔军队原本凶悍的气势也是为之一滞，一些异魔眼中，终是掠过恐惧之色，失去了那些真王的统领，他们显然再不复之前的威风。
“杀！”
唐心莲美目中寒芒闪过，立即抓住异魔军队气势变弱的瞬间，一声冷喝，那联盟大军便是爆发出惊天杀气，滔滔元力，席卷而出，顿时将异魔军队冲击得有些七零八落。
不过这些异魔的确是凶悍，即便是面对着这般局势，他们依旧是死死的抵挡着，同时也是给联盟大军造成了一些伤亡，那等鲜血，染红海面。
但这种拼死抵抗在大势已去的情况下，显然也起不到太大的作用，到得最后，异魔军队，抵抗终是越来越如。
“嗷！”
似也是察觉到了必败无疑，那些异魔军队中，突然爆发出道道嚎叫之声，而后无数异魔突然对着海面冲去，显然是想要打算进入海底暂时撤退了。
这些异魔极为的顽强，只要能够逃走，依旧是麻烦。
不过面对着他们的撤退，唐心莲俏脸上却是掠过一抹冷笑，冷喝声，响彻而起：“海妖军，所有异魔，一个都不准放过！”
轰！
下方大海，在此时突然掀起无数涛浪，只见得那大海仿佛都是被分割而开，无数骑着各种海兽的海妖军队，自海底暴冲而出，隐隐的，似乎是有着一个极端庞大的光阵在海底成形，封锁了海底的退路。
天空上，林动望着那近乎前后左右上上下下都是被堵死了退路的异魔军队，也是缓缓的松开手掌，对着远处的唐心莲竖起大拇指，她还是这么的厉害，所有退路都是设置得妥妥当当，丝毫不给这些异魔军队有丝毫的活路可走。
在那千军万马中，唐心莲也是见到了林动这般动作，那红润性感的嘴唇顿时挑起一抹骄傲的弧度，那凤目倒是不闪不避的盯着林动，眼中的柔软与妩媚，让得林动顶不住的闪开了眼睛，而后她这才娇笑得花枝乱颤。
林动轻轻的吐了一口气，只要完成接下来的剿杀，乱魔海的战乱就算是被平息了，接下来只要妖域以及西玄域平定的话，或许，就该与那魔狱，进行最后的决战了。
“他们，是在盯着那些魔狱的高层么…”林动望向四玄域的方向，喃喃道。
而此时，在那遥远的东玄域与西玄域交接的一座山峰上，盘坐于山巅的一道冰蓝色倩影，突然缓缓的睁开了美目，她也是有所感的望着乱魔海方向，带着雪白手套的小手缓缓紧握。
他…出关了么。


第1285章 四玄域联盟
乱魔海的战争，联盟军最终取得了胜利，当然，这之中虽然有着林动参战的原因，但更多的，却是因为这支魔狱军队，并没有获得任何的援助，以一种状况，面对着拧成一根绳的乱魔海联盟，他们自然是没有多少的胜算。
不过即便如此，在赢得战争后，乱魔海联盟为了彻底的清除这些异魔，依旧是耗费了整整五日的时间，这才算是彻彻底底的将乱魔海中的异魔铲除。
战争稍落，那些之前被魔狱军队占据的海域，却是惨不忍睹，魔气甚至使渗透到海水之中，令得其中生灵涂炭，那各种尸体漂浮起来，将一片片海域都是堆满了去，恶臭弥漫着天地。
而见到这般人间惨剧，联盟军队也是个个双眼通红，这些异魔，的确是太过的暴虐凶残，若是不将他们彻底的清除，恐怕这世间永无宁日。
不过这些被魔气污染的海域，现在显然是没有太多的时间来将它们净化，乱魔海战争一结束，这里的联盟军队便是在唐心莲的指挥下，一分为二，一半支援四玄域联盟，一半支援妖域联盟，如今只有迅速的将三大区域的异魔尽数荡除，他们才能够没有后顾之忧的与魔狱展开最后的决战。
而林动在略作思量后，带领着一半的联盟军队，先行前往四玄域，那里毕竟接近西玄域，虽然应欢欢他们时刻在盯着魔狱，但他还是尽快赶去为好，免得出了什么岔子。
至于妖域，想来有着唐心莲率领而去另外一半乱魔海联盟军队，应该足以逆转局势，解决掉妖域的动乱。
而对于林动的这般安排，唐心莲也是分外的赞成，所以待得乱魔海局面彻底稳固后，两人便是各自带着铺天盖地的人马，各奔东西而去。
东玄域，落日平原。
这里是东玄域与南玄域交接之点，同时也是四玄域联盟大军驻扎之地，在这片平原的对面百里外，便是肆虐在四玄域上的魔狱军队。
而此时，在那平原上，正操练着四玄域的联盟军队，整天般的喝声响彻而起，而在那军队正前方的高台上，数道身影而立，那最前方，是一道纤细的倩影，她身着黑色裙子，体态纤细苗条，不过那张俏美精致的小脸上，正满是肃杀之意，漆黑得犹如夜空般的美眸，缓缓的扫视着这辽阔无尽的平原。
这般身影，除了青檀之外，还能有着何人，如此的她，不仅仅是黑暗之殿殿主，而且还是四玄域联盟的盟主，她的年龄或许极小，但伴随着这一年来与魔狱的各种血腥战斗，她已是用自己的能力证明了自己联盟统治者的地位，因此，在那平原上，有着无数道目光望向她时，都是弥漫着尊敬之色。
在青檀身后，绫清竹依旧是一身白色衣裙，气质清冷，那绝美的脸颊完美得没有丝毫的瑕疵，不知为何，如今的她比起以往，似乎是变得更为的有灵气，而且隐隐的，还有着一种神秘飘渺的味道，令人看去，便是有些心生敬畏。
“嗯？”
绫清竹注视着平原，突然神色微动，偏过头望向遥远的天边，那里，突然有着强烈的空间波动传来，紧接着，大批的身影，犹如蝗虫一般铺天盖地的暴掠而来，那种滔天般的气息，令得天地都是震荡了起来。
“呜！”
就在这变故出现时，这四玄域联盟之中，顿时爆发出警戒的呜鸣之声，旋即无数身影瞬间腾空而起，元力也是冲上天际。
“不必惊慌，那是乱魔海联盟的盟友。”绫清竹清澈的声音在此时传开，这才令得平原上那剑拔弩张的气氛缓和下来。
“是乱魔海的联军吗？他们难道解决掉乱魔海中的异魔了？真是厉害！”
“有这些兄弟赶来，这下我们倒是不怕那些异魔了，这次一定能清除掉他们！”
“是啊！”
随着戒备散去，四玄域的联军中，则是爆发出惊天般的欢呼声，无数人对着那遥远处犹如蝗虫般掠来的大军激动的挥着手臂。
“咦…那是…林动哥？！”
青檀虽然因为联军感到小脸上的冷肃微微的收了一些，不过还是保留着四玄域联盟盟主的气势，但待得那联军愈发接近时，她方才猛的见到最前方那道身影，当即小脸上冷肃变脸般的散去，那副惊喜的模样，倒是看得一旁那些四玄域联盟高层有些愕然，等到他们听见林动的名字时，这才微微的恍然。
一边的绫清竹也是微点螓首，眸子望向那个方向，虽然她那绝美的脸颊依旧是清清冷冷，但众人便是从其眸子中看出了一些柔软与温度，这不由得让一些人暗叹着摇摇头，两女如今在四玄域联盟中，不知道有着多少天才俊杰对她们生爱慕，如今看来，他们是全没指望了，摊上这么一个妖孽般的竞争对手，哪还有什么竞争的勇气。
远处的联军迅速的掠来，最后也是落到了这辽阔的平原之上，林动身形一动，直接是出现在了那高台上。
“林动哥！”
青檀惊喜的娇声道，那纤细的娇躯已是如同乳燕般的扑了过去，林动连忙仲手将她给接住，一手揽住那盈盈一握的小蛮腰，然后宠溺的摸摸青檀小脑袋，笑道：“都是四玄域联盟的盟主了，还跟长不大一样。”
青檀嘻嘻一笑，那娇憨纯真的模样，哪还有半丝在联盟中的那份冷厉肃杀，全然是一个不韵世事的天真少女，不过唯有尝试了她那些手段的联盟强者，方才知道，在这看似娇憨的外表下，隐藏着多么令人敬畏的手段。
林动与青檀笑闹一番，然后看向绫清竹，冲着她温暖一笑，后者见到他的笑容，素来清冷的脸颊倒是微微一红，侧开了脸去。
“这边情况如何了？”林动问道。
“我们四玄域联盟一直在与魔狱的这支军队僵持，不过林动哥你也知道，论起实力，三大联盟中，恐怕我们四玄域联盟最弱，所以一直未能取得太大的上风。”青檀有些无奈的道。
林动对此倒是不意外，四玄域之中，西玄域已经尽数的落在魔狱掌控中，这般实力，与乱魔海与妖域两大联盟比起来，是要差一些，青檀她们能够一直的僵持下来，已经很不错了。
“没事，我此次带来了乱魔海联盟一半的力量，应该足以抹除掉这支魔狱军队了。”林动笑道。
青檀点点小脑袋，有了这些生力军的加入，接下来的局面，必将会是一面倒了。
林动拉着青檀走近绫清竹，他望着后者，突然愣了愣，显然是察觉到后者的一些奇异之处，而且最令得他惊讶的是，他竟然察觉不到绫清竹如今究竟处于什么层次。
“不用感应了，我将元力都散去了。”察觉到林动的目光，绫清竹却是微微一笑，道。
“散去？”林动心头一震，有些骇然的道：“你怎么能给元力给散了？那你现在…”
话到此处，他又是一顿，虽然他的确没有自绫清竹身上察觉到元力的波动，但隐隐的，却是有种熟悉的感觉。
绫清竹仲出玉手，只见得在其玉手上，突然有着淡淡的混沌之光凝聚出来，一种神秘而强大的波动，散发而出。
“这是…太上之力？！”林动震惊的道，这种力量，竟然是那种感应太上而来的力量。
“嗯，我换了一条路子走，不修元力，静心感悟太上，将那种力量收敛入体。”绫清竹微微点头，道：“现在我也不清楚我处于什么层次，不过似乎比以前强很多了。”
这时候即便是以林动的心境，都是忍不住的倒吸一口冷气，什么叫做换一条路子走？这世界上除了修炼元力与精神力，还有其他路子走？
他知道绫清竹所说的这条路，应该便是收纳太上之力，但这世界上，有几人能够感应到太上？而且就算感应到了，又怎么可能将那种太上之力吸纳入体？这一点，恐怕现在的林动都做不到。
再有就是，绫清竹也太胆大了吧，辛辛苦苦修炼数十载的元力，说散就散，简直比他摧毁泥丸宫还干脆利落。
“没有了其他的力量，才能保持一颗纯净的感应之心，这似乎也挺不错的。”绫清竹轻声道，那绝美的脸颊，在此时有着一种飘渺若仙的味道，她气质本就清冷，再加上这般味道，真是有种仙女如凡尘的感觉。
林动摸摸鼻子，他原本以为自己已经挺变态的了，没想到绫清竹更变态，这种事情，他可从未想过，看来在对于感应那所谓太上这一上面，绫清竹显然有着他比不了的天赋。
“这些乱魔海联盟军队，就交给我与青檀吧，接下来我们会将这支魔狱军队解决掉，现在的话，我建议你去看看她。”绫清竹玉手锊起一缕青丝，清澈眸子看着林动，道。
“她？”
林动先是一愣，旋即猛的回过神来：“是欢欢？她怎么了？”
“你还是去看看吧，你应该能感应到他们的位置。”绫清竹微微摇头，道。
林动见状，心中不由得升起一抹不安的感觉，旋即点了点头，也就不再多说，身体上银光闪烁，直接是消失而去。
绫清竹见到他消失的身影，也是轻轻一叹。


第1286章 争吵
东玄域与西玄域交接处，这里是接近西玄域的最前线，只要再前个千里左右，便是能够进入西玄域的界限，那里，已经是魔狱的大本营。
而在这交接处的一座巍峨山峰上，数道身影静静盘坐，他们的身形在山峰之巅显得颇为的渺小，但却自有着一股屹然之态，令人心生敬畏。
符祖一脉，自那远古开始，便是受这片天地无数生灵尊之敬之，这不仅仅是因为他们那屹立在天地之巅的实力，还有着他们那种舍身大义之胸怀。
当年为了护得这无数生灵，符祖燃烧轮回，封印异魔皇与位面裂缝，这才避免了这天地间惨遭沦陷之局，他的八位弟子，也是为这天地倾尽了所有，天地间所有的强者在谈及他们时，无不是有着一种发自内心的敬佩，而这也是为什么当应欢欢他们露面，却是能够轻易的促成三大联盟，这并非是单纯的实力压制，而是他们曾经为这天地间的付出。
在那山巅最前方，应欢欢静静盘坐，此时的她并没有闭目修炼，那带着白色手套的纤细玉指，漫无目的的在石头上画着圈圈，那副心不在焉的模样，即便是一旁的炎主他们都是看得出来。
而这种情绪，一般在她的身上，可是极难看见的。
炎主他们对视一眼，皆是苦笑着摇摇头，自从感应到林动出关后，她便是这般模样了…不过至少比起之前冷冰冰的模样要好许多。
“乱魔海现在已经平定，据说唐心莲已经率领一半乱魔海联军前往妖域相助了，而林动则是率领了另外一半来四玄域。”炎主突然开口道。
听得那个名字，应欢欢画着圈圈的玉手终是顿了顿，然后微微点“等妖域与四玄域这边将那些魔狱的军队解决后，我们就能进攻西玄域了。”黑暗之主眉尖微微的粗了蹙，目光看向遥远的西玄域，即便是隔着如此遥远的距离，但他们依旧还是能够察觉到那里所涌动的恐怖魔气。
“也不知道魔狱究竟是在做什么，他们…可是太平静了。”
雷主等人也是点点头，这段时间魔狱几乎没有任何的动作，竟是任由他们将那三支魔狱军队清除，显然，以魔狱那狡诈的性子，绝对不会做出这种平白牺牲的蠢事。
这样说来的话，他们必然是有着一些不为人知的目的。
提及正事，应欢欢美目中冷色也是凝聚过来，她玉手紧握，略作沉吟，道：“我们可以准备招魂引了。”
炎主他们闻言皆是一怔，旋即也是点点头，的确是差不多了啊。
“那就准备吧…”
黑暗之主起身，话音刚落，她脸色突然古怪了一些，偏过头望向远处的天空，然后来到应欢欢身边，低声道：“似乎现在还不行啊。”
炎主他们也是有所察觉，望向那个方向，只见得那远处空间扭曲，一道光影以一种惊人的速度暴掠而来。
应欢欢显然也是感觉到了那股熟悉的气息，那冰彻的眸子中，竟是掠过一抹急促与紧张之色，那玉手也是忍不住的紧握起来。
“呵呵，诸位都在这里啊。”空间扭曲间，一道身影已是出现在了这山巅上，林动望着炎主等人，拱手一笑，道。
炎主几人也是看向林动，旋即他们眼中皆是掠过一抹惊异之色：“你凝聚神宫了？”
他们的声音中有些难掩的震动，对于凝聚神宫有着多么的困难，他们再清楚不过，即便是现在的他们，都不能说是神宫已成，而眼下见到林动能够完成这一步，自然是略感震惊。
“侥幸而已。”
林动落下身来，笑着摇摇头，然后便是将目光看向了那山巅前的应欢欢，后者身着素色长裙，冰蓝色的长发在山风的吹拂下轻轻的飘舞着，那清澈的美目在见到他看过来时，略微有点躲闪。
“你们聊聊吧。”
黑暗之主见状，也就对着炎主他们挥挥手，就欲转身而去。
“师姐！”应欢欢见到他们要离开，那冰彻的心中竟是掠过淡淡的慌乱，急忙想要留下他们，但几人身形一动，已是消散而去。
林动瞧得黑暗之主他们暂时离开，那视线便是停留在了应欢欢身上，然后大步的走了上去，而应欢欢见状，那面对着魔狱天王殿都丝毫不曾有丝毫减弱的气势竟是滞了滞，不由得小退了半步。
“你怎么了？我出关了难道还不开心？”林动站在林动面前，笑道。
“没有啊。”应欢欢摇摇头，美目却是微微闪烁。
“那你就看着我说话。”林动也是第一次见到她这般躲闪姿态，不由觉得有些有趣，竟是伸出手来，轻轻的挑着应欢欢那尖俏的雪白下巴，将其缓缓的抬起，然后四目对视。
应欢欢也是因为林动这番举动怔了下来，她望着面前那张似乎愈发坚毅的年轻脸庞，后者眼中涌动的微笑暖意，仿佛令得她心中都是微微的一烫。
“恭喜你啊，凝聚出神宫了。”应欢欢轻声道。
林动笑了笑，然后他便是看见了应欢欢那在长袖雪白手套的包裹下显得格外纤细的双手，而应欢欢似乎也是察觉到他的目光，双手不由得微微收了收。
不过林动还不待她将双手收回，便是一把将其玉臂抓了出来，手掌握在手套上，有着一股惊人的寒气散发出来。
“怎么回事？”林动眉头微皱，道。
“没什么的。”应欢欢挣扎了一下，想要将手臂收回来，但林动手掌却是犹如铁钳一般，目光盯着她，旋即伸出手来，将其手套轻轻的摘了下来。
应欢欢见状，心中一声轻叹，只能任他为之。
雪白的手套褪下来，露出来的，却是一片晶莹，那原本洁白如玉的纤细手臂，却是在此时变得晶莹如冰，看上去，就犹如寒冰所铸一般。
林动有些震惊的望着这近乎寒冰般的双臂，他手掌握在上面，一种极端恐怖的寒气顿时渗透出来，竟是令得他手掌传出剧烈的刺痛感。
嗤嗤。
寒气在林动手掌上凝聚，很快的，林动手臂上便是有着薄薄的冰层覆盖，在这些冰层的覆盖，他感觉到体内的元力，都是变得滞涩了起来。
“这是怎么回事？！”林动却是不理会那些覆盖手臂的寒冰，盯着应欢欢，声音之中都是有了一丝怒意。
应欢欢贝齿轻咬着嘴唇，却是倔强的不肯说话。
“是你开启祖宫阙的后遗症？”林动心思转动，很快的便是想起了那时候应欢欢的一些古怪之举。
“为什么不告诉我？！”
“告诉了你，你还会进去吗？”应欢欢也是盯着林动，道。
“进不进去是我的事！我似乎跟你说过，让你不准骗我！”林动恼怒的道。
“就算不是为了你，我也会开启祖宫阙，如果不是因为祖宫阙，现在的乱魔海，妖域，四玄域，你知道还会死多少人吗？！”应欢欢显然也是第一次见到林动这么对她发火，那心中不由得升腾起一些委屈，但依旧是咬着银牙道。
“你就这么喜欢去当什么救世主吗？！”林动火道，他拼了命的去修炼啊，她倒好，也拼了命的折腾自己。
“那不然呢？任由魔狱扫荡，然后大家一起死？！”应欢欢紧握着玉手，道。
“好，好，你这么喜欢去做救世主，那你就去做吧，我不管你了，可以了吧？！”林动怒目圆睁，道。
应欢欢娇躯一颤，她紧紧的盯着林动，娇躯颤抖着，然后偏过头去，眼眶却是通红了起来，水花顺着脸颊滑落而下。
“那你就别管了！我死了也不要你管！”应欢欢甩开林动的手掌，声音仿佛是多了一丝哽咽。
林动望着她那轻轻颤抖的娇躯，此时的她显得格外的柔弱，再没了那种冰冷的气势，这令得他心疼了一下，伸手再度抓去，但却被应欢欢倔强的闪避开去。
林动却是不理，一把蛮横的抓过她的手，狠狠一用力，便是将其扯进怀中，双臂犹如铁钳般将那带着一些冰凉，但却触感极佳的柳腰搂住。
应欢欢则是使劲的挣扎着，不过她似乎是忘了，她实力比林动还强，真要动手的话，恐怕林动也挡不住。
“不许动！”
林动恼怒的喝道。
应欢欢咬着银牙，道：“要你管个什么。”
林动却是板着脸，直接低头，狠狠的对着那一抹红润吻了下去，碰触之间，两人的身体仿佛都是陡然僵硬了下来。
有玉微凉，是为樱琅。
应欢欢显然是被他这般举动惊呆了去，冰蓝美目望着那近在咫尺的脸庞，竟是不敢再有丝毫的动弹，那俏脸上，也是有着一抹淡淡的红霞涌了起来。
山巅上，两人相拥，久久之后，方才唇分。
“你…”应欢欢终于回过神来，有些红的眼睛将林动羞恼的瞪着。
“你什么你？”林动舔了舔嘴唇，然后他紧搂着怀中的女孩，漆黑眸子中掠过一抹淡淡的神采，将嘴凑近她的耳畔，轻声道：“刚才你的样子，好像以前的时候…”
像以前那个活泼娇俏，偶尔会任性，偶尔会娇蛮，但却让人忍不住的将她想要捧在手心里面的少女。
应欢欢轻轻的靠在林动胸前，感受着那温暖的温度，她美目中再度有着水花凝聚起来，而后她近乎呢喃的轻声道：“我…我也不想骗你啊，可是…可是我真的没有选择啊。”
听得怀中女孩那带着一丝哽咽的声音，林动心中也是升起一抹怜惜，然后将她紧紧的搂着，重重的道：“我会保护你的。”
应欢欢微微闭目，娇嫩脸颊轻轻的磨挲着林动胸膛，唇角有着一抹微笑，只不过那微笑中，带着一丝淡淡的苦涩。
有你这句话，无论结局怎样，我都心满意足了呢。


第1287章 生死之主
山巅之上，两人相拥许久，应欢欢最后方才将林动推开，她抿了抿红润的嘴唇，有些羞恼的盯了林动一眼，道：“都怪你，肯定都被他们看见了。”
“看就看见了吧。”林动倒是无所谓，旋即他微笑的看着应欢欢，道：“怎么发现现在你反而变得更像以前那样子了？”
“可能是回光返照吧。”应欢欢漫不经心的道，不过在见到那陡然又板起来的脸庞时，这才淡淡一笑。
“我们准备进行“招魂引”了。”
“招魂引？”林动一愣。
“大师姐一直没有动静，不过我能感觉到她似乎已经成功轮回，接下来待得三大地域平定，我们就会与魔狱决战了，所以也得将她召出来。”应欢欢的道。
“大师姐？”林动皱了皱眉，想了想，这才道：“是，生死之主？她真的还活着啊？”
“大师姐掌控生死之力，我们几人中，要说谁最不容易陨落的话，恐怕就要非大师姐莫属了。”应欢欢道。
“这样强行召唤，不会有什么坏处吧？”林动问道。
“我能感应到大师姐轮回成功，如今再召唤的话，应该不会出问题。”应欢欢道。
林动微微点头，生死之主也是远古八主之一，若是大决战开始，能够有着她的帮忙，想来也是能够让他们多一些胜算。
“我们布置招魂引的时候，就麻烦你帮我们镇守了。”
“嗯，交给我吧。”
两人谈话完毕，应欢欢玉手一招，山巅上光芒闪烁，炎主几人便是闪现而出，他们望着两人，眼神还略有点古怪。
“咳。”
林动见到他们的眼神，不由得干咳一声，一旁的应欢欢俏脸也是微红，旋即稳下心神，道：“我们开始吧。”
炎主几人也是点点头，旋即他们身形一动，皆是掠至半空，恐怖的元力波动，犹如潮水一般的自他们体内弥漫出来。
元力扩散，最后化为一道道光线交织，隐隐的，这些光线，似乎是化为了一个极端晦涩复杂的阵法，而应欢欢六人，则正好处于正好的边缘六角。
一圈光罩升腾，最后将六人尽数的笼罩，应欢欢看了一眼黑暗之主，后者也是轻轻点头，屈指一弹，一道黑白光芒突然掠出，悬浮在阵法中央。
那是一个细小的光团，光团之中，似乎有着一缕黑色发丝盘踞着，隐隐的，有着一种奇特的波动散发出来。
应欢欢六人皆是凌空盘坐下来，双目缓缓紧闭，而那黑色发丝则是无火自燃，一道玄妙无比的波动，悄然扩散，最后消失不见。
天空上的六人，也是尽数的寂灭无声。
林动盯着他们，片刻后，在并未见到有什么异状后，这才微微点头，在那山巅盘坐下来，他知道，接下来便需要静静的等待了，或许，等下次再出现在天边的身影，便是那位传说之中的生死之主了…
而对于这位一直未曾见过面的远古之主，林动的心中，倒是有着一点好奇之心。
想要将那生死之主召唤而来，显然也是需要一些时间，林动在山巅这一坐，便是十日时间过去。
这十日中，天空上的应欢欢六人没有丝毫停歇的迹象，一道道奇异的波动若有若无的散发着，但至于那生死之主，林动倒是一直未曾见到她出现过，看来，要将其召唤出来，也不是什么简简单单的事。
而在林动守在这里的十日中，妖域与四玄域也是异常的震动，有了乱魔海联军的加入，两大联盟实力皆是暴涨，从而直接开始对那些魔狱军队动手。
双方杀得天昏地暗，整整十日时间，那战斗的动静，几乎传遍了妖域以及四玄域，战斗唯有以惨烈来形容，鲜血洒满大地，天空都是被浩浩荡荡的身影所遮掩。
那种战争，唯有以可怕二字方能形容。
只是魔狱军队虽说强横，但毕竟毫无后援，而且三大联盟互相联手，士气高涨，短短十日间，魔狱那两支西玄域之外的军队便是尽数的溃败，接着在联盟军队的追杀中，溃不成军，死伤殆尽。
而局势到了这一步，三大地域，几乎都算是逐渐的平定，等到他们将战场收拾，略作整顿，然后，那三大联盟的联军，开始在此时尽数的对着四大玄域汇聚而来。
那种声势，无疑是遮天蔽日，当四玄域上的人见到天空之上那犹如黑云一波波掠过的军队，无不是满脸的震撼，与这一次的真正战争相比，东玄域元门那一次，真是只能算作小儿科。
三大联军，齐聚四玄域，最后经过整顿，开始逐渐的对着西玄域逼近，那种浩浩荡荡之势，所形成的一种威压，甚至连天空都是变得有些昏暗。
那种由无数人汇聚在一起磅礴战意，令得天地都是有些颤抖。
而在三大联军对着西玄域逼近时，那远在西玄域边缘巍峨山峰上林动也是有所感觉，他睁开双目，面色略显凝重的望着极为遥远的方向。
“决战要开始了吗¨
林动喃喃自语，那种山雨欲来之前的宁静，却是连他都是有种喘不过气的感觉，这一次，他们再没有符祖那种能够顶起大梁的传奇存在，想要护住这片天地，就只能依靠他们自己了。
林动深吸一口气，然后目光望着天空上的光阵，这已经第十五天了，但那召唤，已经没有什么动静，如果不是应欢欢那般肯定的话，林动都会有些怀疑，那位生死之主，是不是已经轮回失败了…
而如果真是这样的话，这对于他们而言，无疑是一个相当重大的打击。
他皱着眉头，不过就在他心中有点不安时，天空上的光阵突然一颤，然后他便是见到一道耀眼的光束，猛的直冲天际而起。
那道光束极为的奇妙，虽然声势浩大，但仿佛常人感觉不到，就连林动，都是在晋入了神宫境后，方才能够感觉到。
应欢欢六人也是在此时微微睁开眼睛，眼中有着一抹紧张之意，若是连这招魂之光都无法将生死之主招来的话，那就说明他们失败了。
“现在，就只能等待回音了啊…”他们对视一眼，他们所能做的，已经尽数做完了啊而就在那道奇异光束冲天而起时，远在东玄域的大炎王朝大荒郡，却是起了一些变化。
在大荒郡中心的一座城市中，一道娇小的身影坐在高高的阁楼上，细白的小腿轻轻晃着，小嘴中还哼着欢快的歌曲。
小女孩嘴中的歌声，突然间停了下来，她抬起小脸，有些茫然的望着遥远的地方，在她那纯净的大眼睛中，能够看见遥远天空上那一道显眼的光束。
“那是…”
小女孩摆动的小腿缓缓的停下，她那大眼睛中，仿佛是在此时变幻起了无数色彩，旋即其小脸上浮现一抹痛苦之色，似乎有着什么东西，从她脑海深处苏醒了过来。
轰！
一股极端恐怖的气息，突然在此时爆发而起，城市上空，云层扭曲，隐隐的，竟是化为一道约莫千丈庞大的黑白云层，云层蠕动间，化为一道圆形的巨云。
巨云呈现黑白色彩，犹如生死。
一道光影陡然自城市中掠出，然后迅速的出现在那高高的楼阁上，她那个着大眼睛中神采变幻的小女孩，脸颊上掠过一抹惶色，急忙道：“茵茵？你怎么了？！”
名为茵茵的小女孩缓缓抬起小脸，她望着眼前漂亮而亲切的女子，旋即冲着她微微一笑，只不过那笑容，似乎没了以往的那种天真烂漫。
姜雪俏脸有些苍白的望着此刻的茵茵，眼中都是有着泪花凝聚起来：“你…觉醒了吗？”
茵茵从高高的楼阁上走出，凌空踏着，然后来到姜雪的面前，轻轻的投入她的怀中，那嗓音依旧还有些稚嫩：“姐姐，谢谢你。”
姜雪有些颤抖的望着怀中的小女孩，眼睛泛红，道：“你现在，还是我的妹妹吗？”
“觉醒并不代表抹除以前的记忆，虽然我有些变化，不过，我既是生死之主，也是茵茵。”茵茵轻轻的道。
“生死之主？”
姜雪娇躯一震，她只知道茵茵是轮回者，但却没想到，她的前世，竟然会是那远古八主之一的生死之主！
“你…你现在要离开我们了吗？”姜雪喃喃道。
“没有呢，我去帮一下他们，魔狱的事姐姐也知道，不将他们解决掉的话，我们也无法安宁的。
茵茵摇着头，冲着姜雪一笑，道：“姐姐，放心吧，等解决掉这些事，我就回来，好吗？”
姜雪无言，只能点点头，仅仅的将小女孩抱入怀中，仿佛生怕失去她一般。
茵茵也是抱着姜雪的颈项，然后她在姜雪雪白脖间轻吻了一下，接着她的身体便是缓缓的变淡，直到最后彻彻底底的消失而去。
“一定要回来啊。”
姜雪望着空荡的怀中，泪水终于是忍不住的掉落了下来。
山巅上，林动望着天空上那道光束，旋即其心神突然一动，目光望向远处，那里光芒闪烁，数个呼吸后，一道娇小的身影便是出现在了他们那紧张的目光中。
光芒散去，那小女孩也是露出身来，再然后，林动众人，便是目瞪口呆下来。
“你…”
林动望着那有些熟悉的可爱小女孩，眼中却是涌上了难以置信之色：“你…你是茵茵？！”
“大哥哥。”
茵茵冲着林动甜甜一笑，然后便是在应欢欢他们愕然的目光中直接对着林动飞扑了过去，林动望着小女孩那乌黑的大眼睛，那里原本的纯真之色，已是出现了一种深邃，那种深邃，犹如勘破了生死。
他的身体逐渐的僵硬，眼中满是震撼之色，谁能想到，那被他们寻找了许久的生死之主，竟然便是…茵茵！


第1288章 大军齐聚
山巅之上，林动望着那扑进他怀中的小女孩，犹自还有些没从那震撼中回过神来，好片刻后，他方才深吸一口气，低头望着茵茵，只见得后者那乌黑的大眼睛笑嘻嘻的将他给盯着，只不过那眼中，却并没有曾经的天真，反而是有着一抹戏谑之色。
“真是没想到，我那么多年遇见的人，竟然便会是大名鼎鼎的生死之主。”林动叹了一口气，心情颇有点复杂，现在的茵茵，还是之前看见的那个天真烂漫的小女孩吗？以她的眼中，林动真是半丝都寻找不到，或许茵茵的记忆与她相融，但现在的她，恐怕更多的，还是生死之主吧。
“表情要不要这么沉重啊，轮回转世可不是重生，两种记忆融合，可不是单一的抹除，我的心中，在面对着姐姐的时候，依旧是与茵茵一般无二。”茵茵那乌黑的大眼睛，仿佛是看穿了林动的心中所想，笑嘻嘻的道。
林动捎了捎头，然后看向一旁的应欢欢他们，耸耸肩：“你们的大师姐回来了。”
“嗨，各位，想我没有！”茵茵也是冲着而应欢欢他们挥挥小手，小脸上的笑容显得格外的天真。
“大师姐，这模样，可真适合你啊。”
空间之主笑眯眯的走上来，伸出手掌扯了扯茵茵的长发，不过却是被后者一跃而起，狠狠的扯住他的耳朵：“小空，对大师姐尊敬点你懂不懂礼貌啊？！跟你说了多少次了啊？！”
炎主他们见状，顿时苦笑一声，没想到虽然变成了这幅模样，那暴力的性子还是没改变。
生死之主将空间之主教训了一顿，这才拍拍小手目光看向了西玄域的方向，大眼睛中掠过一抹暗沉之色，道：“看来我所镇压的那座镇魔狱，被他们找到了啊。”
应欢欢点点头，道：“如今三大联军正在赶来，等大军齐至，我们便进攻西玄域将魔狱尽数清除。”
“没想到…又要大战了啊，只不过这一次，我们可没有师傅帮我们承担了这一切，都得依靠我们自己了。”生死之主叹了一声，那稚嫩的嗓音中，却是有着一种令人压抑的沉重。
炎主他们听得此话，神色也是沉重了一些。
“大师姐也不用悲观，师傅固然不在了但那魔狱之中，同样没了异魔皇，他们被困在这天地中，就犹如囚徒一般，终归难逃败亡一途。”应欢欢道。
“呵呵，是啊师傅虽然不在了，但那最为可怕的异魔皇，也不在了呢。”生死之主笑着点点头，那望着西玄域的大眼睛中，有着凌厉的寒芒掠过：“那这一次，可就要真正的将他们斩草除根了啊。”
“或许再有两日时间，联军就会抵达这里。”炎主也是开口说道。
“嗯。”
生死之主仲着那小小的胳膊，笑眯眯的道：“那就再等两天吧，到时候我们再去把那些异魔杀得干干净净！”
在说着这话的时候，她虽然天真的笑着，但那恐怖的杀意，却是令得这天地间温度都是下降了许多。
“大哥哥。”生死之主突然歪着小脑袋看向林动。
“你能别这么叫我吗？”林动忍不住的打了一个寒颤，有些无奈的道。
生死之主忍不住的娇笑着，旋即她懒洋洋的道：“不过你倒是厉害啊，连我们这冰山一样的小师妹都能勾走，来给我说说，你用什么手段了？”
“大师姐。”应欢欢有些无奈的叫了一声。
“哈哈，那好，不说这个，不过这小子倒的确是有些能耐，这才多少年时间，竟然连神宫都修炼出来了，简直不比当年的小师妹弱啊。”生死之主笑道，她自然是察觉到了林动的实力，虽然那仅仅普通轮回境的元力修为不算什么，但那神宫所散发出来的波动，却是让得她心中有些惊异。
一旁的炎主等人都是冲着林动无奈的耸耸肩，想来已经习惯了。
“你们先聊着吧，我有事和小师妹说。”
生死之主拉着应欢欢对着远处走去，然后在那山崖边停下脚步，她那小脸上的笑容这才逐渐的淡了一些，道：“你压制了体内的力量吧？”
“嗯。”应欢欢也是望着远处，淡淡的点了点头。
生死之主坐在崖边，喃喃道：“你这种状态，可是打不过天王殿的啊。”
“大师姐放心吧，我会有分寸的。”应欢欢微微一笑，道。
生死之主叹了一口气，道：“这轮回一次，变化还真是不小啊…不过，你自己斟酌着吧，我们这里能够打败天王殿的人，也就你了。”
应欢欢也是在她旁边坐下来，纤细的双手抱着膝盖，轻声道：“若是没有这些异魔，那该多好啊…”
那样的话，她就不用觉醒力量，也不用将所有的情绪冰冻，承受着那种无尽的孤寂冰冷。
生死之主小脸复杂，道：“苦了你了，这种责任，却是要你来承担。”
应欢欢微微摇头，她将脸颊埋在膝间，低声道：“放心吧，这一次，我们一定会赢的。”
在将生死之主召来之后，接下来的两日，则是平静了许多，偶尔他们目光会望向北方，在那遥远的地方，他们能够察觉到，一股恐怖的战意，正在缓缓的席卷而来。
那种战意，惊天动地，即便以他们的强横，都是略感凝重，这天地间所有的强者汇聚在一起，那般气势，可不是单独的谁能够抵挡的。
而这种暴风雨之前的平静，待得第三日晨辉降临大地时，也终于是被逐渐的打破，林动他们抬起头，只见得在那遥远的天边，无数道身影，犹如蝗虫一般，铺天盖地的暴掠而来，尖锐的破风之声，即便是千里之内，皆是能够清晰可闻。
一支支庞大的强者军队，从远处掠来，最后在这座山峰之前逐渐的落下，放眼望去，天空地面，几乎是被浩浩荡荡的人海所弥漫，视线顺着望去，却是无法见到联盟大军的尽头。
林动八人站在山巅之上，那联盟大军无数强者也是将他们所发现，紧接着，那恭敬的整齐喝声，便是响彻而起：“见过诸位元老！”
庞大的联军中，有着光影掠出，然后迅速的接近他们所在的山巅。
“大哥！”
一道雄浑的声音响起，而后一座铁塔般的身影，直接是掠向林动，然后重重的落到山峰上，令得整座大山都是颤抖了一下。
“小炎？！”林动望着那铁塔般的身影，眼中也是浮现一抹欣喜之色，如今的小炎，比起以往，似乎更为的壮硕，只不过他的皮肤似乎显得格外的粗糙，隐隐的，有着一种细微的紫金色彩散发出来。
“这种波动…”
林动目光一闪，他从小炎体内察觉到了一股相当熟悉的波动，那是祖符的力量。
“是洪荒祖符。”洪荒之主淡淡一笑，目光打量了一下小炎，然后点点头，倒是对洪荒之主选择的新主人算是满意。
小炎也是捎了捎头，笑道：“一年前突然发现的，然后试了一下，竟然还真的获得洪荒祖符认可了。
林动也是笑着点点头，为小炎有这番际遇感到欣喜，他能够感应得出来，如今的小炎，也已晋入了轮回之境，再加上洪荒祖符，即便是面对着真王级别的异魔，都不会有丝毫的忌惮。
林动笑着，然后目光便是转向了小炎身旁，那里，身着黑衣的俊美青年修长而立，那番模样，除了小貂之外还能有着何人，只不过如今的他，或许是因为掌管妖域联盟的缘故，那俊美的脸庞上，倒是多了一些冷厉之色。
而此时，小貂那目光也是望着林动，那俊美脸庞上，有着一抹淡淡的暖意，两人对视，皆是一笑，那之间的兄弟情谊，已是不用多言。
“对了，有个人倒是很想见你。”小貂似是想起什么，突然对着后方妖域联军中一招手，而后一道虹光掠来，接着虹光散去，只见到一名窈窕少女，便是这般出现在了林动面前。
少女身段柔软而修长，她身着罗裙，青丝被随意的束着，肌肤犹如白雪，腰肢如柳，眉如远山，玉鼻挺翘，性感的红唇微微抿着，唇角的笑容，透着一股令人心神荡漾的妖媚，光是目光看去，就有些精神恍惚。
“你…你是心晴？”林动倒没怎么因为她这般勾人心魄的妖媚有什么失态，只是愕然的将她看着，惊喜的道。
“心晴见过林动大人。”
少女冲着林动盈盈一礼，浅浅的笑容，却是有着一股纯天然的魅惑涌动，她那水汪汪的灵动美目，也是泛着难掩的喜色，大胆而火热的看着林动。
这少女，赫然便是当初那陪同林动前往妖域，但最后却是因为接受九尾灵狐的传承，最终封闭整个寨子的心晴！
只不过这短短数年时间，当年的少女，已是大变了模样，再不复那般娇怯，那水汪汪的眼睛盯着林动，即便是后者的定力，都是忍不住的有种腰疼的感觉。
这九尾灵狐的媚惑，的确相当的厉害，难怪当年就连吞噬之主，就算有老婆了，都还是与那九尾灵狐有些瓜葛…
“你们寨子还好吧？”林动笑着问道。
心晴抿着小嘴点点头，道：“我们九尾族如今已再度成为妖域霸族之一，这还多亏了林动大人当年相助，大恩大德，心晴没齿难忘。”
林动笑着，心中也是为九尾寨的变化而感到欣喜。
远处，再度有着一道道身影掠来，青檀，绫清竹，唐心莲，青雉等诸多联军高层都是赶来。
应欢欢望着这些齐聚的联军，也是深吸一口气，道：“既然大家都到了，那么接下来，三大联军，就由心莲姑娘指挥大局，其余人等，尽力协助。”
“此战，若是输掉的话，或许这天地生灵，都将会落入魔狱之手！”
“是！”
众人闻言，也是郑重抱拳，显然都是明白这最后决战的重要性。
“既然如此…”
应欢欢与生死之主他们对视一眼，眼中皆是掠过极端凌厉之色。
“那便大军动身，进攻西玄域！”


第1289章 进攻西玄域
西玄域，曾经四大玄域之一，不过如今的这片辽阔地域，却与以往几乎是大变了模样，大地与天空，都是呈现暗黑色色彩，粘稠的魔气飘荡在空中，而待得魔气汇聚到某种程度，便是在天空上化为魔云，无数魔雨，铺天盖地的降落下来。
在那魔气缭绕的深处，仿佛是有着无数嘶吼的声音传出，吼声中，弥漫着凶残与暴戾。
西玄域，一座山脉中，原本宁静的气氛早已破坏殆尽，山脉中，无数妖兽奔逃着，在其中隐约还能够见到不少躲藏在深山之中的人影，而此时，他们却是混迹在一起，那望向后方的目光中，充满着恐惧。
嗤嗤。
而在他们那遥远的后方，魔气滚滚席卷而来，在那魔气之中，无数道邪恶的猩红眼睛出现着，他们快若鬼魅，发出刺耳的尖笑声，魔气掠出间，将那前方的无数妖兽以及其中的人类尽数的洞穿，浓浓的血腥味道伴随着凄厉的惨叫声，弥漫开来。
这就如同一场狩猎。
只不过此时的这些生灵，成为了猎物，那些异魔，成为了无情的猎人。
西玄域虽然被魔狱占据许久，但毕竟这地域太过的辽阔，其中很多人类以及其他的生灵都来不及逃离便是被封锁，而面对着强大的异魔，他们唯有如司老鼠般小心翼翼的躲藏着，一旦被发现，等待他们的，便是那残酷的结局。
这里，已是接近西玄域的边缘，只要能够逃出去，便是能够进入到安全的地带，那里与这里，犹如天堂与地狱。
所有人都是抱着这般的期望，拼了命的克制着心中的恐惧，不过当他们在见到那越来越近的无数异魔时，那心中依旧还是忍不住的升腾起一些凄然的无力感，总归，还是逃不掉吗…血雾从后方飘来，染红不少人的脸庞，他们抹去脸庞的血迹，不过就在他们眼中的绝望浓郁到极致时，突然有人停下来脚步，呆呆的望着远处的天空。
在那遥远处的天空，突然有着急促到极点的破风声响起，然后一片片乌云飞速的掠来，而随着乌云的接近，他们方才发现，那竟是由无数人类强者汇聚而成的军队！
无数身影铺天盖地的掠来，他们犹如蝗虫一般，遮蔽天日，那凝聚在一起的可怕战意，竟是将弥漫这西玄域的恐怖魔气都是震慑得不断的后退。
“那是”
山脉中逃亡的所有人都是停下脚步，他们望着那铺天盖地而来的军队，绝望的眼中，突然有着浓浓的狂喜涌出来。
“啊啊啊！”
他们疯狂的挥舞着手臂，有些凄厉的嚎叫声响彻而起，这种绝望之下出现的希冀，几乎让得他们有种痛哭的冲动。
那后方追杀他们的异魔部队也是在此时停了下来，那猩红的目光望着出现的联盟大军，其中也是闪过惊色，旋即一道尖锐之声响彻，魔气竟是飞快的后退。
“杀！”
然而就在他们后退间，只听得一道整齐得近乎令得天地颤抖的暴喝声响彻而起，旋即天地间元力暴动，无数道元力匹练，铺天盖地的呼啸而过，最后疯狂的落向那些异魔部队。
轰轰轰！
大地剧烈的颤抖着，那本来极为凶悍的异魔部队，却是直接是在这种覆盖性的攻击下，尽数的被轰爆而去，甚是连魔气，都是被彻底的荡除而去。
“三大联军听令，结战阵，全军推进，沿途若遇异魔，即刻出手，杀无赦！”天际之上，一道被磅礴元力包裹的悦耳冷喝之声，弥漫着浓浓的威严传荡而开，最后响彻天地。
“是！”
无数声音整齐应嘴，那种千军万马汇聚的声音，仿佛连空气都是被震得发出了嗡鸣的声音。
无数军队，呼啸而过，他们行进之间，显然也是形成了战阵，一旦遇见任何袭击，都是能够随时应变。
那些山脉上被及时拯救的众多身影，他们呆呆的望着天空上一波波掠过的蝗虫般的军队，那种凝聚在一起的惊人战意，竟是令得他们身体都是有些发烫。
“你们速速退出西玄域！”天空上，有着一名三大联军高层掠出，他望着这些人，手掌一挥，大喝一声，然后也就不再停留，立即归队。
“那是三大联盟的军队！他们这是要夺回西玄域，与魔狱决战了！”一些人显然也是知道不少的情况，他们见到这般阵仗，顿时惊呼出声。
“我们…也走吧？”
有着人眼中突然有着浓浓的仇恨涌出来，他们紧咬着牙，望向那被魔气弥漫的西玄域，旋即猛的转身，咆哮着跟随着大军之后冲出。
“我的师兄弟，全被异魔杀了，还能走哪去？西玄域是我们的家，死，也在死在这里！”
不知道是谁有些凄厉的嘶吼一声，旋即无数人眼睛都是通红起来，下一刻，那些之前还因为恐惧而狼狈逃命的身影仿佛是再度具备了勇气一般，竟是齐刷刷的转身，跟随在联盟大军之下奔掠，只是那偶尔想起的一道怒吼声，方才能够显示他们的痛苦。
“队长？”天空上，一支联军小队望着下方的动静，然后看向他们的队长。
“让他们跟来吧，他们也需要报仇，有盟主指挥大局以及诸位元老坐镇，不会出事，我们只管出手围剿异魔！”那名队长摇了摇头，他能够感受到他们的那种仇恨，因为所在的家族，也是只剩下他一人。
“是！”
联盟军队，犹如弥漫天地的浪潮，自西玄域边境进入，然后以一种钢铁般的姿态，步步紧逼，沿途所过处，无数异魔被尽数的抹除，大军所过之点，战火弥漫，一片狼藉。
不过，面对着联盟军队这种大军压制，魔狱竟是依旧没有任何的动静，那种安静，然而愈发的令人感到不安。
“据我们所知的情报，如今魔狱真正的力量，全部都是聚在西玄城，那里原本是西玄宗的总部，不过如今西玄宗已被魔狱所灭。”在那大军挺进间，大军中央，唐心莲正望着遥远的方向，俏脸之上，布满着冷肃。
“这段时间以来，魔狱一直都是毫无动静，这其中必然是在密谋着什么，还望诸位警惕。”
应欢欢与林动他们也是微微点头，以魔狱的性子，可绝对不会坐以待毙，那远古时的那场大战，双方不知道打了多少年，也没见他们有过类似的举动。
“他们之所以不敢主动出手，怕也是有着忌惮的原因，毕竟如今再不是当年，位面裂缝被封印，他们没有了源源不断的援军，更没有异魔皇，与我们相比，他们毕竟是处于劣势。”炎主缓缓的道。
“不管怎样，还是小心一些。”生死之主那稚嫩的小脸此时有着一抹深沉之色，她遥望着远处，喃喃道：“师傅燃烧了轮回，方才让得我们有了现在的优势，所以，不论如何，这一战，我们都输不得啊。”
众人也是重重的点了点头，这里汇聚了这片天地间所有的力量，一旦连他们都是失败，恐怕将再也无法抵挡魔狱的脚步，而那时候，这片天地，都将会犹如西玄域一般，所有生灵，都将会沦为魔狱的猎物。
“按照我们进军的速度，两日后，便是能够抵达西玄大沙漠。”
唐心莲深吸一口气，丰满胸部轻轻起伏：“我们没有退路，所以，为了这天地间所有的生灵，也为了彼此所在乎重要的人。”
“请让我们……竭力一搏吧！”
西玄大沙漠。
这是西玄域之中最为辽阔的地域，在以往，黄色的沙漠连绵不尽，只不过如今那些沙漠，却是变成了漆黑之色。
邪恶的魔气从沙漠中升腾起来，这片曾经壮阔的大沙漠，也已经被污染。
而在那沙漠的中心，一座巍峨的城市孤寂矗立，这里曾经是西玄域最为繁华的地方，然而现在，却是不见丝毫人气，粘稠的魔气将其笼罩着，隐隐间，似乎是能够见到无数道猩红而凶残的目光，自魔雾中闪现。
城市的中央位置，是一座暗黑色的高塔，高塔约莫数千丈，俯览着整座城市，而如今，在那塔顶之上，数道黑影矗立，他们的目光，皆是望向北方，在那里，他们能够感觉到一种惊人的压迫，正在缓缓的接近。
“终于是要来了呢”
数人首位，那身着黑袍的天王殿轻轻一笑，他的脸庞格外的白哲，只不过唇角的笑容，看上去甚至有些温和，只是那对黑色眸子中，却不见丝毫的情感。
“这么多年了，终于可以不用再潜伏了啊，真是迫不及待的想要将他们都杀了啊”在天王殿身旁，是一名身材瘦弱的少年，他伸出猩红的舌头，舔了舔嘴，笑嘻嘻的道。
“都准备好了吧？”天王殿一笑，道。
“嗯。”
其后众人，皆是笑着点点头，那笑声中，有着无尽的凶残在涌动。
“那就好。”
天王殿伸了一个懒腰，唇角的笑容愈发的温和，他望着北方，眼瞳深处，有着一股疯狂的利器堆积起来，而后他喃喃自语。
“真是好期待啊。”


第1290章 西玄大沙漠
大漠黄沙，夕阳斜挂，本该是一副壮阔之景，然而此时，却是被那滔滔魔气所遮掩，偶尔有着无数尖利的啸声传出，令得这壮阔之景多了一丝悲色。
咚。
而这般之景不知道持续了多久，突然天地仿佛是颤抖了一瞬，这弥漫大沙漠的魔气都是受到波及的颤了颤，再然后，视线望去，只见得在那天边的尽头，突然有着一道黑线浮现，那道黑线飞快的扩大，短短数分钟，竟是化为望不见尽头的人海，铺天盖地的对着大沙漠包围而来。
咻咻咻！
无数的破风声响彻不停，那千军万马所汇聚在一起的势力以及凛然战意，遮天蔽日般的涌来，那种阵仗，足以让任何人为之胆寒。
而那弥漫了大沙漠的魔气，也是在那种惊人战意的压迫下，不断的后退。
“停！”
在那主阵之中，唐心莲望着那出现在眼中的大沙漠，俏目中也是寒芒涌动，冷喝之声，已是响彻三军。
“三军结阵，扫荡魔气！”
“是！”
无数道整齐的喝声立即应道，而后只见得浩瀚元力席卷而出，直接是冲进那大沙漠之中，而随着元力过处，那原本显得乌黑的大沙漠，竟然也是再度的回复了几分原本的色泽，天空也是再度的清明了一些。
联盟大军，前不着头，后不着尾彼此结成首尾相顾的战阵，然后成扇形一般，缓缓的逼近西玄大沙漠，最后终是步入其中，荡除魔气迈着地动山摇的步伐，直接锁定大沙漠最为中央的位置。
整座沙漠，都是在此时微微的颤抖着，天空地面，几乎全都是被人影所占据，这里，聚集了这片天地之间所有的强者！
联军步步挺进但奇怪的是，中途依旧没有碰见任何的阻碍，因此短短半个时辰后，联军开始逐渐的停步，那无数道弥漫着仇恨的通红眼睛，已是望向了遥远的前方。
在那片黑色的大沙漠中央，一座极端庞大的城市，犹如地狱深处的邪兽一般安静的匍匐，但隐隐的，却是有着一种令人心悸的气势散发出来。
在那种气势下，即便是联军那滔天般的战意都是微微一滞，这里，就是魔狱的大本营了吗？
联军在那庞大城市百里之外停下双方默然无声，寂静之下，气氛却是近乎凝固，谁都知道，一旦这般凝固被打破，那么接下来的，便将会这天地之间千载以来，最为恐怖的一场战争。
“呵呵，诸位终于是来了啊我率魔狱，已在此静候多时。”
气氛凝固了半晌，那城市之上，突然有着魔气汇聚，一道黑袍身影缓缓的出现，他望着远处那近乎弥漫了天空的联军，微微一笑，那温和的声音，几乎在整个大沙漠上空回荡了起来。
联军之中，众多光影掠出，最后也是在联军前方现出身来，应欢欢冰蓝美目望着那道熟悉的黑影，淡淡的道：“天王殿，真是好久不见了。”
“呵呵，是啊，千载时间不见，没想到冰主依旧是风华绝代，令人仰慕。”城墙上方，天王殿冲着应欢欢微微一笑，道。
“我们斗了这么多年，其实我也有点累了，要不这次，我们就握手言和吧？如何？”天王殿轻笑道。
“肮脏的东西，你们还是滚出这片天地吧，这里，可不属于你们。”生死之主大眼睛一抬，懒洋洋的道。
天王殿笑了笑，他仰起头，目光仿佛是看透了天际，道：“回去？我们倒也是挺想的呢，要不，你们帮我们把位面裂缝打开，就让我们回去吧？”
“等我们将你们尽数净化了，你们应该能够魂回你们想去的地方。”炎主淡淡的道。
“哎呀呀，你们这些人啊，总是这样不客气，这么多年了，也都不变一变。”在天王殿身旁，也是有着笑声传出来，黑雾涌动，数道身影浮现出来，那少年模样的二王殿冲着炎主笑着摇了摇头，道。
“你们这些东西，还真是死而不僵啊。”空间之主望着那些熟脸，也是忍不住的冷笑道。
“彼此彼此了。”
应欢欢美目冰冷，其中没有丝毫的波动，她望着天王殿，缓缓的道：“放心吧，这一次，应该能够终结到这场延续上千载的战争，你们，都不会再有第三次机会了。”
“话说得这么满可不好哦。”
天王殿笑着摇了摇头，旋即他手掌轻轻挥下：“魔狱众，也出来一下吧，免得被人小看了呢。”
轰隆。
随着他手掌的落下，这片大沙漠突然剧烈的颤抖起来，然后众人便是见到，沙漠的大地开始撕裂开来，粘稠的魔气犹如液体般的出来，无数的异魔，犹如自地狱之中爬出来一般，很快的，便是遍布了这大沙漠的另外一边。
这些异魔的数量，多得完全无法数清，视线所及处，魔影充斥眼球，显然，魔狱的所有力量，也全部都是汇聚在了这里。
那般阵仗，看上去倒是丝毫不比三大联军弱上多少。
“咚！”
大地颤抖着，那城市中一道庞然大物缓缓的站起，那八只巨大的魔臂伸展间，仿佛连天空都是被他所撕裂，正是那大天邪魔王。
“还真是都齐聚了啊。”生死之主望着那大天邪魔王，小脸上也是掠过一抹冷色，道。
“天王殿等人由我们阻拦，其余魔狱强者以及军队，便交给你们了。”应欢欢偏过头，望着唐心莲，小貂，绫清竹，青檀，青雉等人，道。
“嗯！”
众人缓缓点头，眼下这一幕，显然是开启真正的决战了吗？
“三军听令，这是我们最后一战，若是胜，自当永绝后患，若是败，这西玄域之惨剧，必将波及整片天地，无一处能够幸免！”
唐心莲玉手紧握长剑，而后猛的挥下：“想要守护你们所在意的人，那，就战吧！”
“战！”
无数人眼神赤红，厉声咆哮，那惊天动地般的声音，犹如飓风般的席卷而开，在这大沙漠中，刮起无数黄沙。
“轰！”
三大联军在此时轰然而动，滔滔元力席卷，犹如潮水一般的涌出，彼此之间，形成战阵，眼睛通红的对着那魔狱展开冲锋。
“呵呵，魔狱听令，为了完成皇的意志，那就将他们给我全部杀光吧！”天王殿也是在此时大笑出声，笑声之中，充满着暴戾的杀意。
“杀！”
尖锐的嘶啸声，也是自那魔狱大军之中爆发而起，魔气浩荡，无数道魔影也是铺天盖地的冲击而出，最后直接迎上那洪水般的联军。
嘭！
两股看不见尽头的洪流冲撞在一起，整座大沙漠，都是在此时颤抖起来，天地间，元力魔气，疯狂的的交汇，冲锋。
鲜血几乎是瞬间便是染红大地，血腥的味道以及那疯狂的厮杀声，回荡在天地，令得那斜阳都是颤抖着躲进云层之中。
“我们也动手！”
应欢欢娇躯一动，也是暴掠而出，同时那城墙上空的天王殿等人也是瞬间消失而去，再度出现时，已在应欢欢他们前方。
天王殿微笑的望着阻拦在前方的应欢欢等人，旋即最终目光停留在应欢欢身上，道：“冰主，你这状态，可不是我的对手啊。”
应欢欢微微沉默，旋即她突然的转身，来到林动面前，在他那微愕的目光中，扬起俏脸，那冰凉而柔软的红唇，轻轻的吻在林动嘴唇之周围众人望着这一幕，眼中顿时涌出一抹复杂之色，因为他们见到，应欢欢的身体上，开始有着一道道奇异的冰纹浮现出来，那从她体内散发出来的寒气，愈发的恐怖。
林动眼睛望着那近在咫尺的俏脸，他同样能够看见，应欢欢那本就冰蓝的长发以及双目，竟然是在此时，逐渐的变得晶莹。
一种可怕而陌生的冰冷，犹如苏醒一般，缓缓的自应欢欢体内散发出来，竟是令得他嘴上传出许些冰冷的刺痛。
“对不起。”
应欢欢美目中掠过一抹歉意之色，然后缓缓的退开，那对晶莹的美目看着有些惶急的将她盯着的林动，旋即她猛的转身，将那纤细的雪白手套摘下，露出那晶莹的玉臂，在那玉臂上，一道道冰纹，开始闪烁着奇异的光泽。
“这一次的天地大战，就在此终结吧，我没耐心陪你们继续玩下去了。”
应欢欢盯着天王殿，她如今的声音，比先前变得空灵了许多，只是那其中，仿佛没有了太多的情绪。
她那晶莹剔透般的长发轻轻飘舞，此时的她，有着一种凛然不可侵犯般的冰冷，那种冰冷甚至是有着一种神圣的味道。
只不过，那却是显得有些陌生。
林动望着那道冰冷而陌生的倩影，双手忍不住的缓缓紧握，身体颤抖着，他知道，这一刻，真正的冰主，终于彻彻底底的苏醒了…
“为¨什么？”
他微微低着头，身影颤抖着，最后有些嘶哑的声音传出来，其中，仿佛也是有着无尽的怒火。
听得后面那道嘶哑的声音，那自应欢欢体内渗透出来，仿佛连时间都是能够冰冻的寒气，也是在此时一滞。


第1291章 天地大战
仿佛连时间都是能够冻结的寒气，在那蕴含着一些怒气的嘶哑声音下微微一滞，炎主他们望着林动，眼神也是略显复杂。
林动缓缓抬头，目光盯着前方的那道倩影，后者显然也是察觉到了他那充满着怒火的视线，玉手紧紧的握着。
他们这边突然情势复杂，天王殿倒是微微一笑，他看了一眼林动，道：“为什么吗？这个答案我倒是能够告诉你。”
天王殿步伐轻轻的踏出，那原本温和的脸庞，却是在此时变得无比阴厉，一股无法形容的恐怖魔气自其体内荡漾而出，那种魔气的恐怖程度，竟是连那自应欢欢体内散发出来的惊人寒气都是被阻拦了下来。
天地，都是在这股魔气之下颤抖。
那是一种真正毁天灭地般的力量。
炎主他们感受到这股魔气的可怕，眼神也是凝重起来，这天王殿，果然真的全部恢复了啊…
“因为她不这样的话，你们，都得死！”天王殿狰狞一笑，此时的他，在那滔滔魔气衬托之下，犹如一尊魔神，可怕之极。
“那我倒是要试试！”
林动心中怒火涌动，眉心中神宫发出嗡鸣之声，他盯着那天王殿，身形一动，暴掠而出，不过就在其身形刚动时，一只冰冷的玉手却是握上了他的手臂。
“你现在还不是他的对手。”一道清脆但却显得格外冰冷的声音在林动耳边响起。
林动面沉如水，却是手臂一震将那冰冷的玉手震开，而后其身体之上银光闪烁，一闪之下，直接是出现那天王殿上空，璀璨紫金光芒爆发将近十万道紫金龙纹在此时齐齐长啸。
嘭！
林动一拳轰出，那近十万道紫金龙纹顿时化为一道数万丈庞大的紫金巨龙，咆哮着撕裂空间对着那天王殿暴冲而去。
“倒的确是有些能耐，不过，这可还不够。”天王殿望着林动这般攻势，却是淡淡一笑，他白皙的双手猛的探出旋即虚撕而下，只见得两道深邃到极致的黑光闪现，竟是直接将那紫金巨龙生生的撕裂而开。
“唰！”
天王殿撕裂紫金巨龙眼中也是掠过一抹残忍之色，他能够感觉到林动那不俗的实力，这种对手，已是能够对他们魔狱造成威胁，所以，还是尽早解决掉为好。
心中闪过这般念头天王殿一步跨出，旋即出现在林动前方，双指并曲，凌厉双指上，竟是有着黑色的魔晶覆盖，那是魔气在凝聚到极致后的变化一指之下，足以洞穿天地。
林动望着天王殿这等攻势，眼神也是一凛，体内三道不同光芒爆发而起，整条手臂瞬间化为液体之状，在那之中，三道祖符之力，席卷而开。
砰！
天王殿那魔指重重的点在林动那液化的祖符之掌上，一道闷声响彻周围空间都是崩裂开来，林动更是身体一颤，急退了数十步，体内气血翻涌。
虽然他的实力在以一种惊人的速度精进着，不过毕竟与天王殿这些远古时期的巅峰强者依旧有着一些差距。
天王殿见到一指击退林动，却并未给其造成重伤，似也是略感不满意，身形鬼魅掠出，再度出现在林动前方，就欲继续攻击。
不过就在他想要再度出手时，一道冰蓝倩影出现在林动前方，纤细玉指也是含怒凌空点出，与那天王殿正面硬憾在了一起。
咔嚓！
魔气寒气席卷开来，令得空间震荡不堪。
两人对轰，身体皆是一震，各自退后了数步，天王殿淡淡一笑，道：“冰主何必动怒，我是见他竟然不识你顾全大局的好心，这才要出手帮你让他明白一些事情的。”
“你还没这资格！”
应欢欢那晶莹剔透的美目泛着凌厉寒意的盯着天王殿，那眼神深处，似是有着一抹极深的杀意涌出来。
“呵呵，发怒了呢，真是难得啊。”天王殿戏谑一笑，只是那眼中，却毫无笑意，有的只是那种纯粹的无情之色。
“不要乱来了，等所有事情完结后，我再给你一个交代。”应欢欢微微侧头，晶莹长发飘舞着，她望着身后的那道削瘦身影，轻轻的道。
林动紧紧的盯着那道优美的身影，半晌后，他颓然的摆了摆手，有些疲惫的道：“随你吧，你想怎样就怎样。”
说着，他已是退后而去，应欢欢见状，玉手不由得紧紧的握着，在听见这话的时候，即便是以她如今那犹如冰封的心境，竟都是有着一丝难过之感涌出来。
呼。
不过这般时候，的确不再是说这些的时候，她深吸一口气，压抑着心境波动，然后晶莹美目看向了笑吟吟的天王殿，争斗这么多年，她第一次对这家伙的杀意强盛到连她都是有点压制不住的地步。
恐怖的寒气，弥漫开来，她玉手轻握，一柄寒冰长枪缓缓的延伸出来，美目中杀意涌动，终是不再有任何的忍耐，娇躯掠出，天空之上，冰雪席卷，凌厉得足以冻结时间与空间的枪影寒气，已是对着那天王殿冲了过去。
“哈哈，忍耐不住了吗？”
天王殿见状，也是仰天大笑，魔气暴涌，瞬间掠出，再然后天空之上，便是有着极端恐怖的波动席卷开来。
“动手吧！”
生死之主见到两人已是开战，眼中也是寒芒闪过，其身形一动，直奔那魔狱二王殿而去：“这家伙交给我来。”
“那大家伙，交给我来！”
洪荒之主则是望向那大天邪魔王，他的身体上，紫金光芒涌动，而后身体飞快的膨胀，很快的便是化为万丈巨人，脚踏大地，轰隆隆的对着那大天邪魔王冲去。
“既然你们都开始挑选对手，那我也来吧。”
那魔狱三王殿淡淡一笑，目光却是投向了林动，道：“看起来你似乎最好捏呢，要不先解决掉你吧？”
林动面无表情，一步步的踏空走出，眉心之中，突然有着万丈神光爆发而出，隐隐的，似乎是有着一座神宫浮现。
“神宫境？”那三王殿见到那座神宫，眼神终是一凝，这才明白，为什么林动有着与诸位远古之主站在一起的资格。
“他交给我来吧。”
林动眼神有些狰狞的盯着那三王殿，对着炎主他们说了一声，便是不再犹豫，只见得其眉心处，一道璀璨神光暴掠而出，直奔那三王殿而去。
炎主他们见状，也是微微点头，以如今林动的实力，并不比他们弱多少，凭借着神宫之威，想来要抗衡那三王殿也并非是不可能的事。
三王殿脸庞上倒依旧是布满着笑意，他掌心一翻，那掌心中的邪恶之眼也是射出一道魔光，将那自林动眉心神宫中射出的神光抵御而下。
林动身形一动，出现在那三王殿前方，而炎主等人也是齐齐出手，将魔狱的其他高层尽数的抵挡下来。
此时的天空地面，几乎尽数被分割而开的战场所弥漫，那种恐怖的能量波动，波及整个西玄域，甚至，连那其他三玄域的人都是能够感觉到，当即那天地间便是有着无数紧张的目光对着那极为遥远的方向投射而去。
“你还真是有些不简单呢，这才一年的时间不见，竟然连神宫都修炼出来。”三王殿望着那阻拦在他前方的林动，淡笑道。
林动漠然的看了他一眼，此时的他显然没有任何与其废话的心情，身形一颤，双臂都是迅速的液化，三大祖符之力，尽数的爆发而开。
伴随着如今晋入神宫境，林动对祖符的掌控，显然而有到了真正出神入化的境界。
“唰！”
林动身形一动，仿佛瞬间移动般出现在那三王殿头顶之上，那蕴含着浩瀚元力的祖符之掌已是带起恐怖能量，狠狠的对着后者天灵盖怒拍而下。
“轰！”
不过虽说林动如今实力暴涨，但那三王殿显然也并非是省油的灯，身体一震，粘稠的魔气在其身体上凝聚，直接是化为一件狰狞黝黑的魔皇甲，而后一拳迎上，重重的与林动怒憾在一起。
拳掌怒憾，周遭空间顿时寸寸崩裂，那破碎的痕迹，犹如玻璃一般，触目惊心。
一拳轰出，那三王殿下手却是毫不留情，那狰狞的魔皇甲上，竟是蔓延出一根根漆黑锋利无比的尖刺，而后其身形转动，犹如旋风般的席卷向林动，那等锋利之气，连空间都是被割裂。
林动见状，却同样是未曾后退，他身体之上，三道祖符光芒疯狂的闪烁着，而后他整个身体都是在此时逐渐的液化，下一瞬，液化的祖符之力又是陡然凝固，竟然也是在他的身体之上，化为了呈现黑，雷，银三色的凌厉战甲。
砰！
三大祖符所形成的战甲，包裹着林动全身，他双拳轰出，只见得无数黑洞，雷霆，银光在其拳下凝聚成形，然后狠狠的轰在那锋利无比的黑色飓风之上。
铛铛铛！
两者硬憾，火花暴射，那等波动，连天地都是有些颤抖。
两人状若疯魔般的冲击在一起，拳拳轰出，全部都是采取的搏命之斗，每一拳所蕴含的力量，都足以将一名轮回境的巅峰强者轰杀，不过他们却是凭借着各自身体之上的强大战甲，硬生生的承受了下来。
而在林动与三王殿疯狂交手间，天空上其他战圈也是飞快的进入白热化，双方都是斗了无数年，也是清楚对方的手段，这般决战动手，更是丝毫不留情面。
空间撕裂，天空蹦碎，这一战，几乎是要将这天地毁灭一般。
这一战，惨烈至极！


第1292章 激斗三王殿
天地之间，魔气汹涌，元力奔腾，一处处可怕的战场在这天地中演绎，那种惊世骇俗的战斗，当属这千载之间首次所见。
总体说来，如今这天地间，共分两大战域，地面以及低空领域，那是联军与魔狱大军厮杀的地方，还有着双方的顶尖强者在互相纠缠，而那高空之上，大致便是林动以及远古之主与魔狱最高层之间的战斗。
双方自远古打到现在，对于对方实力都是极其的了解，那等战况，自然是激烈异常，彼此下手，都是毫不留情。
嘭嘭！
林动此时正与那三王殿战成一团，凭借着如今他成功晋入神宫境，实力倒也并不比后者弱上多少，因此这三王殿想要再像一年之前那般轻易的击溃林动，显然是不可能的事情。
咻。
两人所化的光虹交错而过，云层不断的自两人身旁对着下方掠去，他们已是身处万丈高空，低头看去，隐约能够见到两股洪流疯狂的冲撞。
杀气以及血腥，直冲云霄。
林动身体之上，被那祖符战甲包裹着，此时的他同样是犹如人形杀戮机器，毫不忌惮那三王殿的魔皇甲，每一拳轰出，都是有着无数黑洞，雷霆，银光成形，三大祖符的力量在此时尽数的融合，那等狂暴程度，即便是三王殿也不得不凝重以待。
“咚！”
两人眼神凶狠，彼此一拳轰出蕴含着全力的拳头硬憾在一起，空间蹦碎，两人都是倒飞出数千丈，而后脚掌一跺虚空，再度暴掠而出。
将近十万道紫金龙纹腾飞如今林动施展这青天化龙诀，威力显然比青雉还要强大，但即便是如此，却依旧难以取得太过明显的上风，三王殿同样不是省油的灯，粘稠般的魔海涌动，直接是将来自林动的狂暴攻势尽数的接下。
“嗤嗤！”
林动掠出，其手掌紧握，三大祖符的力量席卷而出直接是化为一柄呈现三种三色的权杖，权杖顶端，雷霆闪烁，中部位置，银光弥漫，权杖尾部则是一片漆黑，犹如黑洞旋转。
唰唰！
林动手臂颤动，那祖符权杖便是化为铺天盖地的凌厉杖影，对着那三王殿笼罩而去。
“黑魔之门！”
三王殿见状，眼中凶光一闪，魔掌重重拍在前方虚空粘稠的魔气席卷而出，直接是化为一道千丈庞大的黑暗门户，在那门户之上，仿佛是有着极端狰狞的魔像。
砰砰砰！
蕴含着三道祖符力量的杖影狠狠的轰来，顿时将那黑魔门户震出道道裂纹，林动身形如电，犹如与那祖符权杖相融，暴掠而出，直接是生生的将那巨门洞穿而去。
咻！
祖符权杖洞穿巨门其上银光突然一闪，只见得那权杖一端还握在林动手中，然后那权杖顶端，却是诡异的自那三王殿后方出现，毫不客气的重重点在其后背心之上。
铛！
火花溅射，那三王殿身形顿时狼狈的射出，后背的魔皇甲上，更是出现了一个深深的印痕，不过这还不待他恼怒，他便是感觉到周身空间波动泛起，无数泛着雷霆的权杖之影，诡异的自空间中洞穿而开，对着他全身要害笼罩而去。
“铛铛铛！”
短短十数息的时间，那蕴含着可怕力量的杖影已是尽数的落到三王殿身上，那狰狞的魔皇甲上，顿时有着一道道深深的痕迹出现，看上去略显狼狈。
“吼！”
被林动这番狂轰猛打，那三王殿眼睛也是陡然通红起来，喉咙间爆发出一道厉吼之声，猛的一拳轰出，只见得恐怖的魔气在其拳上凝聚，竟是化为一道黑色晶光，最后狠狠的轰出，那晶光之中，压缩着极端浓郁的魔气，直接是轰爆了空间，在林动身体之上爆炸而开。
砰！
林动的身体也是被震得倒飞而去，身体之上的祖符战甲浮现层层裂纹，他嘴角也是有着血迹渗透出来，体内气血翻涌。
这三王殿，的确是不愧是魔狱之中的顶尖存在，那等战力，算是林动这么多年来所交手的人之最，此战，必然艰难。
震退林动，那三王殿低头看了一眼布满深痕，犹如即将破碎的魔皇甲，眼中凶芒更甚，他舔了舔嘴唇，阴森的盯着林动，没想到这个看似最软的软柿子，竟然也是这么的难缠。
“虽然今天的主要事情不是与你们战斗，不过被你打成这样，还是很不爽的啊…”三王殿冲着林动咧咧嘴，笑道。
林动双目微眯：“不是战斗？”
三王殿淡淡一笑，他低头看了一眼下方那近乎看不见尽头的厮杀，只不过他却是能够感觉到那以一种惊人的速度堆积起来的血腥以及凶戾之气。
“很久以前我便是听元门那三个废物说起过你，原本我以为只是一个无足轻重的蝼蚁而已，没想到，我今日竟然会被我曾经所视为的蝼蚁逼成这样。”
三王殿缓缓的抬起他的右手，在他掌心中，那只邪恶的眼睛，轻轻眨动的看着林动，令人毛骨悚然。
“不过，我想，在那之前把你解决掉会更让我舒心一些，呵呵，总不能留下一些遗憾，你说对吗？”
林动望着这突然间有些胡言乱语的三王殿，不知为何，心中掠过一抹淡淡的不安。
“这只眼睛，名为狱魔眼，是吾皇赐予我的，当年我曾用它偷袭了符祖，我想，你若是能够死在它上面，也应该心满意足吧？”三王殿手掌轻轻合拢，嘴角的笑容，显得格外的狰狞。
林动眼神漠然的盯着他，却是并没有与其答话的意思。
“真是没劲呢。”
三王殿见到林动那没有太多变色脸庞，不由得撇了撇嘴，旋即他的体内，开始有着恐怖的魔气席卷而出，那些魔气，极为的粘稠，甚至其中还能够看见无数细碎的晶体之状，那是当魔气凝聚到一种恐怖的程度后方才会出现的变化。
“嗡嗡。”
滔天般的魔气，顺着三王殿手臂灌注而进，他的那只手掌，也是在此时变得漆黑如墨，掌心中的那枚邪恶眼睛，竟是爆发出刺耳的嗡呜尖啸之声。
那种邪恶的波动，瞬间暴涨了无数倍。
三王殿冲着林动狰狞的笑着，然后林动便是见到，他的那只手臂开始以一种惊人的速度萎缩着，短短数息，仿佛其中的血液都是尽数的消失而去，唯有着那枚邪恶的眼球，愈发的狰狞与可怖。
“砰。”
三王殿的一条手臂，在此时爆碎开来，那枚眼球脱离掌心，然后魔气涌动间，竟然是开始迅速的膨胀，数息之后，那可眼球，已是有着千丈大小。
邪恶的眼球，悬浮万丈高空，滔天般的邪恶魔气席卷出来，一种极为恐怖的波动，悄然的散发着。
林动望着那狰狞而邪恶的巨大眼球，感受着其中那惊人的波动，他眼神也是变得凝重许多，旋即他深吸一口气，掌心一握，一道古老的光阵自其手中暴掠而出，然后迎风暴涨，化为数千丈大小。
古老的阵法，缓缓的运转着，一股纯正而古老的力量，散发而出。
那是真正的乾坤古阵！
“乾坤古阵吗？”
三王殿眉头微微一皱，旋即他盯着林动，而那邪恶的巨大眼球，却是在此时缓缓的闭拢而上。
轰隆。
乾坤古阵运转而起，奇异的光幕倾泻而下，漫天魔气都是有着消退的迹象，唯有着那颗邪恶的眼球，依旧是纹丝不动，但林动却是能够隐约的感觉到，一种毁灭般的力量，正在飞快的凝聚而起。
那种力量，令人不安。
呼。
林动深吸一口气，眼中寒芒爆闪，双手陡然结印，浩瀚元力，席卷而出：“乾坤之手！”
天空上，那乾坤古阵剧烈的颤抖起来，只见得铺天盖地的奇异之光弥漫出来，竟是化为一只数千丈的巨手，巨手之上，布满着古老的光纹。
巨手探出，直接是穿透空间，对着那邪恶眼球，狠狠的拍下。
“呵呵。”
三王殿淡淡一笑，他的双目，缓缓的闭上，在其眼角，仿佛有着黑色血液流出来，而随着他双目的闭上，那巨大的邪恶之眼，却是在此时猛的睁开。
眼球睁开的霎那，天地仿佛都是陡然间黑暗了下来，所有人都是能够见到，一道仿佛世间最邪恶的光束，自那眼球之中暴掠而出，最后与那乾坤巨手，重重相撞。
嗤！
两者相触，没有任何的恐怖波动传出，紧接着，那乾坤大手便是剧烈的颤抖起来，裂纹浮现间，砰的一声，彻彻底底的被那道光束轰爆而去。
而就在乾坤大手爆裂间，突然那后方有着万丈神光浮现，一座巨大无比的神宫，凭空闪现，神宫巍峨，犹如神灵居住之所，浩荡而威严。
砰！
洞穿乾坤大手的邪恶光束，掠过天际，然后也是在此时，再度狠狠的轰在了那巍峨神宫之中。
轰隆！
巨大的嗡鸣之声，响彻天地。
整座神宫，都是在此时剧烈的颤抖起来，而此时，在那神宫之中，一道身影，修长而立，在他的手中，一柄雷弓拉成满月，其上箭支，呈现三种颜色，而且，那神宫内，浩瀚无尽的精神力，也是在此时滚滚涌来，最后尽数的灌注进入那箭支之中。
嗡。
箭支一颤，爆发出滔滔神芒，那种光芒，竟是连那种滔天邪恶之气，都是压制而去。
林动手臂颤抖，鲜血自双手中飞快的流出来，然后，他轻轻闭目，拉着弓弦的手指，缓缓的松开。


第1293章 魔皇之像
嗡嗡！
就在林动手指松开弓弦的那一霎那，整座神宫仿佛都是在此时爆发出清脆的嗡鸣之声，那璀璨的万道神光，仿佛也是在此时，尽数的汇入那闪烁着三种颜色的箭支之上。
这一支箭，凝聚了林动体内所有的力量。
唰！
一道虹光，自神宫中贯穿而出，那霎那间绽放而出的光芒，连那弥漫天地的邪恶之气，都是被尽数的蒸发而去。
无数道有些震动的目光望去，他们望着那道虹光掠出，然后便是与那一道邪恶无比的黑色光束，狠狠的碰撞在一起。
咚！
惊天之声响彻，那等光芒，犹如一轮烈日，冉冉升起，而在那光芒下，无数魔气，烟消云散。
“破！”
在那神宫之中，仿佛是有着一道嘶哑的声音缓缓的传出，而后众人便是见到，那邪恶得连一名渡过三次轮回劫的巅峰强者都得退避的光束，竟是在此时，崩裂出了一道道细微的裂纹。
咔嚓咔嚓。
邪恶光束，最终是在那虹光的贯穿下尽数的破碎，虹光散去，一支三色箭支去势依旧不减，撕裂长空，快若闪电般的狠狠射在那巨大无比的邪恶眼球之上。
啊！
一道凄厉无比的惨叫声，陡然自那三王殿嘴中传出，只见得其双目处竟是传出破碎的声音，双眼迅速的凹陷下去，黑色血液疯狂的流出来，看上去极为的骇人。
邪恶的巨大眼球，也是在此时飞快的缩小，最后化为一道黯淡的黑光，掠回那三王殿的一只手中，但他依旧仅仅的捂着双目，狰狞的嘶吼声，从其喉咙间传出。
神宫缓缓的散去，最后掠进林动眉心，他现身出来，手掌颤抖着抹去嘴角的血迹，这三王殿的确心狠手辣，此番若不是他实力大涨，先借助着乾坤古阵消耗，再催动神宫，将三大祖符也是完美的融入那箭支之中，恐怕现在惨叫的，就该是他了。
那三王殿凄厉的惨叫声自高空上传开，但诡异的是，随着他这惨叫声的响起，下方那些魔狱军队，竟是微微一滞，天空上的那些战圈，也是出现了停止的迹象，只不过，这似乎只是魔狱那边单方面。
林动也是注意到这奇怪的一幕，顿时一愣，他看了看那嚎得极为不甘的三王殿，心中有些疑惑，这三王殿虽然被他重创，但显然依旧还有着再战之力，怎么现在却是跟必死无疑一般？
最高层的天空上，天王殿闪身后退，他的衣衫如今有些破碎，显然是在先前与应欢欢的交手中所造成，此时的他，也是将目光看向那惨叫中的三王殿，然后再看了一眼这天地间那弥漫的血腥与戾气，突然冲着不远处那周身弥漫着恐怖寒气的应欢欢一笑。
“老三，没想到最先受创的，竟然是你啊。”天王殿转头，望向那三王殿，笑道。
三王殿的惨嚎声逐渐的停歇，他脸庞上布满着黑色的血液，看上去格外的恐怖。
其他的魔狱高层，也是盯着三王殿，那神色有些诡异与狂热。
“桀桀，是啊…”三王殿搽了搽脸庞的的黑血，竟也是有点诡异的笑出声来，他望向林动的方向，道：“你还真是厉害啊，原本还以为至少能再斗上一些时间的，没想到…这么快就被你伤了。”
林动眉头紧皱，他望着有些诡异的三王殿等人，心中掠过一抹不安，视线转去，与炎主他们对视一眼，却是见到他们的眉头也是紧紧的皱着。
“战斗可还没完呢，现在哭嚎，恐怕还不是时候吧？”雷主冷笑道。
“呵呵，我们斗了这么多年，彼此手段还不清楚吗？我们或许奈何不了你们，但你们想要将我们彻底根除，又谈何容易呢？”天王殿轻笑一声，道。
“冰主说对这种纠缠已经厌倦，我们又何尝不是？所以，这一次，我们真的，很想把你们都杀了啊。”
“你们办得到吗？”生死之主嘲讽的道。
“你说呢？”天王殿脸庞上，再度掀起一抹诡异之色，旋即他对着三王殿一笑，道：“老三，既然你输了，那就由你来开启吧。”
三王殿闻言，顿时诡异笑着点点头，然后他直接是在那无数道目光的注视下，凌空盘坐而下，单手结印，隐隐的，他脸庞上流下来的黑色血液，愈发的浓郁。
一种极为不安的味道，弥漫了开来。
林动见到他们这般诡异举动，黑色眸子陡然掠过寒芒，身体之上银光一闪，快若鬼魅的出现在那三王殿前方，手中祖符权杖暴掠而出，闪电般的对着三王殿咽喉洞穿而去。
噗！
祖符权杖，毫无阻碍的洞穿了三王殿的防御，在那一道道难以置信的目光中，自三王殿喉咙间传出，黑色血液，狂射而出。
这三王殿，被杀了？
联盟大军中，爆发出惊天般的哗然声，但此时出奇的没人欢呼出声，因为谁都能够感觉到一些不对劲的气氛。
那曾经将远古八主都是逼得沉睡与轮回的魔狱最顶层存在，怎么会这么容易击杀？
“呵呵。”
三王殿抬头，那布满着黑血的脸庞，冲着林动露出一抹狰狞的笑容，旋即他的身体，竟是疯狂的膨胀起来。
“不好。”
林动见状，心头一震，周身银光闪烁，身形一闪便是消失而嘭！
就在林动身形消失时，那三王殿的身体，突然爆炸开来，粘稠得黑色魔气，带着鲜血爆碎开来，整个天空都是被遮掩而去。
犹如液体般的邪恶魔气，夹杂着粘稠的鲜血，铺天盖地的降落下来落至下方的大地之上。
远处，林动身形闪现出来，他喉咙间涌上一股甜意，虽然先前逃得快，但依旧是在那三王殿的自爆中受到了震荡。
“这家伙…竟然自爆了？”
林动有些茫然的望着那弥漫开来的邪恶魔气他怎么都料不到，这三王殿，竟然会选择自爆，这些家伙，究竟是想要做什么？
“呵呵。”
天王殿望着这一幕，微微抿嘴，然后他冲着林动等人笑了笑：“接下来，就请你们看一看我们这千载岁月的杰作吧。”
“我早便说过，这一次的天地大战，你们，输了。”
轰隆！
就在天王殿此话落下的那一瞬，下方的大地，突然间剧烈的颤抖起来，只见得地面撕裂，魔气弥漫在那大地的最深处，似乎是有着一道庞大得看不到尽头的魔影，缓缓的站起。
那道魔影，自无尽深渊中爬出，它头顶天空，脚踏大地魔气弥漫间，似乎是犹如那魔中之帝皇。
“这是…”
林动瞳孔急缩的望着那带着一种惊人威压的魔影，那魔影似乎是有着无数只巨大的魔臂每一只魔臂掌心，都是有着一颗紧闭的邪恶魔眼，那种眼睛，比起那三王殿掌心的邪眼，还要可怕！
“魔皇之像？！”应欢欢望着这恐怖至极的魔影，俏脸之上也是有着无尽的寒气涌出来，一字一顿的道。
“诸位盛宴开始，感谢配合。”
天王殿轻轻一笑，他在天空之上，对着联军方向轻轻鞠躬弯身。
嘭嘭嘭！
就在他声音落下的瞬间，那魔狱大军之中，猛的响起无数道自爆之声，旋即联军无数强者便是惊骇的见到，那些魔狱军队，在此时接二连三的爆炸开来。
浓浓的魔气，夹杂着黑色血肉溅射而开，最后尽数的掠进那魔皇之像之中，而随着那无数魔气以及血肉的灌注，只见得那魔皇之像那些魔臂之上的邪恶之眼，竟然是开始一只一只的缓缓睁开。
而每伴随着一只邪眼的睁开，这天地，便是黯淡一分。
砰砰！
爆炸依旧在飞快的持续着，整片沙漠仿佛都是变成了黑色血液的海洋，联盟大军急忙升空，目光骇然的望着这一幕。
滔天般的魔气以及之前厮杀间创造出来的血腥之气以及戾气，都是在此时不断的涌进那魔皇之像中，将那一只只邪眼开启。
不过想要将这魔皇之像所有的邪眼显然不是什么简单的事，当那魔狱军队尽数自爆完毕时，可那魔皇之像上的邪眼，却连一半都未能开启。
“到你们了。”
天王殿看向魔狱其他的高层，淡笑道。
二王殿等人闻言，脸庞上也是浮现诡异的笑容，那笑容中，有着一种狂热浮现，旋即他们凌空盘坐，最后身体也是在生死之主他们那震动的目光中，尽数的自爆。
砰砰！
恐怖的魔气，遮天蔽日的席卷而开，生死之主他们面色阴沉，他们与这些家伙纠缠了上千载都未能将他们尽数的抹杀，然而现在，不过才短短小半日的时间…他们却是尽数的选择自爆。
这些家伙，究竟要干什么？
滔天般的魔气以及血肉，尽数的冲进那魔皇之像中，再度开启了不少邪眼。
天王殿孤立天际，他笑望着应欢欢，脸庞上的诡异之色，令人不寒而栗。
“开启魔皇之像，的确能够让你们拥有无限接近异魔皇的力量，但姑且不说这种力量无法长存，即便你们真的能够开启，但没有异魔皇的血肉做引，你们也不可能成功。”应欢欢晶莹冰彻的美目盯着天王殿，缓缓的道。
“只要能够将你们抹除在这天地间，即便我们也是尽数陨落在此，那至少，以后这片天地，不会再遭有异魔肆虐。”
“那样的话，我们，也算赢了！”
天王殿轻轻一笑：“真是大义呢，不过…恐怕输的，是你们呢。”
天王殿偏头，望向遥远的东方，微微笑道：“冰主，你可还记得，当年符祖封印吾皇时，曾将吾皇手臂斩断一只？”
应欢欢听得此言，眼神终于是有所剧变。
“呵呵，你猜的没错呢，吾皇手臂，其实还留在你们这片天地中，其实，从符祖燃烧轮回封印吾皇的那一刻，你们就已经输了…”
“你们的符祖，终归还是比不上吾皇。”
天王殿对着东方，恭敬的跪拜而下。
“吾皇之手，请归位吧。”
就在他这话响彻天空的霎那，在那遥远的东玄域，异魔域之中，大地突然开始疯狂的颤抖起来，而后地面蹦碎，滔滔魔气席卷而出，一道魔光，直接是洞穿了那封印此处的古老阵法，而后撕裂长空，对着西玄域暴掠而去。
在那黑光之中，似乎是有着一只巨大的苍白手臂，若隐若现，一种无法言语的邪恶，笼罩了这片天地。


第1294章 魔皇之手
魔气滚滚，几乎遮掩了整个大沙漠上空，甚至连那西玄域都是有所被波及，在那无尽的魔气之下，那道魔像，犹如从那异域而来的毁灭者，那种邪恶到极点的波动，令得联军大军皆是为之失色。
林动他们皆是面色难看的望着这一幕，旋即突然转头，望向那遥远处的天空，那里的空间突然蹦碎开来，一种令人头皮发麻的魔气席卷而出，在那魔气之中，一只苍白的巨大手臂，若隐若现。
“魔皇之手？！”应欢欢以及生死之主等人见到那苍白的巨大手臂，瞳孔顿时紧缩起来。
那只大手，异常的苍白，看上去甚至显得有些柔弱，但就是那种诡异的柔弱之下，反而透着一种令人颤粟的味道。
这，可是那位曾经将符祖都是逼得燃烧轮回的魔中皇者的手臂啊！
“当年在那最后被封印的时刻，这异魔皇的手臂，恐怕不仅仅是被师傅斩断，更多的，恐怕是他暗中留下的这一手。”应欢欢玉手紧握，缓缓的道。
生死之主他们瞳孔紧缩，隐隐的有些骇然，真不愧是魔中之皇，竟然在那种时候，还能够留下暗手，以待千载之后的翻盘之时。
那天王殿目光狂热的望着破空而来的魔皇之手，恭敬的弯身而下，旋即其印法一变，只见得那魔皇之手便是掠来，最后与那魔皇之像，连接在一起。
嗡！
而随着那魔皇之手与魔皇之像连接，那庞大无比的魔皇之像，竟是爆发出刺耳的嗡鸣之声，滔天魔气席卷而开，隐隐的，有着一种恐怖得无法形容的波动，在缓缓的诞生着。
那种波动，有着那位异魔皇的味道！
魔皇之像上，那一只只紧闭的邪眼，也是在此时加速的睁开，其中布满着无情与暴戾之色，犹如要将这天地毁灭。
“哈哈。”
天王殿仰天大笑，笑声之中满是得意之色：“等魔皇之像一成，你们将如蝼蚁一般，哈哈，这片天地，终归落到了我们的手中，当年吾皇没有完成的事，竟然被我做到了！”
“你高兴得太早了一些！”
可怕的寒气在此时爆发而开，只见得天空上，一层层冰浪以一种惊人的速度席卷而开，那滔天的魔气，也是被冰冻而住。
应欢欢手持寒冰长枪，美目之中寒光暴涌，而后她身形暴掠而出，直奔那天王殿而去，那枪尖凝聚的寒气，仿佛能够冰冻时间。
“哈哈，冰主，现在做这些，可是无用了。”
天王殿大笑着，他却是没有丝毫要与应欢欢交手的意思，身形一动，身体之上魔气爆发而开，竟是退进了那魔皇之像庞大的身躯中，而后两者诡异的融合在了一起。
“轰隆隆！”
随着天王殿的融入而进，那狰狞的魔皇之像竟是再度膨胀了一圈，云层都是在它的俯览之下，无数道魔臂挥舞着，仿佛是要撕裂天地。
“哈哈，现在的我，将拥有着无限接近吾皇的力量，凭你们，又如何与我斗？”魔皇之像仰天大笑，笑声如雷，将天地震慑得索索发抖。
“九重冰山狱！”
应欢欢玉手结印，一声冷喝，只见得天空之上，寒气疯狂的凝聚，最后化为九座看不见尽头的冰山之狱，而后狠狠的对着那魔皇之像镇压而去。
“哈哈。”
然而面对着应欢欢的全力攻击，那魔皇之像却是不屑大笑，一只魔臂爆轰而出，带起滔滔魔气，直接是生生的憾在那冰山之上。
砰！
天空颤抖，魔气奔涌间，只见得那庞大得看不见尽头的冰山，竟然是直接被其一拳轰爆而去。
天空上，应欢欢娇躯微震，而后急退而回，柳眉微蹙的望着那天地间的魔像巨人，后者此时，的确拥有了无限接近异魔皇的力量。
“怎么办？”
生死之主等人也是凑了上来，沉声问道，局面发展成这样，显然是出乎了他们的意料。
“想要具备异魔皇的力量，不是依靠这些手段就能达到的。”应欢欢盯着那魔皇之像，道。
如果光靠这种融合就能拥有跟异魔皇一样的力量，那未免也太小看这位魔中皇者了。
“他们施展这种手段，的确获得了极为强大的力量，但这却并不稳定，而且这种力量，天王殿也根本无法操控，只要我们能够将其纠缠住，恐怕要不了多久，这魔皇之像将会自动崩溃。”应欢欢道。
生死之主他们闻言也是点点头，旋即又是皱了皱眉头，眼下这魔皇之像的力量，就算是应欢欢都阻拦不住，眼下恐怕即便他们联手，能否阻拦，也是两说的事情。
“现在也没其他的办法了，若是能够将这魔皇之像解决掉的话，这天地，也将会彻底的平静下来了，这对于我们而言，应该算是真正的胜利。”应欢欢轻声道，旋即她看了一眼站在后面一直未曾再说过话的林动，冰凉的玉手，忍不住的紧握了一下。
“时间不多了，趁天王殿现在还不熟悉这魔皇之像的力量，我先将其缠住，你们调动所有的力量，对他进行攻击！”
应欢欢将心中的一分纷乱心绪压下，然后也就不再多说，玉足之下，寒气涌动，化为一座冰莲，而她则是盘坐下来。
天地间的温度，在此时陡然降低，甚至连涌动的天地元力，都是变得僵硬起来。
“莲生之界。”
应欢欢修长指尖轻点，殷红鲜血化为光点暴掠而出，落入下方地面，而后只见得大地瞬间被冰冻，无数巨大的冰莲生长而出，而在那冰莲中，则是有着犹如巨龙般的蔓藤呼啸出来，最后铺天盖地的将那魔皇之像缠绕而住。
吼！
魔皇之像挣扎着，巨龙般的蔓藤便是被震断而去，不过紧接着便是有着更多的蔓藤涌来，那种恐怖的寒气侵入魔皇之中，到是将它的速度减缓了下来。
生死之主等人见到这一幕，也不敢有丝毫的怠慢，目光一转，看向林动，沉声道：“林动，召唤乾坤古阵，让联军所有强者一起出手，将元力灌注进入古阵，与我们一动发动攻势！”
林动同样也很清楚眼下这紧急的情况，当即也没有任何废话，心神一动，乾坤古阵便是迎风暴涨，笼罩了天地。
“所有联军听令，立即将元力尽数灌入乾坤古阵，轮回境强者，随我们一同出手，此战胜负，在此一举！”
生死之主那稚嫩但却充满着凝重的声音，也是响彻而开，无数联军强者闻言，皆是紧握双拳，齐齐应喝。
“唰唰唰！”
一道道身影腾飞而起，最后出现在林动他们后方，那些全部都是轮回境的强者，只不过如今他们这巅峰般的实力，在那庞大无比的魔皇之像下，却是显得颇为的渺小。
那下方，联军之中，无数人盘坐而下，元力犹如海洋般的涌荡起来，最后化为一道道匹练，尽数的灌注进入天空上的乾坤古阵之中。
“动手吧！”
生死之主见状，也是一声沉喝，旋即她双目紧闭，黑白色的生死之力在其周身呼啸。
炎主，雷主，黑暗之主等人也是同时闭目，体内力量，运转到了极致。
“大哥。”
小炎几人来到林动身旁，他们看着那魔皇之像，眼中也是掠过一抹担忧。
“不用担心，不论结果如何，至少要与他们拼上一拼。”林动冲着他们微微一笑，道。
“你没事吧？”绫清竹莲步轻移而来，那清澈的美目，静静的盯着林动，之前所发生的那些事情她也是看在眼中，虽然眼下林动表面上没有什么，但心中怕是没那么平静。
“没事。”
感受着绫清竹那柔和中带着一丝关切的目光，林动心中微微一暖，缓缓摇了摇头，旋即看向小炎，青檀，小貂等人，道：“我们也动手吧，这一次，若是失败的话，或许这天地，也就完了…”
“嗯！”
众人重重点头，眼中皆是掠过一抹坚定之色，都已经到了这种地步，根本就没有后退的选择了！
小炎喉咙间发出低吼之声，紫金光芒爆发间，他的身躯，也是飞快的膨胀起来，很快的便是化为万丈巨人，不过这与那魔皇之像比起来，依旧还是弱了一筹。
“拼了吧，我这条命当年算是你帮忙捡回来的，能够再活这么多年，也算够本了。”小貂拍了拍林动的肩膀，俊美的脸庞上罕见的浮现一抹柔和的笑容，他望着林动，在那多年之前，一个稚弱的少年，带着一个随时都会烟消云散的妖灵，以及一头蠢虎，一人一灵一兽，便是这般开始了那漫长的历练，生生死死，谁能想到最后，他们竟能够达到这般高度。
“放心吧，不会让你们那么容易死掉的。”林动轻声道。
“我相信你。”
小貂微微一笑，狠狠的拥抱了林动一下，声音低沉的道：“老大。”
紫黑光芒自小貂体内席卷而开，万丈蝠翼伸展开来，他也是腾空而上，体内力量，催动到极致。
“呼。”
林动望着小貂的身影，淡淡一笑，旋即他凌空盘坐下来，双手结印，一股奇特的波动，飞快的蔓延而开，而在那股波动蔓延间，下方数十万里内的大地，都是开始变得荒芜。
曾经的大荒芜经，在此时，开始展现出它的可怕之处！


第1295章 自己来守护
荒芜以一种惊人的速度自下方沙漠中蔓延而开，然后那原本呈现暗黑色彩的大地，竟是开始呈现一种枯黄之色。
如今的林动再施展大荒芜经，显然不仅仅只是吸取大地表面上的能量那般简单，那些吸取，深入到地底之中，而后将那大地之中的能量，犹如长鲸吸水一般的汲取而来。
浓浓的荒芜味道，从大地深处散发出来，最后波及这方圆十万里之内，若是从高空看下去，此时整座大沙漠，都是被囊括在了其中。
大地震动着，一股股能量脉冲散发出来，最后对着林动所在的方向凝聚而来，所有人都是能够感觉到其中的那种可怕。
呼。
林动望着这一幕，轻吐了一口气，那漆黑眸子，倒是平静如水，旋即他轻轻一笑，这样的话，可还不够呢…
虽然这种汲取范围，几乎达到了一种极限，但大荒芜经的可怕之处，可并不在这里。
所谓荒芜，并非是将一切生机力量强行剥夺，荒芜之中，留有生机，待得春来时，那些掩藏的生机，便是会爆发出比以往更大的活力。
荒芜，还能再生！
淡淡的明悟，自林动的心中流淌而开，而后便是有着强者感觉到，下方大地那狂暴的能量突然间变得温和起来，再接着，他们便是见到，枯黄的大地，突然有着点点碧绿之色浮现出来，那是细小的嫩芽，嫩芽以一种惊人的速度长大，最后化为青草覆盖。
短短十数息的时间，这片大沙漠，竟然便是变成了一望无际的大草原，一种盎然的生机，弥漫而开。
“将你们的力量，再借给我吧。”
林动低头望着下方那看不见尽头的大草原，轻声自语，而后只见得那草原，再度开始变得荒芜，可怕的力量，与之前林动所汲取的那股恐怖能量，开始汇聚。
轰隆。
林动所处的下方万丈大地，在此时开始崩塌，无数闪烁着光芒的液体，自大地中破土而出，飞快涌上。
那些液体，乃是由压缩到极致的能量所凝聚而成，其中浩瀚程度，即便是轮回境的巅峰强者都是暗感骇然。
哗啦啦。
随着那些液体掠上的速度越来越快，到得后来，那几乎是化为了一道万丈碧绿水柱从地底之中喷射而出，而在那水柱的最顶端，则是林动的身影。
林动面色凝重，他的双臂，在此时也是开始液化，然后双手一合，一道巨弓，再度自其手中缓缓的凝现而出。
只不过这一次的巨弓，呈现一种极端古朴之色，上面没有丝毫光芒流溢，但却是能够感觉到那种内敛到极点的恐怖波动。
林动那化为液体般的手掌缓缓拉开弓弦，一道清澈的尖鸣之声，仿佛在此时自天地间响彻而起，那万丈能量液体水柱，犹如水龙般奔腾而上，最后在那弓弦中，化为一只碧绿色的长箭。
三道古老的祖符，自林动掌心升腾而起，互相交织，冲进长箭之内，顿时长箭颜色，便是变得绚丽起来。
嗡。
林动眉心，神光绽放，那神宫之内的精神力在此时倾巢而出，也是汇入长箭。
吼！
数十万道紫金龙纹呼啸而出，再度钻进。
温暖的白光，暴射而出，化为一道古老的符师，祖石摇动，一片片温和纯净的能量，也是加注在那长箭上。
大荒芜碑，玄天殿紧随而至，一波波的能量，全部的灌注进入那长箭之中。
咔嚓。
林动周身的空间，在此时开始蹦碎，甚至他那握着弓弦的手掌都是不断的爆裂着，如果不是此时他双臂化为祖符之手，恐怕早便是被那种恐怖到极点的能量，生生震爆而去。
这一箭，汇聚了林动所能够调动的一切力量的极限，感受着那种波动，甚至就连生死之主他们眼角都是微微抽搐了一下。
在林动不远处，绫清竹凌空盘坐，她周身没有任何的元力波动，只是唯有着林动这种级别的强者方才能够隐约的感觉到，在她内心的感应与呼喊中，一股神秘的力量正在降临而来。
嗡。
绫清竹玉手轻抬，轻轻一握，她面前的空间，开始裂开，旋即一柄约莫百丈左右的青锋长剑，缓缓的凝聚而出。
在那长剑之上，弥漫着那种来自神秘太上的力量，令人望而生畏。
与此同时，生死之主，炎主等人，也是将力量催动到极致，顿时这一片天空上，一道道恐怖的力量分别凝聚，那种程度的攻势，恐怕在场根本没有任何一个人敢说能够将其阻挡下来。
这是凝聚了这片天地中所有强者的最强攻击！
虽然如今这天地间已经没有符祖那种能够以一己之力扛起大梁的超级存在，但至少，当他们凝聚一心的时候，那种力量，依旧无法小觑。
轰轰！
那魔皇之像仿佛也是感觉到了天地间凝聚的那恐怖攻势，挣扎顿时变得剧烈了许多，那犹如巨龙一般将它缠绕的寒冰蔓藤，也是被它迅速的震断而去。
应欢欢银牙紧咬，美目之中晶莹之光不断的流转，体内力量也是催动到极致，这种时候，绝不能让这魔皇这像挣脱出来！
不过，如今这魔皇之像所拥有的力量，显然是超越了应欢欢，因此即便是她竭力阻拦，但随着时间的推移，也是开始逐渐的有些力不从心。
“不会让你过去的。”
应欢欢盯着那挣扎得连空间都是一片片的蹦碎的魔皇之像，声音之中显得有种毋庸置疑的坚定与冰冷，而后她轻咬舌尖，一道殷红精血顿时喷射而出，在寒气弥漫中，瞬间化为一道庞大无比的血红冰龙，而后冰龙缠绕，将那魔像双腿死死的捆缚住。
这般招式一施展出来，应欢欢脸颊也是微微白了一分，显然这种消耗并不小。
不过所幸的是，她的拖延也是取到了至关重要的效果，当那血红冰龙再度被魔像震碎时，后方天地间，无数道攻势，已是尽数准备待发。
应欢欢望着后方那无数凌空而立的强者，他们此时的面庞，显得有些狂热，虽然他们明知道局势已经到了最凶险的时候，但这种时候，依靠自己的力量来守护自己所想要守护的东西，想来反而会让人意志变得坚定。
“这一次，没有了师傅，就让我们自己来守护吧。”
应欢欢自冰莲上站起，而后她微微偏头，晶莹美目望向了那最前方的一道削瘦身影，一种在那很多年前从未有过的波动，涌上了她的心头。
以前，守护什么的对于我而言，只是一个不得不去完成的任务而已，可现在…我也想要心甘情愿的守护着什么，所以，这天地，可不能就这样的毁在你们手中了啊。
冰冷的唇角，轻轻的掀起一抹极为细微的弧度，应欢欢玉手抬起，只见得一点晶莹在其指尖凝聚，最后飞快的膨胀，短短数息间，便是化为一道庞大无比的冰莲，在那冰莲之中，隐隐有着血丝弥漫，莲心之中，竟是有着奇异的心脏跳动之声传出。
“去吧。”
轻柔了一些的声音，自应欢欢嘴中传出，而后那冰莲率先掠出。
轰！
就在这道声音落下时，这片天地仿佛都是开始崩溃，后方无数道凝聚着所有人最强力量的攻击，犹如掠过天空的陨石群，带着轰隆隆的巨声呼啸而过，最后铺天盖地的对着那魔皇之像轰击而去。
绫清竹玉指也是在此时轻轻点出，那由神秘太上之力凝聚而成的巨剑呼啸而出，闪电般的洞穿向那魔皇之像。
吼！
小炎发出震天怒吼，一道紫金光拳喷薄而出，犹如一头噬天猛虎，奔腾而过。
一轮万丈弯月，自小貂手中暴掠而出。
生死之主，炎主等人也是暴喝出声，那酝酿到极限的攻击，在此时撕裂天际，浩浩荡荡的席卷出去。
“杀！”
天空上，那乾坤古阵中，仿佛是有着古老的声音响彻而起，古阵疯狂的旋转着，一道几乎遮掩了整个大沙漠的古老巨手，自其中探出，拍碎虚空，狠狠的落向那魔皇之像。
嗡。
听得那响彻天地的厮杀声，林动那原本跳动的心，也是逐渐的平静下来，他凝望着那充斥着毁灭的魔皇之像，那紧扣的弓弦，也是在此时缓缓的松开。
嘭！
犹如石子落进幽潭之中，一圈涟漪荡漾而开，林动那液体双臂陡然炸裂而开，那一只古朴的长箭，颤颤巍巍的飞出。
没有惊人的速度，没有惊人的力量，甚至那种颤颤巍巍的飞行，让人怀疑它能够击中目标，但就是这么一支普普通通的长箭，却是成为了那漫天恐怖攻势之中，最为显眼的一道。
长箭过处，所有的攻势，都是悄然的为它避开了一条直通那魔皇之像的道路。
咻！
长箭追上最前方的那朵冰莲，而后后方剑啸传来，一道百丈青锋长箭与之相随，最后带着后方那铺天盖地的可怕攻势，尽数的落在那道魔皇之像庞大的身躯之上。
此时此刻，天地变色。


第1296章 位面裂缝
无数道璀璨攻势，犹如陨石群一般掠过天际，最后尽数的落至那庞大无比的魔皇之像上，霎那间，天崩地裂。
咚咚！
一圈圈可怕至极的能量涟漪，疯狂的席卷开来，这方圆数十万丈内的空间，几乎是在顷刻间蹦碎，下方大沙漠，数以万丈的沙浪层层的扩散而开，整座死玄大沙漠，都是在此时被生生的挪移了位置。
联盟大军也是被搞得狼狈不堪，无数强者被震飞而去，原本的战阵也是在此时荡然无存，不过此时也无人再理会于此，力量强到那魔皇之像的程度，已经不是数量什么的可以弥补。
天空上，应欢欢，林动他们同样是被这股狂暴无比的能量涟漪震得急退了数百丈，一些实力晋入轮回境的巅峰强者，也是忍不住的一口鲜血喷了出来，那一幕，委实有些壮观。
然而此时他们倒是顾不得这些，目光紧紧的望向那大沙漠的中央地带，那里狂暴的能量令得空间扭曲，谁也看不清楚其中景况。
那种扭曲的空间，足足持续了十来分钟，方才在那漫天弥漫的黄沙中逐渐的褪去，而无数人的视线，也是在此时立即的投射了过去。
空间逐渐的恢复，一道庞大得看不见尽头的魔影，缓缓的出现，再然后，变得清晰。
魔皇之像矗立在天地间，犹如从那异域而来的毁灭者。
“怎么会没事？！”
而当视线清晰时这天地间猛的爆发出无数惊骇的声音，那些联军中的强者惊恐的望着那近乎毫发无损的魔皇之像，面色都是煞白了起来。
那凝聚了他们所有人最强力量的攻击，竟然没有对这魔皇之像造成半点伤害？
“怎么可能？！”小炎也是惊呼出声，那魔皇之像虽然恐怖但断然也不可能无视他们的攻击吧？
林动双目微眯，紧紧的盯着那魔皇之像，旋即微微摇头：“那些攻击，的确被它强行吸收了，但这却并不代表没有对它造成伤害。”
咔嚓。
就在林动声音刚刚落下时，突然有着一道细微的声音在这天地间响起，然后众人便是见到那魔皇之像庞大的身躯上，竟然是出现了一道细小的裂纹。
咔嚓咔嚓。
这道裂纹一出现，就犹如引起了连锁反应一般无数道裂纹自魔皇之像身躯上浮现，最后遍布了它将近半个身躯。
嗤。
狂暴得近乎毁灭般的光线，自那些裂纹之中射出来，众人都是能够感觉到，此时那魔皇之像身躯之中所汇聚的可怕能量。
不过那种能量，显然这魔皇之像已是无法承受。
“赢了？！”
联军无数强者怔怔的望着这一幕眼神还略微有点茫然，这种情况，他们应该已经算是赢了吧？那魔皇之像此时就犹如一个炸弹，再也无法展开攻势了。
林动微微皱眉的望着这一幕，这魔皇之像似乎是故意强行将那些来自他们所有的攻击吸收进体，这样的话反而是导致它体内能量澎湃到某种恐怖的地步，这样一来，即便是那天王殿都是无法再操控，这种行为，显然是有些自寻死路。
不过，这天王殿，真是这么蠢吗？
“他是想要与我们同归于尽？”绫清竹来到林动身旁，轻声道，那种程度的能量一旦爆炸开来恐怕半个西玄域都将被毁灭，而他们这里，能够逃脱的人，或许也是寥寥无几。
林动闻言，目光微闪，心中也是凝重了一些，若真是那样的话，那他们这里可真是会死伤惨重了。
“林动，你我随时准备运转空间之力。”空间之主目光看过来，沉声说道，他显然也是想到了这一茬，眼下众人想要逃跑已是来不及，唯有他二人掌控着空间之力，能够在瞬间护住一些人脱身。
不过…那也是颇为的有限，毕竟即便是他二人联手，也不可能将这里所有人瞬间移动到西玄域之外去，那样的话，依旧会出现极重的损失，但现在的话，似乎也没有太多的办法了。
林动点了点头，心中一声轻叹，他也是只能尽力而为了。
应欢欢站在最前方的位置，她晶莹美目盯着那布满着裂纹，犹如世间最恐怖的毁灭炸弹一般的魔皇之像，弯月般的柳眉微微蹙了蹙，一抹不安，涌上心头。
“天王殿，这便是你最后的手段吗？我先前便是说过，只要能够把你们清除在这世间，还这天地一个清平，即便我们付出再大的代价，那也是值得。”
“你如此这般，终归还是输了。”
“呵呵…”
在应欢欢声音落下间，那魔皇之像中，也是发出了一道低沉的笑声，那笑声中，有着一点疯狂，也有着一些期盼已久的狂热。
在魔像眉心位置，光芒闪烁，只见得那天王殿的身体缓缓的融出，他探出半个身子，披头散发的望着应欢欢等人，脸庞上的笑容，甚是诡异：“冰主，你真的认为，你们已经赢了吗？”
“那你还能如何？现在的你，恐怕再下，就会自爆吧？”应欢欢淡淡的道。
“呵呵，是啊，我的确不能如何了。”
天王殿笑着：“因为当年符祖封印了位面裂缝的缘故，我们这些逗留在这片天地中的异魔，则是犹如囚徒一般被锁在这里，我从一开始就很明白，长久以往下去，我们终归是输掉，毕竟，这里是你们的主战场。”
“所以，想要逆转这种局面，只有一种办法。”
应欢欢，林动等人眼瞳都是微微一缩心中的不安，愈发的浓郁。
“那就是…”
天王殿脸庞上的神色在此时狂热到了极限，他扬起头，目光仿佛是穿透了苍穹：“破坏那位面裂缝封印，让吾族真正的进入这天地！”
“位面裂缝乃是师傅燃烧轮回所化，莫说是你，即便是异魔皇都无法破坏，你想将其破坏，真是痴人说梦。”生死之主冷笑道。
“说得没错，光靠我们想要撼动位面封印的确不可能，所以…我才要借助你们的力量啊。”天王殿桀桀怪笑他指了指这布满着裂纹的魔皇之像，其中那种毁灭性的力量，让人头皮发麻。
“如今这魔皇之像中不仅凝聚了我们魔狱所有人的力量，而且还有着你们所有人的力量…”
“我想，这应该足够了。”
天王殿的笑声陡然间变得大声与疯狂起来：“哈哈，这一切，都还要感谢你们呢，如果不是你们这般计划，也是无从施展！”
无数人的面色，都是在此时变得煞白起来，他们终于是明白，原来这天王殿从一开始就没想着和他们决战，他们一直都是抱着必死的心态试图借助他们的力量，将那位面封印所撼动！
而一旦封印被破，那异魔族便将会真正的倾巢而来，那时候，远古那场天地大战，将会再度的降临，那时候，他们有着符祖力挽狂澜，然而现在没有了符祖，他们根本就不可能从异魔的进攻中，将这天地守护下来。
想到那一幕，就算是以林动，生死之主他们的心性，眼中都是掠过了一抹骇然之色。
“现在，就让我们一起来观赏这数千载岁月中，最为壮观的一幕吧！”
天王殿疯狂大笑，所有人都是能够看见，那魔像庞大身躯之上的光线，愈发的狂暴，显然那魔像也是无法再承受。
应欢欢见状，玉手猛的紧握起来，惊人寒气自其体内弥漫而开，旋即她娇躯猛的暴掠而出，她那晶莹的双目，竟是在此时涌上了一些赤红之意，而后，血雾陡然自其体内喷射而出，最后弥漫了天地。
“冰之大世界！”
弥漫着血雾的寒气，飞快的涌出，最后将那魔像包裹，只听得咔嚓声响彻，一道十数万丈庞大的血红寒冰囚牢，凭空的凝结而出，将那魔像尽数的覆盖。
囚牢成形，一道道古老的符文也是浮现出来，将那种封印之力，开启到极致，此时只要将这魔像困住，那种毁灭性的力量就将会爆发，即便是以毁灭西玄域为代价，但那比起位面封印被撼动的结果，却是会好上无数倍。
“哈哈，现在你还想阻拦我吗？冰主，你真是太天真了！”
然而面对着应欢欢这般最后的阻拦，那天王殿疯狂的笑声，却是穿透出来，而后冰牢震动，无数道魔拳轰出来，直接是生生的将其轰成漫天冰屑。
噗嗤。
应欢欢俏脸微白，一口鲜血喷出，娇躯踉跄的退后了数十步，柔软的娇躯在此时显得极为的纤弱。
“哈哈。”
天王殿大笑着，而后他手印一变，众人便是感觉到一股毁灭般的力量喷薄而来，那魔像之上，裂纹愈发的明显，最后终是在那无数道骇然的目光中，彻彻底底的爆发开来。
轰！
那一幕，仿佛万轮烈日同时绽放，强烈的光芒波及整个西玄域，甚至其他三大玄域，都是能够看见，那从西玄域中升腾而起的强烈光芒。
“咻！”
不过出奇的是，那般毁灭力量，并没有在西玄域释放，而是直接化为一道庞大得看不见尽头的光柱，冲上云霄，穿透苍穹，直奔那遥远的位面封印所在的位置而去。
“哈哈，对了，忘记告诉你们最后一件事…吾皇已经脱离了符祖的封印，只要这位面封印一破，吾皇，就将会再度降临这片世间！”
“到时候，这片世界生灵，将尽为吾族之奴！”
“哈哈哈哈哈！”
毁灭光柱，穿过苍穹，然而那天王殿最后的疯狂笑声，却几乎是轰隆隆的响彻了四玄域，乱魔海以及妖域…
无数人呆呆的抬头，望着那远去的毁灭光柱，一种绝望，蔓延上心那曾经的异魔皇，又将要再现了吗？
在那远古时期，尚还有着符祖与其争锋，可如今，没有了符祖，这片天地，如何能够抵挡那般恐怖的存在？


第1297章 后手
毁灭光柱破开苍穹远去，然而这整个天地，仿佛依旧都还是处于天王殿临死之前的那轰隆巨声之中，那声音之中，充满着狂热与残忍。
远古时期，为了赢得那场天地大战，这片天地付出了极为恐怖的代价，无数强者陨落，甚至连那位符祖大人，都是在最后燃烧轮回，这才为那场天地大战打成了一个句号，不过现在，那曾经的一幕，又将会上演了吗？
大沙漠中，无数人面色惨白的望着天空，虽然他们看不见那苍穹之外，但隐隐的确是能够感觉到那种绝望的味道，他们此时方才知道，若真是当那异魔皇降临世间的话，这片天地，恐怕再也无人能够将其阻拦。
难道，这片天地无数生灵，真是要沦为那异魔族之奴的凄惨境地吗？
“异魔皇…竟然脱离封印了…”
生死之主他们也是目光有些茫然的望着苍穹，眼神之中，竟是有着一丝恐惧之色，如果说在面对着魔狱时，他们尚还有着将他们尽数斩除的信心的话，那么现在，他们却是连战意都是被浇灭了许多。
这一切，只是因为那异魔皇三字，经历了远古那场天地大战的他们，非常清楚那魔中皇者的恐怖。
林动望着瞬间低落下来的无数人，双掌也是忍不住的紧握起来，可笑他们一直在以天王殿等人作为对手，哪料到后者等人根本就没有与他们拼斗的心思他们的计划，狂热而令人恐惧，而且一旦成功，那么就真正的必胜无疑。
因为这个天地间，再无人能够阻拦异魔皇的脚步。
“大家不用太过的担心即便他们拼尽了一切，但位面封印乃是师傅倾力所为，他们想要将其破坏，哪有那么容易？”
而就在众人情绪低落间，一道冰冷的声音突然响起，而后众人便是见到应欢欢抹去嘴角的血迹，晶莹美目望向苍穹之外。
应欢欢玉手结印寒气突然在天空上凝聚成一面巨大无比的冰镜，冰镜晶莹，其中仿佛是有着日月穿梭而过最后镜面逐渐的黑暗，仿佛是出现了一片黑暗的虚无之地。
而在那黑暗的虚无中，众人逐渐的看见，一道巍峨庞大得近乎看不见尽头的古老阵法浮现，阵法晦涩令人看上一眼便是元力沸腾，有着失控的迹象。
一道道的光线重叠将阵法后方之物尽数的遮掩，不过隐隐的，众人似乎依旧能够看见，在那阵法之后，似乎是一道庞大的裂缝，裂缝犹如恶魔之嘴有着无尽的邪恶喷薄而出，但却是被那古老阵法牢牢的挡住，根本呜啊侵入丝毫。
“那就是…符祖大人留下的位面封印吗？”众人望着那黑暗虚无中的浩瀚阵法，喃喃自语。
轰！
光镜所反馈回来的画面，显然便是之前的一幕，因此众人也是能够看见，一道毁灭光柱，划过黑暗虚无，最后直接是犹如万颗陨石般狠狠的冲撞在了那古老的阵法之上。
嗡！
即便是隔着极为遥远的距离，但在撞击的那一霎，所有人都是能够感觉到，这片天地仿佛是微微的颤抖起来，天地间元力，都是纷纷溃逃而散。
所有人的目光，都是紧紧的望着那古老的阵法，心脏都是被悬到了喉间，他们现在唯一所能够寄托的，便是这道符祖所留下的封印，若这封印真的被破坏，那么，这天地，必将生灵涂炭。
所有人都是能够见到，随着那道毁灭光柱的冲击，一道数十万丈庞大的能量涟漪，犹如风暴一般，自那封印之上席卷而开。
而那封印之上，也是在此时爆发出古老光芒，那光芒隐隐的，似是化为一道看不见尽头的苍老光影，光影巨手拂过，将那毁灭波动，悄然的化解而去。
“师傅…”
生死之主他们望着那道苍老的光影，神色也是一顿，喃喃道。
一道道毁灭波动，不断的冲击着，又不断被那光影阻拦下来，两者仿佛是形成了一种僵持。
所有人都是心惊肉跳的望着那种僵持，不知不觉间，手心中，满是冷汗。
林动同样是眨也不眨的望着那冰镜中反馈回来的景象，对于那里，他并不陌生，当初他炼化空间祖符时，便是抵达过那里，而且还亲身体验了那封印阵法的可怕。
虽然那魔像自爆同样恐怖，但想要轻易的突破封印，也不是那么容易的事吧？
那种僵持，持续了许久，然后众人便是见到，那毁灭光柱，竟然在开始逐渐的减弱，显然，那种力量，已经开始被消耗殆尽。
大沙漠上，顿时爆发出惊天动地般的欢呼之声，果然，虽然那天王殿使劲了手段，但是想要突破祖符封印，又谈何容易？
不论如何，那天王殿，总归是没办法与符祖相比的。
林动听得那震天动地般欢呼声，神色却是没有丝毫的放松，他紧紧的盯着冰镜中，他望着那封印之上的古老光影，心中反而是有着一抹不安涌起来。
这道不安，究竟是怎么回事？
林动抬头望向应欢欢，只见得她玉手仿佛也是紧握了起来，脸颊上，满是冰冷的凝重。
“轰！”
而就在林动心中不安时，那毕竟之中的位面封印，突然剧烈的颤抖了一下，在那封印后方，仿佛是有着邪恶邪恶到了极点的模糊影子浮现。
咻。
封印颤抖得越来越剧烈，旋即突然一道细小的黑色光束，猛的自那封印后方暴射而出，最后径直的从那道符祖光影眉心之间，洞穿而出。
漫天欢呼声噶然而止。
无数道呆呆的望着那道并不怎么起眼的攻击，眼中满是茫然之色，那道攻击，是从哪里来的？怎么竟然能够穿透那位面封印？
“异魔皇！”
应欢欢银牙紧咬，冰冷的寒气，自其体内席卷而出。
生死之主他们的面色，也是在此时苍白下来，从先前那道攻击中，他们察觉到了一股极为熟悉的波动，那是属于异魔皇所有。
林动也是目露震动的望着这一幕，借助着天王殿他们所造成的空隙，猛然出手，内外夹攻，以此来破坏位面封印吗？
那天王殿，恐怕从一开始就明白他们没办法破坏位面封印，而想要做到这一步，他们必须需要异魔皇的力量。
他们所取到的作用，其实只是造成一些空隙，令得封印出现颤动，从而让那位异魔皇，寻找到破绽出手！
但这种时机，怎么会把握得如此的完美？！
林动想到此处，身体猛的一颤，是那魔皇之手的感应？莫非，那位异魔皇，在那千载之前被封印时，就想到了这一天？所以方才自断手臂，以其作为内应，以待来日反攻？
这般心机，竟是深到如此地步？！连那十载之后，都是预算到了？
林动身体微微颤抖，这究竟是多么恐怖的敌人啊…
“异魔…皇？”
联军无数强者惨白的望着冰镜中，那里，那道古老的光影，开始逐渐的淡化，显然先前那一道细小黑光的洞穿，也是将封印的完美平衡所打破。
“符祖大人！”
无数强者望着那逐渐淡化的古老光影，竟是忍耐不住的跪伏下来，嘶吼声响彻天地，若是连这最后一道防护都是被破去的话，那这片天地，就将会彻底的暴露在异魔的觊觎之下！
林动听得那漫天的嘶吼声，其中充满着绝望以及恐惧，那是一种天地将亡的感觉。
天空上，应欢欢玉手也是紧紧的握着，银牙咬着红唇，甚至是有着一丝鲜血渗透出来。
生死之主等人也是面色苍白。
古老光影越来越微弱，而就在它即将消散时，光影那原本空洞的眼中，仿佛是有了一种情感的出现，他望向冰镜，苍老的脸庞上，竟是浮现了一抹笑容。
“不用感到绝望。
苍老的声音，犹如从那天外传来，悠悠的响彻在了这天地间，顿时让得无数人身体一震，震惊的抬头，望向冰镜。
“师傅！”
生死之主他们也是面露震惊之色，他们望着冰镜之中那即将消散古老身影，猛的跪伏下来。
应欢欢也是仰起俏脸，怔怔的望着那道光影。
“我的弟子们，异魔皇留有后手，我也同样有所后手，那便是你们…这一次的天地大战，我相信你们能够守护自己想要守护的东西。”
光影的目光，仿佛是穿透时空，凝聚在了应欢欢的身上。
“冰儿，当你真正想要守护时，你将会拥有真正的力量，当年我第一次看见你时，便是知晓你所拥有的力量。”
“呵呵，这一次，就靠你了啊，不要怪为师给你留了这么重的担子。”
古老光影慈和一笑，那光影，最终也是完全的散去，唯有着那并不完整的位面封印，还在顽固的存在着。
应欢欢望着那消散而去的光影，那俏脸上，有着晶莹的泪水滑落下来，她轻轻的搽去，然后似是对着自己微笑了一下。
“师傅，您放心吧，弟子知道怎么做了。”
她凌空而立，晶莹的长发随风舞动，那一霎，她柔弱得令人心疼。


第1298章 抉择
大沙漠之中，狂风呼啸，黄沙弥漫，那联军之中无数强者则是面色苍白的望着天空上那巨大的冰镜，冰镜中，虽然那位面封印依旧存在着，不过那道古老光影却是消失而去，显然，这阵法的中枢，随着光影的消散，将会出现削弱。
而这种中枢被破坏的位面封印，还能阻挡那位已经从封印中脱离而出的异魔皇吗？
而如果到时候位面封印被破，异魔皇再度降临，这世间还有谁能够将其阻拦？
无数人面色苍白，眼中弥漫着悲戚之色，这难道便是他们努力的结果吗？
天空上，生死之主他们望着这一幕，也是一声暗叹，然后他们抬起头，目光望向天空上的那道纤柔身影，她凌空而立，晶莹的长发飘舞着，这种局面，若是还能够有所转机的话，恐怕，也就只有在她的身上了。
或许是察觉到了生死之主他们的目光，那联军中无数强者也是开始转移目光，最后投注在应欢欢身上，先前符祖消散所留下来的话语中，显然这最后的转机，已是落到了这位传说之中的冰主头上。
这天地生灵能否免去那异魔践踏，或许，也唯有她方才有着能力逆转了。
林动望着那天地间无数道看向应欢欢的目光，那些目光中，充满着期盼与希冀，那是绝望中所看见的一点点光亮。
只不过，这种最后的希冀，凝聚在一起，却仿佛是形成了一种无形的压迫，这些压迫，犹如重重大山，重重的压在她那柔弱的肩膀之林动盯着那道凌空而立的倩影，心中掠过一抹心疼，旋即微微咬牙，偏过头去。
天空上，在那无数道希冀的目光注视中，应欢欢终是缓缓低头，她并未看向那些将她视为救世主般的人，只是将视线投射到了林动的身上林动看着远方，仿佛并未有所察觉。
她的目光并没有转开的迹象，于是天地间那无数道目光也是有些微愕的看向林动，略微有些不明白他们之间的关系。
两人这般，持续了好半晌，林动终是撇了撇嘴，转过头，目光直视着应欢欢，后者那晶莹的眸子，静静的看着他，其中仿佛是有一种极为复杂的情感在涌动。
然而林动望着她这般目光，心中却是掠过一抹不安。
“异魔皇的确已经破开了封印，如今正在位面裂缝之外虎视眈眈，不过有着师傅留下的封印，即便是异魔皇想要将其真正破坏，那也至少还需要一年的时间。”应欢欢的声音，在天空上传开，令人有些惊喜，又有着忧愁。
喜的是他们还有着一年的时间，忧的是熬过了这一年，又能如何？
“现在联军已是没有了多大的作用，想要真正避免一年之后的末日，我们唯有一个办法。”应欢欢美目微垂，轻声道：“那便是再诞生一位符祖。”
无数强者哑口无言，谁都知道如果再有第二位符祖，劫难就可避免，可，符祖那般境界，岂是说到就到？
林动紧紧的盯着应欢欢，心中那抹不安，越来越浓。
“接下来，我会调动这天地间所有的力量，再联合远古之主，八大祖符，九大神物…”应欢欢玉手陡然紧握，那冰冷的声音中，都是多了一丝凌厉。
“助我成祖！”
哗。
联军之中，顿时爆发出惊天般的哗然声，那一道道目光中，皆是有着浓浓的震惊涌出来，但紧接着，狂喜开始涌上脸庞，如果应欢欢真的能够晋入那传说之中的祖境，那他们，岂不是便是拥有了第二位符祖？！
生死之主他们也是有些震动的望着应欢欢，她，终于是要动用那种力量，冲击祖境了吗？
“冲击祖境…”
林动同样是被应欢欢此话震得心中掀起滔天骇浪，许久后方才逐渐的平静下来，他咬了咬牙，道：“你冲击祖境，有着多少成功把握？若是失败的话，你会如何？”
应欢欢晶莹美目看着林动，道：“三成把握，若是失败…轮回破碎，彻底陨落。”
大沙漠中，狂喜气氛一滞，三成把握？这么低？无数强者面面相觑，旋即苦笑一声，这到是他们奢望了，祖境哪是那么容易晋入的，即便是强如冰主这等人物，也唯有着如此低的成功率…
而且，那失败的后果，也太可怕了点吧…
林动紧紧的盯着应欢欢，这结果算是在他的意料之中，只不过，他还是感觉到一些不对劲，因此他犹豫了一下，再问道：“若是成功了呢？”
应欢欢那晶莹的瞳孔似是在此时微微缩了缩，道：“成功便成功了，这还有什么好问的？”
“生死之主，敢问她说的可是实话？”
林动看向生死之主他们，后者等人微微犹豫，旋即暗叹了一声，道：“小师妹，这时候了，也没必要再瞒他了，你想要冲击祖境，若是失败，自然轮回破碎，但若是成功的话，恐怕你也将会彻底的冰冻一切的情感，师傅以前便是说过，你是与众不同的。”
“果然。”
林动双拳忍不住的紧握起来，他眼中掠过一抹怒火，眼睛盯着应欢欢，冷笑道：“真是舍身大义啊。”
这样说来，不论是成与败，都不会有任何的好结果，这女人，总是这样！
应欢欢望着林动那充满着怒火的目光，却是毫不退缩的与他对视着，那眼中，同样是有着一抹不容改变的坚定。
对视了片刻，林动神情突然缓和了一点他能够看见她那坚定之后所隐藏的无奈与彷徨，所以他轻声道：“可以换个人吗？”
“还有比我更好的人选？”应欢欢弯月般的柳眉似是高傲的挑了一下，道。
“现在的我，应该也不至于差到哪里去吧？”林动咬着牙道。
的确，如今的他也是踏入了神宫境实力或许比应欢欢略弱一些，但也绝对不比其他远古之主弱了。
“不行！”
应欢欢月眉微竖，毫不犹豫的回绝。
“你！”林动一怒，喝道：“你就这么想去当那救世主？”
“这是我的使命。”应欢欢轻声道。
林动双拳紧握，身体微微颤抖着，他盯着应欢欢，许久后突然深吸一口气，神情变得落寞下来，他声音有些沙哑的道：“这就是你的选择了吗？”
明明现在的我已经能够具备了那种资格为何你还是要一意孤行？
应欢欢望着林动那落寞之色，玉手也是忍不住的紧了紧，旋即轻咬着银牙缓缓的点了点头。
林动淡淡一笑，笑容中有着掩饰不住的疲惫，旋即他神色也是变得漠然下来：“那就祝你成功吧。”说着，他便是转身欲要离去。
“等等，最后的冲击，还需要祖符与祖石他们的力量，我希望你能够操控他们来帮我。”应欢欢望着林动转身而去，冰冷的心中有着一抹酸楚涌上来，不过却是被她生生的压制下来旋即她开口道。
林动身体微顿，他转过身，疲惫的凝视着应欢欢，望着他的目光，即便是此时的应欢欢都有些不敢与他对视，微微的偏开眼睛。
“能问你最后一个问题吗？”
“问吧。”
林动手掌缓缓的紧握，他盯着应欢欢，一字一顿的道：“现在的你，究竟是冰主还是，应欢欢？”
应欢欢沉默下来。
联军中无数强者望着两人，显然也是察觉到两人之间那种复杂的关系，面面相觑间，也是不敢言语。
林动望着沉默着不说话的应欢欢，似是明白了什么，淡淡一笑，道：“也好，让她在我心中吧，多谢了。”
话语一落，他手掌一抬，三道古老的符文自其体内射出，然后飞向应欢欢，林动指尖血迹掠过，似是隔空轻轻一划。
血迹掠过，那三道古老的符文突然剧烈的一颤，隐约的有着嗡鸣之声爆发而起，一种不舍的情感弥漫出来。
咳。
林动捂着嘴咳嗽了一声，嘴角浮现一抹血迹，面色也是苍白了许多，他竟是直接单方面的将三道祖符与他的联系中断了去。
“林动！”
“林动哥！”
“大哥！”
绫清竹，青檀，小炎他们见状，面色顿时一变。
林动摆了摆手，再度一挥手，祖石，大荒芜碑，玄天殿皆是掠出，他淡淡的道：“你们去帮她吧。”
“林动，你…”岩闪现出来，他望着林动，急忙道。
林动笑了笑，道：“我接下来也有事情，恐怕分不了身，只能这样做了。”
“你要做什么？”岩连忙问道。
“渡轮回劫。”林动轻声道。
岩一愣，他望着林动的面色，突然有些不安涌出来：“渡第一重轮回劫？”
林动笑着摇了摇头，道：“三重。”
岩神色一滞，那天空上正呆呆望着面前三道祖符的应欢欢，娇躯也是猛的一颤。
“冰主大人，祝你成功了，接下来应该没我什么事了，告辞。”
林动对着应欢欢一抱拳，然后不再有丝毫的停留，身形一动，便是化为一道虹光，决绝的对着西玄域之外暴掠而去。
青檀，绫清竹两女以及小炎小貂见状，也是连忙跟了上去。
无数人望着这般变化，都是呐呐无语，显然是搞不清楚为什么会变成这样。
应欢欢伸出冰凉而修长的玉手，接过那三道祖符以及祖石等物，晶莹的美目有些失神的望着那道远去的身影，内心深处突然涌出来的难受之感，令得她紧紧的咬着红唇，一丝殷红渗透出来。
她在那无数道目光的注视着，微微的仰起俏脸，用仅有她方才能够听见的声音，喃喃自语。
“一直都在骗你…对不起…真是，活该被讨厌呢…”


第1299章 青阳镇
原本与魔狱的最终决战，却是因为天王殿等人那般疯狂之举而终结，不过天地间的危机，却并没有因为天王殿他们的消失而淡化，反而更加的让人感到绝望，因为那位面封印，已是失去平衡，而且，最令人恐惧的是，那曾经被符祖燃烧轮回彻底封印的异魔皇，竟然也是脱离了封印，如今正在那位面封印之外虎视眈眈，随时准备再度降临这天地之间。
而那时，必将是一场真正的浩劫。
三大联军自西玄域中撤退而出，除了其中真正的顶尖强者外，其余的联军，都是各自退回了他们所来之地，这种时候，联军已是是没了意义，如果那位异魔皇再度踏足这片世间，而他们又并没有再诞生第二位符祖的话，联军再多，也是于事无补，那种存在，并不是靠单纯的数量可以弥补的。
而随着三大联军的撤回，那西玄域中所发生的事情，也是在极短的时间内，扩散到了整个天地，顿时无数人为之骇然，他们抬起头看着那苍穹，仿佛隐约的能够感觉到，在那极为遥远的虚无中，一道毁灭的身影，正在越来越近。
一种无法形容的压迫感，笼罩在了这天地之间。
无数人为之颤粟，不过他们终归并没有彻底的绝望，因为他们知道，那位传说中的冰主大人，如今正在竭尽全力的冲击祖境，若是她最终能够成功的话那么这天地，就将会再度获得拯救。
那是这天地生灵绝望之中所能够寻找到的唯一希冀。
林动离开西玄域后，直接回了大炎王朝，这是他这么多年来第一次感到一种疲倦，那种疲倦令得他并未再去林氏宗族也并未再回道宗。
站在大炎王朝的上空，林动想了想，便是对着大炎王朝西部掠出，不久后，重重山脉自下方掠过，最后，一座繁华的小镇出现在了其视野中。
小镇并不算太大，隐约还能够有着一些熟悉的感觉，那种感觉令得林动疲倦的心中涌上了一抹特殊的情感。
那是青阳镇，他出生与长大的地方。
林动心神一动，已是出现在了那小镇之中，熙熙攘攘的人流伴随着吵杂的声音传开，林动缓步走于其中，看似拥挤的人流却并未对他造成丝毫的阻碍。
小镇这么多年已经是变化了不少，但依稀还是能够凭借着记忆找到一些熟悉的地方，林动带着一丝恍惚的走出街道，而后他感觉到身旁有着幽香浮现，偏过头，只见得一身白裙容颜绝美，气质脱俗犹如谪仙般的绫清竹正俏然的站在他身旁，见到他望过来，那绝美的脸颊上也是浮现一抹浅笑。
“还好吗？”
绫清竹轻声道，声音有着前所未有的柔软，这么多年，她也是第一次见到林动露出这般疲态，这个家伙，一直都是气势凌厉不管前面是刀山还是火海，风风火火直接冲上，即便是被人打趴下了，他依旧能够抹着鲜血，眼神凶狠的再度爬起来，无数人都只能看见他那种惊人的成就，但却是忘记了那种成就之下他的付出。
他遇见了一个又一个的强敌，但他，始终都是站在最后的人。
林动轻轻点头，只是情绪依旧不高，绫清竹见状，银牙轻轻咬了咬嘴唇，然后便是伸出那修长而纤细的玉手，伸入林动手中，十指相合，紧紧的将其握住。
林动微微一怔，他能够感觉到绫清竹那僵硬了一瞬的娇躯，后者性子清傲，很多时候都是将情绪隐藏在心中，默默的承认着苦与甜，从不与外人言起，类似这种主动时候，倒真是极少极少。
林动轻叹了一声，为绫清竹这能够放下她固有的清傲来安慰他的举动感到有些温暖，他盯着她，轻声道：“抱歉了。”
从别的人身上受到了挫，却是要她放下性子来照顾他，这对她而言显然是有些不公平，她一直都是这样，默默承受着，坚强得令人心疼。
“不用道歉啊。”绫清竹微微摇头。
“走吧，我带你回林家。”
林动笑了笑，拉着绫清竹对着记忆之中的林家走去，那并不是林氏宗族，而是林家，一个小小的分家，但却是他心中最重的地方。
绫清竹螓首轻点，嫣然轻笑，百花失色。
两人走过街头，然后林动脚步停了停，目光有点讶异的望着前方，那是数条街道的交叉口，只不过如今的那里显得有些混乱，看那模样，似乎是两批人马在打群架，拳拳到肉的声音响起，还伴随着惨叫。
林动目光看向那一批人马前方，那里有着一名面色凶悍的青年正拖着一根木棍将对面的人一个个的扇飞过去，那股凶狠劲，吓得无人敢拦。
林动望着那隐隐有些熟悉的青年，微微有些恍惚，旋即终于是从记忆中将其翻了出来。
狂刀武馆，吴云。
那青年实力．颇为不弱，一人解决了绝大部分的对手，然后他得意的拍拍手，将手中木棍丢弃，笑道：“敢在青阳镇招惹我们狂刀武馆，不想混了是不是？”
“呸，敢情你认为青阳镇就你们狂刀武馆最牛吗？！”
“你有意见？”吴云眼睛一瞪，喝道。
“你难道忘了我们青阳镇的林家了吗？！”那人冷哼道。
吴云表情一僵，旋即恼羞成怒的道：“你懂个屁，老子跟林动一起联手对付谢雷两家那些兔崽子的时候，你他娘的还不知道在哪个角落呢！”
他声音一落，不远处似是有着轻笑声响起吴云顿时怒火满面的转过头，准备去找敢在这时候嘲笑他的家伙的麻烦。
不过，就在他转身时，便是见到了街头所立的黑衫青年以及站在他身旁那美丽得让得所有人呼吸都是一滞的绝美女孩。
“你…”吴云也是因为绫清竹的容颜失神了一下，不过很快他便是回过神来双目陡然睁大的望着那黑衫青年，手指惊骇而颤抖的指着他：“林，林动？！”
原本喧闹的街道，瞬间鸦雀无声，一道道难以置信的目光望向那黑衫青年，这位，便是那位传说中的林家林动？
众多目光凝聚而来那难以置信开始转变，一些狂热涌了出来，如今大炎王朝谁人不知林动之名而且，那种传说人物，正是从他们青阳镇走出去的！
察觉到他们那种狂热的目光，林动远远的冲着吴云拱手一笑，然后便是拉着绫清竹对着街道的另外一头走去。
望着他们的背影，却是无一人敢追上去甚至连那吴云都是摸着脑袋，有些亢奋的一拳打在身旁之人胸膛上：“你看见没？林动对我打招呼了，妈的，老子平常跟你们说了你们还不信…他竟然回青阳镇了，待会一定要去看看他！”
“哎哟，老大轻点。”
林家大门之外，与别的地方相比，这里显得清冷许多，林家后来大多都是搬迁到林氏宗族去了，所以青阳镇这里的老宅倒是并没有留太多的人。
不过当林动出现在大门前时，依旧是被一名打扫庭院的下人发现，然后后者一阵发愣，下一刻直接拔腿飞奔，那尖锐的喊声传遍了整个林家。
“林动少爷回来了！”
原本有些幽静的林家，瞬间沸腾起来，不少人涌出来，目光激动与狂喜的望着那自大门处走进来的挺拔青年。
“动儿！”
那人群突然分裂开来，一名头发有点发白的中年男子面色激动的快步走了出来。
“大伯。”
林动望着那中年男子，也是连忙走上将其扶着，此人正是他大伯林肯，这些年他一直留在青阳镇，守着老宅。
“你这小子，终于回来了，我还以为你忘记这里了呢。”林肯比起当年变得苍老了许多，但他望着林动的目光中，却满是欣喜之色，虽然他留在青阳镇，但对于林动所闯出来的名声依旧是有所耳闻，这可是他们整个林家的骄傲啊。
林动心中感触，看向绫清竹，捎了捎头，道：“这是我大伯，你，也这么叫吧。”
绫清竹绝美的脸颊上飞上一抹红霞，旋即她对着林肯盈盈一礼，道：“见过大伯。”
“呵呵，你便是清竹吧？不错，林啸都已经给我说过了，呵呵，动儿这小子还真有眼光。”林肯笑眯眯的点点头，目光打量了一下绫清竹，眼中满是满意之色，以绫清竹的容颜气质，整个东玄域都寻不出多少女子能够与其媲美，在这小小的青阳镇，那更是无人见识过，这从周围那些偷偷打量过来的不少年轻而火热的视线中便是能够看出来。
“回来就好，你的房间可一直有人打扫着呢，需要我让人带你去吗？”
林动摇了摇头，陪林肯聊了一会，然后便是带着绫清竹对着后山而去，这后山随着他们的离去已是很久未有人搭理，乱草纷杂，轻风吹来，漫山遍野的青草野花飘舞着。
在后山中，有着数间屋子，屋子虽然简陋，但却是极为的整洁，林动来到屋前，然后怔怔的望着那些深深插入地面的木桩，木桩上，还隐约可见各种掌印深痕。
林动站在那些木桩前，有些失神，犹自还记得，很多年前，那个身躯单薄的稚嫩少年，在这里挥汗如雨，一遍又一遍的苦修着简单的武学，那时候的他，只是很简单很单纯的想要变强，然后帮自己的父亲讨回一些公道而已…
或许那时候的单纯少年也并不会想象到，曾经的某一天，他竟然会成为这天地间最为顶尖的强者之一。
只不过，那样又如何呢？
林动缓缓的闭上双眼，脸庞上满是倦色。
接下来的日子，林动一直的留在青阳镇，并没有要离开的迹象，而他每日也过得异常的平静，只是在那幽静的后  山之中漫步而行绫清竹则是静静的跟在他的身旁。
林肯似乎也是察觉到了林动的情绪，因此将那些各种拜访都是推了去，不准任何人去打扰他们。
而在林动来到青阳镇后的第三天，小貂与青檀也是赶了过来，而且他们还去都城将林啸，柳妍他们都是带了过来。
柳妍一见到林动，便是连忙跑过去将他抱在怀中眼睛都是红了许多，在来时小貂显然已经将所有的事情都告诉了她，而她也是第一次见到这个随时都充满着闯劲无论遇见什么事都绝不会认输的儿子第一次这么的疲倦，那种神情，让得她心都要碎了。
“动儿，世界上很多事情都没办法圆满的，你的努力，并没有白费你是我们心中的骄傲。”
柳妍抱着林动，流着眼泪，道：“你如果累了，还有爹娘呢，那些什么拯救无数生灵的事，太远了一些不管你们说的那什么异魔皇有着多么的可怕，不过只要娘在你身边，就不会让你比娘先死掉。”
这一刻，绕是以林动这些年修炼出来的坚韧心性，都是忍不住的紧紧抱着柳妍，眼泪流下来，露出了这么多年来，他最为脆弱的一面。
一旁的林啸也是红着眼睛，他拍了拍林动的肩膀道：“你已经做得很好了，爹为你自豪，接下来的时间，我们就都留在青阳镇吧，我让林霞林宏他们都回来，一直陪着你。”
林动抹去眼泪，笑着点了点头。
林啸最终将眼睛通红的柳妍拉了开去，一旁的小貂看着林动，然后看向那后山深处，笑道：“我记得你便是在那里得到祖石的吧？那时候我就感应到你了。”
“如果那时候早知道里面躲了一个妖灵的话，恐怕我就直接把它给扔了。”林动道，然后与小貂对视一眼，皆是笑了出来，笑声中有着浓浓的情谊。
“小炎本来死活也是要跟来的，他还想把洪荒祖符也还给他们，不过被我制止了下来。”小貂道。
林动点点头，青檀也是将黑暗祖符给了黑暗之主，没必要为了他这边再去降低那边的力量，现在那里应该很需要这种力量。
“这些时候，我们就陪着你吧，反正现在什么事都跟我们没关系了，如果他们成功的话，那天地自然得救，若是失败，也不用挣扎了，大家一起死也挺干脆的。”小貂笑着说道。
林动一笑，点点头，只是笑容有点勉强。
时间一天天的过去，林家再度热阄起来，林宏，林霞等人被叫了回来，甚至连一些林氏宗族的年轻子弟也是死皮赖脸的跟着他们跑来了青阳镇，一时间原本冷清的林家变得火热许多。
林动仿佛也是放下了很多东西，偶尔与他们笑闹着，想想那小时候的一些事，也是会忍不住的莞尔失笑。
这般时日，一晃便是半月过去。
后山，林动盘坐在山崖边，目光怔怔的望着那缭绕的云雾以及山峦叠嶂，这半月时间，他几乎没有修炼过，体内原本存在的三道祖符，祖石，大荒芜碑等等神物，皆是被他尽数的单方面的斩断了联系，那种感觉，犹如断了一切的因缘。
身后有着细微的脚步声传来，林动微微偏头，然后便是见到青檀轻盈而来，如今的她，换下了她在黑暗之殿经常穿着的黑色衣裙，换上简简单单的碧绿衣衫，隐隐的，仿佛是有着一些当年那般小女孩的青涩。
青檀来到林动身旁，然后跪坐下来，清澈的大眼睛看着他，轻声道：“林动哥你还是不开心啊？”
“没有啊。”
林动笑着揉了揉她的小脑袋，道：“只是感觉这么多年的修炼，好像都白费了一样。”
“没有啊，林动哥，你还是改变了很多东西的，一直以来你都是我心中最厉害的人，当年那林琅天在我们眼中是多么的厉害啊，即便我们每个人都很仇恨他，但却没人敢真正的做什么，只有你在努力修炼，最后还真正的成功，你知道吗，在当你斩杀林琅天的消息传回来后，连爹都哭了呢。”青檀歪着头，娇笑道。
“不管别人怎么想，但你在我们心中，是最重要的。”
林动望着一脸认真的少女，微微失神，心中有着暖意涌动。
唰。
一道破风声传来，小貂的身形突然出现在了悬崖边，他看着林动，欲言欲止。
“怎么了？”林动看向他，问道。
“据传来的消息，他们那边已经在乱魔海布置阵法，准备动手了林动神色微微一滞，旋即他低着头沉默了许久，方才缓缓的抬起头，用力的深吸了一口气，然后他轻轻一笑。
“那我也开始吧。”
为了我所在意的人，我会用我办法来守护。


第1300章 一年
乱魔海，上空。
与无人得知的小小青阳镇相比，这里，却几乎是成为了这片天地之间最受瞩目的地方，因为远古八主之一的冰主，将会在这里，冲击祖境！
这将会是天地无数生灵最后的希望。
海域之上，黑压压的人海弥漫开来，几乎将下方的海域都是遮掩了去，而此时，他们全部都是将那紧张的目光投向高空，在那天空上，一道道身影凌空矗立，强大的气息扩散开来，笼罩着天地间。
那些人，几乎全部都是处于轮回境的层次，这显然已经算是这天地间最为顶尖的战斗群了。
而在那天空中央的位置，则是数道人影静静矗立，彼此间的气氛，却是略微有点沉默。
应欢欢则是望着苍穹，那晶莹的美目中寒光流溢，仿佛是透过了遥远的距离，看贝了那虚无之中的位面封印。
那里的封印，正在逐渐的被削弱。
“我们也准备开始吧。”应欢欢收回目光，看向生死之主他们，道。 她此时的声音，有着一种深入骨髓般的冰寒，其中似乎是并没有半丝的情绪波动，这令得她看起来，犹如一块万载玄冰，冰封了情感。
生死之主他们望着这般状态的应欢欢，也是忍不住的苦笑一声，自从当日自西玄域回来后，她便是变成了这样。
“也不知道林动那小子是不是在渡三重轮回劫了这家伙，也是个疯子啊。”空间之主嘀咕道。
应欢欢美目微垂，仿佛没听见一般，只是那天地间突然变得冰寒许多的寒气，还是暴露了一些她内心的波动。
在那个名字面前，她显然还是不可能保持真正的冰封心境。
“我说你为什么不让林动来试试？现在的他已经够了这种资格，他那般拼了命的修炼，不就是为了阻拦这一幕的出现吗？为什么要在这种时候否决他？”炎主终于还是叹了一口气道。
“你就少说两句吧。”生死之主皱了皱眉，她望着那犹如一块寒冰般的应欢欢，心中也是一声暗叹，林动固然难受，但她又何曾好过？
只不过这种时候，她又能怎样？
“准备吧。”
应欢欢只是盯着下方的人海淡淡的说了一声，然后终于是抬起头，目光望向了东玄域的方向，那袖中的玉手紧紧的握着指甲深入掌心，殷红的鲜血顺着纤细的指尖滑下，然后化为殷红的冰珠，化为粉末。 她凌空盘坐下来，而见到她这般举动，这片天地都是陡然变得安静下来，生死之主六人在其周身呈环形般盘坐而下再更外围，那无数强者也是凌空而坐，似乎是形成了一个极端庞大的阵法。
“八大祖符。”
应欢欢红唇微启，冰冷的声音，传荡而开。
咻咻咻。
只听得破风之声响起 一道道光华掠出，最后悬浮在天空之上 光芒散去，化为一道道古老的符文。
不远处，慕灵珊也是轻咬嘴唇，娇躯一动，光芒从其体内散发而出，化为一道黑白交融的古老符文，冲了出去。
八道祖符，悬空旋转浩瀚无穷的力量，散发而出。
“九大神物。”
祖石，大荒芜碑，黑暗圣镰，灭王天盘洪荒石斧一道道在那远古声名赫赫的强大神物也是浮现，最后在那八大祖符之外形成光圈。
祖符与神物光芒映射一道道光线开始蔓延而出，最后仿佛是形成了一道极端复杂的庞大阵法，而在那阵法中央，则正好是应欢欢盘坐的位置。
“阵法启动！”
听得这道冰冷之声，那外围所有踏入轮回境的巅峰强者皆是齐声应喝，而后心神一动，磅礴浩瀚的元力席卷而出，顿时这片海域都是为之疯狂，万丈巨浪滚滚而来，那粘稠的元力，也是滔滔不尽的灌注进入那阵法之中。
嗡嗡。
一道方圆十万丈庞大的光圈，缓缓的自那天空上扩散而开，壮观之极。
而那无穷无尽的元力，则是顺着阵法的运转，最后化为道道光柱，融入了那阵法中央的应欢欢娇躯之中。
生死之主六人见状，也是双手结印，七道粘稠的光束自他们掌心中射出，最后融入应欢欢身体。
咔嚓。
随着那般恐怖的能量灌注，应欢欢身体表面，开始有着一层层冰蓝色的坚冰浮现，坚冰逐渐的覆盖，最后淹没了她整个身体。
眼前的视线逐渐的模糊，应欢欢却是提起最后的精神，美目望向遥远的东方，那眼中充满着留恋，她的唇角，也是有着一抹微笑浮现出来。
坚冰最终覆盖了她的身躯，一具栩栩如生的冰雕，在这天空上浮现，然而却是无人停歇，那般大阵，依旧是在不知停歇的全力运转。
无数道希冀的目光望着那道冰雕，这，已经是他们最后的希望了。青阳镇后山。
一座孤山之上，林动盘坐在山巅，他望着那周围那笼罩着云雾的山脉叠嶂，心境也是逐渐的平静，旋即他转头看向远处山崖边上的绫清竹众人，微微一笑，然后双目开始缓缓的闭上。
轮回劫由内部而生，并没有任何惊天动地的声势，但却是每个轮回境巅峰强者最为恐惧的东西，因为一旦陷入轮回之中，那就将会真的无从脱身，永远的忘却本身的自己，直到肉身腐化，彻彻底底的陨落。
而同渡三重轮回劫，这天地间，却是无人敢以想象，从古至今，除了冰主之外，似乎便是再无人能够成祖。
随着他双目闭上的瞬间，他周身涌动的元力波动也是散于无形，甚至连其气息，都是在此时，彻彻底底的散去。
偶有云雾飘来，遮掩了他的身形，这一刻，他犹如许失在了天地之间，无从感知。
山崖边，众人见到林动气息消失，也是明白他已开始渡轮回劫，当即小貂袖袍一挥，便是有着一道璀璨光罩笼罩而来，将那孤峰隔绝。
“接下来就要看他自己的了”小髅看向绫清竹等人，道。
“林动哥一定会成功的！”青檀无比肯定的道，对于林动，她始终是有着盲目的信心。
绫清竹也是微微一笑，轻声道：“这次的时间恐怕不会短，我也趁此修炼一番吧。”
她望着那孤峰中若隐若现的身影，总是这样的让你一个人，这一次，让我也来帮帮你吧。
她身形一动，也是出现在另外的一座陡峭山峰之上，盘坐而下，美目微闭，一种玄奥的波动，悄然的蔓延。
“我们接下来，就安静你给的等着吧。”
小貂见状，一声轻叹，现在他们所能做的，便是等待了。
光阴流逝，岁月如梭。
时间在天地间悄然的流逝着，而随着时间的流逝，这天地间的那种压迫感越来越浓，因为越来越多的人能够感觉到，那虚无之中的毁灭身影，越来越接近。
面对着那种毁灭身影，天地间无数人人心惶惶，若不是在那乱魔海还有着一丝希冀存在的话，恐怕如今的天地，早已彻底的大乱。
越来越多的强者，开始赶向乱魔海，他们很想要知道，冰主究竟能否成功的晋入祖境。
乱魔海也是因此而变得从未有过的热闹与拥挤，特别是在那大阵所在的海域，几乎每一座岛屿之上都是布满着人海，那一道道紧张而焦灼的目光，一直都是停留在高空上。
在那庞大的大阵中央，那座冰雕，依旧没有任何的波动，她就犹如一座无底洞一般，任由那恐怖的能量源源不断的灌注，却是不见有丝毫苏醒的迹象。
这种情况，无疑是让得人有些不安，但这般时候，他们除了等待外，同样没有任何的办法。
时间一晃，便是半年时间过去。
在乱魔海逐渐的有些不安时，那平静的青阳镇后山，同样是没有太大的波澜，孤峰之上的那道身影，不知不觉，已是布满了尘埃。
小貂等人站在悬崖边，这半年时间，林动的身体起了不小的变化，在最初的时候，他的身体以一种惊人的速度变得衰老，黑色的头发也是变得灰白，一种浓浓的死．气散发出来。
这一幕看得他们心都是提到了喉咙，不过好在的是，在这种衰老达到极致时，却是开始逐渐的恢复，只是．那种恢复有些过头，原本成熟而坚毅的脸庞，变得青涩稚嫩，那副模样，伊然是当年林动年少之时。
在后面的那些时间中，林动的身体，便是不断的在衰老与年幼之间进行着一种极端奇妙的转变，而小貂他们也是能够隐隐的感觉到，在这种外貌的变化中，林动的体内，必然是在出现在翻天地覆的变化。
而这种变化的原因，必然是因为那轮回之中一场又异常的磨练。
那种磨练，看不见摸不着，但却凶险得足以让任何人心生恐惧。
时间依旧在流逝，乱魔海的冰雕以及青阳镇那不断苍老与幼稚之中变幻的身影依旧是没有丝毫苏醒的迹象。
而这一晃，又是五月时间过去。
天地间的气氛，开始彻底的紧绷，一种无法言语的气氛弥漫开来，无数人甚至是停止了任何的事情，辽阔无尽的乱魔海开始变得拥挤不堪。
无数道目光，期盼而狂热的望着天空上的冰雕，甚至有着人开始跪拜下来，祈求着奇迹的出现。
时间，已经是越来越少，因为，不知何时起，那遥远的虚无中，仿佛是有着一道巨大无比的裂缝，正在缓缓的撕裂而开。
在那裂缝之后，魔气涌动，似乎一只邪恶眼睛，漠然的看向了这片曾经被他踏足过的天地。


第1301章 成功与否
乱魔海之中，人海涌动，如今的这里，显然已经成为了这天地之间最受瞩目的地方。
无数人怀着希冀与祈祷的来到这里，然后抬头望着那天空上庞大无比的阵法，在那阵法中央，一道栩栩如生的冰雕静静盘坐，在她的周围，仿佛连虚无都是被冰冻。
没有任何人能够感觉到那冰雕之内，是否还存有气息。
同样他们也并不敢去探测，这一年来，绝望弥漫了天地，最终支撑着这片天地还留有安宁的，便是着这一具冰雕。
他们无法想象，若是当那冰雕之内，真的再无气息存在后，那该会是一种如何让人灰暗与绝望的一幕。
不过这种想法，毕竟是有些掩耳盗铃，不论是成功还是失败，最终都是会有着结果出现，因为，那种危机，已经开始临近。
辽阔的海面上，黑压压的人海弥漫到视线的尽头，而此时，他们的目光首次的并没有停留在那栩栩如生的冰雕上，而是带着一些浓浓的恐惧，看向苍穹之外。
那原本蔚蓝的天空，在此时变得昏暗了许多，甚至所有人都是能够看见，一道裂缝，仿佛正在从那虚无之中缓缓的裂开。
那裂缝之外的位面封印，也是越来越黯淡。
位面封印所在的地方，原本距这里极为的遥远，极端是轮回境的巅峰强者都是无法抵达，但眼下，他们却是真真切切的看见了位面裂缝，显然，这是因为位面封印在减弱，而那魔气，正在逐渐的渗透而来，最终，那道毁灭者，将会降临。
暗沉的天地，犹如绝望的源头，让得无数人的脸庞都是涌上了悲戚之色，莫非，这等劫难，真的就无法避免了吗？
一些目光转向那大阵中央，那里的冰雕依旧毫无动静，这都一年时间了啊…耗尽看无数强者的力量，但却并没有出现他们想象中的奇迹，莫非，真的失败了吗？
悲戚而绝望的气氛，笼罩在这天地间。
“啊？那是什么？！”
突然间，乱魔海中有着惊慌失措的尖叫声响起，无数人顺着声音猛的抬头，然后惊骇欲绝之色便是涌上了脸庞。
因为他们见到，在那苍穹之外的裂缝处，竟是在此时有着邪恶之极的魔气涌进，那些魔气，竟是穿透了黯淡的位面封印。
这些魔气极为的恐怖，一出现，甚至连天空上的烈日都是失去了光彩，再接着，那位面封印之上，开始有着一道裂纹浮现出来。
裂纹浮现，然后这天地无数人便是头皮炸了起来，因为他们见到，在那裂纹之后，仿佛是有着一只巨大的眼睛，不带丝毫情感的窥视着他们。
那只邪恶无比的巨大眼睛微微转动，所有人都是能够感觉到一种阴冷之意扫过，在那种阴冷之下，他们体内的元力仿佛都是在此时僵硬了下来。
无数人索索发抖，面庞惨白，能够拥有着这种恐怖无比实力的人，除了那位曾经被符祖大人封印的异魔皇之外，还能有着谁？
冷漠得犹如魔神俯览蝼蚁般的扫视，仿佛是在极短的时间内掠过了这片天地所有的地方，最终停留在了乱魔海上空那庞大的阵法中，准确的说，是那阵法中央的冰雕之上。
在当看见那栩栩如生的冰雕时，那只邪恶的巨眼中，终于是涌上了浓浓的波动，显然，他从那冰雕上，察觉到了一丝熟悉的波动。
“轰隆。”
苍穹之外，仿佛是有着魔雷窜动的声音，所有人都是能够察觉到那位面封印外停留在大阵冰雕上的那目光，这令得他们头皮猛的发麻起来。
“他要干什么？！”
有着人惊骇的失声出来，只见得那位面封印处，竟是有着滚滚魔气强行的涌出来，然后竟是化为一只苍白的手掌。
那只手掌，巨大无比，其上并没有太过邪恶的魔气缭绕，但却是给人一种毁灭的感觉。
苍白的手掌，硬生生的自那尚还未完全被破坏的位面封印中伸进来，然后直接洞穿虚空，对着那乱魔海上空的大阵拍了下去。
他竟然是要将那正在冲击祖境的冰主提前抹杀！
显然，他也是察觉到了冰主试图冲击祖境与他抗衡的目的！
这种情况，他绝对不会再允许出现第二次！
“他要对冰主出手！”
无数人惊恐的失声，眼睛都是在此时通红了起来，冰主冲击祖境已是这天地间最后的希望，若是在此时被打断的话，那他们就彻彻底底的失去机会。
“拦住他！”
无数强者咆哮着，虽然心中恐惧，但这在那种即将彻底绝望之下，反而是变得有些疯狂起来，只见得无数破风声响彻而起，一道道光影犹如蝗虫般略出，无数道璀璨的元力匹练呼啸过天际，对着那穿透虚空的苍白大手狠狠的轰了过去。
砰砰砰！
数以千万计的疯狂攻击，尽数的轰在那苍白大手上，然而却是未能让得他有丝毫的颤动，那只魔皇之手，根本就没有理会那些蝼蚁的攻势，直奔那大阵而去。
轰！
下方的海域，在此时被生生的撕裂而开，一道数十万丈庞大的漩涡空洞被压迫而出，方圆数十万丈内所有的海水，都是被挤压开来。
苍白大手速度极快，穿透重重阻碍，最终在那无数道绝望的目光中，落在那庞大无比的阵法之上。
咔嚓。
阵法根本就没有形成丝毫的阻碍，最外层的光罩，瞬间崩溃，那所有踏入轮回境的巅峰强者，皆是在此时一口鲜血狂喷而出，身形狼狈的倒射而出，尽数的落进下方海水之中。
“拦住他！”
生死之主六人也是在此时陡然睁开双目，一声厉喝，手印变化，旋即彼此相触，恐怖能量，席卷而出。
“祖之守护！”
浩瀚的光芒自他们体内弥漫开来，竟是化为一道古老的光影，光影将他们包裹，守护在其中。
苍白大手落到那古老的光影上，终是微微顿了顿，但那古老光影，却仅仅只是持续了瞬间，便是开始有着密密麻麻的裂纹浮现。
生死之主他们鼻息之间，都是有着血迹流出来，仅仅只是一道穿越位面封印的手掌而已，便是如此的恐怖，真不愧是连师傅都必须燃烧轮回方才能够对付的超级存在啊。
嘭！
鼻息间的血迹越来越浓，感受着那即将蹦碎的光影，生死之主他们眼中也是掠过一抹无奈之色，他们已经力竭了。
咔。
光影下一瞬间，也是爆裂而开，生死之主六人身体陡然萎靡而下，鲜血自他们七窍中被震出来，而后一口鲜血喷出，身体犹如断翅的翅膀，自天空上栽落而下。
在身形急坠时，他们见到，那苍白大手，终是重重的落在那冰雕之一切，都完了吗？
他们心中掠过这般念头，一种无力与绝望，涌上心头。
“完了吗？”
那天地间无数强者望着这一幕，也是通体冰凉下来，眼中仿佛是失去了所有的神采，心中有着一道脆声响起，那是希望彻底破裂的声音。
无数人眼神呆滞，摇摇欲坠，仿佛就欲栽倒。
咔嚓。
而就在天地间瞬间死寂时，突然有着清脆的声音响起，仿佛是坚冰破碎…
一些呆滞的目光茫然的转移而去，旋即那木然的瞳孔便是剧烈的缩起来，他们浑身颤抖着，只见得在那苍白打巨手之下，冰屑犹如冰雨般的飘落而下，而在那里，一道冰封了一年时间的倩影，再度现身，晶莹的长发，随风飘舞。
嗡。
璀璨的冰蓝光芒，在此时席卷而开，竟是生生的将那魔皇之手的落势彻底的阻拦而下，旋即冰雪涌动，那魔皇之手，直接是被震得急退。
“冰主！”
“冰主苏醒了！”
“她成功了？？！”
无数人望着这震撼的一幕，魔皇的攻击，终于是首次被阻拦下来并且被震退，乱魔海中，瞬间沸腾，无数道狂喜与激动的欢呼声如雷鸣般响彻，在大海上掀起阵阵涛浪。
而在那无数道激动得近乎颤粟般的目光中，天空之上，那道冰蓝倩影一飞冲天，一种恐怖的寒气，在天地间弥漫开来。
咔嚓。
下方的大海，瞬间冰冻，冰线以一种惊人的速度弥漫开来，方圆数十万丈内，直接被冰封，那种恐怖得无法形容的力量，却是让得无数人眼中涌上了狂喜。
“咻！”
冰蓝倩影暴掠而出，而后她似是一掌拍出，与那魔皇之手，正面硬憾在了一起。
嘭！
天地仿佛都是在那种对碰中蹦碎了开来，那魔皇之手，再度被震退，一层层坚冰，自那苍白大手之上弥漫而开，最后直接是将其彻底的冰封。
魔皇之手剧烈一颤，然后开始飞快的缩回，很快的便是退进位面封印之中，而后寒气紧接而至，将那位面封印上的裂缝封堵而上。
“竟然是祖的波动…”
“不过，这可还比不上符祖啊，千载岁月，你们这位面，你便是最强的了么？”
“一月之后，本皇将会真身降临，到时，便是你们这位面终结之日了。”
随着裂缝被冰封，一道毫无情感的漠然声音，却是自那裂缝处传出，最后回荡在了这天地之间。
无数人因为这从天外而来的声音静了下来，心中涌上浓浓的恐惧，不过当他们在看见天空上那道凌空而立的冰蓝身影时，眼神又是明亮了下来，只要有着冰主的守护，这天地，应该能够阻拦下那异魔皇了吧？
无数道目光，泛着浓浓的狂热与希冀，紧紧的望着那道倩影，一些人，甚至是忍不住的跪拜了下去，这一刻，她仿佛成了他们的神。
天空上，应欢欢望着那虚无之中的残破不堪的位面封印，脸颊之上，突然有着一抹苍白浮现出来，旋即她娇躯一颤，一口血液自其嘴中喷出，那血液，竟然是呈现冰蓝色彩。
应欢欢娇躯摇摇欲坠，然后竟是在那无数道惊骇的目光中倒飞了出去，在其娇躯上，不断的有着血雾爆炸出来。
遥远的青阳镇后山，那孤峰之上布满尘埃的身影，突然在此时陡然睁开了双目，下一霎，他周身银光暴涌，直接是凭空消失而去。
乱魔海上空，应欢欢娇躯自天空坠落而下，她微闭着美目，也没有丝毫要控制身体的想法，再接着，下一瞬，她便是感觉到自己落到了一个充满着熟悉味道的怀抱之中。
冰蓝的美目这才睁开，然后她便是见到了那张仿佛深深印入了她灵魂深处的面庞。
“你来了啊。”她望着他，轻声道，那声音，竟是异常的柔和，没有了丝毫的冰冷。
“你…失败了？”
林动抱着她，望着她嘴角的冰蓝色血迹，心头忍不住的抽搐了一下，他能够感觉到，虽然此时应欢欢的实力达到了一种恐怖的地步，但显然，还并不算晋入真正的祖境，不然的话，她不会被那异魔皇伤成这样。
应欢欢此时犹如小猫一般的蜷缩在林动的怀中，冰凉的俏脸贴着他那温暖的胸膛处，唇角却是有着一抹妩媚的笑容浮现出来，她纤细的玉臂轻轻的揽着林动的脖子，然后优雅的仰起那雪白的脖颈，柔软的唇，印在了林动唇上，冰凉的水花，顺着俏脸流淌下来。
“不…我成功了。”


第1302章 祈愿
“成功了？”
感受着那印上嘴唇的柔软与冰凉，林动却是微微一愣，然后他望着应欢欢那带着笑颜，但却流着泪花的美丽脸颊，一时间心尖都是颤了颤。
“什么意思？”
应欢欢自林动身上落下来，玉手轻轻抚着他的脸庞，喃喃道：“你真的渡过三重轮回劫了呢…对不起，竟然都没在你身边。”
她轻轻退开两步，而后那晶莹的美眸望向下方那无数期盼的将她给看着的人海，声音幽幽的传荡开来：“现在的我，并没有真正的抵达祖境，充其量只能算是半祖之境，这与真正的祖境，依旧是有着极为遥远的距离。”
乱魔海中，逐渐的安静下来，无数人听得这话有些失神起来，半祖？这是什么意思？
“凭我现在的实力，依旧不可能是异魔皇的对手，等到一月后他真身彻底降临这世间时，我将再也无法阻拦他的脚步。”
无数人的面庞，逐渐的惨白，眼中那幸存的希望火苗，正在一点点的湮灭，他们最后的希冀，也是在此时破碎了吗？
冰主并没有如同他们意料之中的晋入祖境…而是半祖之境，虽是一字之差，但其中，却是天与地的差距。
“真的…末日到了吗？”
无数人喃喃自语，深深的恐惧与绝望，从内心深处，攀爬而出。
生死之主他们也是抹去嘴角的血迹望着天空，心中一声暗叹。
“没到祖境也没关系，我们一起，再联合其他的力量，未必不能与异魔皇一战！”林动望着那娇躯微微颤抖的应欢欢那无数人的期盼，犹如重重山岳般的压在她那柔嫩的肩上，那种拯救天地的压力与责任，足以让任何人都是喘不过气来。
应欢欢望着林动，却是轻轻一笑，道：“其实我早便是知道是这个结果的，即便是集合了这些力量也根本不可能让任何人都踏入祖境，而且那种强行提升，还有着极大的后遗症现在的我，恐怕再也无法晋入祖境。”
“想要达到真正的祖境，哪有这么容易啊，不过，这都在意料中呢，所以其实我还是成功了的。”
林动身体猛的一颤呆呆的望着应欢欢。
“正因为我知道这个结果，所以，我否决了你想要代替我的心，对不起，我不是要否决你的努力…你所做的，我都知道的…”
应欢欢玉手捂着嘴唇声音竟是变得哽咽了起来，泪花从其眼中流淌出来，那望着林动的目光中，满是柔情。
之前的冰封，在此时彻底的荡然无存。
天地间，无数人都是怔怔的望着天空上那捂着嘴留着泪的女孩，这时候的她，似乎不再是那个他们聚集了所有期盼的救世主，而是一个柔弱而可怜的普通女孩。
“你究竟…想要做什么啊？”林动望着那娇躯微微颤抖的女孩喃喃道。
应欢欢搽拭去脸颊上的泪花，望着林动，脸颊上绽放出一抹极为动人的笑容：“我想让你成为第二位符祖啊。
林动一惊，下方无数强者也是惊异无比的望着应欢欢，她这话，是什么意思？
“给我一些时间，我会到那一步的！”林动紧紧的盯着应欢欢，心中有着不安涌出来。
“可是…我们并没有时间了。”
应欢欢扬起俏脸，那虚无处，位面封印正在迅速的黯淡，上面被她封堵的冰层，也是在以一种惊人的速度融化着，显然，那异魔皇正在全力攻破封印。
“对不起…一直都在骗你，我让你进祖宫阙凝炼神宫，还逼你连渡三重轮回劫…我真的是一个很令你讨厌的人呢…”应欢欢脸颊上的笑容，显得凄婉之极，她指甲深深的掐入掌心，冰蓝色的血液犹如冰珠般的不断滴落下来。
“师傅所说的我的力量，其实并不是说我能够达到祖境，而是，我拥有着助人达到祖境的力量，这片天地，其实还有救的，当然，那前提是我要达到半祖之境。”
林动咬着牙，他紧紧的盯着应欢欢，我不想当什么第二位符祖，我是很自私的人，所以我也不想做什么以拯救天地为己任的事，我只想与我所在意的人在一起，即便最后是一切灭亡，那也至少无怨无悔！
应欢欢看着林动，仿佛是看穿了他心中的奔涌，她轻咬着嘴唇，哽咽的道：“可是…我只想要你活着啊。”
我只想要你活着。
林动如遭雷击，依稀是熟悉的一句话，在那多年之前的异魔域，那般绝境中，少女也是这般的红着眼眶对他说着，简简单单的要求，却是让得林动有着被撕碎心的剧痛之感。
“你究竟，想要做什么啊？！”
林动颤抖着，再度重复那一句话，他盯着应欢欢，喃喃道：“你就不能听我一次话吗？！”
“我真的不想这样啊…可是真的还有其他任何的办法吗？其实，从一开始，这一切都是注定了的，我根本就改变不了什么…”
应欢欢美目通红，水花在其眼中凝聚着。
“对不起…我只要你好好的活着。”
应欢欢缓缓的退后，她轻轻的摇着头，在她退后间，她那一头晶莹的长发竟然开始逐渐的变得乌黑，那冰蓝的美目也是再度的恢复了很久以前的漆黑灵动，那一霎那，那曾经扎着乌黑马尾，巧笑焉熙，娇俏活泼的少女，仿佛又是出现了。
下方的生死之主他们见到应欢欢这般变化，面色却是猛的剧变起来。
林动也是察觉到不对身形一动，直接对着应欢欢暴掠而去。
嗤！
不过就在其身形刚刚冲出时，周围空间瞬间被冻结，寒冰形成蔓藤，缠绕在林动身体上而后冰雪在其脚下凝聚，竟是化为一朵巨大无比的冰莲。
那种力量，即便是如今林动凝聚神宫，渡过三重轮回劫都是有些难以抗拒，虽然他的修炼速度放眼今古都是足以排上前三，但他修炼的时间，毕竟太短这是他最大的弱处，若是能够再给予他一些时间，他相信他必定会触摸到祖境！
可是，他没有时间了！
“应欢欢！你再敢乱来，我绝不会放过你！”林动眼睛通红，咆哮道。
应欢欢冲着林动轻轻一笑，笑容凄婉得犹如那冰山上即将消逝的雪莲花，旋即她双眼缓缓的闭上。
在那很遥远的时候。
老人从一座万年冰山中抱出来了一个小小的女婴。
女婴逐渐的长大，变成了一个扎着小马尾的小女孩。
“冰儿，你拥有着很强大的力量，或许在很久以后，师傅不在了，那时候的天地需要你来守护。”老人微笑的看着身旁粉雕玉琢般的小女孩，声音温和。
“师傅为什么会不在啊？”小女孩嗓音稚嫩，乌黑的大眼睛中，满是天真与不解。
老人笑着，继续说道：“只不过那种力量，需要你完全心甘情愿的释放，而且你会付出极大的代价，那种代价，或许会是你的生命如果在那个时候，你能够寻找到一个让你心甘情愿如此付出的人，那么，就请你拯救一下这天地生灵吧。”
小女孩似懂非懂的眨了眨大眼睛，道：“那如果没有找到呢？干嘛要为别人付出自己的生命啊，我不喜欢。
“没有找到的话，那就是这天地注定的劫难，师傅与你的约定，也做不得数。”
“哦…”
小女孩抱着冰块啃了一口，乌黑的小马尾甩了甩，为了别人付出生命？虽然她还年幼，但那深藏在内心最深处的冰冷却是让得她认为这种事情不可能会出现的。
“师傅…最终，我还是找到了那个能让我心甘情愿去付出所有的人呢…冰儿也很开心呢…”
遥远的记忆，自尘封处涌出来，应欢欢在心中喃喃自语，旋即她俏脸上的笑容，开始变得温暖，她纤细玉手轻轻相合，似是形成了一个古老无比的印法。
“吾以吾之灵祈愿…”
“以吾之身…”
“以吾之魂…”
“以吾之血…”
空灵的声音，仿佛是伴随着古老歌谣的响起，悠悠的响遍在这天地之间的任何角落，四大玄域，乱魔海，妖域，无数人都是抬起头，仿佛有所感应的望向那个方向，一种莫名的震撼，从心灵深处涌出来。
“号天地之灵，神化，祖之路！”
当那最后一字古老音节落下时，应欢欢娇躯突然剧烈一颤，而后这天地开始颤抖，天空呈现绚丽色彩，仿佛有着无数道灵光自天地之中涌出来，最后在乱魔海上空，化为数百万丈庞大的绚丽光幕。
灵光呼啸，最后尽数的灌注进入林动脚下的巨大冰莲之中，而后冰莲开始变得绚丽…
然而林动却并没有时间理会冰莲的变化，他惊骇欲绝的望着应欢欢，因为此时的后者身体上，竟是有着冰蓝色的火焰升腾起来。
此时此刻，他终于是明白应欢欢想要做什么，她是在燃烧自己，催动她最为强大的力量来助他成祖，只不过，这种代价，显然将会是她的生命！
他也终于是明白，为什么在西玄域，应欢欢会那般冷漠的拒绝他…因为，那根本就不是什么冲击祖境的阵法，而是为了让她拥有着催动这种力量的计划！
她从一开始，就想到了这一步！
“停下来！”
“你给我停下来啊！”
“应欢欢！”
无数血丝，纷纷的缠绕上了林动的眼球，他疯狂的挣扎着，愤怒的咆哮着，那嘶吼的声音，犹如野兽一般响彻在这天地间。
下方炎主他们见状，面色剧变，也是要急忙冲上去，但却是被生死之主一把拉住，她眼睛通红，喃喃道：“这是她的选择，不要去干扰她了。”
“为什么会这样？”炎主他们面色苍白，喃喃道。
“那我们…还有其他的办法吗？祖境，不是那么容易达到的，即便是小师妹，也只能使用这般办法，才能让林动晋入祖境，也只有这样，才能避免这次的劫难。”生死之主抹去眼泪，道。
炎主他们默然无语，他们望着天空，这一刻，绕是他们那般经历的心性，都是忍不住的涨红了眼睛。
冰蓝色的火焰升腾着，应欢欢望着那状若疯狂般的林动，泪花凝聚着，然后滚落下来。
“对不起…我并不想守护什么天地，也不想做什么救世主，可是，我想要你活着…”
“林动，谢谢你在我未曾觉醒之前就让我喜欢上你…也谢谢你给予我的这么多美好，你让我知道，再冰冷的心，都会有绽放开花的时候。
“你曾经问我是冰主还是应欢欢…”
“现在我能告诉你…傻瓜，哪有什么冰主，我一直…都是应欢欢啊。”
冰蓝色的火焰，袅袅而上，最终包裹了应欢欢整个身体，而她那带着一些哽咽的声音，也是在此时传荡开来。
“啊！啊！啊！”
林动凄厉长啸，啸声远远的传开，那之中所蕴含的撕心裂肺的痛苦与无助，让得无数人眼睛瞬间红了起来。
咻。
巨大而绚丽的冰莲，在此时爆发出万丈霞光，而后冰莲花瓣开始缓缓的合拢，而林动的视线，也是伴随着冰莲的合拢开始变得模糊，意识，开始黑暗。
在那视线即将彻底模糊间，似是有着一道身影，浮现而出。
她背着双手，乌黑而修长的马尾，跳动着活泼动人的弧度，那张俏脸之上，布满着狡黠而娇蛮的笑容，一如多年之前，在那道宗，首次相遇。


第1303章 轮回
黑暗伴随着撕心裂肺的痛苦，湮没了林动的神智，他的意识似乎是陷入了无尽的黑暗之中，隐约的，仿佛是还有着犹如野兽般痛苦的嘶嚎声自那黑暗之中传出。
那般嘶嚎，不知道持续了多久，终于是开始逐渐的减弱，那道声音的主人，仿佛是力竭而去。
不知何时，绚丽的光彩自黑暗中爆发出来，将那沉沦在其中的意识包裹，光彩犹如无数的画面，闪烁而过，那每一个画面，仿佛都是一个轮回。
林动的意识，被这些轮回漩涡强行扯进去，再然后，他的意识彻彻底底的失去，那种感觉，比起他渡三重轮回劫时还要可怕。
轮回转动，那道意识最终也是无可自拔的陷入了其中，他的记忆犹如被剥夺，一种又一种的新生记忆，将他所占据。
一世轮回，他依旧是青阳镇的那个林动，他依旧是在为了能够给父亲讨回公道而努力的修炼着，只不过这一次，他再没有了什么祖石，而只是一个林家中平凡而执着的少年。
他努力的修炼着，试图向那林氏宗族之中最为耀眼的天才发动复仇。
然而这一次，他没有了曾经的隐忍，虽然经过努力，他已经让得林家成为了青阳镇最为强大的家族，但最终，却是因为对林琅天仇恨的泄露，彻底的引来了杀机。
血与火，弥漫了林家。
身着林氏宗族服饰的内族之人，面色冷漠的将那长剑刺进了柳妍的身体之中，鲜血喷洒，她却是竭力的对着不远处呆呆的望着这一幕灾难的青年凄厉的叫喊着：“动儿，快逃！”
他的心中，涌上浓浓的恐惧，而后他便是见到，一道身体修长，面容俊逸的男子缓步而来，在他手中的长剑上，还有着他的亲人鲜血滴落下来。
“你就是那个要对我复仇的林动？”那俊逸男子站在林动的面前，脸庞上，有着一抹淡淡的嘲讽浮现出来，那种目光，犹如俯视着蝼蚁一般。
“我要杀了你！”
林动眼睛血红，刻骨的仇恨涌上心间，而后他咆哮着冲向林琅天，但却只是见到后者嘴角那冷漠而讥讽的笑容。
“卑贱的分家之人，真是连上下都分不清楚，留在这世间，也是玷污我林氏宗族的名声。”
讥讽的声音自林动的耳边响起，旋即锋利的剑芒闪掠而来，毫不犹豫的洞穿了他的脖子，鲜血喷洒间，他开始无力的倒地，在那血泊中，他见到林啸，林震天他们，都是跪倒在不远处，而后被那锋利剑锋，自脖间劈砍而过。
一颗颗人头掉落下来，那睁大的眼睛中，满是不甘的绝望之色。
他的视线，开始黑暗，最终带着无尽的悔恨，消散而去。
有一世，没有了所谓与林琅天之间的仇恨，他天赋过人，最终凭借着自己的努力，让得林家重回宗族，而且最后，他也是成为了林氏宗族中最为耀眼的人。
后来，在他掌管之下，林氏宗族成为了大炎王朝最为强大的家族，而他，也是成为了大炎王朝的第一强者。
只是，这一世，没有应欢欢，也没有绫清竹。
最后，随着寿命的极限，他在那林氏宗族无数族人悲伤而敬畏的目光中，躺进了棺木之中，只是在视线黑暗的那一霎，他隐隐的感觉到，自己似乎是失去了什么最为重要的东西。
轮回，一世接一世，犹如永无止境，林动的意识，陷入那种轮回之中，再也找不到真正的自我。
他荣耀过，卑贱过，受人敬畏过，也遭人耻笑过，人生百态，尽受无疑。
他就犹如行尸走肉一般的渡过一世又一世，唯有每当在生命走到尽头时，他方才能够感觉到，他似乎是依旧没有找到什么，同样的，他也并没有找回自我。
后来又一世，他遇见了绫清竹，不过那仅仅只是惊鸿一瞥，两人之间，并没有发生过石墓那香艳的荒唐，后者依旧是高高在上犹如谪仙般的仙女，而他，却是无数仰望着她的人之一。
那一世，他异常的平凡，一事无成，最终郁郁而终。
轮回在转动，已是不知道经历了多少轮回，但那意识，却是在轮回之中越来越浑浊，仿佛将会永久的沉沦下去。
一世又一世。
这一世，他又成为了道宗的弟子，然后，在那里，他再度见到了一道活泼而俏丽的倩影，那乌黑的马尾轻轻的跳动着，仿佛能够为人心中增添无数的活力。
她依旧是道宗中的小公主，而他略显普通，只是在那重重人群中，两人对视，仿佛都是微微颤了一下，一种莫名的情绪，充斥了他的心中。
他喜欢上了她。
于是他开始奋力的修炼，他开始脱离平凡，从那道宗弟子之中脱颖而出，伴随着他在道宗弟子中的呼声越来越高，那道悄然注视着他的俏目，也是愈发的明亮。
他们最后成为了道宗之中最令人艳羡的两道身影。
他们一起修炼，一起执行任务，生死之中，情意涌动。
道宗的后山，漫山遍野的鲜艳花朵，风一吹来，幽香顿时弥漫了天林动盘坐在那花海中，望着前方，那里，身段窈窕柔软的少女，正轻灵而舞，漫天鲜花伴随着她的腰肢的扭动，汇聚在她的周身，少女那清脆如银铃般的咯咯笑声，犹如天地间最为动听的音律。
他目光柔软的望着少女，那一霎那的心灵最深处仿佛是有着一种复杂得连他都无法分辩的情绪涌了出来，那种情绪深处，似乎是有着一种撕心的痛苦。
不知不觉，他红了一些眼睛。
“喂，你怎么啦？”
清脆的声音在耳边响起少女漂亮的大眼睛疑惑的看着他，乌黑的马尾在阳光的照耀下，闪烁着一些动人的光泽。
他望着那张容颜，突然伸手抓住她的柔荑，他似是沉默了许久，最后，喃喃的道：“嫁给我吧。”
当他这句话说出来的时候他能够感觉到，一种特殊的情感，仿佛是穿越了无尽轮回重重的击中了他的心脏。
要给她幸福。
那种情感，似乎是在这样的说着。
少女也是被他突然的话吓了一跳，旋即那俏美顿时变得绯红下来，大眼睛中弥漫着娇羞之色，旋即她轻轻的点头。
整个道宗，弥漫在了喜庆之中。
作为道宗最为优秀的弟子他与掌教之女相合，显然是众望所归的事。
在那红烛遍布的新房之中，他轻轻挑起那鲜艳的头帘，他望着红帘之下那张娇羞动人的俏脸，眼睛却是不由自主的再度通红了起来。
然后，他在新娘那疑惑而羞涩的目光中低下头，将那一抹柔软，重重的含入嘴中。
那一夜，有着春光涌动，只是少女那带着一丝痛楚的轻哼声中，却是包含着无尽的幸福。
大婚之后，两人更是形影不离，那般不舍不弃的情感，让得不少人羡慕不已人世间，能够彼此寻找到所珍惜的人，那的确是一件幸福得让人不愿苏醒的美好事情。
只不过，应欢欢却是觉得，在那大婚之后，林动突然有时候会变得沉默许多，他呆呆的坐在山崖上，望着到宗内众多弟子的修炼，那眼神似是有些茫然。
不过这种茫然的眼神，每当转移到应欢欢身上时，便是会化为一种温暖，只是，那温暖深处，仿佛隐藏着什么不敢言起的情绪。
“你是不是有什么瞒着我？”她最终忍不住的问道。
不过面对着她的发问，林动却是微微一笑，将她轻轻的揽进怀中，那种柔软让得她身心都是化了开去，再也不记得质问的缘由。
“我会让你永远都开开心心的。”林动将脸埋在她乌黑而幽香的长发中，心中仿佛是有着喃喃的声音响起。
时间，一年又一年的过去，不知不觉，已是大婚后的三年。
在那道宗山崖上，应欢欢那洁白修长的双腿在崖外轻轻的摆动着，然后她微偏着头，望着一旁的那望着道宗内的青年，后者的身影，愈发的沉稳。
她望着他，抿嘴一笑，有些妩媚少妇的动人风采。
“爹爹说按照你的修炼进度，恐怕两年后就有资格继承他的位置了呢…到时候，我是不是也要叫你林大掌教？”她俏皮的笑着道。
“那你就是林大夫人。”林动笑着伸出手指弹了弹她光洁的额间，眼中满是宠溺。
应欢欢笑吟吟的望着他，突然轻叹了一口气，道：“你是不是有什么话要对我说？”
“嗯？”
“你不觉得吗？自从我们大婚后，你似乎变了一些，不是说变得不好，只是对我太好了…而且那种好，让我感觉到你似乎是在补欠着什么。”应欢欢微微有些低落的道。
“我只想让你知道，你并不欠我任何东西，我爱你…胜过我爱我自己。”应欢欢轻咬着红唇，轻声道。
林动脸庞上的笑容仿佛是在此时逐渐的僵硬，他轻轻的抚着应欢欢的脸颊，喃喃道：“为什么你总是这么傻啊…”
“那你要告诉我吗？”应欢欢轻声道。
林动沉默着，他望着遥远的地方，漆黑的双目中似乎是有着极端复杂的情感涌出来，许久后，他轻声道：“愿意听我讲一个故事吗？”
“嗯。”应欢欢点着小脑袋。
林动笑着，笑容苦涩，然后他开始讲起一个故事，在那个故事里，也有着一个叫做林动的人，同样的，还有着一个叫做应欢欢的女孩，而且，那个女孩还拥有着所谓远古八主之一的冰主转世的身份在那里，还有着可怕的异魔…
在那里，他们聚少分多，但那种感情，同样真挚而且，他们最终也没有如同这里那般结为夫妻。
他的声音，微微的有些低沉，其中仿佛是有着无尽的悲伤在涌动。
应欢欢望着此时的林动，不知不觉，通红了眼眶，特别是当她在听见那个应欢欢最后燃烧自己将他送入祖之路时，晶莹的水花已是顺着脸颊流淌了下来。
“她总是这么不听你的话，你一定很恼她的吧？”
欢欢红着眼睛，道。
“是啊…若是能够最后真的一起灭亡，其实也是一种幸福，总好过将那种悲伤留给活着的人独自去承受要好啊，那样…真的很难受很痛苦的啊。”林动轻声道。
“可是有些东西，终归是无法避免的，你要承受这份悲伤′但她也要承受欺骗挚爱之人的痛苦。”
“是啊，我没资格恼她的…”林动突然微微一愣，笑道：“那只是一个故事罢了。”
应欢欢却并没有回答，那大眼睛只是静静的看着他，泪水止不住的流下来：“其实…我们便是那祖之路中的轮回吧？”
“这些…都是假的吧？”
林动望着她，然后拉起她的小手放在自己心脏处，道：“真的假的，你难道感觉不到吗？有些东西，即便是千般轮回，依旧无法改变。”
“而且，如果这真的是轮回的话，那我宁愿为之沉沦。”
“我现在，只想陪着你，好吗？”
应欢欢轻轻的搽拭着脸颊上的泪花，又哭又笑：“我突然很嫉妒那个我了，怎么办？”
“我知道你这是想要补偿我，不过，这不是我所想要的，虽然我知道你对我的情感同样的真实。”应欢欢轻轻一笑：“因为，我也是她，没有你心中的所想，这一切，都不会出现。”
“而且，她能够为了你燃烧轮回，你认为，我可能会因此让你沉沦在这般轮回之中吗？”
林动望着她，怔怔无言，即便是轮回之中，她的性子，依旧是没有丝毫的变化。
“我为你弹琴好吗？她最后没有做的事，我来帮她做。”
应欢欢松开林动的手掌，玉手一挥，便是有着翠绿色的古筝闪现出来，她对着林动微微一笑，脸颊上的泪水闪烁着光泽。
纤细的素手缓缓落下，修长的玉指在琴弦之上游走，悲伤的琴声，悠扬的飘扬。
就像是那多年前首次相遇，跳动的乌黑马尾，像是那曾经银铃般清脆的娇笑声，像是那燃烧轮回最后时候的温柔笑容…
林动的眼睛，在此时彻底的通红下来，巨大的酸楚冲击着他的眼睛，令得他视线模糊，他身体微微的颤抖着，脑海深处，无数次的轮回爆炸开来，沉沦的意识，开始在此时彻彻底底的苏醒。
那漆黑的双眸，由茫然变得深邃与沧桑，最终凝聚在面前那女孩身上，再然后，泪水流淌了出来。
一如在那轮回之前，火焰之中所燃烧的那流着眼泪，但却面带着温柔笑容的一幕。
“啊！”
他仰着头，撕心裂肺般的痛苦咆哮声远远的传荡开来，回荡在这天地之间，其中所蕴含的悲伤，让得天地都是变得暗沉了下来。
“啊！”
“啊！”
“为什么！为什么你总是不肯听我的话啊！”
眼泪疯狂的流着，他扑了过去，紧紧的将应欢欢搂在怀中，再也压抑不住心中的情感，犹如无助的孩子一般，嚎啕大哭。
应欢欢抱着他的脑袋，将冰凉湿润的下巴抵在他的头上，眼泪滴答答的落下来，哽咽的道：“那个我付出了那么大的代价，你怎么能在这里沉沦，这种补偿，不是我们想要看见的。”
林动眼睛模糊，喃喃的道：“让我陪你渡过这一世轮回吧。”
“那这样你就又将会陷入那无尽的轮回之中。”应欢欢纤细指尖轻轻的触着他的心脏，道：“其实这一切，都是真实的，因为它发生在你的内心最深处，如果不是你所想，就不会出现…我，也一直在这里。”
“这一切，都够了。”
“所以…”
应欢欢大眼睛看着林动，然后那柔软的唇吻上他的嘴，水花顺着脸颊流淌下来。
“林动，请苏醒吧。”
林动紧紧的搂住怀中的人儿，那股大力，仿佛是要将她揉进身体之中一般，而后深深的吻了下去。
山崖上，男女相拥，轻风拂来，仿佛还伴随着悠扬的琴音，与那漫山遍野的鲜艳花草，轻轻摇摆。
万丈霞光，突然自林动体内暴射出来，然后天地开始模糊，怀中的人儿，也是越来越淡，林动虽然双臂紧紧的搂住她，但依旧无法阻止她身形的变淡。
“谢谢你，这一世，我很开心，她应该，也能感受到的。”
倩影越来越淡，只是那俏脸之上的笑容，却是无尽的留恋与幸福，而后，终是散去。
天地再度黑暗，仿佛归于了混沌。
林动则是沉寂的跪在那黑暗之中，许久许久后，终于是缓缓的抬起头，黑色的眸子有着清明再度回归，再然后，他猛的起身，一种无可撼动的坚毅涌了出来。
我走遍轮回，只为与你相遇。
不管如何，不管将会付出多大的代价，即便是上穷碧落下黄泉，我都要把你找回来！


第1304章 封印破碎
乱魔海上空，天地绚丽，那种斑斓的颜色，已经笼罩了乱魔海整整一月时间。
在那种绚丽斑斓下，连日月仿佛都是失去了光彩，光芒流转间，有着一种极端神奇的波动在暗中的涌动。
乱魔海之中，依旧是人海涌动，而他们的目光，几乎全部都是凝聚在天空之上，那里，一朵巨大无比的绚丽冰莲，紧紧闭拢，万般色彩从中散发出来，美丽至极。
所有的目光，都是紧张的盯着那绚丽冰莲，随着一月之前冰主燃烧轮回，倾尽所有的力量要助林动成主时，他就已经成为了这天地无数生灵最后的希望。
而当他们在紧张的注视着那绚丽冰莲时，那遥遥的天空之上，黑暗仿佛压顶而来，一道长达十万丈庞大的裂缝，正在自虚无中撕裂开来，浓郁到极致的邪恶之气，源源不断的自那其中涌出来。
在那裂缝之前，还有着一道古老的阵法，只不过如今，那阵法已经是极为的黯淡，显然濒临破碎的边缘，甚至，一些巅峰强者，已经是能够隐隐的感觉到，那道毁灭般的身影，将要降临。
无数人悄悄的对视，眼中有着掩饰不住的恐惧。
“异魔皇要降临了。”
生死之主也是抬起头，小脸凝重的望着遥远虚空处的异变，缓缓的道。
“可是林动还没出来…”炎主看了一眼那紧闭的绚丽冰莲，这一月之中，这冰莲除了不断的释放着绚丽光芒外，便是没有了任何的异动，他们甚至是感觉不到林动气息的存在。
“管不了这些了，小师妹燃烧轮回方才创造出来的机会，我们绝不能将其白费！”生死之主眼中凌厉之色一闪而过，果断的道：“一旦异魔皇降临，我们便拼尽全力出手，能够阻拦一时是一时！”
炎主几人也是面色凝重的点了点头，不管如何，他们必须拼尽全力为林动争取那紧迫的时间…
轰隆！
遥远的虚无处，仿佛开始有着低沉的魔雷声传出，那雷鸣传荡间，邪恶的波动，开始真正的侵入了这片天地之间。
无数道目光紧紧的望着天空，然后他们开始索索发抖起来，因为他们看见，那位面裂缝之外的位面裂缝，竟然在此时开始燃烧起来。
黑色的火焰，粘附在封印之上，封印仿佛都是在此时爆发出刺耳的声音，这片位面的最后守护之门，终于是在此时将它的使命，尽到了最咔嚓。
封印崩裂开来，最后终于是彻彻底底的消散而去，而随着封印的散去，只见得那十万丈庞大的位面裂缝，竟是缓缓的蠕动起来，那番模样，犹如恶魔的巨嘴，开始噬咬着这片天地。
滚滚魔气，粘稠得犹如实质一般，自那位面裂缝之后涌来，这一霎，天地日月，皆是为之失色。
轰！
魔气弥漫天地，其中仿佛是有着一道邪恶到极点的眼睛，带着漠然的扫视着这片天地，他仿佛是在寻找着一月之前曾经将他击退的那道倩影。
而扫视的结果，却是令得他有些讶异，因为他发现，那道气息，竟然仿佛是彻彻底底的消失在了这天地之间，即便是他，都是无法察觉。
扫视着天地的目光，突然看向了乱魔海的方向，在那看不见尽头的人海中央的天空上，绚丽的光彩弥漫着，那种有些熟悉的光芒，顿时令得那邪恶的眼睛微微一缩。
那是一种极为熟悉而危险的波动！
“哼！”
弥漫的魔气中，似是有着一道冷漠的哼声传出，旋即乱魔海那无数强者便是惊骇的见到，一道极为粘稠的魔气光柱，猛的自那虚无之外暴掠而下，最后撕裂虚空，狠狠的对着那七彩光芒中央处的绚丽冰莲暴射而去。
“动手！”
生死之主六人见状，面色顿时剧变，一声厉喝，六人齐齐掠出，然后悬浮在那冰莲上空。
噗嗤！
他们猛的一拍胸膛，一口精血喷射而出，面色苍白间，只见得六团精血陡然的融合在一起，璀璨的光芒伴随着恐怖的波动，席卷而开。
光芒散开，化为一座古老的巨钟，巨钟垂下，将他们以及下方那绚丽的冰莲，尽数的笼罩。
咚！
魔光洞穿空间而至，直接重重的轰击在那巨钟之上，而后清脆的钟吟之声，自天地间响彻而开，一圈肉眼可见的音波弥漫开来，周遭那些强者也是被震得狼狈射出。
咔嚓。
凝聚了六主全力的巨钟防御，却并没有取到太过明显的效果，一道道裂纹迅速的自巨钟之上蔓延开来，最后终是砰的一声巨响，彻彻底底的炸裂。
噗！
巨钟炸裂，六道身影倒飞而出，一口鲜血喷出来，本就苍白的面色更加的惨白，气息也是显得萎靡了许多，他们虽然在这天地间已经算是最为顶尖的强者，但要与那异魔皇相比，依旧是有着太过庞大的差距。
不过虽然他们受创极重，但终归还是将那异魔皇这道攻击阻拦了下来，遥远的虚无中，邪恶的眼中，仿佛是掠过冰冷之色，而后魔气再度奔涌。
魔气滚滚而来，所有人都是能够感觉到，那道跨过位面裂缝的毁灭身影，正在迅速的接近着他们。
咻！
一道粘稠魔气再度射出，再度化为一根巨大魔指，按爆虚空，对着那绚丽冰莲狠狠按去，在那魔指之尖，有着邪恶的纹路在闪烁。
“我们一起出手！”
乱魔海中，无数强者见到那异魔皇攻势再临，恐惧到极点之后，竟反而是显得愤怒起来，那冰莲之中的林动，已经是最后的希望，若是在这里被异魔皇阻拦或者破坏的话，那他们，也将会失去最后的反抗之力。
那一幕，绝对不能出现！
无数强者眼中有着猩红涌出来，而后一道道咆哮声响彻而起，只见得数以千万计的元力匹练呼啸而出，整片乱魔海，都是在此时剧烈的震动起来。
砰砰砰！
无数道攻击，疯狂的轰击在那魔指之上，后者也是剧烈的颤抖着，不过那遥远之外正在急速赶来的异魔皇显然也是察觉到这种情况，当即一声冷哼，魔指之上，邪恶的纹路突然爆发开来，竟是生生的将那些攻势尽数的抵挡了下来，而后去势依旧不减，狠狠的轰向那绚丽冰莲。
“燃烧轮回，阻止他！”
生死之主见状，眼中也是红了起来，一声厉喝，双手陡然结印，竟然便是打算直接燃烧轮回。
这般时候，他们已经被逼得无路可走！
咻！
然而，就在他们准备彻彻底底的拼死一搏时，那冰莲上空的空间突然扭曲，旋即一道白衣倩影，凭空闪现出来。
那道倩影，气质清冷，绝美的脸颊此时也是笼罩着寒霜，她望着那轰来的魔指，玉手一握，一柄三尺青锋长剑便是闪现出来。
她手持长剑，猛然劈下！
一剑落下，并没有任何元力的波动，那一剑，仿佛是没有蕴含丝毫的力量。
咔嚓！
然而，就在这般毫无力量的一剑，却是直接自那恐怖魔指之上竖劈而过，然后无数人便是惊骇的见到，那道魔指，竟然是在此时被硬生生的斩成了两截。
哗！
整个乱魔海都是暴动了一些，无数人目瞪口呆的望着那白裙如仙般的绝美女子，显然是不明白这突然间冒出来，仿佛比远古之主还要强横的她，究竟是何方神圣？
“绫清竹？！”
生死之主他们也是有些震动的望着这一幕，旋即惊声道。
“她力量怎么变得这般强大了？”空间之主震惊的道，他能够感觉到，绫清竹的体内，似乎并没有任何元力的存在，但从她的身上，他却是察觉到了一种熟悉而古老的波动。
“是太上的力量！”
生死之主面色凝重的道，她盯着那道倩影，喃喃道：“她竟然能够将太上的力量运用到这般地步？真是可怕啊，若是要光论对太上力量的了解与掌控，或许连小师妹都不及她…”
“她放弃了所有的力量，专注的追逐太上之力，这般心性以及大毅力，的确是我们所不及。”炎主也是点了点头，道。
“林动这小子，还真是厉害，这绫清竹与小师妹，谁不是这天地最惊才绝艳的女子，竟然都喜欢这的家伙，厉害啊厉害。”空间之主咂了咂嘴，语气之中不无敬佩，只不过他的话，最终只是换来了几个白眼。
“不过绫清竹，恐怕也阻拦不住那异魔皇的，现在异魔皇已经穿越过位面裂缝，很快就能彻底降临，到时候…他的恐怖实力，方才会真正的释放出来。”生死之主有些担忧的道。
炎主他们也是一叹，然后看向那绚丽的冰莲，现在，就只能期盼林动能够在这所剩不多的时间中，破封而出，并且真正的晋入祖境吧，不然的话，这天地，便是将会彻底的落入那异魔皇的魔爪之中。
魔指被一剑劈开，那遥远的虚无中，那邪恶的眼睛中，突然暴射出惊人的光芒，他仿佛穿透了空间，看见了那道白色的绝美倩影，那原本毫无情感的漠然声音中，竟然是在此时，多出了一丝微微的震动以及…狂喜？
“这种力量…终于又出现了吗？”


第1305章 晋入祖境
乱魔海之上，一道绝美的白裙倩影凌空而立，她手持三尺青锋，三千青丝垂落自那纤细腰肢处，那张完美得没有丝毫瑕疵般的容颜，正带着许些清冷以及罕有的凌厉，紧紧的望着那遥远的虚无处，那里，一种极端可怕的威压，正在急速的接近。
那异魔皇，要来了。
绫清竹玉手轻轻紧握着青锋长剑，红唇微抿，旋即她低头看了一眼下方那无数道惊惶而绝望的目光，接着又是转向那绚丽的冰莲，清冷的眸子中，有着一抹柔软浮现出来。
“以前，总是让你追逐着我，这一次，就让我来保护你吧。”
握着长剑的玉手再度用力，细微的青筋纹路自那白玉般的肌肤上凸显出来，绫清竹那绝美的脸颊上，也是掠过一抹决然之色。
无数道目光望着那道凌空而立的倩影，微风吹拂而来，长发飘飘，那种清冷脱俗的气质，犹如谪仙一般，令人为之沉迷。
轰隆。
低沉的魔雷之声，越来越近，只见得那九天之上，魔气犹如乌云般的压顶而来，最终在那一道道恐惧的目光中，弥漫了乱魔海上空。
而在那滚滚魔气深处，似乎是有着邪恶而冷漠的目光望向了这片天地，最终，那目光，停留在了绫清竹窈窕的娇躯之上。
“真是令人熟悉的力量啊…”
魔云深处，一道漠然的声音缓缓的传出，那声音犹如九幽之中的魔神，没有丝毫的情感，但却是有着一种令天地颤粟的恐怖力量弥漫出来。
“异魔皇…”
生死之主他们面色苍白的望着那笼罩天空的魔云，那里隐隐有着一道魔气王座出现，在那王座之上，仿佛是还有着一道魔神般的身影，只不过任谁都是看不清楚他的容貌。
“真是令人怀念的地方，不过…符祖那家伙，似乎不在了呢，真是可惜。”
恐怖的威压弥漫着天地，那道声音缓缓的响起，旋即他似是一笑笑声终于透露了一些讥讽：“这家伙，明明与我是一般的目的，结果偏偏要去走那所谓的大义之道…”
“算了…终归还是我赢了。”
那声音轻笑着，旋即他的目光自绫清竹身上转移而过，看向了那绚丽的冰莲，魔气中的邪恶双目微微一凝：“冲击祖境吗？倒的确是小觑了你们这天地中的生灵不过，这种机会，我恐怕不会给予你们的…”
轰！
就在他话音落下间，只见得那漫天魔气陡然奔涌，粘稠的魔气飞快的凝聚而来，最后竟是化为一团巨大的黑色火焰。
火焰袅袅升腾连空间都是发出不堪炙烤的破裂之声，那火焰中，仿佛是有着无尽的邪恶在涌动。
“咻！”
黑色火焰蠕动着，隐约的似乎是化为了一道狰狞的鬼脸，而后魔火直接呼啸而出，对着那绚丽冰莲暴射而去。
唰。
绫清竹娇躯一动，直接是出现在绚丽冰莲之前，那绝美脸颊之上布满着冰寒，旋即突然有着一波波混沌之光陡然自其体内席卷而出。
随着那种混沌光芒的散发，这天地仿佛都是在此时共鸣了起来，无穷无尽的天地之力，在此时疯狂的汇聚而来。
“太上，斩魔！”
绫清竹玉手紧握三尺青锋，而后一剑挥下，其中并没有任何的精妙招式，只是伴随着其剑势的挥下，只见得下方乱魔海中，都是被生生的撕裂出了一道数以万丈庞大的深深沟壑。
一道混沌剑芒，呼啸而过，最后劈砍在那恐怖的鬼脸魔火之上，顿时后者爆发出刺耳的惨叫声，竟是被那并不算特别凌厉的剑芒抵挡了下来，只不过魔火涌动间，也是在不断的侵蚀着那种混沌光芒。
“竟然能够将这种力量运用到这种程度了…果然还是本世界的人对此更占优势啊…”魔云深处，那魔神般的身影见到这一幕，似是感叹了一声。
“不过，这可还远远不够啊…”
随着他这道声音的落下，只见得那鬼脸魔火竟是在此时爆发出惊天般的尖锐之声，魔火铺天盖地的升腾而开，那混沌剑芒，直接是在此时被逼得节节败退，那种混沌之光，也是愈发的黯淡。
绫清竹贝齿紧咬着红唇，光洁额间有着细密的汗水浮现出来，绝美的脸颊上也是掠过一抹淡淡的苍白，虽然她另辟蹊径，令得自己拥有了极为特殊而且强大的力量，但这面对着真身降临的异魔皇，显然依旧是有些无力。
那种绝对压制的力量，不是现在的她所能够抗衡。
“吱！”
狰狞的鬼脸魔火猛的尖啸出声，巨嘴一张间，竟然是将那混沌剑芒一口吞进，而绫清竹娇躯一颤，一丝血迹自唇角溢出来，但却被她是生生的忍了下来。
下方生死之主他们见状，面色也是一变，果然，还是阻拦不住啊。
“你对我倒是有些作用，将你擒下来也好。”
魔云之中，那魔神身影淡淡一笑，旋即他手掌伸出，只见得一只巨大无比的苍白手掌，便是自魔云中洞穿出来，一把对着绫清竹抓去。
随着那苍白手掌的落下，这天地仿佛都是爆发出了不堪重负的呻吟之声。
绫清竹周身的空间，几乎是在此刻尽数的被禁锢，这令得她根本就无法动弹，不过即便是这般险境，她眼中依旧未有多少慌乱之色，只是深深吸了一口气，眼中的决然，愈发的浓郁。
绫清竹修长玉手轻轻相合，然后化为一道极为古老的印法而随着这道印法的结成，所有人都感觉到，这天地仿佛爆发出了嗡鸣之声。
“咻咻！”
一道道混沌光芒，从天地各处射来，最后笼罩在绫清竹娇躯之上而在那种光芒的笼罩下，她那晶莹如玉般的肌肤上，竟然是浮现了一道又一道的古老纹路。
那种纹理，犹如天地的纹理，神奇而自然。
开始有着强烈的混沌光芒自绫清竹体内涌出来，周遭禁锢的空间竟然都是逐渐的松开，那一只对着她抓来的苍白大手也是被那种混沌光芒抵御了下来。
不过，这种抵御，显然也是暂时性的那苍白大手上，黑得如虬龙般的魔筋急速的蠕动着，然后开始将那种混沌光芒逼退。
两种恐怖的力量，在天空上交织对恃，但任谁都是看得出来，那混沌光芒已是油尽灯枯。
生死之主他们双手紧握，随时准备出手，而他们也明白，一旦出手，他们就必然要燃烧轮回，不然的话根本不可能与异魔皇抗衡一“嗡。”
混沌光芒，在那苍白大手下，越来越薄弱，已是被压制到绫清竹周身十丈范围，她见状，心中也是轻轻一叹，旋即不再又任何的犹豫，娇躯之上布满着的古老纹路，开始在此时变得炽热一种剧烈的疼痛，弥漫身体。
“咻！”
原本被压制的混沌光芒，在此时以一种惊人的速度反扑出来，竟是将那苍白大手都是震退了一些，而所有人都是能够见到，绫清竹周身混沌光芒越来越浓烈，而后，仿佛这整个天地，都是在此时震动起来。
“她想要…”
生死之主他们见状，心头却是一惊，显然也是察觉到了绫清竹也是在施展一种类似燃烧轮回般的手段来抗衡异魔皇。
“真的只能一个个的牺牲吗…”他们对视着，一种深深的无力，涌上心间。
绫清竹双目，缓缓的闭上，她能够感觉到她的身体，仿佛是在这一与整个天地融合在了一起，只要再催动一步，便是能够调动最为强大的力量，将那异魔皇阻拦下来，为林动争取到最后的时间。
“林动，再见了。”
一道低低的喃喃声从绫清竹心中掠过，然后她印法陡然一变，不过，就在她即将彻底释放体内力量的那一霎那，天地间，突然有着绚丽的光芒爆发而开。
而在那种绚丽光芒的照耀下，绫清竹突然感觉到，她的那种与天地融合的状态，竟然是被另外一股极端恐怖的力量，生生的逼退了出来。
而在她被逼退出那种状态时，一道熟悉的味道，已是扑面而来，一只有力的臂弯紧紧的环绕过她那纤细的腰肢，将她搂进怀中。
“不要再这样了，那种痛苦，不要再让我尝试第二次了，好吗？”他将她搂紧怀中，然后喃喃道，那声音中，有着难掩的沧桑。
熟悉的味道，熟悉的声音，这一刻即便是绫清竹那幽静的心境都是微微一颤，她缓缓抬头，望着那张脸庞，仅仅只是一月的时间，那对黑色的眸子却是沧桑与深邃了许多，那番模样，仿佛他经历了千般轮回。
“接下来，就交给我吧。”
林动身后，还有着铺天盖地的绚丽光芒在涌动，他望着绫清竹，微微一笑，道。
“嗯。”
绫清竹那紧绷的娇躯缓缓的柔软，旋即螓首轻点。
下方的乱魔海，也是在此时暴动起来，无数人望着那在此刻出现的林动，脸庞上皆是有着浓浓的狂喜涌出来…他们，终于等到了吗？
“他成功了？”
生死之主他们望着此时的林动，心头一震，目带震撼的对视着，心中在惊骇之余，也是有着难掩的惊喜，他们能够感觉到，林动此时身体上的波动，竟是当年的符祖，一般无二！
显然，他真的晋入到了祖境！
这天地，终于有救了！
天空上，林动松开绫清竹，然后抬起头望着那魔云深处，淡淡一笑，道：“堂堂异魔皇大驾光临，林动在此代这天地无数生灵说一声，久等了。”
天空上，原本呼啸的魔气，仿佛也是在此时受到那绚丽光芒的压制，变得安静了一些，而后仿佛是有着一道魔光垂落下来，在那魔光中，一道黑影，逐渐的浮现，最后光芒散去，一道人影，也是出现在了天地间无数道目光的注视下。
那依旧是一道人影，他身着黑袍，身体欣长，面目俊逸，只不过，当林动见到他那面目时，双目却是微微的眯了起来。
因为那道人影的模样他并不陌生，甚至可以说是深刻无比，因为…那面目，赫然便是，林琅天！


第1306章 最后一战
“林琅天？！”
绫清竹以及小炎这些曾经与林琅天认识的人望着这一幕，心头却是猛的一震，眼中有着浓浓的荒谬以及难以置信涌出来。
那林琅天竟然会是异魔皇？怎么可能！
“他并不是林琅天…”
林动黑色眸子望着那道熟悉的身影，眼中却是并没有多少波动，他能够感觉到，眼前这林琅天只不过是虚有其表而已。
“呵呵，只是感觉到你的情绪中对这人似乎有着很激烈的反应…”异魔皇微微一笑，他也是打量了一下自己，笑道：“看来这模样对你们刺激挺大呢。”
“这幅模样，真是让我有把你杀第二次的冲动。”林动淡淡的道，虽然当年那林琅天已经死在了他的手中，但他并不介意，再将“他”杀第二次。
不过…这样也好，我的修炼之路，是由林琅天开始，而现在，就在我达到巅峰时，再来与你解决所有的恩怨，虽然眼前的“林琅天”，并不是真正的他。
林琅天笑着，他缓缓的打量着林动，最后叹了一声，道：“没想到，竟然还真的有人能够达到祖境，倒真是我失策了啊，符祖这家伙，看来也是留有后手。”
“这世界不属于你们，哪来的，就回哪去吧。”林动眼睛微垂，淡漠的道。
“我能感觉到你对我那种怨恨之意，看来你为了达到祖境也付出了很大的代价啊，这样让我回去，你甘心吗？”林琅天笑道。
“是有些不甘啊…”
林动仰起头，深吸一口气，他望着天空，那里，仿佛是有着一道有着乌黑马尾的娇俏少女身影闪现，而后渐渐的逝去。
为了这一步，我连她都是失去了…若是不把你这王八蛋宰了的话，那真是会…很不爽的啊。
冰寒的狰狞，一点点的自林动眼中涌出来，旋即所有人都是能够感觉到，这天地，开始嗡鸣颤抖，无数绚丽的光芒，铺天盖地的涌来，最后汇聚在林动的身后。
“既然这样的话，那你就留下来吧！”
林动手掌一握，天地间有着八道光芒暴掠而来，最后环绕在其周身，赫然便是那八大祖符。
咻咻。
紧接着，再度有着一道道光芒掠来，那是九大神物，祖石，大荒芜碑……
“你终于成功了…”
祖石之上，光芒浮现，岩望着此时的林动，后者身上的那种威压与当年的符祖如出一辙，这令得他眼中有着浓浓的欣慰涌出来。
谁能够想到，当年那个青阳镇的稚弱少年，如今，却是站在了这天地最为巅峰的层次，古往今来，唯有着符祖，方才能够与其媲美。
“暂时改变你们一下。”
林动冲着岩微微一笑，只是那笑容有些沧桑，然后他大手一握，九大神物开始彼此连接，仿佛是化为了一柄闪烁着七彩颜色的古朴大枪，枪身之上，流溢着恐怖的力量。
如今的林动，已是真正的晋入了这天地最为至高的层次，实力丝毫不弱于当年的符祖，虽说九大神物厉害，但依旧是随着他的心意汇合转变。
林动手持七彩长枪，周身八大祖符欢快飞舞，一种仿佛凌驾于这片天地的威压弥漫出来，令得下方乱魔海中无数强者身体都是激动得有些颤抖起来。
这才是真正的希望！
唯有着这种真正的至高实力，方才能够将那魔神般的异魔皇阻拦下来！
“杀我？符祖都办不到的事，你即便能够达到符祖的地步，又能如何？”林琅天望着林动身后那铺天盖地涌动的七彩光芒，淡笑道。
“轰！”
林动并未回答，其手掌对着林琅天猛然握下，旋即那片万丈空间，便是尽数的崩塌，足以将一名渡过三次轮回劫的巅峰强者都挤压成碎片的可怕力量，蜂拥而出。
林琅天双手一合，漆黑如墨般的火焰自其体内席卷开来，直接是硬生生的将那种空间蹦碎的挤压之力抵挡下来。
“哈哈，那就让本皇来领教一下，这新生之祖，究竟又能厉害到哪里去？！”
林琅天大笑着，滔滔魔气在其身后涌动，竟是化为一道道数十万丈庞大的粘稠魔气洪流，洪流浩浩荡荡的呼啸过天际，然后犹如蜿蜒盘踞的魔龙，暴掠而出。
砰砰！
魔气洪流所过之处，无数人便是骇然的见到空间尽数的蹦碎，一道道黑色裂纹，自天空上飞快的蔓延开来。
魔气洪流暴掠而至，林动脚掌轻轻一踏，铺天盖地的绚丽光芒席卷出来，那种绚丽光芒之中，不仅有着无尽元力，还有着浩瀚的精神力，甚至其中还包涵着整个天地的力量。
七彩光芒，化为数十万丈庞大的七彩光盘，缓缓旋转，任由那魔气洪流呼啸而至，最后狠狠的撞击在上面。
咚咚！
天地都是在此时颤抖着，那种对碰，已经无法用可怕来形容，光是那些溢开的恐怖能量，便是让得那些轮回境的巅峰强者面色惨白，他们明白，以他们的实力，只要被稍稍触及，怕就是魂飞魄散的下场。
生死之主他们也是面色凝重的望着遥远天际之上两人的交手，那异魔皇的实力，应该也是类似祖境的地步，说起来，林动与他都是处于相同的层次，两人这种交手，堪称惊天动地。
“不过…当年师傅也是与异魔皇拼斗得极为的厉害，但正是因为差距不大，想要取胜方才格外的困难…”
生死之主犹豫了一下，道：“那场的天地大战，持续了好多年，最后师傅迫不得已，只能燃烧轮回，封印了异魔皇与位面裂缝…”
炎主他们闻言眉头也是紧皱起来，当年如此，莫非如今，也是要让林动燃烧轮回来封印异魔皇吗？那样的话，岂不是又是进入了那种循环之中？
“那倒也不至于…林动要彻底斩杀异魔皇也不是没有办法。”生死之主沉吟道：。
“你是说…第一神物？”炎主他们微微一惊，旋即皱眉道：“第一神物…究竟是什么？”
对于那所谓的第一神物，就连他们都是知之不深，他们唯一知道的，便是那第一神物即便是他们的师傅都未能掌控，不然的话，当年的天地大战，早便是以他们彻底胜利而告终了。
他们面面相觑着，眼中皆是有点茫然，那第一神物，实在是太过神秘了一些…
“那第一神物…应该便是那太上之力的源头。”一旁突然有着轻声传来，生死之主转头，便是见到了来到身旁的绫清竹。
“太上之力的源头？”生死之主他们一愣，他们对于太上之力倒是知道，当年符祖也让他们感应过，不过除了冰主之外，其他人都并未将其感应成功。
“嗯。”
绫清竹螓首轻点，那美丽的眸子中，有着一种淡淡的波光在流转，她似乎是凝望着这片天地，道：“我也只能感应到这一步，至于想要将其掌控却是不可能的事，这一步，恐怕唯有踏入祖境的强者方才有可能办到。”
“那林动能否办到？”生死之主急忙问道。
“他也修炼了太上感应诀，并且感应到了太上，我想，现在的他应该也是猜测到了一些亻旦至于他能否掌控，我也不太清楚，毕竟当初连符祖都是失败了事…”绫清竹眸子深处流过一抹微涩之意，道。
生死之主他们点了点头，现在，他们也帮不上半点忙，只能期盼着林动，能够真的将那第一神物操控了…
咚咚！
九天之上，两道弥漫着恐怖威压的光影，各自操控着恐怖的力量，狠狠的对轰在一起，而伴随着两人每一次的交锋，那空间都将会迅速的蹦碎。
天地间，狂风大作，天颤地动，犹如末日之景。
林动晋入祖境，天地力量任由他操控，而那异魔皇也是魔气无尽，因此即便两人疯狂交锋，但却依旧不显丝毫的疲态。
叮！
七彩长枪携带着滔天般的光华暴掠而出，而后与那魔枪闪电般的撞击在一起，两人眼神都是在这一瞬变得冰冷凌厉起来，绚丽光芒以及魔气在两人身后疯狂涌动。
吼！
林动身后，绚丽光芒疯狂凝聚，竟是化为一头数十万丈庞大的七彩巨龙，巨龙盘踞，仰天长啸，那龙吟之声，几乎响彻了天地的每一个角落。
嚎！
那异魔皇身后，同样是有着一道滔天魔影浮现出来，那魔影丝毫不比七彩巨龙小，周身布满着无数魔臂，每一只魔掌之中，都有着一颗邪恶的巨眼，闪烁着冷漠残忍的光泽。
咚！
七彩巨龙与魔影狠狠冲出，然后在两人上空硬憾在一起，一股无法形容的波动席卷而开，下方乱魔海中，顿时被掀起数万丈庞大的巨浪，而后轰隆隆的扩散开去。
能量涟漪从两人对碰之间席卷开来，两道身影皆是一震，急退了数百丈，而当他们每一步落下来时，那片空间都是会被生生的踏碎而去。
两人这般惊天动地般的交手，却依旧是未能分出胜负。
“我说过，即便是祖境强者，也唯有与我持平而已，当年符祖都奈何不了本皇，更何况你？”异魔皇手中魔枪轻轻一震，空间碎裂，他淡笑道。
“我异魔族大军很快就将会自位面裂缝源源不断的进入这天地，到时候，你们这片天地，又如何能够抵挡？”
下方无数强者听得此话，面色皆是一变，目光急忙看向那虚无处的位面裂缝，果然是见到，那里开始有着无尽的魔气涌动，其中仿佛是有着无穷无尽的魔影。
生死之主他们也是面色凝重，一旦异魔族侵入的话，那么便是真正的天地大战，而到时候，他们也必然会付出极端惨烈的代价。
林动也是抬头望着那位面裂缝处，旋即他望着那变幻成林琅天模样的异魔皇，声音轻缓：“符祖会失败的事，不代表我也会失败。”
“因为这里，是我们的世界。”
异魔皇魔瞳深处，一抹异光掠过。
“你能告诉我，你们来我们这位面的目的吗？不只是为了单纯的杀戮与侵占吧？”林动道。
异魔皇握着魔枪的手掌缓缓紧握，魔瞳之中，寒芒涌动，却并不打算再与林动多说什么，恐怖的魔气，从其体内席卷而出。
“看来你不想说呢，不过…即便你不说，我也是知道的。”林动一笑，道：“你们应该是为了某种东西而来的吧？”
“你们想要的那种东西，在我们这里，应该是被称为，第一神物吧？”
异魔皇盯着林动，终于是讥讽一笑，道：“看来你知道得还真不少呢…既然如此，那你又知道你们这天地无数人敬仰的那位符祖大人，又是什么来历吗？”
林动双目微眯。
“那位符祖大人，同样不是你们这世界的人，他来自与我相同的地方，而且来这里的目的，也是与我一模一样。”
异魔皇冷笑道：“是不是很讽刺？你们曾经的救世主，其实也是别有用心而已，只不过他选择的路子与我不同罢了。”
生死之主他们面色苍白，身体都是剧烈的颤抖起来，显然是被这消息震撼得不轻。
“这一点，在我晋入祖境是便感觉到了。”
天地间唯一还算平静的便是林动了，他盯着异魔皇，淡淡的道：“不要将你自己与符祖相提并论，虽然他并非是我们这世界的人，但不论他有什么目的，至少我们都知道，他曾经救了这无数生灵，有这一点，就足够了。
乱魔海中的震动稍稍的平息，无数人都是点了点头，不管符祖有什么目的，但不论如何，如果没有符祖，这天地，早便已经沦陷。
“嘁。”
异魔皇冷笑一声，却是不想再多说废话，他盯着林动：“如今的你，便是这天地的希望吧？只要将你杀了的话，你们这天地，就该绝望了吧？”
“你与我处于相同的层次，要轻易的斩杀你的确不容易，不过…其实在我被封印的岁月中，我同样也是想到了这种最坏的局面。”
“当年舍不下这般代价来对付符祖，结果被他反将一军，这种错误，可不会再出现第二次了！”
“只要将你镇压千载，便是本皇赢了！”
异魔皇的脸庞上，掠过一抹诡异之色，旋即他的身体，开始在此时疯狂的膨胀起来，短短数息时间，便是在那无数道惊骇的目光中，化为了一尊脚踏天地的万臂魔影，那些狰狞的魔臂之上，无数只邪恶的眼睛，缓缓的睁开，一种毁灭的光线，散发而出。
砰砰砰！
而就在那尊狰狞无比的魔神真身出现时，他身体之上那无数魔臂，突然在此时活生生的爆炸开来，魔血铺天盖地的暴射出来。
咻咻。
魔臂爆炸间，那魔掌之中的一颗颗邪恶魔眼也是在此时暴射出来，无数悬浮在异魔皇上空，那无数邪恶的眼睛一起眨动，那一幕，看得人头皮发麻。
“无始魔碑！”
低沉而弥漫着阴冷的声音，突然响彻天地，那无数邪恶的魔眼竟是开始了融合，隐隐的，似乎是在天空上化为了一座百万丈庞大的黑暗魔碑，魔碑之上，镶嵌着无数魔眼，那魔眼眨动间，所有人都是能够感觉到，天地间的能量，仿佛是在被污染。
“嗡嗡！”
而就在那魔碑成形时，那位面裂缝之后，突然传来惊天动地般的尖啸声，然后只见得铺天盖地般的魔影自位面裂缝之后穿越而来。
那是异魔族的军队！
那种规模，比起魔狱，超越了不知道多少倍！
“他们…这一次，真的是倾巢而来了…”
生死之主望着这一幕，面色顿时苍白起来，这种规模，比起远古时期的那天地大战，还要厉害，显然，这一次，这异魔族竟然是举族之力！
这一次，真是糟了。
生死之主他们手掌紧握，不论他们承不承认，他们都明白，异魔族实力的确超越了他们这位面，若是异魔族举族而来，那么对他们显然是太过不妙。
“祭碑！”
异魔皇阴冷而残酷的声音，再度响彻。
砰砰砰！
而就在他声音落下间，只见得那无数冲进这天地的异魔，竟然仰天嘶啸，而后不断的爆炸开来，粘稠的黑色血肉，不断的飞向那座布满着魔眼的魔碑。
嗡！
短短十数息的时间，已是不知道爆炸了多少异魔，只见得那魔碑之上，布满着了黑色血肉，那些血肉蠕动间，再配合着那些魔眼，显得分外的可怖。
一种阴冷得连这天地都是颤抖的波动，缓缓的自那魔碑之中散发出来。
那种波动，就连生死之主，绫清竹他们都是感到一种心悸，浓浓的不安，自心中涌了出来。
林动眼神也是因为那恐怖魔碑的出现微微变了变，旋即其心神一动，八大祖符暴掠而出，滔天火焰，寒冰，雷霆成形，万丈黑洞也是出现在那异魔皇上空，八大祖符的力量，被其催动到极致。
轰轰！
然而面对着八大祖符的围剿，那魔碑之上，无数魔眼中猛的暴射出滔天般的魔气光束，竟然是生生的将八大祖符震退而去，魔气缠绕间，甚至连祖符都是黯淡了一些。
“本皇以灭真身为代价，耗尽过半族之力，今日便将你镇压！”
异魔皇所化的真身，仅仅只剩下两只魔臂，他魔瞳狰狞的望着林动，这般代价，即便是他都得休养千载方才能够再回复，不过只要能够在这里将林动解决，那他的目的，也是能够达到。
只要能够获得那东西，那么他将会变得更为强大！
“无始魔碑，万魔之印！”
低沉的咆哮，自天空上轰隆隆的传开，那百万丈庞大的魔碑，一闪之下，便是出现在了林动上空，一道道魔光席卷而开，瞬间便是禁锢了这天地，一道猩红而狰狞的魔印，在魔碑之底成形，然后对着林动轰隆隆的镇压而下。
下方的乱魔海，上百万里的海域，在此时被生生的撕裂而开，天地碎裂，发出不堪重负般的哀鸣。
林动面色凝重，手中七彩长枪暴掠而出，八大祖符紧随而至，犹如巨龙般的轰向那镇压而来的魔碑。
砰砰！
然而，当七彩长枪刚刚与那魔碑相触时，便是爆发出哀鸣之声，光芒黯淡，竟是再度被震裂开来，化为九大神物。
那八大祖符同样是遭受到重创，那古老的符文上，竟是有着一丝丝的裂纹浮现。
“林动，小心，这异魔皇要镇压你！”
岩面色惨白的闪现出来，尖声叫道，这异魔皇吃了当年的亏，显然是想要先下手，那种毁灭真身以及半个种族的力量，太过可怕了。
那乱魔海中，无数强者也是见到了这一幕，当即身体都是颤抖了起来，眼中涌出来浓浓的恐惧，若是连林动都失败的话，这世界上，还有人能够阻拦住这异魔皇吗？
生死之主他们同样是紧握双手，现在的异魔皇，比起当年，更为的疯狂与可怕。
无数道目光铺天盖地的呼啸下来，禁锢着林动周身天地，那一道猩红而狰狞的魔印，也是呼啸而来，那上面的力量，就算是林动都感到了浓浓的危险。
这种力量，即便是晋入了祖境，那怕也是无法扰下来。
这异魔皇，直接打算下大代价将他斩杀，那样一来，即便他遭受到了难以弥补的重创，可至少，这天地，将再无人能够将其阻拦。
只是，这异魔皇孤注一掷的手段，真的能够以常规手段来阻拦吗？
“这种攻击，林动根本阻挡不了的，除非燃烧轮回…”生死之主他们望着天空上这种局势，心头也是不断的下沉。
天空上，林动望着那镇压而来的魔碑，竟是缓缓的闭上双眼他知道，想要真正的解决掉来自异魔族的威胁，就必须真正的斩杀异魔皇，而不是封印。
但想要将其斩杀，即便是他晋入了祖境也难以办到，除非…掌控那第一神物！
虽然林动对于那第一神物同样知道得并不深，但毕竟如今他已晋入祖境，隐约的能够察觉到一些什么…而那种察觉，似曾相识。
那是…太上。
何谓太上？
林动的意识，仿佛都是在这一霎融入了这天地，意念所动处天地任何之处，随意所至，无数山川河流，平原自其意识中掠过，最后又仿佛是归于虚无…
太上是什么？
“你们护住我，我去助他！”
绫清竹仿佛是察觉到了什么，突然对着生死之主他们说道，而后美目立即闭上意识闪电般的融入天地。
林动的意识在沉思，而就在他探寻之间，突然感觉到一股柔软的意识也是出现，那意识对着他依靠过来，其中有着浓浓的情感。
那是来自绫清竹的意识。
“我带你去寻找太上…”
那意识中，传来轻柔的声音声音中，似是带着一丝羞涩之意，而后林动便是感觉到那柔软意识竟然与他相融合起来。
两道意识相融，两人仿佛都是狠狠的颤了一颤，那是一种精神的融合，真正的你中有我，我中有你。
意识相融，两人的意识仿佛都是恍惚了一些，而后意识弥漫向虚无整个天地，都是在他们的意识反射之中。
意识俯览着天地，再然后，虚无中有着混沌之光出现，正是那曾经感应太上之地。
两人的意识，进入那混沌之中，这一次，那种曾经所出现的刺痛与排斥竟然尽数的消失而去，林动意识开始凝聚，在那混沌之中，化出身来。
在他的身旁，绫清竹也是浮现出来，只不过她那绝美而白皙的脸颊上，此时有着红霞弥漫。
“这里就是那所谓的太上…你能感应到它吗？”绫清竹轻声道。
林动微闭着双目，意识仿佛彻底的融入了那混沌之中，在那里，他感觉到了一股类似生灵般的意念，只不过那意念极为的稚嫩，犹如毫无思想的胎儿。
那是位面之胎。
林动的心中掠过一抹明悟，仿佛是在这一霎那明白了什么，所谓的太上，便是位面之灵，也是这天地最为至高无上的存在。
只不过，那存在，并没有任何的思想，但它，却是诞生在这天地中最为强大的生灵。
所谓的太上之力，便是来自于它。
而那异魔皇以及当初的符祖，便是为了它而来，因为只要获得了它，那么他便是真正的位面之主，拥有着这整个位面的力量！
只不过，他们毕竟不是自这天地间诞生的人，与那位面之胎之间，彼此是有着一些隔阂。
符祖曾经感应到了它，但在接触中，却是掌控失败…
“果然不愧是这天地之间的第一神物啊。”
林动意识，缓缓的接近那位面之胎，而后者似乎也是察觉到了他的接近，混沌光芒涌动间，便是消失而去。
追逐无果，林动也是停了下来，他仰起头，望着那混沌之地，双手缓缓的张开，低沉而雄浑的声音，在这混沌之中响起。
“跟随我吧，我生于斯，长于斯，从此以后，这片位面，我将，守护它！”
“守护它！”
“守护！”
低沉的声音，回荡在混沌之中，轰隆隆的不断有着回音传开。
林动的目光，看向那混沌最深处，他再没有主动去靠近，只是那眸子深处的坚决，却是纹丝不动，那种执着，令人动容。
他的目光，凝视着混沌深处，可那里却并无丝毫动静，许久，就在一旁绫清竹轻轻一叹间，混沌深处，突然有着耀眼的光芒席卷而开，整片混沌之地，都是开始剧烈的颤抖起来。
混沌光芒，开始在林动身前凝聚，旋即化为了一道约莫巴掌大小的灵胎，灵胎轻轻的靠近着林动，最后终于是落到了他的手中。
当林动手掌握着那位面之胎时一种奇特的感觉，自心底深处涌出来，那种感觉…就犹如他成为了这位面之主！
那是对这片天地的绝对掌控。
位面之主！
乱魔海上空，魔碑在那无数道惊骇的目光中，最终狠狠的镇压在了林动身体之上那异魔皇的脸庞上，也是有着一抹狰狞浮现出来。
这位面，该属于他了！
嗡！
而就在他脸庞上狰狞浮现的那一刻，只见得那魔碑之下，突然有着万丈混沌之光席卷而出，那镇压而下的魔碑，竟然是在此时凝固了下来。
“这里毕竟是属于我们的天地可轮不到你来撒野啊，异魔皇…”
有些沙哑的声音，缓缓的自那魔碑底下响起那里，一道身影，抬起头脸庞，他望着近在咫尺的魔碑，旋即手掌缓缓的伸出，与其接触在一起。
“所以破碎吧！”
凌厉无匹之色，陡然自林动眼中暴涌出来，一道仿佛心脏跳动的闷声，响彻在了这天地的每一个角落，那种心跳，犹如这无数生灵所汇聚。
砰砰砰！
魔碑之上无数只邪恶的眼睛，竟是在此时一个接一个的爆炸开来，凄厉的惨叫声，也是自那魔碑之上传出来。
无数人震惊的望着这突发变故。
“嘭！”
魔碑之上，裂纹浮现，最终是彻彻底底的爆炸而开，而那异魔皇身躯也是猛的一颤，他望着这一幕，眼中突然涌出一抹骇然之色。
“位面之力？！你竟然掌控了位面之胎？！”
异魔皇那暴怒的咆哮之声，响彻着天地，但任由谁都是听得出来其中蕴含的那一丝惊骇之意。
无数道目光仰望着天空，只见得在那混沌之光中，一道削瘦的身影缓缓的走出，而每当他脚步落下时，所有人都是能够感觉到天地仿佛都是在此时随之共鸣起来。
一种无法言语的威严之感，自他的身体上弥漫出来。
而在那种奇特的威严之下，下方海面上，竟突然是呼啦啦的跪下无数身影，他们眼神有些狂热与迷离的望着那道身影，那种威严，犹如俗世中的臣子看见了帝王一般。
那是威严，凌驾了天地。
“武祖…”
不知道是谁喃喃着说道，而后声音以一种惊人的速度扩散而开，下一刻，突然无数道声音猛然轰鸣响彻，震荡天宇。
“武祖护我众生！”
天际之上，伴随着那道身影一步步的走出，只见得那原本弥漫了天地的邪恶魔气，竟然是被逼得节节败退，那模样，就犹如是整座位面都是在排斥着它们一般。
异魔皇真身也是逐渐的后退，气势被压迫得略显狼狈。
“这位面…终于也出现位面之主了吗…”异魔皇咬了咬牙，眼中的暴怒与不甘几欲将他理智湮灭，他为此付出了巨大的代价，甚至还被符祖封印这么多年，谁料到，最终反而是让林动渔翁得利，成为了这位面之主！
林动面庞威严的望着那异魔皇，然后转向虚无之中那位面裂缝，大手一挥，只见得混沌之光席卷而开，再接着，那异魔皇便是骇然的见到，那位面裂缝，竟然是在此时缓缓的愈合。
这种修补位面的力量，唯有一个位面之主，方才能够办到。
“撤！”
异魔皇眼中神色不断的变幻，最终怒吼出声，那些异魔大军闻言顿时纷纷溃逃，显然都是察觉到了不妙。
“我说了，既然来了，那就永远的留下来吧，符祖未能抹杀你，便由我来帮他完成。”
林动声音平淡，那声音之中，却是有着无尽的威严，仿佛天地共鸣，而后他手掌仲出，只见得无尽混沌之光暴涌而出，最后在那异魔皇上空，化为一道巨大无比的光胎。
光胎仿佛还在轻轻的跳动着，而没伴随着它的一次跳动，天地间无数生灵的心脏，仿佛也是随之跳动。
“位面之胎？！”
异魔皇望着那光胎，眼中掠过一抹浓浓的贪婪以及骇然，旋即他咬牙切齿，脸庞上满是不甘之色，不过他也是果断，如今的林动，已经成为了位面之主，他根本就不再是他的对手。
心中念头转过，异魔皇身影已是暴掠而出，想要在那位面裂缝未曾全部愈合时，逃出这里。
“咻！”
不过就在他身形刚动时，却是骇然的发现，天旋地转，他所处的空间仿佛都是在此时被转移而去，待得他回过神来时，便是发现，他竟然已经身处那位面之胎中！
“林动，放我离去，我与你发誓，从此以后，再也不侵入你们位面！”那异魔皇急忙骇然的喝道，他这一刻，终于是感到了致命的危险，这种危险，即便是当年符祖燃烧轮回都未曾给予过他。
林动眼神冰冷的望着他，为了对付你，我失去了一个最为重要的人，这种代价，光是将你驱逐可尝还不了！
“位面净化，归于虚无！”
低沉的声音，自林动嘴中传出，犹如无尽雷霆般，在天地回荡，旋即那位面之胎中，猛的暴射出无尽的光线，光线照耀在在异魔皇身体之上，顿时爆发出凄厉的惨叫声，异魔皇那庞大的身躯，迅速的缩小，周身粘稠魔气，也是在以一种惊人的速度消退。
“你们的侵入，造成这天地间浩劫荡荡，如今，便用你的生命来赔偿吧。”
林动手掌猛然握下，那巨大的位面之胎开始迅速的缩小，最后化为巴掌大小，缓缓的落在林动手中，在那光胎的中心，还能够见到一道细小的黑影，这异魔皇生命顽强，即便是催动位面之胎想要将其彻底净化，也是需要上百年的时间，不过这种时间，林动等得起，从今以后，他将再无翻身之日！
这祸患天地之间数千载的异魔之乱，终于是在今日，被彻彻底底的根除！
无数人激动的浑身颤抖的望着这一幕，那曾经让得他们感到绝望的毁灭身影，如今，终于是被制服了！
生死之主他们脸庞上同样是有着难以遏制的喜悦在涌动，这一天，他们可等待太久了…
“武祖救世之恩，众生莫敢忘！”
绝望之后的重生，让得无数人激动得热泪盈眶，无数人颤抖的跪拜而下，那真挚而激动的声音，凝聚在一起，震动着天地。
林动望着这片满目苍夷的大地，听得那轰鸣天地的敬畏之音，旋即缓缓抬头，他望着那缓慢愈合的位面裂缝，那里，仿佛是有着一道巧笑焉熙的倩影，正在缓缓的消散。
林动手掌紧握，眼中弥漫着憾不动的坚定：“你曾经与符祖的约定，我帮你完成了，接下来，就该完成我与你的约定了。”
上穷碧落下黄泉，我都要把你找回来！


第1307章 我要把你找回来（大结局！）
第二次天地大战，终是伴随着异魔皇的陨落而谢幕。
这种结果，让得这天地间，彻彻底底的陷入了欢腾，那种绝望之后的希望，有着一种令人难以抑制的激动与狂喜。
原本以为这片天地将会沦为异魔掌控，但谁能料到那最后的峰回路转，不仅逆转了局面，而且还彻彻底底的根除了来自异魔的危机。
这片久遭异魔肆虐的天地，终是得以安宁。
而在天地大战谢幕后的一月之中，三大联盟开始陆陆续续的解散，一切都是重回正轨，或许这世界上竞争残酷依旧不会少，但在这种竞争中，却是会不断有着强者被磨练出来，或许很久很久以后，也将会再度有着天才妖孽横空出世，晋入那传说之中的祖境。
世界，终归是会不断的进步。
不过，所有人都知道，即便是再度有人晋入祖境，恐怕也难以超越那道曾经将他们从最为绝望时刻拯救出来的身影。
武祖，林动。
祖境的至高强者或许能够多重的出现，但他的另外一个身份，却是独一无二，无人能够超越，那便是，位面之主。
这片位面的真正掌控者！
当年的符祖以及异魔皇，他们来到这片天地，所为的便是掌控位面之胎，成为位面之主，进而获得更为强大的力量。
只不过，最终他们都失败了而林动，却成功了。
道宗之内，一座巍峨山巅之上，林动站在那山崖之边，低头望着那云雾缭绕之下的道宗之景在其身后，绫清竹，青檀，小貂，小炎，生死之主等人皆是望着他的背影，那道背影虽然削瘦但却是有着一种无法言语的威严。
那种威严，来自位面之主。
“我准备动手，她虽然燃烧了轮回但毕竟时间尚短，应该会有轮回碎片散落在天地间，若是能够将其轮回碎片找到，我便是能够将其送入轮回，并且保其记忆不失。”林动凝望着道宗，半晌后突然缓缓的道。
“你有多少把握？”炎主等人闻言，眼中也是掠过一抹喜色，旋即有点担忧的问道，虽然林动如今实力通玄，但燃烧轮回对于他们而言几乎相当于彻底的毁灭，这种程度想要再救活，谈何容易？
“应该有五成吧。”
林动喃喃道，心中不知为何却是掠过一丝惶悸之色，那手掌也是忍不住的紧握了起来，虽然如今他拥有了这天地间最强大的力量，若是连心爱的人都是无法找寻回来，那种力量，又能有什么意义？
他修炼的目的，便是想要保护想要保护的人可如今，他却发现自己根本无法保护…
一只柔软而娇嫩的玉手，轻轻的握着他的大手，其上传来的滑嫩以及温暖之感，令得林动心境微微平复，旋即他偏头望着身旁那白裙如仙般的绫清竹，她那对优美的眸子，静静的看着他，其中的柔意，却是掩饰不住。
“谢谢你。”
林动望着那轻轻望着他，却是并不多言的绫清竹，心中有着感动与温暖流淌而过，他反手紧紧的握住她的玉手，道。
她总是这般，默默的将所有的情绪隐藏在自己的内心深处，为了他，她能够放弃固有的清傲，变得柔软来安慰他，在他心中最脆弱的时候，她也总是会静静的站在他的身旁，即便是不言不语，但却令得林动倍感暖意，只是，她的这种坚强，有时候反而更令得人微微心疼。
“以前只是站在远处看着你，所以现在，要加倍的补偿回来啊。”绫清竹微笑着轻声道。
林动闻言也是一笑，类似绫清竹这种清冷性子，唯有真正的走到她内心深处，方才能够打破那层拒人千里的寒冰，享受到那种芸芸众生，只为一人而绽放的火热与柔软，显然，最开始的林动，可达不到这种程度，所以那时候的绫清竹可没什么责任来兼顾着他，补偿之言，也是无从说起。
“补偿的话，在你教给我太上感应诀的时候就已经做了…”
绫清竹一怔，旋即那绝美的脸颊瞬间绯红了起来，她咬着红唇，盯着林动，眸子中有着难掩的羞涩之意：“你…你都知道了？”
“从一开始就知道的…”林动望着绫清竹，眼中有着浓浓的怜惜。
绫清竹银牙咬着红唇，有些羞恼的扬起玉手在林动手臂上轻轻锤了一拳，原本她以为那事她做得很隐晦的，没想到这家伙，竟然一直都在装。
“你快开始吧。”
林动这才笑着点了点头，然后便是不再多说，在那山巅之上盘坐而下，双目也是缓缓的闭上。
而一旁的绫清竹等人，都是能够感觉到，伴随着林动双目的闭上，似乎是有着一种极为奇妙的波动扩散而开，那种波动速度极快，一眨眼便是笼罩了天地，然后开始寸寸探寻，搜索着那可能散落于天地之间的轮回碎片。
林动的意念，散于天地之间，他催动着位面之力，找寻着那存在内心深处的熟悉…
而这一探寻，便是整整一月时间。
然而，伴随着时间的推移，林动的面庞，却是逐渐的苍白起来，因为他发现，即便是他催动了位面之力搜寻天地间的每一寸，竟然都是未能寻找到一点熟悉的轮回碎片。
这种无果，让得他那古井般的心境中，涌上了丝丝慌惧。
这种结果，让得他实在是有些难以接受，因此他猛的一咬牙，再度催动位面之力，这一次他着重的搜寻着与她去过的任何一个地方，道宗，异魔域，以及那曾经抢夺着仙元古树的地方…
按照常理来说，轮回碎片会逗留在生前最为执着的地方如果应欢欢的轮回碎片还存在的话，也一定会在这些地方！
只是…
林动加大了搜寻，但最后的结果，依旧是残酷得令得他不敢相信。
两月之后，山巅上的林动睁开了双目，他的眼神变得空洞了一些，其中甚至是有着血丝攀爬出来他的身体微微的颤抖着，喃喃道：“怎么…会找不到啊…怎么会啊…”
原本这种办法，应该能够办到的啊！她才燃烧轮回没多长的时间不可能连轮回碎片都消散的啊！
“怎么会…这样…”
他痛苦的抓着头，心中仿佛是突然间空了一大块，那种难言的难受之感，令得他眼睛都是血红了许多。
“林动，你怎么了？”
一旁有着焦急的声音传来，绫清竹急急的出现在林动身旁她望着后者那披头散发的模样，俏鼻也是涌上一股酸意，连忙在其身旁跪坐下来。
林动呆呆的抬起头，他望着绫清竹，眼睛突然犹如无助的孩子一般红了起来：“我…我找不到她了…找不到了…”
望着他这幅罕有的脆弱，绫清竹心头也是一疼眼眶泛红，如今的世人，都是沉侵在那种劫后余生的欢喜之中，但谁又知道，这个拯救了世界的男人心中又是何等的悲苦。
“不急不急，我们慢慢找，一次不行就两次，一定能找到的。”她伸出纤细玉臂，轻轻的将林动抱在怀中声音轻柔得犹如呵护脆弱的瓷器。
林动也是紧紧的搂着绫清竹那纤细的腰肢，许久后，他咬了咬牙，眼神有些疯狂：“我一定会把她找回来！”
他再度闭目，将意识沉入天地，只是其眉宇间，有着一抹深深的惧色，他害怕，如果真的找不回来，那他将会是何等的痛苦。
绫清竹望着那张坚毅而略显疲态的脸庞，眼眶通红，她知道后者那种执着的性子，当年追逐着她从大炎王朝走出来时，他的目光便是这般，或许，打动她的，也正是他这种令人动人的执着吧…
在不远处的一座山上，生死之主望着山巅上相拥的两人，轻轻叹息了一声，眼中掠过一抹复杂之色。
当林动再度睁开眼时，又是两个月时间过去，这一次，他的眼神已是有些灰暗，身体微微颤抖着，再没有了身为位面之主的威严。
一个连心爱女人都是找不回来的人，拥有着再强大的力量，那又有什么作用？
绫清竹望着那眼神越来越空洞的林动，心中也是越来越疼，不过现在她所能做的，也只是静静的陪在他身边，她知道现在的他，心中必定是极为的痛苦。
希望，已经是极为的渺茫，但林动却始终不肯以及不敢放弃。
时间流逝。
山巅上，林动一次又一次的睁开眼睛，但那眼中的神采却是越来越微弱，灰暗弥漫着眼球，山巅上原本葱郁的山林仿佛都是伴随着他的心境变化，而逐渐的枯萎。
希望在消逝。
这半年时间，小貂，青檀，生死之主他们都是来过，但他们望着林动那番模样，却是一句话都说不出来，最终只能默默的黯然离开。
只是在他身旁，却始终有着一道倩影静静的等候与照顾着。
当林动最后一次睁开眼时，那眼中的神采几乎全部消失，天空上，雪花飘落下来，令得天地显得银白而凄凉。
绫清竹望着林动那无神的眼睛，鼻尖泛红，终是忍不住的侧过脸去，大滴的泪水滴答滴答的落下来，最后溅射在林动脸庞上。
他微微的转过头，望着那张红着眼眶的绝美脸颊，喃喃道：“对不起…”
“我知道的。
绫清竹轻声道，她知道，若是换成她，他同样也会这般。
“只是，或许她也并不太希望看见你这般的折磨自己。”
林动双掌颤抖着，他垂着头，有些嘶哑的道：“…为了获得这种力量我失去了她，可是，我却不能用这种力量去挽回她…”
“为什么？！”
“为什么啊？！”
他猛的仰头怒吼，那吼声之中，有着撕心裂肺般的痛苦天空上，磅礴大雨倾泻下来，扑打在他的脸庞上，已是看不清究竟是雨水还是泪“啊！啊！啊！”
他双拳疯狂的锤在大地上，那咆哮的声音，犹如泣血一般，蕴含着无尽的悲意与痛苦在这天地之间，远远的传开。
他给了这天地希望，但却给自己带来了绝望。
道宗之中无数弟子望向那座最高的山峰，感受着那咆哮声音之中的痛苦，他们眼睛也是通红起来，一股酸意令得他们眼睛湿润下来。
在大殿前，应笑笑望着山巅上那若隐若现的身影，那声音之中的绝望让得她明白那个曾经巧笑焉熙的可爱女孩，彻底的回不来了。
她紧紧的捂着嘴，发出低低的哽咽之声，一旁的应玄子也是红着眼，偏过头去，一下子仿佛是苍老了许多后面的周通，悟道等人也是沉默下来。
整个道宗，都是弥漫在一种悲伤的气氛之中。
“失败了啊…”
小貂，青檀他们望着这一幕，也是喃喃说道，炎主等人有些颓然的坐在地上，当年他们失去了师傅以及吞噬之主，现在…连她也是要失去了么。
生死之主望着颓然的众人，轻轻一叹然后默然的退开。
山巅上，绫清竹望着状若疯狂的林动，忍不住的将他抱住，红着眼睛。
“啊！”
林动紧抱着绫清竹，犹如孩子般的嚎啕大哭：“我找不回来她了，找不回来了，找不回来了！”
“你已经尽力了，我们都知道的。”
“我答应了她，要把她找回来的啊！”
眼泪不断的从他脸庞上流下来，他声音嘶哑。
绫清竹红着眼眶抱着他，她能够感觉到他心中那种到了极致的心痛。
他拯救了这个世界，却拯救不回心爱的人。
一道娇小的身影从远处而来，然后在山崖旁坐下，生死之主望着林动，这也是她第一次见到这个素来坚强的男人第一次这般的悲伤。
“干嘛要这么的贪心呢…你身边已经有一个值得你去爱的人了，把她忘了，不是更好吗？”生死之主轻声叹道。
林动缓缓的摇头，沙哑的道：“我会把她找回来的！”
即便绝望，可我依然不会放弃。
生死之主望着他那疲倦但却异常执着的脸庞，终是苦笑了一声，道：“真是拿你们没办法啊…看来小师妹最后的请求，我是没办法帮她完成了。
林动身体猛的一震，骤然抬头，死死的盯着生死之主。
“你是找不到小师妹的轮回碎片，是吧？”
生死之主抬头，望着遥远的地方，沉默了许久，方才道：“按照正常的情况，若是时间不长的话，即便是燃烧了轮回，以你现在的力量，的确能够找到轮回碎片，但这却是有着一种界限，那便是这个规则只对诞生于这位面的人有用…”
林动呆呆的望着生死之主，突然嘴都是因为心中的颤动而干涩了起来：“你是说？”
“嗯，小师妹并不是我们这位面的人…所以，你找不到她的轮回碎片。”
生死之主苦笑一声，抬起头，脑海之中，有着画面掠过，那是在应欢欢聚集力量冲击祖境的前一夜…
海岛之上，那笑容有些悲伤的女孩。
“大师姐，我想请你帮我一个忙。”海风吹拂而来，女孩那晶莹的长发飘舞起来，她轻声道。
“什么？”
“如果…我出了什么事情，请你帮我保管一个东西，而这东西，我希望你永远不要交给林动。”女孩美目望着东方，低声道。
“为什么？”
女孩沉默着，许久后，她那柔弱的双肩似是微微的颤抖起来，她轻轻的蜷起修长的双腿，将脸颊埋在膝间，哽咽的道：“因为我不想他因为我而变得遍体鳞伤，这个世界有他所爱的人，只要他能够将我忘记，他便是能够快乐，那条路或许会更为的艰难，我只想看见他笑，不想看见他在那条路上受伤，那样的话，我会很心疼的。”
生死之主怔怔的望着那道在夜色中不断颤抖的身影，眼睛也是忍不住的湿润下来。
生死之主望着呆呆的林动，苦笑道：“她倒是了解你，不过身在局中却是难以自清，她却不知道，忘记是有着多么的困难，特别是对于你这种倔性子的人。”
说着，她仲出小手，只见得在其手中有着一颗龙眼大小的雪白冰珠，那珠子中，有着一种惊人的寒意弥漫出来，而在那种寒气内，则是有着一些让林动心神颤抖的熟悉味道。
“这是她燃烧轮回后所剩下的，跟随着它，便是能够找到小师妹，不过，这需要你去那神秘的世界，那一路，或许会很困难，你，确定要吗？”生死之主盯着林动，道。
林动望着那散发着寒气的雪白冰珠，然后颤抖着手掌缓缓的接过，冰珠之上，寒气涌动，仿佛隐约能够看见一张巧笑焉熙的熟悉俏脸。
他深深的吸了一口气，那漆黑的眸子在此时爆发出了惊人的神采，熟悉的坚毅以及笑容，再度自那脸庞之上苏醒过来。
“放心吧，不论如何，我都会把她找回来的！”
上穷碧落下黄泉，不管你离得有多远，我都会把你抓回来，留在我的身边！
“我陪你去。”
绫清竹轻轻的握着林动的大手，嫣然微笑，犹如温暖的阳光，照进林动的心中深处：“若是没有我的陪伴，可不知道你这傻子会做出些什么事。”
林动反手紧握着绫清竹玉手，大笑道：“你当然得陪着我，就算不同意，我也得把你绑在我身边。”
那种失去的感觉，他不想再承受第二次。
“正好我对符祖他们所来的那世界好奇得很，正好趁此去看看，我林动的路，可没任何人能够阻拦下来！”
绫清竹望着那陡然间疲态尽扫，眉宇间显得豪迈的林动，也是微微一笑，那个熟悉的林动，又回来了。
“不过…”
林动突然似是想到什么，皱了皱眉，道：“想要去那新世界，我们并没有位面坐标，除非找到一个属于那个世界的人，方才能够以此为引，穿越位面，抵达那里。”
“那世界的人？”绫清竹想了想，道：“异魔皇？”
“不行，他已被我净化，现在怕是没了这功能。”林动摇了摇头，旋即一咬牙，道：“没事，我动用位面之眼探测一番，既然欢欢能够流落在这世界中，保不齐也会有第二个人同样流落进来。”
声音一落，林动已是动用位面之力，只见得其眉心混沌之光浮现，竟是化为一颗混沌之眼，那眼中仿佛包罗万千，光芒扫视间，飞快的自这天地间每一处角落掠过。
不过想要在这天地中寻找一个外世界的人并不容易，因此伴随着时间的推移，林动的眉头也是紧皱了起来。
而在林动探测着天地间时，后方也是有着数道身影掠来，小貂，青檀，炎主等人闪现出来，他们那突然间变得生龙活虎的林动，也是略感惊奇。
“林动哥，你没事了？”
青檀有些欣喜的道，之前见到林动那般模样，她也是难受得心都碎了。
“嗯，没事了。”
林动转过头，冲着青檀一笑，然后宠溺的拍了拍她的小脑袋，视线一转，那位面之眼便是继续去探测了。
不过，就在林动目光转过时，他的身体猛的僵硬了一瞬，那面色突然也是变得奇异起来，再然后，他一点点的转过头来，位面之眼所化的混沌之光，将青檀包裹。
而在那位面之眼的照射下，青檀娇躯上，似乎是有着许些神异的阴寒波动浮现，那种波动，似乎与这片天地的任何能量都是截然不同。
“林动哥，怎么了？”青檀望着林动那突然呆滞下来的面庞，也是一愣，低头打量了一下自己，茫然的问道。
林动却只是愣愣的看着她，一旁的绫清竹似乎也是察觉到什么有些难以置信的看向青檀，道：“莫非青檀她？”
林动狠狠的咽了一口唾沫，缓缓的点了点头。
青檀，竟然也是来自那世界的人…这世间之事，真是奇妙得令人难以想象¨半年之后青阳镇后山。
林动凌空而立，在那山崖边，林啸，柳妍，小貂等人皆是在场，他们望着天空，只见得那里的空间竟是在缓缓的撕裂开来，那种裂缝，与当初虚无之外出现的巨大位面裂缝一模一样。
“清竹，青檀。”
林动对着山崖上一招手，两道倩影应声掠出，一左一右的俏立在其两侧。
“大哥，等我们渡过三重轮回劫后，可也得带我们去看看！”小炎挥着手大声的道。
“哈哈，好，等我先去打打前站！”林动笑着应道，然后他袖袍一挥，八道光芒自其体内掠出，化为八道祖符其中一道祖符光芒闪烁间，化为一个小女孩的模样，正是慕灵珊。
“灵珊，这是新生的生死祖符，你将她放在体内温养，百年之后，便是能够再度诞生一道生死祖符，到时候，你也是能够脱离限制真正的化人。”林动屈指一弹，一道黑白之光冲进慕灵珊体内。
“林动大哥，你可要常回来啊！”慕灵珊挥着小手，娇憨的道。
林动笑着点点头，他已是这位面之主，要回来的话倒是很简单。
“爹，娘，我先走了。”林动目光看向林啸与柳妍，道。
“小子，你若是给我们带不回两个儿媳妇，就别回来了！”林啸大手一挥，笑骂一声，一旁的柳妍倒是嗔怪的盯了他一眼。
林动闻言也是一笑，望着那缓缓撕裂开来的位面裂缝，手掌紧握。
这一次，我一定会把你找到！
心念一动，林动拉着绫清竹与青檀，已是暴掠而出，位面之力将三人包裹，最后在那种人的注视之下，冲进了那位面裂缝之中。
众人望着那缓缓愈合的位面裂缝，也是长长一叹，惆怅之中，也是生出了一些对那新世界的好奇，那里，究竟有着什么？
乱魔海，炎神殿。
在那一座阁楼上，唐心莲望着遥远的东玄域方向，美目中掠过一抹复杂的神采。
“林动那家伙，应该已经离开这位面了吧，真是羡慕啊，我也好想去那里看看来着…”身后突然有着笑声传来，唐心莲偏过头看了摩罗一眼，却是未曾答话。
“唉，可怜我这得意弟子，单相思太苦了。”摩罗叹道。
“师傅，你胡说个什么呢！”唐心莲俏脸一红，恼羞成怒的道。
摩罗一笑，旋即无奈的道：“喜欢人干嘛不说啊。”
“喜欢他，也不用一定要跟他在一起啊。”
唐心莲明媚一笑，慵懒的伸着懒腰，衣裙勾勒出动人的曲线，她笑着道：“而且这世界上男人那么多，大不了我再喜欢一个不就是了。”
“男人的确是多，不过要比那小子更出色的，还真是很难很难。”摩罗想了想，认真的道：“而且，你真还会喜欢其他男人？”
“那可不一定哦。”唐心莲嫣然笑道：“喜欢我的男人都从乱魔海排到妖域去了，我可是很花心的人呢，才不会对谁从一而终呢。”
摩罗一笑，道：“那我们来打个赌，若是在他下次回来之前，你没有喜欢上别人，那你就去跟他说，如何？”
唐心莲脸一红，狠狠的瞪了他一眼，旋即咬了咬红唇，美目如水，妩媚一笑。
“好啊。”
位面穿梭。
一道光华掠过，在那虹光中，三道身影若隐若现，林动一手抓着青檀，细细的感应着青檀体内的那种来自那神秘世界的波动，而后调整着方向。
他们已是在位面中穿梭了将近一月时间。
“林动哥，我们还没到啊？”青檀看着四周那些位面洪流，无聊的问道。
“快了吧。”
林动笑了笑，神色突然一动，那穿梭的速度开始变缓下来，他的目光望着那极为遥远的地方，眉心处位面之眼浮现出来。
视线穿透过位面迷雾，突然在那遥远的地方，看见了三道人影，那也是一男两女，他们似乎是情侣，那中间的男子，一袭黑袍，在其背后，背负着一柄硕大的黑尺，在他的身体上，林动察觉到了一股极端炽热的波动。
而就在林动透过位面迷雾看见那黑袍男子时，后者似也是有所察觉，抬起头来，黑色眸子望向他所在的方向，而后脸庞上浮现一抹和善的微笑，并且隔空轻轻拱手。
林动见状，也是拱手一笑，却是并未再多留，袖袍一挥，带着绫清竹二人，穿梭而去，那神秘的世界，已经近在咫尺了。
在林动三人远去时，那遥远的地方，黑袍男子身旁的两位女子见到他的举动，那一名清雅极美的女孩微笑道：“萧炎哥哥，怎么了？”
“没什么，遇见一个很厉害的人，不知为何，与他挺投缘的，希望以后还能再见，我们也走吧。”
黑袍男子一笑，旋即不再多言，袖袍一挥，三人也是化为流光消逝而去，而他们去的方向，也是那新世界的所在。
那里，将会是最为精彩的地方。


结局感言以及新书
终于写完了。
结局出来后的争论，应该是少不了的，就如同当年斗破一般，怎么说呢，斗破的结局，其实已经是以一种很正统很完美的结束，我甚至是想不到在那之后还能再写什么，可是呢，从结束到现在，依旧会有人说是斗破烂尾。
而我对此一直都懒得去分辩。
想要做到满足所有人，是一件很困难的事情。
斗破与武动，是两个不同的故事，当在这之中，我们都获得了感动。
萧炎是一个比较完美的人，但林动却并不显得完美，但我却觉得他更加的真实，为了所爱之人，他会鲁莽，会自私，会愤怒，而他们又有着一个相同点，坚毅以及不放弃。
萧炎毕竟还拥有着远古萧族的身份，而林动，却至始至终一个普通的人，他从一个小小的青阳镇中走出来，披荆斩棘，负伤累累，其中的苦与累，血与火，唯有我们知道。
而也唯有我们方才知道他的那种无奈，他一直所修炼的目的，便是保护想要保护的人，但最终他却是不得不眼睁睁的看着应欢欢燃烧轮回，他的成功之路，较之萧炎，更为的艰难。
他的情感，比起萧炎，也更为的坎坷。
武祖较之炎帝，终归是少了一分潇洒，只不过，那却是无奈之举，因为他所背负的，也比萧炎更重。
武动的世界，比起斗破，多了一分无可奈何的残酷。
这毕竟是两个世界。
林动，也不是萧炎。
至于结局，同样也是包含着一丝无奈，但却又无可避免，他要去把她找回来，一如他所给予的那句约定。
上穷碧落下黄泉。
最终林动依旧是获得了希望，去往那更为遥远的神秘世界，在那里，他将会找回她。
武动两大女主的争论，即便是到结束的一刻都未曾停歇过，这即便是斗破中都未曾出现过，但无疑她们的血肉更为的丰满。
其实这样写，对我而言挺累的，想要写出这种支持率各占一半的女主，实在是要耗费不小的心思，不过这也算是一种挑战吧。
有人喜欢绫清竹那种安静之中的付出，不言不语中，自有温润情感流淌。
也有人喜欢初始应欢欢的那份活泼娇蛮以及后来化身为冰主那种冰冷之下的炽热感情，她从一开始便是猜到了会是什么样的结局，但正是这种明知之下的冰冷坚强，反而令人心疼。
林动努力的修炼着，遍体鳞伤的想要为自己所喜欢的人接过所有的责任，如果在斗破的世界，或许他会很成功，最终圆满。
但可惜，这里是武动。
不过执着的他，最终却依旧未曾放弃，希望，还是存在的。
下面便说说新书吧。
其实接下来的新书，在斗破完结时便已经是有了构想，只不过却并未立即开始写，因为那个世界太过的宏伟浩大，所以，我再写了一本武动，来作为其基石。
斗破大结局时，萧炎所去的新世界，与林动最终去往的神秘世界，是相同的，因为那个神秘的大世界，便是我们新书开始的地方。
当初并不太认为自己有驾驭那个世界的能力，而现在，终归是可以开始了。
那会是一个很精彩的世界。
只不过，那里的主角，不再是萧炎，也不再是林动，但无疑，他会极为的精彩。
或许在很久后，他会在新书之中突然遇见林动与萧炎，那时候的林动身旁，有了绫清竹与应欢欢，萧炎的的身边，有着薰儿与彩鳞，以他们的天赋以及能力，想来在那浩瀚的神秘新世界中，也是能够如鱼得水。
那里，会很精彩。
但是，那种精彩，属于我们新的主角。
我很期待。
新书名字，其实已经有读者知道了，嗯，【大主宰】，没错，这就是新书名字，那将会是一个崭新而精彩的故事。
不过现在不会马上开始写，我写书五年多了，还真没彻底的休息过一下，这一次，便真正的休息一下，暂时什么都不想，静静的把这脑子休养一下。
新书发布的时间，应该会是六月底七月初的样子，到时候，请让我再为大家奉献一个精彩的故事。
感谢大家陪伴，至此，武动谢幕。
希望在这个过程中，我们收获了欢笑与感动，有时候，看书，不就是寻求这么一个过程吗？
感谢大家。
请让我们，新书再见！

本文内容由【yuyu不在家】整理，久久小说网（www.txt99.com）转载。
附：【本作品来自互联网, 本人不做任何负责】内容版权归作者所有!*/
