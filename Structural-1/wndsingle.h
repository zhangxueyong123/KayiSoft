#ifndef WNDSINGLE_H
#define WNDSINGLE_H
#include <iostream>
#include <string>
#include <regex>
#include <map>
#include <cmath>
#include "qbuttongroup.h"
#include "qcheckbox.h"
#include "structuraldata.h"
#include "qlabel.h"
#include "qradiobutton.h"
#include "qscreen.h"
#include "qguiapplication.h"
#include "qcombobox.h"
#include "qgroupbox.h"
#include "qboxlayout.h"
#include "qscrollarea.h"

#include "mylayerwnd.h"
//#include "mycheckbox.h"
//#include "myradiobutton.h"
//#include "mylineedit.h"
#include "mycombobox.h"
#include "qsinglelinewidget.h"
#include "CLabelSplitLayout.h"
#include "mymeasureparameter.h"

//class MyLayerWnd;
class MyCheckBox;
//class MyComboBox;
class MyRadioButton;
class MyLineEdit;
//class MapNameAnldIdList;
//文本显示方向
enum eDataPostion
{
    eDataPostion_Left = 0,
    eDataPostion_Right,
};
//控件列表模板类 对应一组相关的控件
template <typename T>
struct stBoxList
{
    std::map<stTableKey, T*> mapControl;
    QButtonGroup *pButtonGroup = nullptr;
    int nLineMax = 4;
    CLabelSplitLayout *pLayout = nullptr;
    int nCols = 0;
    int nRows = 0;
    bool bShow = false;
    ~stBoxList()
    {
    }
    void SetExclusive(bool bExc)
    {
        //添加控件是否互斥
        if(pButtonGroup != nullptr)
        {
            pButtonGroup->setExclusive(bExc);
        }
    }
    void InitLayout(QWidget *parent,eDataPostion pos)
    {
        if(pLayout == nullptr)
        {
            pLayout = new CLabelSplitLayout();
            pLayout->setAlignment(pos == eDataPostion_Right ? Qt::AlignRight : Qt::AlignLeft );
            pButtonGroup = new QButtonGroup(parent);
        }
    }


    void Add2MeasureParameterWidget(const QString &strId, T *pWidget, const QString& strCheckTip)
    {
        if(strCheckTip.isEmpty() || pWidget == nullptr )
        {
            return;
        }
        auto pData = MyMeasureParameterManage::GetSingle();
        if(pData != nullptr)
        {
            pData->AddWidget(strCheckTip.toInt(), stMeasureParameterWidget(pWidget,strId));
        }
    }
    template <typename CALL>
    T *AddSingle(QWidget *parent, CALL callBack, const QString &strId, const QString &strText
                 , const QString &strPre, const QString &strSuff, bool bCheck, eDataPostion pos, const QString &strCheckTip,
                 const QString &strDataTypeSummary, const QString &strExplain,const QString & strFormula )
    {
        //添加一个控件
        InitLayout(parent, pos);
        auto itFind = mapControl.begin();
        while(itFind != mapControl.end())
        {
            if(itFind->first.strKey == strId)
            {
                break;
            }
            ++itFind;
        }
        T *pBox  = nullptr;
        if(itFind == mapControl.end())
        {
            //创建控件
            pBox = new T(strId,strText,strPre, strSuff,strDataTypeSummary, callBack, parent);
            pBox->setText(strText);
            pBox->SetExplain(strExplain);
            pBox->m_ExclusiveCall = std::bind(&stBoxList::SetExclusive, this,std::placeholders::_1);
           // pBox->m_strDataTypeSummary = strDataTypeSummary;
            pBox->blockSignals(true);
            pBox->setChecked(bCheck);
            pBox->blockSignals(false);
            pBox->setFormula(strFormula);
            if(!strPre.isEmpty())
            {
                pLayout->addWidget(pBox->m_pPreLabel);
                pBox->m_pPreLabel->setVisible(true);
            }
            pLayout->addWidget(pBox);
            if(pBox->m_label != nullptr)
            {
                pLayout->addWidget(pBox->m_label);
            }
            if(!strSuff.isEmpty())
            {
                pLayout->addWidget(pBox->m_pSuffLabel);
                pBox->m_pSuffLabel->setVisible(true);
            }
            if(pBox->m_bIsButton)
            {
                pButtonGroup->addButton((QAbstractButton*)pBox->GetPtr());
            }
            int nSize = (int)mapControl.size();
            Add2MeasureParameterWidget(strId, pBox, strCheckTip);
            mapControl[stTableKey(strId,nSize)] = pBox;
        }
        return pBox;
    }
    void Change(const QString &strId,  bool bCheck)
    {

    }
    bool Empty()
    {
        return mapControl.empty();
    }
    bool Find(const QString &strId)
    {
        for(auto &itMap : mapControl)
        {
            if(itMap.second->m_strId == strId)
            {
                return true;
            }
        }
        return false;
    }
    void Clear()
    {
        mapControl.clear();
    }
    int Size()
    {
        return (int)mapControl.size();
    }
    void setVisible(bool bVis)
    {
        bShow = bVis;
    }
    bool GetReportKeyValueState(std::vector<stTableState> *pVecState, bool bOnlyTrue , bool bSingleCheck, bool bIsEdit )
    {
        //获取控件状态
        QString strLabel;
        bool bCombobox = false;
        if(bSingleCheck)
        {
            for(auto &itMap : mapControl)
            {
                std::vector<stTableState> vecState = itMap.second->GetState();
                if(vecState.size() != 1)
                {
                    bCombobox = true;
                    break;
                }
                QString &strState = vecState.begin()->strState;
                if(strState == "true")
                {
                    strLabel = itMap.second->m_strTitle;
                    break;
                }
            }
        }
        for(auto &itMap : mapControl)
        {
            bool bAdd = false;
            std::vector<stTableState> vecState = itMap.second->GetState();
            if(!bOnlyTrue)
            {
                bAdd = true;
            }
            else if(bShow)
            {
                QString &strState = vecState.begin()->strState;
                if(vecState.size() == 1)
                {
                    if(!itMap.second->m_bIsButton && !strState.isEmpty())
                    {
                        bAdd = true;
                    }
                    else if(strState == "true")
                    {
                        bAdd = true;
                    }
                }
                else
                {
                    bAdd = false;
                    auto vecTmp = vecState;
                    vecState.clear();;
                    for(auto &itVec : vecTmp)
                    {
                        if(itVec.strState == "true")
                        {
                            bAdd = true;
                            vecState.push_back(itVec);
                        }
                    }
                }
            }
            if(bAdd)
            {
                if(!strLabel.isEmpty())
                {
                    pVecState->push_back(stTableState(strLabel,itMap.second->m_strId, itMap.second->isVisible(),bIsEdit));
                }
                else
                {
                    for(auto &itVec : vecState)
                    {
                        pVecState->push_back(itVec);
                    }
                }
            }

        }

        if(bOnlyTrue && !bShow)
        {
            return false;
        }
        return true;
    }
};
//延迟加载参数
template <typename CALL>
struct stBoxDelayPar
{
    QWidget *pParent = nullptr;
    CALL callBack = nullptr;
    QString strId;
    QString strText;
    QString strPre;
    QString strSuff;
    bool bCheck = false;
    eDataPostion dataPos;
    //    QString strCheckTable;
    QString strCheckTip;
    QString strDataTypeSummary;
    QString strExplain;
    QString strFormula;
};
//延迟加载控件列表模板类
template <typename T, typename CALL>
struct stBoxDelayList
{
    stBoxList<T> list;
    std::vector<stBoxDelayPar<CALL>> vecDelayPar;
    bool bIsInit = false;
    bool bExclusive = false;
    //  template <typename CALL>

