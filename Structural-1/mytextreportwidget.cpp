#include "mytextreportwidget.h"
#include "qmenu.h"
#include "qaction.h"
#include "quuid.h"
#include "structuraldata.h"
#include <iostream>
const QString REPORTMENU_GSS = QStringLiteral("QMenu{background-color:rgb(255, 255, 255);}"
                                              " QMenu::item {color:rgb(0,0,0);}"
                                              );


MyTextReportWidget::MyTextReportWidget(const QString &strRemarks,QWidget *parent)
    : m_strRemarks(strRemarks)
    ,QTextEdit{parent}
    ,m_timerTimes(0)
{
    setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    connect(this, &QTextEdit::cursorPositionChanged, this, &MyTextReportWidget::PositionChanged);
    connect(this, &QTextEdit::textChanged, this, &MyTextReportWidget::IsInputChangePostion);




}

void MyTextReportWidget::SetRemarks(const QString &strRemarks)
{
    //设置当前文档类别
    m_strRemarks = strRemarks;
}
bool CheckPosInRange(int nPos, int nLeft, int nRight, bool bMax)
{
    return nPos >= nLeft && (bMax ? (nPos <= nRight) : (nPos < nRight));
}

void MyTextReportWidget::SetReportData( listReportData *pListData, const QString &strChangeId, eCopyState copy)
{
    int nOldPos = m_nSelectLeft ;

    blockSignals(true);
    CopyList(pListData,strChangeId, copy);
    ResetListLineState();
    ReDrawList();

    blockSignals(false);
    auto str = this->toPlainText();
    m_nNowSize =  this->toPlainText().size();
    if(m_bChangePos)
    {
        auto cur = textCursor();
        if(nOldPos > m_nNowSize)
        {
            nOldPos =m_nNowSize;
        }
        cur.setPosition(nOldPos );
        setTextCursor(cur);
    }

    ChangeHeightByData();
    if(m_bChangePos)
    {
        emit cursorPositionChanged();
    }
    // emit cursorPositionChanged();
}

void MyTextReportWidget::SetTransListDataCallBack(UpdateDataCallBack callBack)
{
    m_TransListDataCallBack = callBack;
}

QString MyTextReportWidget::GetListData()
{
    QString strList;
    for(auto &itList : m_listReportData)
    {
        strList += (itList.drawData.strDrawPre + itList.drawData.strDrawData + itList.drawData.strDrawSuff  + itList.drawData.strDrawSuffAdd);
    }
    return strList;
}
bool FindDataRange(const QString &strData,  std::vector<int> *pVecLeft, std::vector<int >*pVecRight)
{
    QRegExp rx("\\[.*\\|.*\\]");
    rx.setMinimal(true);
    int pos = 0;
    while ((pos = rx.indexIn(strData, pos)) != -1)
    {
        // pList->append(rx.cap(0));
        pVecLeft->push_back(pos);
        pos += rx.matchedLength();
        pVecRight->push_back(pos);
    }
    return true;
}
int CreateAddReportData(const QString &strData, listReportData *pList, int nAddPos, const QString &strRemarks)
{
    std::vector<int> vecLeft;
    std::vector<int> vecRight;
    FindDataRange(strData, &vecLeft, &vecRight);
    auto ReMathDrawData = [](stReportData *pData)
    {
        QString strData = pData->drawData.strDrawData;
        if(strData.size() < 2)
        {
            return ;
        }
        strData = strData.mid(1, strData.size() - 2);
        while(!strData.isEmpty())
        {
            int nPos = strData.indexOf("|");
            if(nPos < 0)
            {
                break;
            }
            pData->drawData.vecData.push_back(strData.left(nPos));
            QUuid id = QUuid::createUuid();
            QString strId = id.toString();
            pData->drawData.vecId.push_back(strId);
            strData = strData.right(strData.size() - nPos - 1);
        }
        if(!strData.isEmpty())
        {
            QUuid id = QUuid::createUuid();
            QString strId = id.toString();
            pData->drawData.vecData.push_back(strData);
            pData->drawData.vecId.push_back(strId);
        }
        if(!pData->drawData.vecData.empty())
        {
            pData->drawData.strDrawData = *pData->drawData.vecData.begin();
            pData->strId = *pData->drawData.vecId.begin();
        }
    };
    int nSumSize = 0;
    int nInsertPost = nAddPos;
    if(vecLeft.empty())
    {
        stReportData data;
        data.InitEmpty(strData, nInsertPost);
        data.strRemarks = strRemarks;
        pList->push_back(data);
        nSumSize = strData.size();
    }
    else
    {
        auto itLeft = vecLeft.begin();
        auto itRight = vecRight.begin();
        //int nLast = strData.size();
        int nPos = 0;
        while(itLeft != vecLeft.end())
        {
            stReportData data;
            data.strRemarks = strRemarks;
            data.nLeft[0] = nInsertPost;
            data.nRight[0] = data.nLeft[0] + *itLeft;
            data.nLeft[1] = data.nRight[0];
            data.nRight[1] = data.nLeft[0] + *itRight;
            data.nLeft[2] = data.nLeft[1];
            data.nRight[2] = data.nRight[1];
            int nSingleSize = *itRight - *itLeft;
            data.drawData.strDrawPre = strData.mid(nPos, *itLeft - nPos);
            data.drawData.strDrawData = strData.mid(*itLeft, nSingleSize);
            nPos = *itRight;
            nInsertPost = nAddPos + nPos;
            // nLast -= nSingleSize;
            ++itLeft;
            int nRight = *itRight ;
            ++itRight;
            if(itLeft == vecLeft.end())
            {
                data.nLeft[2] = data.nRight[1];
                data.nRight[2] = data.nLeft[2] + strData.size() - nRight;
                data.drawData.strDrawSuff = strData.right( strData.size() - nRight);
            }
            //符号跟结尾相同
            data.nLeft[3] = data.nLeft[2];
            data.nRight[3] = data.nRight[2];
            data.drawData.strDrawSuffAdd.clear();

            ReMathDrawData(&data);
            nSumSize += data.drawData.strDrawPre.size() + data.drawData.strDrawData.size() + data.drawData.strDrawSuff.size() + data.drawData.strDrawSuffAdd.size();
            pList->push_back(data);
        }
    }
    return nSumSize;

}
void MyTextReportWidget::append(const QString &strData)
{
    //添加文本内容
    QString strAdd = strData;
    strAdd = strAdd.replace("\r\n", "\n");
    strAdd = strAdd.replace("\r", "\n");
    //将当前数据设置为历史数据
    AddOldList();
    //添加当前输入数据
    CreateAddReportData(strAdd, &m_listReportData, m_nNowSize, m_strRemarks);
    //设置显示list
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
}
QString * FindDrawDataPtr(stSingleDrawData *pDrawPtr, int nCul)
{
    QString *pStrData = &pDrawPtr->strDrawPre;
    switch (nCul) {
    case 0: pStrData =&pDrawPtr->strDrawPre;break;
    case 1: pStrData =&pDrawPtr->strDrawData;break;
    case 2: pStrData =&pDrawPtr->strDrawSuff;break;
    case 3: pStrData =&pDrawPtr->strDrawSuffAdd;break;
    default:
        break;
    }
    return pStrData;
}

