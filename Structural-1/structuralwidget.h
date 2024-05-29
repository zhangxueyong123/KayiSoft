#ifndef STRUCTURALWIDGET_H
#define STRUCTURALWIDGET_H

#include <QWidget>
//#include "CNetWork.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "wndsingle.h"
#include "qscrollbar.h"
#include "contrl_center.h"
#include "mytextreportwidget.h"
namespace Ui {
class StructuralWidget;
}

enum eListDataType
{
    eListDataType_Structural = 0, //结构化列表数据
    eListDataType_Diagnosis, //诊断数据
};
static int kkkk = 0;
#include <QtConcurrent>
using namespace std::chrono;
static std::chrono::time_point<std::chrono::high_resolution_clock> start1;
static std::chrono::time_point<std::chrono::high_resolution_clock> end1;
//结构化数据窗口，每个类型一个

struct stTemplateWnd
{
    //内容显示子控件
    WndSingle *pWndSingle = nullptr;
    //内容显示子控件
    WndSingle* temp_pWndSingle = nullptr;
    //外围widget
    QWidget *pGroupBox = nullptr;
    //外围布局
    QVBoxLayout  *pLayout = nullptr;
    //滚动条
    QScrollArea *scrollArea =nullptr;
    bool bInit = false;
    static QString     m_currentPre ;
    //回调函数
    std::map<QString, stCallBack> mapCallBack;
    GetMeasureParameterCallBack measureParameterCallBack  = nullptr;
    GetExplainParameterCallBack explainParameterCallBack  = nullptr;
    GetTemplateDataCallBack templateCallBack = nullptr;
    bool bCanCallBackEmpty = false;
    listReportData m_LastList;
    void memberFuncB(QString  data, QString parent, QString child) {
        if (templateCallBack != nullptr/* && !vecCheckList.isEmpty()*/)
        {
            templateCallBack(&data,&parent,&child);
        }
    }
    void ShowWnd(bool bShow)
    {
        //显示窗口
        if(pGroupBox == nullptr)
        {
            return ;
        }
        pGroupBox->setVisible(bShow);
        scrollArea->setVisible(bShow);
    }
    void AddLayout( WndSingle *pWnd)
    {
        //start1 = high_resolution_clock::now();
        pGroupBox->setUpdatesEnabled(false);
        //添加子控件layout
        if(!pWnd->m_pLayerWnd->m_bIsEmpty)
        {
            pLayout->addWidget(pWnd->m_pLayerWnd->m_pGroupBox);
        }
        for(auto &itWnd : pWnd->m_vecChildWnd)
        {
            if(itWnd->m_bAddChild)
            {         
                pLayout->addWidget(itWnd->m_pLayerWnd->m_pGroupBox);
            }
            else
            {
                AddLayout(itWnd);
            }
        }
        pGroupBox->setUpdatesEnabled(true);
        pGroupBox->update();
        //end1 = high_resolution_clock::now();
        //auto duration = duration_cast<std::chrono::microseconds>(end1 - start1);
        //kkkk += duration.count();
        //qDebug() << "addWidget total spend times:" << kkkk;
    }

    void ClearAll()
    {
        //清空
        bInit = false;
        if(pGroupBox != nullptr)
        {
            delete pGroupBox;
            pGroupBox = nullptr;
        }
        if(pWndSingle != nullptr)
        {
            pWndSingle->ClearAll();
            delete pWndSingle;
            pWndSingle = nullptr;
            //Timer t;
            //t.StartTimer(1000, [=]() {          
            //    delete pWndSingle;
            //    pWndSingle = nullptr; 
            //    }
            //);
            //std::this_thread::sleep_for(std::chrono::seconds(4));
      
  
        }
    }
    