    T *AddSingle(QWidget *parent, CALL call, const QString &id, const QString &text
                 , const QString &pre, const QString &suff, bool check, eDataPostion pos, bool bInit, const QString &strCheckTip,
                 const QString &strDataTypeSummary, const QString &strExplain,const QString &strFormula)
    {
        //添加一个控件
        if(bInit)
        {
            bIsInit = true;
            return list.AddSingle(parent, call, id, text, pre,suff, check, pos,strCheckTip ,strDataTypeSummary,strExplain, strFormula);
        }
        else
        {
            //暂不加载
            list.InitLayout(parent, pos);
            vecDelayPar.push_back(stBoxDelayPar<CALL>{parent, call,id, text, pre,suff, check, pos, strCheckTip,strDataTypeSummary,strExplain, strFormula});
            return nullptr;
        }
    }
    void setVisible(bool bVis)
    {
        if(!bIsInit)
        {
            return ;
        }
        list.setVisible(bVis);
    }
    void Init()
    {
        //此时加载控件
        if(bIsInit)
        {
            return ;
        }
        else if(!vecDelayPar.empty())
        {
            bIsInit = true;
            for(auto &itVec : vecDelayPar)
            {
                list.AddSingle(itVec.pParent, itVec.callBack, itVec.strId, itVec.strText, itVec.strPre,itVec.strSuff,
                               itVec.bCheck, itVec.dataPos, itVec.strCheckTip ,itVec.strDataTypeSummary, itVec.strExplain,itVec.strFormula);
            }
        }
    }
    void SetExclusive(bool bExc)
    {
        bExclusive = bExc;
        list.SetExclusive(bExc);
    }

    void Change(const QString &strId,  bool bCheck)
    {
        if(!bIsInit)
        {
            return;
        }
        list.Change(strId, bCheck);
    }
    bool Empty()
    {
        if(!bIsInit)
        {
            return false;
        }
        return list.Empty();
    }
    int Size()
    {
        return list.Size();
    }
    bool Find(const QString &strId)
    {
        if(!bIsInit)
        {
            return false;
        }
        return list.Find(strId);

    }
    void Clear()
    {
        if(!bIsInit)
        {
            return;
        }
        list.Clear();
    }
    bool GetReportKeyValueState(std::vector<stTableState> *pVecState, bool bOnlyTrue , bool bSingleCheck, bool bIsEdit )
    {
        if(!bIsInit)
        {
            return !bOnlyTrue;
        }
        return  list.GetReportKeyValueState(pVecState,bOnlyTrue, bSingleCheck,bIsEdit );
    }

    
    //T*GetItemAt(int index) 
    //{
    //    if (index < 0 || index >= mapControl.size()) 
    //    {
    //        return nullptr; // 索引无效
    //    }