void MyTextReportWidget::insertPlainText(const QString &strData )
{
    //插入文本
    QString strAdd = strData;
    strAdd = strAdd.replace("\r\n", "\n");
    strAdd = strAdd.replace("\r", "\n");
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
    if(m_listReportData.empty())
    {
        //如果当前为空，直接加到后面
        append(strAdd);
        return;
    }
    //获取当前光标位置
    QTextCursor cursor = textCursor();
    int nPos = cursor.position();
    int nInsertCul = -1;
    int nDataSize = strAdd.size();
    auto itList = FindReportIt(nPos, &nInsertCul);
    //判断当前光标数据哪个渲染数据集
    if(itList == m_listReportData.end() )
    {
        //如果在最后 直接加
        append(strAdd);
        return;
    }
    //添加到指定位置
    int nLeft =itList->nLeft[nInsertCul];
    int nMove = nPos - nLeft;
    if(nInsertCul == 1)
    {
        QString *pStrData = FindDrawDataPtr(&itList->drawData, nInsertCul);
        *pStrData = pStrData->insert(nMove, strAdd);
//        itList->nRight[nInsertCul] += nDataSize;
//        for(int i = nInsertCul + 1; i < MAXDRAWSIZE; ++i)
//        {
//            itList->nLeft[i] += nDataSize;
//            itList->nRight[i] += nDataSize;
//        }
//        ++itList;
//        while(itList != m_listReportData.end())
//        {
//            for(int i = 0; i < MAXDRAWSIZE; ++i)
//            {
//                itList->nLeft[i] += nDataSize;
//                itList->nRight[i] += nDataSize;
//            }
//            ++itList;
//        }
    }
    else
    {
        QString *pStrData = FindDrawDataPtr(&itList->drawData, nInsertCul);
        listReportData report;
        int nSumSize = CreateAddReportData(strAdd, &report, nMove, m_strRemarks);
        if(report.empty())
        {
            return;
        }
        bool bAdd = true;
        if((int)report.size() == 1 && report.begin()->drawData.strDrawData.isEmpty())
        {
            bAdd = false;
            *pStrData = pStrData->insert(nMove, strAdd);
           // itList->nRight[nInsertCul] += nSumSize;
        }
        else if(nInsertCul == 0)
        {
            itList->drawData.nDrawDeleteBefer = 0;
            itList->drawData.strDrawDeleteBefer = *pStrData;
            QString strLeft = pStrData->left(nMove);
            *pStrData = pStrData->right(pStrData->size() - nMove);
           // itList->nLeft[0] += nMove;
            auto itReportBegin = report.begin();
            itReportBegin->drawData.strDrawPre = strLeft + itReportBegin->drawData.strDrawPre;
            //itReportBegin->nLeft[0] -= nMove;
        }
        else
        {
            itList->drawData.nDrawDeleteBefer = nInsertCul;
            itList->drawData.strDrawDeleteBefer = *pStrData;
//            QString strDrawDeleteBefer;
//            int nDrawDeleteBefer = -1;
            QString strRight = pStrData->right(pStrData->size() - nMove);
            *pStrData =  pStrData->left(nMove);
            //itList->nRight[nInsertCul] -= nMove;
            auto itReportEnd = report.end();
            --itReportEnd;
            itReportEnd->drawData.strDrawSuff =  itReportEnd->drawData.strDrawSuff + strRight;
           // itReportEnd->nRight[nInsertCul] += nMove;
        }
//        for(int i = nInsertCul + 1; i < MAXDRAWSIZE; ++i)
//        {
//            itList->nLeft[i] += nSumSize;
//            itList->nRight[i] += nSumSize;
//        }
//        auto itListNow = itList;
//        ++itListNow;
//        while(itListNow != m_listReportData.end())
//        {
//            for(int i = 0; i < MAXDRAWSIZE; ++i)
//            {
//                itListNow->nLeft[i] += nSumSize;
//                itListNow->nRight[i] += nSumSize;
//            }
//            ++itListNow;
//        }
        if(bAdd)
        {
            m_listReportData.insert(itList + (nInsertCul == 0 ? 0 : 1), report.begin(), report.end());
        }
    }
    //设置数据集
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
    AddOldList();
}