    void CrateByStructuralData(QWidget *parent, StructuralData &data, eTemplateType eType, bool bReset )
    {
        //根据结构化节点数据构建显示控件
        if(bReset)
        {
            ClearAll();
        }
        if(bInit)
        {
            return;
        }
        auto s = clock();
        //初始化布局相关控件
        if(scrollArea == nullptr)
        {
            scrollArea = new QScrollArea(parent);
            scrollArea->setWidgetResizable(true);
            scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
        if(pGroupBox == nullptr)
        {
            pGroupBox = new QWidget();
            pGroupBox->setObjectName("StructuralWidgetScrollAreaContents");
            pLayout = new QVBoxLayout();
            pLayout->setContentsMargins(0,0,0,0);
            pGroupBox->setLayout(pLayout);
        }
        if(pWndSingle == nullptr)
        {
            //初始化显示子控件
            pWndSingle = new WndSingle(&data,std::bind(&stTemplateWnd::GetReportData, this,std::placeholders::_1,std::placeholders::_2), parent);
            pWndSingle->setCallback(std::bind(&stTemplateWnd::memberFuncB, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        }
        bCanCallBackEmpty = false;
        auto e = clock();
        std::cout << "init:" <<  e - s << std::endl;
        //根据结构化节点数据构建显示控件
      //  pWndSingle->SetCallBck();
        pWndSingle->CrateByStructuralData(parent, data, eType);

        s = clock();
        std::cout <<"CrateByStructuralData:" <<  e - s << std::endl;
        AddLayout(pWndSingle);
        e = clock();
        std::cout <<"AddLayout:" <<  e - s << std::endl;
        pLayout->addStretch();
        s = clock();
        std::cout <<"addStretch:" <<  e - s << std::endl;

        start1 = high_resolution_clock::now();
       
        //int numcombo = pGroupBox->findChildren<MyComboBox*>().count();
        //int numline = pGroupBox->findChildren<MyLineEdit*>().count();
        //int numradio = pGroupBox->findChildren<MyRadioButton*>().count();
        //int numwid = pGroupBox->findChildren<QWidget*>().count();
        //int numtotal = numcombo + numline + numradio + numwid;
        //auto wids = pGroupBox->findChildren<QWidget*>();
        //for (int i = 100 ;i < wids.size();i++)
        //{
        //    wids[i]->hide();
        //    //QPixmap pixmap = wids[i] ->grab();
        //    //QString fileName = "C:\\temp\\widget\\img\\" + QString::number(i) + ".png";
        //    //pixmap.save(fileName);
        //}
       

        //if(numtotal < 500)
            scrollArea->setWidget(pGroupBox);
 
  
        //QPixmap pixmap = pGroupBox->grab();
        //QString fileName = "C:\\temp\\widget\\" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
        //pixmap.save(fileName);
      
        //while (!end.isFinished())
        //{
        //    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        //}
        
        end1 = high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(end1 - start1);
        qDebug() << "setWidget  spend times:" << duration.count();
        bInit = true;
        //设置回调，用于对MyTextReportWidget输出文本内容

        pWndSingle->AllBoxWork();

        
        s = clock();
        std::cout <<"AllBoxWork:" <<  e - s << std::endl;
        bCanCallBackEmpty = true;
        if(pGroupBox->isVisible())
        {
            GetReportData(nullptr, "");
        }
    }
    void Move(const QRect &rect, bool bSend)
    {
        //设置窗口位置
        if(pGroupBox == nullptr)
        {
            return;
        }
        pGroupBox->setGeometry(rect);
        scrollArea->setGeometry(rect);
        pGroupBox->updateGeometry();
        if(mapCallBack.empty()  && pGroupBox->isVisible() && bSend)
        {
            GetReportData(nullptr, "");
        }
    }
//    void SetBkColor(int nR, int nG, int nB)
//    {
//        //设置窗口背景色
//        if(pGroupBox == nullptr)
//        {
//            return;
//        }
//        pGroupBox->setStyleSheet(QString("QGroupBox {background-color: rgb(%1,%2,%3)}").arg(nR).arg(nG).arg(nB) );
//    }
    void GetReportData(listReportData *pList, const QString &strChangId)
    {
        //用于对MyTextReportWidget输出文本内容
        if(pGroupBox == nullptr)
        {
            return;
        }
        auto pLayOut = pGroupBox->layout();

        listReportData data;
        if(pList == nullptr)
        {
            pList = &data;
        }

        QStringList vecCheckList;
        QStringList vecExplainList;
        //获取子控件状态
        GetReportInLayout(pLayOut, pList, &vecCheckList, &vecExplainList);

        if(measureParameterCallBack != nullptr/* && !vecCheckList.isEmpty()*/)
        {
            measureParameterCallBack(&vecCheckList);
        }
        if(explainParameterCallBack != nullptr/* && !vecCheckList.isEmpty()*/)
        {
            explainParameterCallBack(&vecExplainList);
        }

        //通过回调函数输出
        for(auto &itMap : mapCallBack)
        {
            listReportData singleList;
            auto itList = pList->begin();
            while(itList != pList->end())
            {
                if(itList->strRemarks == itMap.first)
                {
                    singleList.push_back(*itList);
                }
                ++itList;
            }
            
            if(!bCanCallBackEmpty && singleList.empty())
            {
                continue;
            }
            if(itMap.second.updateDataCallBack != nullptr )
            {
                ModifyComma(&singleList);
                itMap.second.updateDataCallBack(&singleList, strChangId);
;            }
        }
    }
    bool VQStringContains(std::vector<QString> vec,QString str)
    {
        bool isHas = false;
        for (auto it : vec)
        {
            if (str == it)
                isHas = true;
        }
        return isHas;
    }

    void ModifyComma(listReportData * List)
    {
        auto it = List->begin();
        while (it != List->end())
        {
            auto vecList = it->drawData.vecId;
            int vecSize = vecList.size();
            if (vecSize > 1)
            {
                for (int i = 0; i < vecSize - 1; i++,it++)
                {
                    
                    if (it == List->end() || (it + 1) == List->end())
                    {
                        it++;
                        break;
                    }
                        
                    if (VQStringContains(vecList, it->strId) && VQStringContains(vecList, (it + 1)->strId))
                    {
                        it->drawData.strDrawSuffAdd = "，";
                    }
                    else
                    {
                        it++;
                        break;
                    }
                }
            }
            else
            {
                it++;
            }
        }
    }

    void UpdateByReportData(listReportData *pList)
    {
        //根据列表状态修改控件状态
        if(pWndSingle == nullptr)
        {
            return;
        }
        pWndSingle->UpdateByReportData(pList);
    }
    void SetCallBck(const QString &strTitle, const stCallBack &callBack/*UpdateDataCallBack updateCall*/)
    {
        //设置回调
        mapCallBack[strTitle] = callBack;
       // callBack = updateCall;
    }
    void ClearCallBack()
    {
        //清空回调
        mapCallBack.clear();
    }
    void GetReportInLayout(QLayout *pLayOut, listReportData *pList,QStringList *pCheckList, QStringList *pExplainList)
    {
        //获取子控件状态
        if(pLayOut != nullptr )
        {
            //遍历子控件
            int nCount = pLayOut->count();
            
            for(int i = 0; i < nCount; ++i)
            {
                auto it = pLayOut->itemAt(i);
                auto childWidget = it->widget();
                QLayout *childLayout = nullptr;
                if(childWidget != nullptr)
                {
                    std::string strName = typeid(*childWidget).name();
                    //QSingleLineWidget为自定义控件窗口
                    if((int)strName.find("QSingleLineWidget") >= 0)
                    {
                        QSingleLineWidget *pSingleWidget = ( QSingleLineWidget *)childWidget;
                        if(pSingleWidget->m_bIsTable && pSingleWidget->isVisible())
                        {
                            //获取子控件状态
                            GetReportInSingleWidget(pSingleWidget, pList, pCheckList, pExplainList);
                            continue ;
                        }
                    }

                    childLayout = childWidget->layout();
                }
                if(childLayout != nullptr)
                {
                    GetReportInLayout(childLayout, pList, pCheckList, pExplainList);
                }
            }
        }
    }
    static void GetReportInSingleWidget(QSingleLineWidget *pSingleWidget, listReportData *pList,QStringList *pCheckList, QStringList *pExplainList)
    {

        //获取子控件状态
        WndSingle *pWnd = (WndSingle *)pSingleWidget->m_pWndSingle;
        auto strParentTitle = pWnd->m_strTitle;
        m_currentPre = "";
        if(!pWnd->m_childTable.empty())
        {
            bool bHas = false;
            int nSize = pList->size();
            //遍历子控件列表
            for(auto &itMap : pWnd->m_childTable)
            {
                for(auto &itWnd : itMap.second)
                {
                    if(!(itWnd.second->m_pLayerWidget != nullptr/* && itWnd.second->m_pLayerWidget->isVisible()*/) ||  !itWnd.second->m_BoxListCollection.isShow )
                    {
                        continue;
                    }
                    std::vector<stSingleDrawData> vecTitle;
                    std::vector<QString> vecId;
                    //获取子控件的选择状态
                    itWnd.second->m_BoxListCollection.GetSelect(&vecTitle,&vecId,"",pCheckList,pExplainList);
                    if(!vecTitle.empty())
                    {
                        bHas = true;
                        stReportData data;
                        auto itVecTitle = vecTitle.begin();
                        auto itVecId = vecId.begin();
                        QString strKey = itWnd.first.strKey;
                        while(itVecTitle != vecTitle.end() && itVecId != vecId.end())
                        {
                            data.drawData = *itVecTitle;
                            bool isFirst = false;
                            if (m_currentPre != strParentTitle)
                            {
                                m_currentPre = strParentTitle;
                                isFirst = true;
                            }
                            //去除重复标题，并且对设置成段落的样式
                            QString isFirstP = isFirst ? ("\n" + strParentTitle + (strParentTitle.contains("：") || strParentTitle.contains(":") ? "" : "：")) + "\n    " : "";

                            if(!itVecTitle->bChangDir)
                            {

                                    
                                //data.drawData.strDrawPre = (isFirst ? (strParentTitle + (strParentTitle.contains("：") ? "" : "：")) : "") + (strKey == strParentTitle + "：" ? "" : strKey) + itVecTitle->strDrawPre + (isFirst ? "：" : "");

                                //data.drawData.strDrawPre = isFirstP + (strKey == strParentTitle + "：" ? "" : (strKey.contains("：") ? strKey : strKey + "：")) + itVecTitle->strDrawPre;
                                data.drawData.strDrawPre = isFirstP + (strKey == strParentTitle ? "" : strKey) + itVecTitle->strDrawPre;
                                data.drawData.strDrawSuff = itVecTitle->strDrawSuff;
                            }
                            else
                            {
                                data.drawData.strDrawPre = isFirstP + itVecTitle->strDrawPre;
                                data.drawData.strDrawSuff = itVecTitle->strDrawSuff + strKey;
                            }

                            strKey.clear();
                            //strParentTitle.clear();
                            data.strId = *itVecId;
                            data.strRemarks = itVecTitle->strRemarks;
                            //data.strPreId = "";
                            pList->push_back(data);
                            ++itVecTitle;
                            ++itVecId;
                        }


                    }
                }
            }

            if(bHas)
            {
                //添加后缀补充数据
                auto itBegin = pList->begin() + nSize;
                itBegin->drawData.bSingleLineFirst = true;
                auto itEnd =pList->end();
                --itEnd;
                while(itBegin != pList->end())
                {
                    if(itBegin == itEnd)
                    {
                        itBegin->drawData.strDrawSuffAdd = /*"。\n";*/"。";
                    }
                    else if(itBegin->drawData.bHasChild)
                    {
                         //itBegin->drawData.strDrawSuff += ":";
                         itBegin->drawData.strDrawSuffAdd = "：";
                    }
                    else if(!itBegin->drawData.bEdit || (itBegin->drawData.bEdit && itBegin->drawData.bIsEndEdit))
                    {
                        //itBegin->drawData.strDrawSuff += ",";
                        itBegin->drawData.strDrawSuffAdd = "； ";
                    }

                    ++itBegin;
                }
                //itEnd->bEnd = true;
            }
        }

    }

   void  GetReportKeyValueState( std::vector<stTableState> *pVecState)
    {
       if(pWndSingle == nullptr)
       {
           return;
       }
        pWndSingle->GetReportKeyValueState(pVecState);
    }
   QString GetReportKeyValueJson()
   {
       if(pWndSingle == nullptr)
       {
           return "";
       }
      return  pWndSingle->GetReportKeyValueJson();
   }
    void GetSaveReportPar(stSaveReportPar &saveReportPar, StructuralData &data, eTemplateType eType, bool bCheckStruct)
    {
        if(pWndSingle == nullptr)
        {
            return ;
        }
         pWndSingle->GetSaveReportPar(saveReportPar,data,eType, mapCallBack,bCheckStruct);

    }
   void SetReportStateByJson( const QString &strJson)
   {
       if(pWndSingle == nullptr)
       {
           return ;
       }
      return  pWndSingle->SetReportStateByJson(strJson);
   }
   void SetReportStateByMap( const std::vector<stTableState> &vecState)
   {
       if(pWndSingle == nullptr)
       {
           return ;
       }
      return  pWndSingle->SetReportStateByMap(vecState);
   }


   void SetDepartment(const QString &strDepartment, const QString & strBodyPart)
   {
       if(pWndSingle != nullptr)
       {
           pWndSingle->SetDepartment(strDepartment, strBodyPart);
       }
   }

   void setFinish(bool sw)
   {
       if (pWndSingle != nullptr)
       {
           pWndSingle->SetFinish(sw);
       }
   }

};
 
//结构化窗口，包含三个类型结构化数据窗口
#include <QVector>
static int count = 0;
struct stStructuralWnd
{
    stTemplateWnd m_RATemplateWnd;  //放射
    stTemplateWnd m_USTemplateWnd; //超声
    stTemplateWnd m_ESTemplateWnd; //内镜
    QString m_strDepartment;
    QString m_strBodyPart;
    eTemplateType m_eViewType = eTemplateType_Null;
    QString m_templateName;
    QMap<QString, int>    m_templateMap;
    void InitCallBaCk(GetMeasureParameterCallBack measureParameterCallBack, GetExplainParameterCallBack explainParameterCallBack, GetTemplateDataCallBack templateDataCallBack)
    {
        m_RATemplateWnd.measureParameterCallBack = measureParameterCallBack;
        m_USTemplateWnd.measureParameterCallBack = measureParameterCallBack;
        m_ESTemplateWnd.measureParameterCallBack = measureParameterCallBack;

        m_RATemplateWnd.explainParameterCallBack = explainParameterCallBack;
        m_USTemplateWnd.explainParameterCallBack = explainParameterCallBack;
        m_ESTemplateWnd.explainParameterCallBack = explainParameterCallBack;

        m_RATemplateWnd.templateCallBack = templateDataCallBack;
        m_USTemplateWnd.templateCallBack = templateDataCallBack;
        m_ESTemplateWnd.templateCallBack = templateDataCallBack;   
    }
    void EnableAll()
    {
        m_RATemplateWnd.ShowWnd(false);
        m_USTemplateWnd.ShowWnd(false);
        m_ESTemplateWnd.ShowWnd(false);
    }
   
    void CrateByStructuralData(QWidget *parent, StructuralData &data, bool bReset, bool bDeleteDelay)
    {
        if(MyMeasureParameterManage::GetSingle() != nullptr)
        {
            MyMeasureParameterManage::GetSingle()->ClearAll();
        }
        if(bDeleteDelay)
        {
            EnableAll();
            Clear();
        }
        if (count == 0)
        {
            m_RATemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRRa, bReset);
            m_USTemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRUS, bReset);
            m_ESTemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRES, bReset);
        }
        else
        {
            switch (m_eViewType)
            {
            case eTemplateType_SRRa:
                m_RATemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRRa, bReset);
                break;
            case eTemplateType_SRUS:
                m_USTemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRUS, bReset);
                break;
            case eTemplateType_SRES:
                m_ESTemplateWnd.CrateByStructuralData(parent, data, eTemplateType_SRES, bReset);
                break;
            default:
                break;
            }
        }
        count = (count == 0 ? 1 : 0);

        if(bDeleteDelay)
        {
            SetDepartment(eTemplateType_SRRa, m_strDepartment, m_strBodyPart);
            SetDepartment(eTemplateType_SRUS, m_strDepartment, m_strBodyPart);
            SetDepartment(eTemplateType_SRES, m_strDepartment, m_strBodyPart);
            if(m_eViewType != eTemplateType_Null)
            {
                SetViewer(m_eViewType);
            }
        }

    }
    void  SetViewer(eTemplateType eViewerType)
    {
        m_eViewType = eViewerType;
        m_RATemplateWnd.ShowWnd(eViewerType == eTemplateType_SRRa);
        m_USTemplateWnd.ShowWnd(eViewerType == eTemplateType_SRUS);
        m_ESTemplateWnd.ShowWnd(eViewerType == eTemplateType_SRES);
    }
    void Move(const QRect &rect, bool bSend)
    {
        m_RATemplateWnd.Move(rect,bSend);
        m_USTemplateWnd.Move(rect,bSend);
        m_ESTemplateWnd.Move(rect,bSend);
    }