    //    int currentIndex = 0;
    //    for (auto& item : mapControl)
    //    {
    //        if (currentIndex == index) 
    //        {
    //            return item.second; // 返回找到的控件指针
    //        }
    //        currentIndex++;
    //    }
    //    return nullptr; // 理论上不应该到达这里
    //}

    //T* GetItemAt(int index)
    //{
    //    if (!bIsInit || index < 0 || index >= list.Size()) {
    //        return nullptr; // 如果列表未初始化或索引无效，则返回nullptr
    //    }
    //    return list.GetItemAt(index); // 调用list的GetItemAt方法获取T类型实例
    //}

};

//控件列表管理类，对应列表一行数据
struct stBoxListCollection
{
    //延迟控件列表s
    stBoxDelayList<MyCheckBox,StateChangeCallBack> boxList;
    stBoxDelayList<MyRadioButton,StateChangeCallBack> radioList;
    stBoxDelayList<MyComboBox,ComboboxChangeCallBack> comboboxList;
    stBoxDelayList<MyLineEdit,StateChangeCallBack> lineEditList;
    //控件类型
    int dataType = eStructuralDataType_UnNow;
    //控件类型集合
    std::map<QString, eStructuralDataType> mapType;
    //子控件
    std::map<stTableKey, std::vector<stBoxListCollection>> childBox;
    //子控件标题
    QLabel *pChildLabel = nullptr;
    QString strRemarks /*= QStringLiteral("影像所见")*/;
    //控件布局
    QVBoxLayout *pSingleLayout = nullptr;
    QWidget *pSingleWidget = nullptr;
    bool bChangeDir = false;
    bool isShow = true;
    QString strChildDrawTitle;
    bool bFirst = true;
    //QString strExplain; //说明内容

    bool CheckId(const QString &strId)
    {
        return boxList.Find(strId) || radioList.Find(strId)  || comboboxList.Find(strId) ||lineEditList.Find(strId)  ;
    }
    void SetListVisible(bool bVis)
    {
        boxList.setVisible(bVis);
        radioList.setVisible(bVis);
        comboboxList.setVisible(bVis);
        lineEditList.setVisible(bVis);
    }
    void ShowChild(const QString &strId, bool bCheck)
    {
        //显示子控件
        bool bIsInThis = CheckId(strId);
        int nCul = 0;
        if(bIsInThis)
        {
            //初始化子控件
            if(bCheck)
            {
                InitChild();
            }
            auto itFind = mapType.find(strId);
            if(itFind == mapType.end())
            {
                return ;
            }
            //遍历子控件
            bool bSelectIsSingle = (itFind->second & eStructuralDataType_Single)== eStructuralDataType_Single;
            for(auto &itChildBox : childBox)
            {
                auto itFind = mapType.find(itChildBox.first.strKey);
                if(itFind == mapType.end())
                {
                    return ;
                }
                bool bSingle = (itFind->second & eStructuralDataType_Single)== eStructuralDataType_Single;
                if(!bSelectIsSingle)
                {
                    bSingle = false;
                }
                if(itChildBox.first.strKey == strId)
                {
                    for(auto &itBox : itChildBox.second)
                    {
                        itBox.pSingleWidget->setVisible(bCheck);
                        if(bCheck && itBox.bFirst)
                        {
                            itBox.pSingleWidget->setVisible(false);
                            itBox.pSingleWidget->setVisible(true);
                            itBox.bFirst = false;
                        }
                        itBox.SetListVisible(bCheck);
                    }
                    if(!bSingle)
                    {
                        break;
                    }
                }
                else if(bSingle)
                {
                    for(auto &itBox : itChildBox.second)
                    {
                        itBox.pSingleWidget->setVisible(false);
                        itBox.SetListVisible(false);
                    }
                }
                ++nCul;
            }
        }
        else
        {
            for(auto &itChildBox : childBox)
            {
                for(auto &itBox : itChildBox.second)
                {
                    itBox.ShowChild(strId, bCheck);
                }
            }
        }
    }
    stBoxListCollection()
    {
        InitSingleLayout();
    }
    void InitSingleLayout()
    {
        if(pSingleLayout == nullptr)
        {
            pSingleLayout = new QVBoxLayout();
        }
        if(pSingleWidget == nullptr)
        {
            pSingleWidget = new QWidget();
            pSingleWidget->setLayout(pSingleLayout);

            //  pSingleWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
        }
    }
    void AddChildLabel(const QString &strText)
    {
        InitSingleLayout();
        if(pChildLabel == nullptr)
        {
            pChildLabel = new QLabel();
            pChildLabel->setWordWrap(true);
            pSingleLayout->addWidget(pChildLabel);
        }
        pChildLabel->setText(strText);
    }
    void AddLayout(QLayout *pLayout)
    {
        InitSingleLayout();
        if (pLayout){
            pSingleLayout->addLayout(pLayout);
        }
    }
    void InitChild()
    {
        boxList.Init();
        radioList.Init();
        comboboxList.Init();
        lineEditList.Init();
        for(auto &itChildBox : childBox)
        {
            for(auto &itBox : itChildBox.second)
            {
                itBox.InitChild();
            }
        }
    }
    //添加参数是否递归生效子控件  zdy 20240416
    void AllBoxWork(bool bWithChild = false)
    {
        for(auto &itMap : boxList.list.mapControl)
        {
            itMap.second->m_bWait = false;
            emit itMap.second->stateChanged(itMap.second->checkState());

        }
        for(auto  &itMap : comboboxList.list.mapControl)
        {
            itMap.second->m_bWait = false;
            emit itMap.second->currentTextChanged(itMap.second->currentText());
        }
        for(auto &itMap :radioList.list.mapControl )
        {
            itMap.second->m_bWait = false;
            emit itMap.second->toggled(itMap.second->isChecked());

        }
        if(bWithChild)
        {
            for(auto &itChildBox : childBox)
            {
                for(auto &itBox : itChildBox.second)
                {
                    itBox.AllBoxWork(bWithChild);
                }
            }
        }

    }