void MyTextReportWidget::clear()
{
    AddOldList();
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
    QTextEdit::clear();
    m_listReportData.clear();
}

void MyTextReportWidget::setPlainText(const QString &text)
{
    //设置文本
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
    clear();
    append(text);

}

QString MyTextReportWidget::GetReportKeyValueJson()
{
    return StructuralData::TransReporttoString(m_listReportData, m_strRemarks);
}

void MyTextReportWidget::SetReportStateByJson(const QString &strJson)
{
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
    AddOldList();
    m_listReportData = StructuralData::TransStringtoReport(strJson, m_strRemarks);
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
}

void MyTextReportWidget::SetMinAndMaxLimit(int nMin, int nMax)
{
    m_nMinLimit = nMin;
    m_nMaxLimit = nMax;
}

void MyTextReportWidget::SetChangeHeightByData(bool bFlg, eChangeDataMode mode)
{
    m_ChangeHeightByDataFlg = bFlg;
    m_ChangeDataMode = mode;
    ChangeHeightByData();
}

void MyTextReportWidget::ChangeShowNumberState(/*bool bDeletEmptyLine, */bool bSend )
{
    AddOldList();
    if(!SetShowNumber(false, false, bSend))
    {
        SetShowNumber(true, false, bSend);
    }

}
bool MyTextReportWidget::SetShowNumber(bool bShow,bool bDeletEmptyLine,  bool bSend)
{
    if(bDeletEmptyLine)
    {
        DeleteEmptyLine();
    }
    int nCul = 1;
    bool bNextWork = false;
    bool bHasDelete = false;
    auto funChangeData = [](bool bShowNumberFlg, QString &strData, int nFind, int &nCul )
    {
        if(bShowNumberFlg)
        {
            strData = strData.insert(nFind + 1, QString("%1.").arg(nCul++));
            return false;
        }
        else
        {
            int nMove = 0;
            QRegExp regExp( "\\d+" );
            int nFindNext = nFind + 1;
            int nFindReg = regExp.indexIn(strData, nFindNext);
            if(nFindReg != nFindNext)
            {
                return false;
            }
            QString strFind = regExp.cap(0);
            nFindNext += strFind.length();
            nFindReg = strData.indexOf(".", nFindNext);
            if(nFindReg != nFindNext)
            {
                return false;
            }
            nMove = strFind.length() + 1;
            int nRight = strData.length() - (nFind + 1) - nMove;
            if(nRight >= 0)
            {
                strData = strData.left(nFind + 1) + strData.right(nRight);
            }
            ++nCul;
            return true;

        }
    };
    // if(m_bShowNumberFlg)
    bool bRet = false;
    {
        auto itBegin = m_listReportData.begin();
        if(itBegin != m_listReportData.end())
        {
            bRet = funChangeData(bShow, itBegin->drawData.strDrawPre, -1, nCul);
            bHasDelete = bRet;
        }
        for(auto &list : m_listReportData)
        {
            if(bNextWork)
            {
                bRet = funChangeData(bShow, list.drawData.strDrawPre, -1, nCul);

                bHasDelete = bRet || bHasDelete ;
                bNextWork  = false;
            }
            QString strTmp = list.drawData.strDrawPre;
            QString strFind = "\n";
            int nFrom = 0;
            int nFind = strTmp.indexOf(strFind, 0);
            while(nFind >= 0)
            {
                bRet = funChangeData(bShow, strTmp, nFind, nCul);
                bHasDelete = bRet || bHasDelete ;
                nFrom = nFind + 1;
                nFind = strTmp.indexOf( strFind, nFrom);

            }
            list.drawData.strDrawPre = strTmp;
            strTmp = list.drawData.strDrawData;
            nFind = strTmp.indexOf(strFind, 0);
            bool bDrawSuff = false;
            if(nFind == 0)
            {
                QString strPre = list.drawData.strDrawPre;
                bRet = funChangeData(bShow, strPre, strPre.length() - 1, nCul);
                bHasDelete = bRet || bHasDelete ;
                list.drawData.strDrawPre = strPre;
            }
            else
            {
                while(nFind >= 0)
                {
                    if(nFind == strTmp.length() - 1)
                    {
                        bDrawSuff = true;
                        break;
                    }
                    bRet = funChangeData(bShow, strTmp, nFind, nCul);
                    bHasDelete = bRet || bHasDelete ;
                    nFrom = nFind + 1;
                    nFind = strTmp.indexOf( strFind, nFrom);

                }
                list.drawData.strDrawData = strTmp;
            }
            if(bDrawSuff)
            {
                bRet = funChangeData(bShow,  list.drawData.strDrawSuff, -1, nCul);
                bHasDelete = bRet || bHasDelete ;
            }
            strTmp = list.drawData.strDrawSuff;
            nFrom = 0;
            nFind = strTmp.indexOf( strFind, 0);

            while(nFind >= 0)
            {
                if(nFind == strTmp.length() - 1)
                {
                    bNextWork = true;
                    break;
                }
                bRet = funChangeData(bShow, strTmp, nFind, nCul);
                bHasDelete = bRet || bHasDelete ;

                nFrom = nFind + 1;
                nFind = strTmp.indexOf( strFind, nFrom);

            }
            list.drawData.strDrawSuff = strTmp;

        }

    }


    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
    return bHasDelete;
}