//    void SetBkColor(int nR, int nG, int nB)
//    {
//        m_RATemplateWnd.SetBkColor(nR, nG, nB);
//        m_USTemplateWnd.SetBkColor(nR, nG, nB);
//        m_ESTemplateWnd.SetBkColor(nR, nG, nB);
//    }

    void GetReportData(eTemplateType eViewerType,listReportData *pList)
    {

        switch (eViewerType) {
        case eTemplateType_SRRa:    m_RATemplateWnd.GetReportData(pList, "");break;
        case eTemplateType_SRUS:    m_USTemplateWnd.GetReportData(pList, "");break;
        case eTemplateType_SRES:    m_ESTemplateWnd.GetReportData(pList, "");break;
        default:
            break;
        }
    }
     void SetDepartment(eTemplateType eViewerType,const QString &strDepartment, const QString & strBodyPart)
     {
         m_strDepartment = strDepartment;
         m_strBodyPart = strBodyPart;
         switch (eViewerType) {
         case eTemplateType_SRRa:   m_RATemplateWnd.SetDepartment(strDepartment, strBodyPart);break;
         case eTemplateType_SRUS:   m_USTemplateWnd.SetDepartment(strDepartment, strBodyPart);break;
         case eTemplateType_SRES:   m_ESTemplateWnd.SetDepartment(strDepartment, strBodyPart);break;
         default:
             break;
         }
     }
    void UpdateByReportData(eTemplateType eViewerType,listReportData *pList)
    {
        switch (eViewerType)
        {
        case eTemplateType_SRRa:    m_RATemplateWnd.UpdateByReportData(pList);break;
        case eTemplateType_SRUS:    m_USTemplateWnd.UpdateByReportData(pList);break;
        case eTemplateType_SRES:    m_ESTemplateWnd.UpdateByReportData(pList);break;
        default:
            break;
        }
    }
    void SetCallBck(const QString &strTitle,const stCallBack &callBack/*UpdateDataCallBack updateCall*/)
    {
        m_RATemplateWnd.SetCallBck(strTitle, callBack);
        m_USTemplateWnd.SetCallBck(strTitle, callBack);
        m_ESTemplateWnd.SetCallBck(strTitle, callBack);
    }
    void ClearCallBack()
    {
        m_RATemplateWnd.ClearCallBack();
        m_USTemplateWnd.ClearCallBack();
        m_ESTemplateWnd.ClearCallBack();
    }
   void  GetReportKeyValueState(eTemplateType eViewerType, std::vector<stTableState> *pVecState)
    {
        switch (eViewerType) {
        case eTemplateType_SRRa:    m_RATemplateWnd.GetReportKeyValueState(pVecState);break;
        case eTemplateType_SRUS:    m_USTemplateWnd.GetReportKeyValueState(pVecState);break;
        case eTemplateType_SRES:    m_ESTemplateWnd.GetReportKeyValueState(pVecState);break;
        default:
            break;
        }
    }
    QString GetReportKeyValueJson(eTemplateType eViewerType)
    {
        switch (eViewerType) {
        case eTemplateType_SRRa:    return m_RATemplateWnd.GetReportKeyValueJson();break;
        case eTemplateType_SRUS:    return m_USTemplateWnd.GetReportKeyValueJson();break;
        case eTemplateType_SRES:    return m_ESTemplateWnd.GetReportKeyValueJson();break;
        default:
            break;
        }
        return "";
    }
    void GetSaveReportPar(eTemplateType eViewerType, stSaveReportPar &saveReportPar, StructuralData &data, bool bCheckStruct)
    {
        switch (eViewerType) {
        case eTemplateType_SRRa:    m_RATemplateWnd.GetSaveReportPar(saveReportPar,data, eViewerType,bCheckStruct);break;
        case eTemplateType_SRUS:    m_USTemplateWnd.GetSaveReportPar(saveReportPar,data, eViewerType,bCheckStruct);break;
        case eTemplateType_SRES:    m_ESTemplateWnd.GetSaveReportPar(saveReportPar,data, eViewerType,bCheckStruct);break;
        default:
            break;
        }
        //return "";
    }
    void SetReportStateByJson(eTemplateType eViewerType, const QString &strJson)
    {
        switch (eViewerType) {
        case eTemplateType_SRRa:    m_RATemplateWnd.SetReportStateByJson(strJson);break;
        case eTemplateType_SRUS:    m_USTemplateWnd.SetReportStateByJson(strJson);break;
        case eTemplateType_SRES:    m_ESTemplateWnd.SetReportStateByJson(strJson);break;
        default:
            break;
        }
    }
    void SetReportStateByMap(eTemplateType eViewerType, const std::vector<stTableState> &vecState)
    {
        switch (eViewerType) {
        case eTemplateType_SRRa:    m_RATemplateWnd.SetReportStateByMap(vecState);break;
        case eTemplateType_SRUS:    m_USTemplateWnd.SetReportStateByMap(vecState);break;
        case eTemplateType_SRES:    m_ESTemplateWnd.SetReportStateByMap(vecState);break;
        default:
            break;
        }
    }


    bool IsInit(eTemplateType viewerType)
    {
        switch (viewerType) {
        case eTemplateType_SRRa:    return m_RATemplateWnd.bInit;break;
        case eTemplateType_SRUS:    return m_USTemplateWnd.bInit;break;
        case eTemplateType_SRES:    return m_ESTemplateWnd.bInit; break;
        case eTemplateType_All:     return m_RATemplateWnd.bInit && m_USTemplateWnd.bInit && m_ESTemplateWnd.bInit  ;  break;
        default:
            break;
        }
        return false;
    }

    //void getTemplateInfo(eTemplateType viewerType,QString &first,QString&second)
    //{
    //    switch (viewerType) 
    //    {
    //        case eTemplateType_SRRa:
    //        {
    //            first =  m_RATemplateWnd.pWndSingle->m_currentFirst; 
    //            second = m_RATemplateWnd.pWndSingle->m_currentSecnd;
    //            break;
    //        }    
    //        case eTemplateType_SRUS:
    //        {
    //            first =  m_USTemplateWnd.pWndSingle->m_currentFirst;
    //            second = m_USTemplateWnd.pWndSingle->m_currentSecnd;
    //            break;
    //        }
    //        case eTemplateType_SRES:
    //        {
    //            first =  m_ESTemplateWnd.pWndSingle->m_currentFirst;
    //            second = m_ESTemplateWnd.pWndSingle->m_currentSecnd;
    //            break;
    //        } 
    //        case eTemplateType_All:
    //        {
    //            //return m_RATemplateWnd[m_nCul].bInit && m_USTemplateWnd[m_nCul].bInit && m_ESTemplateWnd[m_nCul].bInit;  break;
    //        }
    //        default:
    //            break;
    //    }
    //    return ;
    //}

    void Clear()
    {
        m_RATemplateWnd.ClearAll();
        m_USTemplateWnd.ClearAll();
        m_ESTemplateWnd.ClearAll();
    }

    void setFinish(bool sw)
    {
        m_RATemplateWnd.setFinish(sw);
        m_USTemplateWnd.setFinish(sw);
        m_ESTemplateWnd.setFinish(sw);
    }
};
struct stDepartmentAndBodyPart
{
    QString strDepartment;
    QString strBodyPart;
    bool operator == (const stDepartmentAndBodyPart &par) const
    {
        return strDepartment == par.strDepartment &&
                strBodyPart == par.strBodyPart  ;
    }
};

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
//结构化模板窗口
class StructuralWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StructuralWidget(const QString &strUrl,
                              CNetworkAccessManager* network,
                              QWidget *parent = nullptr,
                              eTemplateType eNowTemplateType = eTemplateType_Null,
                              eVersionType eApiVersion = eVersionType_V1);
    //初始化结构
    void InitStructuralData(/*const QString &strDepartment, const QString & strBodyPart*/);
    //部门只可设置1个，部位可以设置多个，多个部位之间用,隔开
    void SetDepartment(const QString &strDepartment, const QString & strBodyPart/*, bool bUpdate*/);
    //设置病人相关参数
    void SetPatientPar(const QString & systemCode, const QString & hospitalCode, const QString & sex, const QString & strAge);

    //修改http-url
    void UpdateUrl(const QString &strUrl)
    {
        m_strUrl = strUrl;
    }
    //修改显示窗口
    bool SetViewerType(eTemplateType eViewerType, bool bReLoad,const QString &strFirstId, const QString &strSecondId, const QString &strJson, bool bSingle);
    
    //获取结构化列表数据
    void GetReportData(listReportData *pList)
    {
        m_StructuralWnd.GetReportData(m_eViewerType, pList);
    }
    ~StructuralWidget();
    //设置数据刷新回调
    void SetCallBck(const QString &strTitle, stCallBack callBack/*UpdateDataCallBack updateCall*/)
    {
        m_StructuralWnd.SetCallBck(strTitle, callBack );
    }
    void ClearCallBack();
    //使用结构化数据更新列表
    void UpdateByReportData(listReportData *pList)
    {
        m_StructuralWnd.UpdateByReportData(m_eViewerType, pList);
    }
    //根据已加载数据刷新页面
    void ResetTreeData();
    //重新加载数据数据刷新页面
    void ReloadTreeDataByNet(eTemplateType viewerType );
    void ReloadTreeDataByNet();
    //根据json数据刷新页面
   // void ReloadTreeDataByJson(/*eTemplateType viewerType, */const QString &strJson, bool bSingle);
    void ReloadTreeDataByJson(const QString& strFirstId, const QString& strSecondId, const QString& strJson, bool bSingle);
    //获取keyvalue数据
    void GetReportKeyValueState( std::vector<stTableState> *pVecState);
    //获取json数据
    QString GetReportKeyValueJson();


    //根据json刷新状态
    void SetReportStateByJson(const QString &strJson);
    void SetReportStateByValue(const std::vector<stTableState> &vecState);
    bool IsInit(eTemplateType viewerType);
    void Clear(bool bWithData);
    eTemplateType GetTemplateType() { return m_eTemplateType; }
     //获取类型 需要在SetViewerType等方法之后
    std::vector<QString> GetDataTypeList();  //返回有哪几种类型  /临床评估/技术评估

    //***获取结构化保存数据***//
    //获取保存结构
    void GetSaveReportPar(stSaveReportPar &saveReportPar, eTemplateType type, bool bCheckStruct = true );
    //获取保存json
    QString GetSaveReportJson( eTemplateType type = eTemplateType_Null);
    //获取QJsonObject
    QJsonObject GetSaveReportPar( eTemplateType type = eTemplateType_Null);

    //根据json数据刷新页面
  //
    void UpdateTreeDataStateByJson(const QString &strJson);
     //************************//
   // void LoadStructuralDataByJson();
    /******模板相关********/
    //保存模板
    void SaveTemplate(const QString &strTemplateName);
    //知道TemplateName的时候调用删除
    void DeleteTemplate(const QString &strTemplateName);
    //不知道TemplateName的时候调用删除，删除当前，可提供校验回调
    void DeleteTemplate(CheckCallBack checkCallBack = nullptr);
    /****************/
    void InitUiCtl(bool sw ,bool bDeleteDelay);
    void sendTemplateData( QString templateName,eTemplateType type = eTemplateType_Null);

    void deleteTemplateData( QString templateName,eTemplateType type = eTemplateType_Null);