    void SetDepartment(const QString &strDepartment, const QString & strBodyPart)
    {
        for(auto  &itMap : comboboxList.list.mapControl)
        {
            itMap.second->SetDepartment(strDepartment, strBodyPart);
            return;
            //itMap.second->
            //emit itMap.second->currentTextChanged(itMap.second->currentText());
        }
    }
    bool SetStateById(const QString &strId, const stTableState & strData, bool bDealMulti, bool bUnknowView)
    {
        bool bViewer = true;
        if(!bUnknowView)
        {
            bViewer = strData.bViewer;
        }
        if((dataType & eStructuralDataType_Single)== eStructuralDataType_Single)
        {
            for(auto &itMap : radioList.list.mapControl)
            {
                if(itMap.second->m_strId == strId)
                {
                    if(!bViewer)
                    {
                        return true;
                    }
                    if(strData.strState.isEmpty())
                    {
                        itMap.second->setChecked(true);
                    }
                    else
                    {
                        itMap.second->setChecked(strData.strState == QStringLiteral("true") || strData.strState == itMap.second->m_strTitle);
                    }
                    return true;
                }
            }
        }
        if((dataType & eStructuralDataType_Multi) == eStructuralDataType_Multi)
        {
            if(!bDealMulti)
            {
                return false;
            }
            for(auto &itMap : boxList.list.mapControl)
            {
                if(itMap.second->m_strId == strId)
                {
                    if(!bViewer)
                    {
                        return true;
                    }
                    itMap.second->setChecked(strData.strState == QStringLiteral("true"));
                    return true;
                }
            }
        }
        if((dataType & eStructuralDataType_Edit) == eStructuralDataType_Edit)
        {
            for(auto &itMap : lineEditList.list.mapControl)
            {
                if(itMap.second->m_strId == strId)
                {
                    if(!bViewer)
                    {
                        return true;
                    }
                    itMap.second->setText(strData.strState);
                    return true;
                }
            }
        }
        if((dataType & eStructuralDataType_Combobox) == eStructuralDataType_Combobox)
        {
            for(auto &itMap : comboboxList.list.mapControl)
            {
                for(auto &itMapId : itMap.second->m_mapComboboxId)
                {
                    if(itMapId.first == strId)
                    {
                        if(!bViewer)
                        {
                            return true;
                        }
                        if(strData.strState == QStringLiteral("true"))
                        {
                            itMap.second->setCurrentText(itMapId.second.strName);
                        }
                        return true;
                    }
                }
            }
        }

        for(auto &itChild : childBox)
        {
            for(auto &itBox : itChild.second)
            {
                if(itBox.SetStateById(strId, strData,bDealMulti,  bUnknowView))
                {
                    return true;
                }
            }
        }
        return false;
    }
    void GetSelect(std::vector<stSingleDrawData> *pVecData, std::vector<QString> *pVecId, const QString &strParentId,
                   QStringList *pCheckList, QStringList *pExplainList)
    {
        int nTypeNum = 0;
        int nStart = (int)pVecData->size();
        QString strFirstDraw = strChildDrawTitle ;
        if(!strFirstDraw.isEmpty())
        {
            strFirstDraw += ":";
        }
        auto FindChild = [](std::map<stTableKey, std::vector<stBoxListCollection>> &childBox, QString &strId)
        {
            auto itChild = childBox.begin();
            while(itChild != childBox.end())
            {
                if(itChild->first.strKey == strId)
                {
                    return itChild;
                }
                ++itChild;
            }
            return itChild;
        };
        if((dataType & eStructuralDataType_Single) == eStructuralDataType_Single)
        {
            std::vector<QString> vecSelect;
            std::vector<QString> vecId;
            for(auto &itMap : radioList.list.mapControl)
            {
                vecSelect.push_back(itMap.second->m_strTitle);
                vecId.push_back(itMap.second->m_strId);

            }
            for(auto &itMap : radioList.list.mapControl)
            {
                if(itMap.second->isChecked() )
                {
                    auto itChild = FindChild(childBox, itMap.second->m_strId);
                    if(itMap.second->m_strExplain.isEmpty() || pExplainList == nullptr)
                    {
                        QString strPre =  strFirstDraw + itMap.second->m_pPreLabel->m_strText;
                        strFirstDraw.clear();
                        QString strSuff = itMap.second->m_pSuffLabel->m_strText;
                        // if(itFind )

                        pVecData->push_back(stSingleDrawData(strPre, itMap.second->m_strTitle, strSuff, strRemarks, vecSelect,vecId,false,false,false,bChangeDir,
                                                             itChild != childBox.end()));
                        pVecId->push_back(itMap.second->m_strId);

                        if(MyMeasureParameterManage::GetSingle() != nullptr && pCheckList != nullptr)
                        {
                            QString strT = MyMeasureParameterManage::GetSingle()->CheckWidget(itMap.second->m_strId, itMap.second->m_strTitle);
                            if(!strT.isEmpty())
                            {
                                pCheckList->push_back(strT);
                            }
                        }

                    }
                    else
                    {
                        pExplainList->push_back(itMap.second->m_strExplain);
                    }
                    if(itChild != childBox.end())
                    {
                        for(auto &itBox : itChild->second)
                        {
                            if(itBox.pSingleWidget != nullptr && itBox.pSingleWidget->isVisible())
                            {
                                itBox.GetSelect(pVecData, pVecId,itChild->first.strKey,pCheckList,pExplainList);
                            }
                        }
                    }
                    break;
                }
            }
            ++nTypeNum;
        }
        if((dataType & eStructuralDataType_Multi) == eStructuralDataType_Multi)
        {
            std::vector<QString> vecSelect;
            std::vector<QString> vecId;
            for(auto &itMap : boxList.list.mapControl)
            {
                vecSelect.push_back(itMap.second->m_strTitle);
                vecId.push_back(itMap.second->m_strId);
            }
            bool bFirst = true;
            for(auto &itMap : boxList.list.mapControl)
            {
                if(itMap.second->checkState() == Qt::Checked)
                {
                    auto itChild = FindChild(childBox, itMap.second->m_strId);
                    if(itMap.second->m_strExplain.isEmpty() || pExplainList == nullptr)
                    {
                        QString strPre = strFirstDraw +  itMap.second->m_pPreLabel->m_strText;
                        strFirstDraw.clear();
                        QString strSuff = itMap.second->m_pSuffLabel->m_strText;

                        pVecData->push_back(stSingleDrawData(strPre,itMap.second->m_strTitle,strSuff,  strRemarks, vecSelect,vecId,false, true,bFirst, bChangeDir,
                                                             itChild != childBox.end()));

                        pVecId->push_back(itMap.second->m_strId);
                        if(MyMeasureParameterManage::GetSingle() != nullptr && pCheckList != nullptr)
                        {
                            QString strT = MyMeasureParameterManage::GetSingle()->CheckWidget(itMap.second->m_strId, itMap.second->m_strTitle);
                            if(!strT.isEmpty())
                            {
                                pCheckList->push_back(strT);
                            }
                        }
                    }
                    else
                    {
                        pExplainList->push_back(itMap.second->m_strExplain);
                    }

                    if(itChild != childBox.end())
                    {
                        if(itChild != childBox.end())
                        {
                            for(auto &itBox : itChild->second)
                            {
                                if(itBox.pSingleWidget != nullptr && itBox.pSingleWidget->isVisible())
                                {
                                    itBox.GetSelect(pVecData, pVecId,itChild->first.strKey,pCheckList,pExplainList);
                                }
                            }
                        }
                    }
                    bFirst = false;
                }
            }
            ++nTypeNum;
        }
        if((dataType & eStructuralDataType_Edit) == eStructuralDataType_Edit)
        {
            for(auto &itMap : lineEditList.list.mapControl)
            {
                QString str = itMap.second->text();
                QString strPre = strFirstDraw +  itMap.second->m_pPreLabel->m_strText;
                strFirstDraw.clear();
                QString strSuff = itMap.second->m_pSuffLabel->m_strText;

                auto itChild = FindChild(childBox, itMap.second->m_strId);
                pVecData->push_back(stSingleDrawData(strPre,str,strSuff, strRemarks,
                                                     std::vector<QString>(),std::vector<QString>(),
                                                     true,false,false,
                                                     bChangeDir,
                                                     itChild != childBox.end()));

                pVecId->push_back(itMap.second->m_strId);
                if(MyMeasureParameterManage::GetSingle() != nullptr && pCheckList != nullptr)
                {
                    QString strT = MyMeasureParameterManage::GetSingle()->CheckWidget(itMap.second->m_strId, str);
                    if(!strT.isEmpty())
                    {
                        pCheckList->push_back(strT);
                    }
                }
                if(itChild != childBox.end())
                {
                    if(itChild != childBox.end())
                    {
                        for(auto &itBox : itChild->second)
                        {
                            if(itBox.pSingleWidget != nullptr && itBox.pSingleWidget->isVisible())
                            {
                                itBox.GetSelect(pVecData, pVecId,itChild->first.strKey,pCheckList,pExplainList);
                            }
                        }
                    }
                }
            }
            if(!lineEditList.list.mapControl.empty())
            {
                auto itEnd = pVecData->end();
                --itEnd;
                itEnd->bIsEndEdit = true;
            }
            ++nTypeNum;
        }
        if(nTypeNum >= 2)
        {
            auto itData = pVecData->begin() + nStart;
            while(itData != pVecData->end())
            {
                itData->bReset = true;
                ++itData;
            }
        }
        if(!strParentId.isEmpty())
        {
            auto itData = pVecData->begin() + nStart;
            while(itData != pVecData->end())
            {
                itData->strParentId = strParentId;
                ++itData;
            }
        }
    }
    void Clear()
    {
        boxList.Clear();
        radioList.Clear();
        comboboxList.Clear();
        lineEditList.Clear();
    }
    void CloseAllMulti()
    {
        if((dataType & eStructuralDataType_Multi) == eStructuralDataType_Multi)
        {
            for(auto &itMap : boxList.list.mapControl)
            {
                itMap.second->setChecked(false);
            }
        }
    }
    bool GetReportKeyValueState(std::vector<stTableState> *pVecState, bool bOnlyTrue = false)
    {
        if(boxList.Size() == 0 && radioList.Size() == 0 && comboboxList.Size() == 0 && lineEditList.Size() == 0)
        {
            return true;
        }
        bool bRet = boxList.GetReportKeyValueState(pVecState,bOnlyTrue, false, false);
        bRet = bRet | radioList.GetReportKeyValueState(pVecState,bOnlyTrue,true, false);
        bRet = bRet | comboboxList.GetReportKeyValueState(pVecState,bOnlyTrue, false, false);
        bRet = bRet | lineEditList.GetReportKeyValueState(pVecState,bOnlyTrue, false, true);
        if(bRet)
        {
            for(auto & itChildBox : childBox)
            {
                for(auto &itBox : itChildBox.second)
                {
                    itBox.GetReportKeyValueState(pVecState, bOnlyTrue);
                }
            }
        }
        return bRet;
    }