void MyTextReportWidget::DeleteEmptyLine()
{
    //    m_bShowNumberFlg = bFlg;
    int nCul = 0;
    auto funcDelete = [](QString &str)
    {
        QString strFind = "\n\n";
        int nFrom = 0;
        int nFind = str.indexOf( strFind, 0);
        while(nFind >= 0)
        {
            int nRight = str.length() - (nFind + 2);
            str = str.left(nFind + 1) + (nRight >= 0 ? str.right(str.length() - (nFind + 2)) :"");
            nFrom = nFind;
            nFind = str.indexOf( strFind, nFrom);
        }
    };
    for(auto &list : m_listReportData)
    {
        funcDelete(list.drawData.strDrawPre);
        funcDelete(list.drawData.strDrawData);
        funcDelete(list.drawData.strDrawSuff);
        //   QString strTmp = list.drawData.strDrawPre;
    }
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
}

void MyTextReportWidget::SetColorShowList(const QStringList &list, const QColor &color)
{
    ClearColorShow();
    AddColorShow(list, color);

}

void MyTextReportWidget::AddColorShow(const QStringList &list, const QColor &color)
{

    auto nowList = CheckColorShow(list,color);
    if(nowList.empty())
    {
        blockSignals(true);
        ReDrawList();
        blockSignals(false);
        return;
    }
    for(auto &itNowList : nowList)
    {
        m_ListColorShow.push_back(stColorShow(itNowList, color));
    }

    blockSignals(true);
    ReDrawList();
    blockSignals(false);
}

void MyTextReportWidget::AddColorShow(const QString &strData, const QColor &color)
{
    QStringList list;
    list.push_back(strData);
    AddColorShow(list, color);
}

void MyTextReportWidget::ClearColorShow()
{
    m_ListColorShow.clear();
}

void MyTextReportWidget::SendData()
{
    m_bSendChange = true;
    m_timerTimes = 0;
    if(m_pTimer == nullptr)
    {
        m_pTimer = new  QTimer();
        m_pTimer->setInterval(20);
        connect(m_pTimer, SIGNAL(timeout()), this, SLOT(OnTimeOut()));
        m_pTimer->start();
    }
   // m_timerTimes = 0;
}

QStringList MyTextReportWidget::CheckColorShow(const QStringList &list,const QColor &color)
{
    QStringList listTmp;

    for(auto &itInputList : list)
    {
        bool bFind = false;
        for(auto &itList : m_ListColorShow)
        {
            if(itInputList == itList.strData)
            {
                itList.color = color;
                bFind = true;
                break;
            }

        }
        if(!bFind)
        {
            listTmp.push_back(itInputList);
        }
    }
    return listTmp;
}

void MyTextReportWidget::ReDrawList()
{
    QColor colorNormal = Qt::black;
    QColor colorTrans = Qt::blue;
    int nLeft = 0;

    QTextEdit:: clear();
    auto funDraw = [&](const QString &strData, const QColor & color)
    {
        listHighLight list;
        stHighLight highLight;
        for(auto &itVec : m_ListColorShow)
        {
            highLight.color = itVec.color;
            auto &itData = itVec.strData;
            // for(auto &itData : itVec.list)
            {
                if(itData.isEmpty())
                {
                    continue;
                }
                int nNow = 0;
                int nPos = strData.indexOf(itData,nNow);
                while(nPos >= 0)
                {
                    highLight.highLightPos = nPos;
                    highLight.highLightRight = nPos + itData.length();
                    list.push_back(highLight);
                    nNow = nPos + 1;
                    nPos = strData.indexOf(itData,nNow);
                }
            }
        }

        std::sort(list.begin(), list.end(), [](const stHighLight &x, const stHighLight &y)
        {
            return x.highLightPos < y.highLightPos;
        }
        );
        auto itList = list.begin();
        if(itList == list.end())
        {
            setTextColor(color);
            QTextEdit::insertPlainText(strData);
        }
        else
        {
            int nLeft = itList->highLightPos;
            int nRight = itList->highLightRight;
            int nSumLeft = nLeft;
            int nSumRight = nRight;
            auto strDraw = strData;

            while(itList != list.end())
            {
                setTextColor(color);
                QString strLeft = strDraw.left(nLeft);
                QTextEdit::insertPlainText(strLeft );
                setTextColor(itList->color);
                QString strMid = strDraw.mid(nLeft, nRight - nLeft);
                QTextEdit::insertPlainText( strMid);
                strDraw = strDraw.right(strDraw.length() - nRight);
                int nPreRight = itList->highLightRight;
                ++itList;
                while(itList != list.end())
                {
                    if(nSumRight >= itList->highLightRight)
                    {
                        ++itList;
                    }
                    else
                    {
                        nSumLeft = itList->highLightPos;
                        nLeft = nSumLeft - nPreRight;
                        nSumRight = itList->highLightRight;
                        nRight = nSumRight - nPreRight;
                        break;
                    }
                }
            }
            if(!strDraw.isEmpty())
            {
                setTextColor(color);
                QTextEdit::insertPlainText(strDraw);
            }
        }
    };
    auto drawColor = colorNormal;
    for(auto &itList : m_listReportData)
    {
        if(!itList.drawData.strDrawPre.isEmpty())
        {
            drawColor = colorNormal;
            funDraw(itList.drawData.strDrawPre /*+ ":"*/, drawColor);
        }
        if(!itList.drawData.bMulti)
        {
            drawColor = colorTrans;
            //setTextColor(colorTrans);
        }

        funDraw(itList.drawData.strDrawData, drawColor);
        //QTextEdit::insertPlainText(itList.drawData.strDrawData);
        if(!itList.drawData.strDrawSuff.isEmpty())
        {
            drawColor = colorNormal;
            funDraw(itList.drawData.strDrawSuff, drawColor);
        }
        if(!itList.drawData.strDrawSuffAdd.isEmpty())
        {
            drawColor = colorNormal;
            funDraw(itList.drawData.strDrawSuffAdd, drawColor);
        }
        itList.nLeft[0] = nLeft;
        itList.nRight[0] = nLeft+ itList.drawData.strDrawPre.size();
        itList.nLeft[1] = itList.nRight[0];
        itList.nRight[1] = itList.nLeft[1]+ itList.drawData.strDrawData.size();
        itList.nLeft[2] = itList.nRight[1];
        itList.nRight[2] = itList.nLeft[2]+ itList.drawData.strDrawSuff.size();
        itList.nLeft[3] = itList.nRight[2];
        itList.nRight[3] = itList.nLeft[3]+ itList.drawData.strDrawSuffAdd.size();
        nLeft =  itList.nRight[3];
    }

}