private:
  
public slots:
    void slotSettingInfo(QString title, bool flag);
public:
    bool m_bIsReset = true;
    void ReloadTreeDataBySelf();
   
    void SetMeasureParamePar(const QString &strUrl, CNetworkAccessManager *network );
    void ClearData();
    bool ChangeViewer(bool bReLoad);
 

    eTemplateType m_eViewerType = eTemplateType_Null;
    eTemplateType m_eTemplateType =eTemplateType_Null;
    eVersionType m_ApiVersion = eVersionType_V1;
    void LoadTreeData();
    void LoadSubList(const std::vector<QString> &vecId);
    void LoadTreeList(const std::map<eTemplateType, stTopPar>  &mapTop, eTemplateType type);
    void LoadDictionaryData(const std::vector<QString> &vecId);
 //   std::vector<stDepartmentAndBodyPart> m_vecDepartmentAndBodyPart;
    QString m_strUrl;
    Ui::StructuralWidget *ui;
    StructuralData m_StructuralData;
    StructuralData m_StructuralDataOri;
  //  StructuralData m_ShowStructuralData;
   // StructuralData m_pShowStructuralData = nullptr;
    void CheckCreate(bool bReset, bool bDeleteDelay);
    void MoveBtn(bool bSend);
    void UpdateUi(bool bSend);
    stStructuralWnd m_StructuralWnd;
    CNetworkAccessManager* m_network;
    stDepartmentAndBodyPart m_DepartmentAndBodyPart;
//    QString m_strDepartment;
//    QString m_strBodyPart;
    bool m_bIsInitStruct = false;
    //提示信息用
    QLabel* label = nullptr;
    QGraphicsOpacityEffect * goe = nullptr;
    QPropertyAnimation* animation = nullptr;
    QTimer* timer;
    float m_opacityVal = 1.0f;
public:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    bool templateRepaint = false;
    QString   m_selectFirstId;
    QString   m_selectSecondId;
public:
Q_SIGNALS:
    void measureParameterCheck(const QStringList & measureParameterList);

    void explainParameterCheck(const QStringList & explainParameterList);

    void signReLoad(QString firstId,QString secondId,QString json);
};

#endif // STRUCTURALWIDGET_H