    void ClearAllSelect()
    {
        for(auto &itMap : boxList.list.mapControl)
        {
            itMap.second->setChecked(false);

        }
        if(!radioList.list.mapControl.empty())
        {
            radioList.SetExclusive(false);
            for(auto &itMap : radioList.list.mapControl)
            {
                itMap.second->setChecked(false);
            }
            radioList.SetExclusive(true);
        }
        for(auto &itMap : lineEditList.list.mapControl)
        {
            itMap.second->setText("");
        }
    }
    void SetAllChildRemarks()
    {
        if(strRemarks.isEmpty())
        {
            return;
        }
        for(auto &child : childBox)
        {
            for(auto &itBox : child.second)
            {
                itBox.strRemarks = strRemarks;
                itBox.SetAllChildRemarks();
            }
        }
    }

    //eStructuralDataType GetItemType(QString id)
    //{
    //    eStructuralDataType type = eStructuralDataType_UnNow;
    //    for (auto it : boxList)
    //    {

    //    }
    //    return type;
    //}
};
struct stChildNodeDraw
{
    QString strTitle;
    QString strId;
    QString strRemarks;
    QString strOther;
    QString strShow;
    QString strShowTitle;
    std::vector<stStructuralNode > vecChild;
    eStructuralDataType eElementAttribute;
};
//结构化数据显示节点
class WndSingle/* : public QWidget*/
{
   // Q_OBJECT
public:
    explicit WndSingle(StructuralData *pData, UpdateDataCallBack updateCall,QWidget *parent = nullptr,QString parentId = "");
     ~WndSingle()
    {
    }
    //显示子序列
    void ShowChild(const QString &strId ,bool bShow, bool bMulti);
    //设置节点名
    bool SetLabel(QWidget *parent,const QString &strTitle);
    //创建空子控件
    void CreateEmpty(QWidget *parent,
                     const QString &strTitle,MyLayerWnd *pLayerWnd, int nDataPostion);
    //创建单选子控件
    QString CreateSingle(QWidget *parent,
                         bool bInit,
                         const QString &strTitle,
                         const std::vector<stStructuralNode> & vecNode,
                         bool bCreateChild,
                         const QString &strRemarks,
                         MyLayerWnd *pLayerWnd = nullptr,
                         stBoxListCollection *pBox = nullptr,
                         bool bUseTitle = true,
                         eDataPostion pos = eDataPostion_Left,
                         int nDataPostion = -1);
    //创建多选子控件
    QString CreateMulti(QWidget *parent,
                        bool bInit,
                        const QString &strTitle,
                        const std::vector<stStructuralNode> & vecNode,
                        bool bCheckState,
                        bool bCreateChild,
                        const QString &strRemarks,
                        MyLayerWnd *pLayerWnd= nullptr,
                        stBoxListCollection *pBox = nullptr,
                        bool bUseTitle = true,
                        eDataPostion pos = eDataPostion_Right,
                        int nDataPostion = -1);
    //创建下拉框子控件
    MyComboBox * CreateCombobox(QWidget *parent,
                                const QString &strId,
                                const std::vector<QString> &vecCombobox,
                                const std::vector<QString> & vecId,
                                const std::vector<QString> vecRemarks,
                                const std::vector<QString>& vecOther,
                                const std::vector<QString>& vecShow,
                                bool hasThrid,
                                int nDefulat,
                                bool bAddLayer2Wnd = true,
                                MapNameAndIdList *pVecNex  = nullptr,bool bNormal = true);
    //创建编辑框子控件
    QString CreateText(QWidget *parent,
                       bool bInit,
                       const QString &strTitle,
                       const std::vector<stStructuralNode> & vecNode,
                       const QString &strRemarks,
                       MyLayerWnd *pLayerWnd = nullptr,
                       stBoxListCollection *pBox = nullptr,
                       bool bUseTitle = true,
                       eDataPostion pos = eDataPostion_Left,
                       int nDataPostion = -1);
    //创建子节点
    void CreateChildNode(QWidget *parent,
                         const std::vector<stStructuralNode> &vecNode,
                         StructuralData &data,
                         WndSingle *pParentWnd,
                         UpdateDataCallBack callBack,
                         const QString & addTitle,
                         eStructuralDataType tableParentType, int nNowDeap, int nCheckDeap,const QString &strRemarks);
    //创建子列表
    void CreateChildTable(QWidget *parent,  StructuralData &data, const std::vector<QString> & vecId, const std::vector<QString> vecRemarks,
                          WndSingle *pParentWnd, UpdateDataCallBack callBack/*, const QString &strRemarks*/);
    //根据结构化数据构建控件
    void CrateByStructuralData(QWidget *parent, StructuralData &data, eTemplateType type);