void MyTextReportWidget::CopyList( listReportData *pListData,const QString &strChangeId, eCopyState copy)
{
    if(copy == eCopyState_CopyAll)
    {
        m_listReportData = *pListData;
    }
    else if(copy == eCopyState_CopyDrawById)
    {
        auto itAdd =  std::find(pListData->begin(), pListData->end(), strChangeId);
        auto itFind = std::find(m_listReportData.begin(), m_listReportData.end(), strChangeId);
        if(itFind == m_listReportData.end() && itAdd != pListData->end() && !itAdd->drawData.bMulti)
        {
            itFind = m_listReportData.begin();
            while(itFind != m_listReportData.end())
            {
                if(itFind->drawData.vecId.end() != std::find(itFind->drawData.vecId.begin(), itFind->drawData.vecId.end(),strChangeId))
                {
                    break;
                }
                ++itFind;
            }
        }
        if(m_listReportData.size() == pListData->size() && itFind != m_listReportData.end() && itAdd != pListData->end())
        {
            itFind->drawData.strDrawData = itAdd->drawData.strDrawData ;
            itFind->strId = itAdd->strId;
            itFind->strPreId = itAdd->strPreId;

            if(itFind->drawData.bReset)
            {
                itFind->drawData.strDrawPre = itAdd->drawData.strDrawPre ;
                itFind->drawData.strDrawSuff = itAdd->drawData.strDrawSuff ;
            }
        }
        else
        {
            auto listTmp = m_listReportData;
            m_listReportData = *pListData;
            auto itLast =m_listReportData.end();
            if(!m_listReportData.empty())
            {
                --itLast;
            }
            for(auto & itTmp : listTmp)
            {
                auto itFind = std::find(m_listReportData.begin(), m_listReportData.end(), itTmp.strId);
                if(itFind != m_listReportData.end() && !itFind->drawData.bReset)
                {
                    if(itFind->drawData.bMulti)
                    {
                        if(itFind->drawData.bMultiFirst)
                        {
                            for(auto &itVec : itFind->drawData.vecId)
                            {
                                auto itTmpFind = std::find(listTmp.begin(), listTmp.end(),itVec );
                                if(itTmpFind != listTmp.end())
                                {
                                    itFind->drawData.strDrawPre = itTmpFind->drawData.strDrawPre;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        itFind->drawData.strDrawPre = itTmp.drawData.strDrawPre;
                        itFind->drawData.strDrawData = itTmp.drawData.strDrawData;
                        itFind->drawData.strDrawSuff = itTmp.drawData.strDrawSuff;
                        itFind->drawData.strDrawSuffAdd = itTmp.drawData.strDrawSuffAdd;

                        itFind->strId = itTmp.strId;
                    }
                }
            }
        }
        auto itList = m_listReportData.begin();
        int nCul = 0;
        while(itList != m_listReportData.end())
        {
            auto strParentId = itList->drawData.strParentId;
            bool bAdd = true;
            if(!strParentId.isEmpty())
            {
                bool bFind = false;
                for(auto &itFindNow : m_listReportData)
                {
                    if(itFindNow.strId == strParentId)
                    {
                        bFind = true;
                        break;
                    }
                }
                if(!bFind)
                {
                    m_listReportData.erase(itList);
                    itList = m_listReportData.begin() + nCul;
                    bAdd = false;
                }
            }
            if(bAdd)
            {
                ++nCul;
                ++itList;
            }
        }
    }
    auto itNow = m_listReportData.begin();
    int nCul = 0;
    while(itNow != m_listReportData.end())
    {
        if(itNow->drawData.strDrawData.isEmpty() &&
                itNow->drawData.strDrawPre.isEmpty() &&
                (itNow->drawData.strDrawSuff.isEmpty() || itNow->drawData.strDrawSuff == "\n"
                 || itNow->drawData.strDrawSuffAdd == "\n") )
        {
            m_listReportData.erase(itNow);
            itNow =  m_listReportData.begin() + nCul;
        }
        else
        {
            ++nCul;
            ++itNow;
        }
    }
}

void MyTextReportWidget::ResetListLineState()
{
    for(auto &itList : m_listReportData)
    {
        int nSize = itList.drawData.strDrawData.length();
        if(nSize >= 1)
        {
            if(itList.drawData.strDrawData[0] == '\n')
            {
                itList.drawData.strDrawPre += '\n';
                itList.drawData.strDrawData = itList.drawData.strDrawData.right(nSize - 1);
                nSize = itList.drawData.strDrawData.length();
            }
        }
        if(nSize >= 1)
        {
            if(itList.drawData.strDrawData[nSize - 1] == '\n')
            {
                itList.drawData.strDrawSuff = "\n" + itList.drawData.strDrawSuff;
                itList.drawData.strDrawData = itList.drawData.strDrawData.left(nSize - 1);
            }
        }
    }
}

listReportData::iterator MyTextReportWidget::FindReportIt(int nPos, int *pCul)
{
    auto itList = m_listReportData.begin();
    *pCul = -1;
    while( itList != m_listReportData.end())
    {
        for(int i = 0;  i < MAXDRAWSIZE; ++i)
        {
            if(CheckPosInRange(nPos, itList->nLeft[i], itList->nRight[i],i == 1))
            {
                *pCul = i;
                break;
            }
        }
        if(*pCul >= 0)
        {
            break;
        }
        ++itList;
    }
    return itList;
}

int MyTextReportWidget::FindChangeLeft(const QString &strNow)
{
    QString strList = GetListData();
    auto itNowBegin = strNow.begin();
    auto itListBegin = strList.begin();
    auto itNowEnd = strNow.end();
    auto itListEnd  = strList.end();
    int nCul = 0;
    while(itNowBegin != itNowEnd && itListBegin != itListEnd)
    {
        if(!(*itNowBegin == *itListBegin))
        {

            return nCul;
        }
        ++nCul;
        ++itNowBegin;
        ++itListBegin;
    }
    return -1;
}

void MyTextReportWidget::AddOldList()
{
    if(m_bEnableReCover)
    {
        int nSize = (int)m_oldList.size();
        if(nSize >= m_nMaxSize)
        {
            m_oldList.pop_front();
        }
        m_oldList.push_back(m_listReportData);
    }
}

void MyTextReportWidget::ClearOldList()
{
    m_oldList.clear();
}

void MyTextReportWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    //  QTextEdit::mousePressEvent(e);
    if(e->buttons() == Qt::LeftButton)
    {
        IsButtonChangePostion();
    }

    //  emit signal_clicked();
}

void MyTextReportWidget::mousePressEvent(QMouseEvent *e)
{
    QTextEdit::mousePressEvent(e);
    emit signal_clicked();

}


void MyTextReportWidget::IsButtonChangePostion()
{
    if(m_bHasChange)
    {

    }
    m_bHasChange = false;
    for(auto &itList : m_listReportData)
    {
        if(!itList.drawData.bMulti  && m_nSelectLeft >0 && CheckPosInRange(m_nSelectLeft, itList.nLeft[1], itList.nRight[1], true) )
        {

            m_strChangeId = itList.strId;
            QMenu *menu = new QMenu(this);
            //menu->setStyleSheet(REPORTMENU_GSS);

            auto itId = itList.drawData.vecId.begin();
            for(auto &itSelect : itList.drawData.vecData)
            {
                QAction* action = new QAction(itSelect, menu);
                menu->addAction(action);
                connect(action,SIGNAL(triggered()),this, SLOT(OnMenuSelect()));
                m_mapAction[*itId] = action;
                ++itId;
            }
            menu->exec(QCursor::pos());
            m_mapAction.clear();
            m_strChangeId.clear();
            delete menu;
            break;
        }
    }

}

enum eMathType
{
    eMathType_Left = 0,
    eMathType_MidAll ,
    eMathType_MidRange ,
    eMathType_Right ,

};


bool MyTextReportWidget::AddPostionData(const QString &str, bool bChange)
{
    int nSize = str.length();
    bool bSend = false;
    int nAdd = nSize - m_nNowSize;
    int nPos = m_nSelectLeft - (bChange ?  0 : nAdd);
    int nIn = -1;
    auto itList = FindReportIt(nPos, &nIn);
    if(itList == m_listReportData.end())
    {
        --itList;
        nIn = 2;
        if(itList->drawData.strDrawSuff.isEmpty() && itList->drawData.strDrawData.isEmpty())
        {
            nIn = 0;
        }
        //return;
    }
    if(nIn == 1 && itList->drawData.bEdit)
    {
        bSend = true;
        // list.push_back(*itList);
    }
    m_nNowSize += nAdd;
    QString strAdd = str.mid(nPos  ,nAdd );
    int nLeftCount = nPos - itList->nLeft[nIn];
    QString *pStrData = FindDrawDataPtr(&itList->drawData, nIn);

    QString strNowLeft =   pStrData->left(nLeftCount);
    QString strNowRight =  pStrData->right( pStrData->size() - nLeftCount);
    *pStrData = strNowLeft + strAdd + strNowRight;
    itList->nRight[nIn] = itList->nLeft[nIn] +  pStrData->size();
    for(int i = nIn + 1; i < MAXDRAWSIZE; ++i)
    {
        itList->nLeft[i] += nAdd;
        itList->nRight[i] += nAdd;
    }
    ++itList;
    while( itList != m_listReportData.end())
    {
        for(int i = 0;  i < MAXDRAWSIZE; ++i)
        {
            itList->nLeft[i] += nAdd;
            itList->nRight[i] += nAdd;
        }
        ++itList;
    }
    return bSend;
}
bool MyTextReportWidget::DeletePostionData(int nLeft, int nRight)
{
    bool bSend = false;
    int nDelete = nRight - nLeft;
    int nStartPos = nLeft;
    int nEndPos = nStartPos + nDelete;
    int nStartCul = -1;
    auto itStart = FindReportIt(nStartPos, &nStartCul);
    int nEndCul = -1;
    auto itEnd = FindReportIt(nEndPos, &nEndCul);
    if(nEndCul == -1)
    {
        itEnd = m_listReportData.end() -1 ;
        nEndCul = MAXDRAWSIZE - 1;
    }
    itStart->drawData.bIsDelete = true;
    itEnd->drawData.bIsDelete = true;
    if(itStart == itEnd)
    {

        if(nStartCul == nEndCul)
        {
            int nS = nStartPos - itStart->nLeft[nStartCul];
            int nE  = nEndPos - itStart->nLeft[nStartCul];
            QString *pData = FindDrawDataPtr(&itStart->drawData, nStartCul);
            *pData = pData->left(nS) +  pData->mid(nE ,pData->size() - nE );

        }
        else
        {
            int nS = nStartPos - itStart->nLeft[nStartCul];
            QString *pData = FindDrawDataPtr(&itStart->drawData, nStartCul);
            *pData = pData->left(nS);
            for(int i = nStartCul + 1; i < nEndCul; ++i)
            {
                pData = FindDrawDataPtr(&itStart->drawData, i);
                *pData  = "";
            }

            int nE  = nEndPos - itStart->nLeft[nEndCul];
            pData = FindDrawDataPtr(&itStart->drawData, nEndCul);
            *pData = pData->right(pData->size() - nE);

        }
        if(nStartCul <= 1 && nEndCul >= 1 && itStart->drawData.bEdit)
        {
            bSend = true;
            //  list.push_back(*itStart);
        }

    }
    else
    {
        int nS = nStartPos - itStart->nLeft[nStartCul];
        QString *pData = FindDrawDataPtr(&itStart->drawData, nStartCul);
        *pData = pData->left(nS);

        for(int i = nStartCul + 1; i < MAXDRAWSIZE; ++i)
        {
            pData = FindDrawDataPtr(&itStart->drawData, i);
            *pData  = "";
        }
        if(nStartCul <= 1 && itStart->drawData.bEdit)
        {
            bSend = true;
            // list.push_back(*itStart);
        }
        ++itStart;
        while(itStart != itEnd)
        {

            for(int i = 0; i < MAXDRAWSIZE; ++i)
            {
                pData = FindDrawDataPtr(&itStart->drawData, i);
                *pData  = "";
            }
            bSend = true;
            itStart->drawData.bIsDelete = true;
            //  list.push_back(*itStart);
            ++itStart;
        }
        for(int i = 0; i < nEndCul; ++i)
        {

            pData = FindDrawDataPtr(&itStart->drawData, i);
            *pData  = "";
        }

        int nE  = nEndPos - itStart->nLeft[nEndCul];
        pData = FindDrawDataPtr(&itStart->drawData, nEndCul);
        *pData = pData->right(pData->size() - nE);
        if(nEndCul >= 1 && itStart->drawData.bEdit)
        {
            bSend = true;
            //list.push_back(*itStart);
        }


    }
    return bSend;
}

void MyTextReportWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->modifiers() == Qt::ControlModifier&& e->key() == Qt::Key_Z)
    {
        if(m_bEnableReCover)
        {
            if(!m_oldList.empty())
            {
                auto itEnd = m_oldList.end();
                --itEnd;
                m_listReportData = *(itEnd);
                /*m_listReportData = */m_oldList.pop_back();

                SetReportData(&m_listReportData, "", eCopyState_NoCopy);
            }
        }
        else
        {
            return;
        }
    }
    else  if (e->modifiers() == Qt::ControlModifier&& e->key() == Qt::Key_A)
    {
        m_nSelectChangeLeft = 0;
        m_nSelectChangeRight = m_nNowSize;
        m_bSelectAll = true;
        QTextEdit::keyPressEvent(e);
    }
    else  if (e->modifiers() == Qt::ControlModifier&& e->key() == Qt::Key_V)
    {
        m_bStick = true;
        QTextEdit::keyPressEvent(e);
        m_bStick = false;
    }
    else
    {
        m_bStick = true;
        QTextEdit::keyPressEvent(e);
        m_bStick = false;
    }
}

void MyTextReportWidget::OnTimeOut()
{
    if(m_bSendChange)
    {
        if(m_timerTimes == 10)
        {
            if(m_TransListDataCallBack != nullptr)
            {
                m_TransListDataCallBack(&m_listReportData, "");
            }
            m_timerTimes = 0;
            m_bSendChange = false;
        }
        else
        {
            ++m_timerTimes;
        }
    }
}
void MyTextReportWidget::IsInputChangePostion()
{
    //输入数据变化
    this->blockSignals(true);
    m_bHasChange = true;
    //获取输入后的文本
    QString str = this->toPlainText();
    int nSize = str.size();
    //将当前数据集添加到历史列表
    AddOldList();
    if(nSize == 0)
    {
        m_listReportData.clear();
        ChangeHeightByData();
        m_nNowSize = 0;
        m_nSelectChangeLeft = -1;
        m_nSelectChangeRight = -1;
        this->blockSignals(false);
        return;
    }

    bool bSend = false;
    bool bChange = false;
    //判断下是否为选择了一个区域
    if(m_nSelectChangeLeft != m_nSelectChangeRight && m_nSelectChangeLeft >= 0 && m_nSelectChangeLeft <= m_nNowSize
            && m_nSelectChangeRight >= 0 && m_nSelectChangeRight <= m_nNowSize)
    {
        //如果选择了 这区域先删除
        bSend = DeletePostionData(m_nSelectChangeLeft,m_nSelectChangeRight ) || bSend  ;
        m_bChangePos = false;
        SetReportData(&m_listReportData, "", eCopyState_NoCopy);
        m_bChangePos = true;
        bChange = true;
        m_nSelectLeft =  m_nSelectChangeLeft;
    }

    //如果当前数据大于历史数据
    if(nSize > m_nNowSize)
    {
        if(m_listReportData.empty())
        {
            //添加数据
            CreateAddReportData(str, &m_listReportData, nSize, m_strRemarks);
            SetReportData(&m_listReportData, "", eCopyState_NoCopy);
            m_nSelectChangeLeft = -1;
            m_nSelectChangeRight = -1;
            this->blockSignals(false);
            return;
        }
        bSend =  AddPostionData(str,bChange) || bSend ;

    }

    else if(nSize < m_nNowSize)
    {
        if(m_nSelectRight + 1 <= m_nNowSize)
        {
            //删除
            bSend =  DeletePostionData(m_nSelectLeft, m_nSelectRight + 1) || bSend ;
        }
    }
    m_nSelectChangeLeft = -1;
    m_nSelectChangeRight = -1;
    int nListSize = (int)m_listReportData.size();
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);

    this->blockSignals(false);
    if(nListSize != m_listReportData.size())
    {
        bSend = true;
    }
    if(bSend)
    {
        SendData();

    }
}