    std::function<void(QString,QString, QString)> callback; // 存储回调函数

 // 设置回调函数
    void setCallback(const std::function<void(QString, QString, QString)>& cb) {
        callback = cb;
    }

    QString m_strId; //id
    QString m_strTitle; //标题
    QString m_strParentId; //父节点id
    //QString         m_childId;
    QHBoxLayout  *m_pLayout = nullptr; //布局
    std::vector< WndSingle *> m_vecChildWnd; //子节点  如左肺下的前段，上叶，用于演示这三个
    
    QSingleLineWidget *m_pLayerWidget = nullptr; //
    QSingleLineWidget *m_pLayerTableWidgt = nullptr;
    MyLayerWnd *m_pLayerWnd = nullptr;
    bool m_bAddChild = true;
    std::map<QString, std::map<stTableKey, WndSingle*>> m_childTable; //子列表节点  列表中的内容
    stBoxListCollection m_BoxListCollection; //控件管理类
    //设置回调
   // void SetCallBck(UpdateDataCallBack updateCall);
    //更新状态
    void UpdateByReportData(listReportData *pList);
    //设置单点状态
    bool SetCheckStateById(const QString &strId, const stTableState & strData, bool bCheckSelf, bool bDealMulti, bool bUnknowView );
    //清空
    void ClearAll();
    //获取列表显示状态
    bool GetReportKeyValueState(std::vector<stTableState> *pMapState, bool bOnlyTrue = false);
    bool GetReportKeyValueState(std::vector<stTableState> *pTableState, std::vector<stTableState> *pDictionaryState,
                                std::vector<stTableState> *pFirstAndSecondState,bool bOnlyTrue = false);
    //void GetReportChildTableKeyValue(std::map<QString, stTableState> *pMapState, bool bOnlyTrue);
    //关闭所有多选
    void CloseAllMulti(const QString &strRemarks);
    //获取状态json
    QString GetReportKeyValueJson();
    //获取结构化保存参数
    void GetSaveReportPar(stSaveReportPar &saveReportPar, StructuralData data,eTemplateType type, std::map<QString, stCallBack> &mapReport, bool bCheckStruct);
    //设置结构化json
    void SetReportStateByJson(const QString &strJson);
    //设置控件状态
    void SetReportStateByMap(const std::vector<stTableState> &vecState);
    void AllBoxWork();
    void SetDepartment(const QString &strDepartment, const QString & strBodyPart);
    void SetFinish(bool sw);
    void ClearAllSelect(const QString &strRemarks);
    bool m_bCanCallBack = true;
    static bool m_bIsUpadteReport;
    bool m_bIsCreateBySelf = false;
    QLabel *m_pLabel = nullptr;
    QWidget *m_pParent = nullptr;
    //    bool SetReportStateById(const QString &strId, const QString &strState);
    GetTemplateDataCallBack templateCallBack = nullptr;
    QString     m_currentFirst;
    QString     m_currentSecnd;

private:
    //设置子节点数据类型
    void SetAllChildRemarks();
    //创建子节点
    void CreateTableChild(QWidget *parent, const QString &strTitle, MyLayerWnd *pLayerWnd,const stStructuralNode &itVec,int pos,
                          stBoxListCollection *pBoxPtr, int nCul,const QString &strRemarks);
    void CreateControlByNodes(QWidget *parent, const QString &strTitle,MyLayerWnd *pLayerWnd, const std::vector<stStructuralNode> &vecNode,int nPos, WndSingle *pWnd
                              ,const QString &strRemarks
                              , stBoxListCollection *pBox =  nullptr);
    bool m_bChildIsConnectCombobox = false;
    UpdateDataCallBack m_UpdateDataCallBack = nullptr;
    //单选/多选/编辑框变化响应
    void StateChange(bool bCheck,const QString &strId, const QString &strTitle, bool bWait, bool* isNewChild,void *pPar);
    //下拉框变化响应
    void ComboboxChange(const QString &strChange,const QString &strId, const QString& parentId, const int& level,  bool* isNewChild,void *pPar);
    void InitLayout(QWidget *parent);
    //创建子列表
    WndSingle *CreateChildTable(QWidget *parent, StructuralData &data, const std::vector<QString> &vecCombobox,
                                const std::vector<QString> &vecId,
                                const std::vector<QString> vecRemarks,
                                const std::vector<QString>& vecOther,
                                const std::vector<QString>& vecShow,
                                UpdateDataCallBack callBack, bool bIsCombobox,  int nDefulat);