void MyTextReportWidget::ChangeHeightByData()
{
    if(!m_ChangeHeightByDataFlg)
    {
        return;
    }
    QTextDocument *doc = this->document();
    int height = doc->size().height();
    if((m_ChangeDataMode & eChangeDataMode_LimitByMin) == eChangeDataMode_LimitByMin)
    {
        height =  height < m_nMinLimit ? m_nMinLimit : height;

    }
    if((m_ChangeDataMode & eChangeDataMode_LimitByMax) == eChangeDataMode_LimitByMax)
    {
        height =  height > m_nMaxLimit ? m_nMaxLimit : height;
    }
    if(height > 0)
    {
        this->setFixedHeight(height);
    }
}

void MyTextReportWidget::OnMenuSelect()
{
    QAction * pAction = (QAction *)sender();
    QString strId ;
    QString strTitle = pAction->text();
    listReportData listChangeData;
    for(auto &itMap : m_mapAction)
    {
        if(itMap.second == pAction)
        {
            strId = itMap.first;
        }
    }
    bool bMuti = false;
    for(auto &itList : m_listReportData)
    {
        if(itList.strId == m_strChangeId)
        {
            itList.strId = strId;
            itList.drawData.strDrawData = strTitle;
            bMuti = itList.drawData.bMulti;
            listChangeData.push_back(itList);
            break;
        }
    }
    SetReportData(&m_listReportData, "", eCopyState_NoCopy);
    SendData();
//    if(m_TransListDataCallBack != nullptr)
//    {
//        m_TransListDataCallBack(&m_listReportData, "");
//    }
    emit cursorPositionChanged();
}

void MyTextReportWidget::PositionChanged()
{
    QTextCursor cursor = textCursor();
    //光标变化回调
    m_nSelectLeft = cursor.selectionStart();
    m_nSelectRight = cursor.selectionEnd();

    if( m_nSelectLeft > m_nSelectRight)
    {
        int nLeft = m_nSelectRight;
        m_nSelectRight = m_nSelectLeft;
        m_nSelectLeft = nLeft;
    }
    if(m_bSelectAll)
    {
        m_bSelectAll = false;
        return;
    }
    if(m_bStick)
    {
        return;
    }

    QString str = this->toPlainText();
    if(!(str == m_strOldData))
    {
        m_strOldData = str;
        return;
    }
    m_strOldData = str;
    if(m_nSelectLeft != m_nSelectRight)
    {
        m_nSelectChangeLeft =m_nSelectLeft;
        m_nSelectChangeRight =m_nSelectRight;
        m_ClickTimes = 0;
    }
    else
    {
        {
            m_nSelectChangeLeft = -1;
            m_nSelectChangeRight = -1;
        }
    }

}