    bool LoadDictionaryData(const QString &strId, bool bCheck, bool bWait);
    StructuralData *m_pData = nullptr;
    QStringList templeteList;
    QString         m_currentTemplateText = "";
    bool        m_isFinish = false;
    //QEventLoop      eventLoop;
    QTimer          timer;
public:
};

class Parser {
public:
    Parser(const std::string& expr, std::map< std::string, std::string > map) : expression(expr), uuid_to_num(map),index(0) {}

    double parseExpression() {
        double result = parseTerm();
        while (index < expression.size() && (expression[index] == '+' || expression[index] == '-')) {
            char op = expression[index++];
            double term = parseTerm();
            if (op == '+') {
                result += term;
            }
            else {
                result -= term;
            }
        }
        return result;
    }

    double getResult()
    {
        std::regex bracket_regex("\\[([^\\]]+)\\]");  // 匹配方括号内的内容

        std::vector<std::string> results;  // 存储匹配结果的向量
        std::smatch match;  // 用于存储每次匹配的结果

        // 使用 std::sregex_iterator 迭代所有匹配项
        auto begin = std::sregex_iterator(expression.begin(), expression.end(), bracket_regex);
        auto end = std::sregex_iterator();

        for (std::sregex_iterator i = begin; i != end; ++i) {
            std::smatch match = *i;
            results.push_back(match[1].str());  // 将捕获的内容添加到结果向量中
        }

        for (auto it : results)
        {
            size_t pos = 0;
            while ((pos = expression.find(it, pos)) != std::string::npos) {
                expression.replace(pos, it.length(), uuid_to_num[it]);
                pos += uuid_to_num[it].length();  // 移动位置到新插入的字符串的末尾，避免替换刚替换的部分
            }
        }
        //移除中括号
        std::string str = "(66+77)/66*77";
        size_t pos = 0;
        while ((pos = expression.find("[", pos)) != std::string::npos) {
            expression.replace(pos, 1, "");
            pos++;  // 移动位置到新插入的字符串的末尾，避免替换刚替换的部分
        }
        pos = 0;
        while ((pos = expression.find("]", pos)) != std::string::npos) {
            expression.replace(pos, 1, "");
            pos++;  // 移动位置到新插入的字符串的末尾，避免替换刚替换的部分
        }
        //移除空格
        pos = 0;
        while ((pos = expression.find(" ", pos)) != std::string::npos) {
            expression.replace(pos, 1, "");
            pos++;  // 移动位置到新插入的字符串的末尾，避免替换刚替换的部分
        }
                
        double r = -1;
        r = parseExpression();
        return r;
    }
private:
    double parseTerm() {
        double result = parseFactor();
        while (index < expression.size() && (expression[index] == '*' || expression[index] == '/')) {
            char op = expression[index++];
            double factor = parseFactor();
            if (op == '*') {
                result *= factor;
            }
            else {
                result /= factor;
            }
        }
        return result;
    }

    double parseFactor() {
        double result = 0.0;
        double divisor = 1.0; // 用于处理小数点后的数字
        bool decimalPointEncountered = false; // 跟踪是否遇到小数点

        if (expression[index] == '(') {
            index++;
            result = parseExpression();
            index++; // 跳过右括号
        }
        else {
            while (index < expression.size() && (std::isdigit(expression[index]) || expression[index] == '.')) {
                if (expression[index] == '.') {
                    decimalPointEncountered = true;
                    index++; // 移过小数点
                }
                else {
                    int digit = expression[index++] - '0';
                    if (!decimalPointEncountered) {
                        result = result * 10 + digit;
                    }
                    else {
                        divisor *= 10.0;
                        result += digit / divisor;
                    }
                }
            }
        }
        return result;
    }

    std::string expression;
    std::map< std::string, std::string > uuid_to_num;
    size_t index;
};




#endif // WNDSINGLE_H
