
#include <QtWidgets>
#include "CLabelSplitLayout.h"
#include "mycheckbox.h"
#include <iostream>
#include "myradiobutton.h"
#include "mylineedit.h"
CLabelSplitLayout::CLabelSplitLayout(QWidget* parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(0, 0, 5, 0);
    m_signleLineSelect << "是" << "否" << "有" << "无" << "1" << "2" << "3" << "4" << "5" << "6" 
        << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15";
}

CLabelSplitLayout::CLabelSplitLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(0, 0, 5, 0);
    m_signleLineSelect << "是" << "否" << "有" << "无" << "1" << "2" << "3" << "4" << "5" << "6"
        << "7" << "8" << "9" << "10" << "11" << "12" << "13" << "14" << "15";
}
CLabelSplitLayout::~CLabelSplitLayout()
{
    QLayoutItem* item;
    while ((item = takeAt(0)))
    {
        if (item != nullptr)
        {
            delete item;
        }
    }
}
void CLabelSplitLayout::addItem(QLayoutItem* item)
{
    if (item != nullptr)
    {
        itemList.append(item);
    }
}
//int CLabelSplitLayout::horizontalSpacing() const
//{
//    if (m_hSpace >= 0)
//    {
//        return m_hSpace;
//    } else
//    {
//        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
//    }
//}

//int CLabelSplitLayout::verticalSpacing() const
//{
//    if (m_vSpace >= 0) {
//        return m_vSpace;
//    } else {
//        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
//    }
//}
int CLabelSplitLayout::count() const
{
    return itemList.size();
}

QLayoutItem* CLabelSplitLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem* CLabelSplitLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    else
        return 0;
}

//处理点击事件，根据点击位置判断哪个标签或控件被点击，并触发相应的操作。这显示了布局不仅管理控件的位置和尺寸，还参与事件处理。
void CLabelSplitLayout::ClickCall(const QPoint& pt)
{
    int nCul = 0;
    for (auto& itVec : m_vecClick)
    {
        ClickableLabel* pLabel = (ClickableLabel*)itVec.pLabel;
        if (pLabel->isVisible() && itVec.Check(pt))
        {
            pLabel->labelIsClicked();


            break;
        }
        ++nCul;
    }

    for (auto& itVec : m_vecControl)
    {
        if (itVec.pLabel->isVisible() && itVec.Check(pt))
        {
            std::string strName = typeid(*itVec.pLabel).name();
            if (strName == "class MyCheckBox")
            {
                MyCheckBox* pCheck = (MyCheckBox*)itVec.pLabel;
                pCheck->labelIsClicked();
            }
            else  if (strName == "class MyRadioButton")
            {
                MyRadioButton* pCheck = (MyRadioButton*)itVec.pLabel;
                pCheck->click();
            }
            else  if (strName == "class MyLineEdit")
            {
                MyLineEdit* pEdit = (MyLineEdit*)itVec.pLabel;
                pEdit->setFocus();
            }
            break;
        }
        ++nCul;
    }
}

Qt::Orientations CLabelSplitLayout::expandingDirections() const
{
    return 0;
}

bool CLabelSplitLayout::hasHeightForWidth() const
{
    return true;
}

int CLabelSplitLayout::heightForWidth(int width) const
{
    if (m_nHeight == 0)
    {
        CLabelSplitLayout* pLayout = const_cast<CLabelSplitLayout*>(this);
        //设置一个最低宽 防止异常扩建子控件
        int height = pLayout->doLayout(QRect(0, 0, width < 300 ? 300 : width, 0), true);
        return height;
    }
    else
    {
        return m_nHeight;
    }

}

void CLabelSplitLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);

    int nHeight =   doLayout(rect, false);
    if(m_nHeight !=  nHeight)
    {
        m_nHeight = nHeight;
      //  ((QWidget*)parent())->update();
    }

}

QSize CLabelSplitLayout::sizeHint() const
{
    return minimumSize();
}

//QSize CLabelSplitLayout::minimumSizeHint() const
//{
//    return minimumSize();
//}

QSize CLabelSplitLayout::minimumSize() const
{

    QSize size;
    QLayoutItem* item;
    foreach(item, itemList)
        size = size.expandedTo(item->sizeHint());


    if (m_nHeight != 0)
    {
        size.setHeight(m_nHeight);
    }
    size += QSize(2 * margin(), 2 * margin());


    return size;
}
//这是实现自定义布局逻辑的核心函数，它根据布局中的项和给定的矩形（表示布局的目标区域）进行布局。testOnly参数表示是否仅测试布局（不实际应用布局更改），这对于计算理想布局尺寸非常有用。
int CLabelSplitLayout::doLayout(const QRect& rect, bool testOnly)
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QLayoutItem* item;
    bool bSingleLine = false;
    int nSum = itemList.size();
    int nCulItem = 0;
    foreach(item, itemList)
    {
        QWidget* wid = item->widget();
        int spaceX = 4;
        if (spaceX == -1)
        {
            spaceX = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        }
        int spaceY = 12;
        if (spaceY == -1)
        {
            spaceY = wid->style()->layoutSpacing(QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        }

        std::string strName = typeid(*wid).name();
        QRect rectNow(0, 0, 1, 1);
        int nextX = 0;
        //如果是自定义的换行可点击label，做特殊换行适配
        if (strName == "class ClickableLabel")
        {
            ClickableLabel* pLabel = (ClickableLabel*)wid;
            QFontMetrics fm(pLabel->font());
            //QRect r(effectiveRect);
            QString strText =  pLabel->m_strText;
            int nRightWidth = effectiveRect.right() - x - right;
            QRect singleRect = fm.boundingRect(effectiveRect,  int(Qt::AlignLeft | Qt::AlignVCenter ), strText);
            int nNeedWidth = singleRect.width();
            QString strDraw = strText;
            if (strDraw.isEmpty())
            {
                continue;
            }
            int nCul = 0;
            int nTop = 0;
            if (pLabel->m_pFriendWnd != nullptr)
            {
                std::string strFriendName = typeid(*(pLabel->m_pFriendWnd)).name();
                //如果换行点击控件绑定了lineedit，多做一些偏移以对齐
                if (strFriendName == "class MyLineEdit")
                {
                    MyLineEdit *pLineEdit = ( MyLineEdit *)pLabel->m_pFriendWnd;
                    if(pLineEdit->m_pPreLabel == pLabel)
                    {
                        bSingleLine = false;
                    }
//                    if(!testOnly && pLineEdit->m_strId == "7aa1570e-3828-4a97-98db-36136c7c4d64")
//                    {
//                        int a = 0;
//                        a = 1;
//                    }
                    nTop = 6;
                }
            }
            lineHeight = qMax(singleRect.height(), lineHeight);
            //如果父控件宽度不足
            while (nNeedWidth > nRightWidth)
            {
                //获取当前行可显示字符
                QString strLeft = fm.elidedText(strDraw, Qt::ElideRight, nRightWidth);
                if (strLeft.size() >= 1)
                {
                    strLeft = strLeft.left(strLeft.size() - 1);
                }
//                if(nRightWidth ==  effectiveRect.width() && strLeft.isEmpty())
//                {
//                    strLeft = strDraw.left(1);
//                }
                int nLeftSize = strLeft.size();
                bool bCreate = false;
                if(!testOnly && !strLeft.isEmpty())
                {
                    //添加子控件
                    ClickableLabel* pChildLabel = pLabel->AddChildLabel(nCul, strLeft, std::bind(&CLabelSplitLayout::ClickCall, this, std::placeholders::_1), bCreate);

                    QPoint pt(x - effectiveRect.x(), y - rect.y() + nTop);
                    //方便后续做点击事件判定
                    if (bCreate)
                    {
                        m_vecClick.push_back(stClickCheck(pChildLabel, QRect(x, y + nTop, nRightWidth, lineHeight)));
                    }
                    else
                    {
                        auto itFind = std::find(m_vecClick.begin(), m_vecClick.end(), pChildLabel);
                        if (itFind != m_vecClick.end())
                        {
                            itFind->rect = QRect(x, y + nTop, nRightWidth, lineHeight);
                        }
                    }
                    QRect rectChild(pt.x(), pt.y(), nRightWidth, lineHeight);
                    pChildLabel->setVisible(true);
                    pChildLabel->setGeometry(rectChild);
                    ++nCul;

                }
                //调整显示位置
                strDraw = strDraw.right(strDraw.size() - nLeftSize);
                nNeedWidth = fm.boundingRect(effectiveRect, int(Qt::AlignLeft | Qt::AlignVCenter), strDraw).width();
                nRightWidth = effectiveRect.width();
                y += spaceY + lineHeight;
                x = effectiveRect.x();

            }
            if (!testOnly)
            {
                pLabel->setVisible(true);
                pLabel->ShowAllChild(false);
            }

            bool bCreate = false;
            if (!testOnly)
            {
                //添加最后一行内容
                ClickableLabel* pChildLabel = pLabel->AddChildLabel(nCul, strDraw, std::bind(&CLabelSplitLayout::ClickCall, this, std::placeholders::_1), bCreate);
                QPoint pt(x - effectiveRect.x(), y - rect.y() + nTop);
                if (bCreate)
                {
                    m_vecClick.push_back(stClickCheck(pChildLabel, QRect(x, y + nTop, nNeedWidth, lineHeight)));
                }
                else
                {
                    auto itFind = std::find(m_vecClick.begin(), m_vecClick.end(), pChildLabel);
                    if (itFind != m_vecClick.end())
                    {
                        itFind->rect = QRect(x, y + nTop, nNeedWidth, lineHeight);
                    }
                }
                QRect rectChild(pt.x(), pt.y(), nNeedWidth, lineHeight);
                pChildLabel->setVisible(true);
                pChildLabel->setGeometry(rectChild);
                pChildLabel->m_rect = rectChild;
            }
            pLabel->m_nNowMax = nCul;
            nextX = x + nNeedWidth;

            //if (pLabel->m_strText.toStdString().data() == "是" || pLabel->m_strText.toStdString().data() == "否")
            //{
            //    int k = 0;
            //}

            auto str = pLabel->m_strText;
            if ((bSingleLine && nCulItem != nSum - 1) || nextX > effectiveRect.right() - right  )
            {
                //换行
                if (!m_signleLineSelect.contains(str))
                {
                    y += spaceY + lineHeight;
                    nextX = effectiveRect.x();
                }
            }
            else//不换行
            {
                nextX += spaceX;
            }
            bSingleLine = false;
            if (!testOnly)
            {
                QRect rectParent(effectiveRect.x(), rect.y(), effectiveRect.width(), y + lineHeight + bottom - rect.y() + nTop);
                pLabel->setGeometry(rectParent);
                pLabel->setVisible(true);
            }
        }
        else
        {
            //如果是其他 控件，根据控件尺寸做布局
            QSize size = item->sizeHint();
            nextX = x + size.width() + spaceX ;
            int nMoveY = 0;
            if (nextX  > effectiveRect.right() - right && lineHeight > 0) {
                x = effectiveRect.x();
                lineHeight = qMax(lineHeight, size.height());
                nMoveY = lineHeight + spaceY;
                y = y + nMoveY;
                nextX = x + size.width() + spaceX;
                lineHeight = 0;
            }
           // int nWidth = size.width();
            int nNeedY = y;
            if(nextX >=  effectiveRect.right() - right  )
            {
                nextX = effectiveRect.x();
                size.setWidth( effectiveRect.width() - right);
                nNeedY = y + nMoveY;
                if(nCulItem == nSum - 1)
                {
                    nNeedY = y;
                }
            }
            rectNow  = QRect(QPoint(x, y),size );
            if(!testOnly)
            {
                auto itFind = std::find(m_vecControl.begin(), m_vecControl.end(), wid);
                if (itFind != m_vecControl.end())
                {
                    itFind->rect = rectNow;
                }
                else
                {
                    m_vecControl.push_back(stClickCheck(wid, rectNow));
                }
                item->setGeometry(rectNow);
            }
            bSingleLine = true;
            y = nNeedY;
            lineHeight = qMax(lineHeight, item->sizeHint().height())/*+ spaceY*/;
        }
        ++nCulItem;
        x = nextX;
    }

    int nS = y + lineHeight  - effectiveRect.y()/* + bottom*//* - rect.y()*//* + bottom*/;

    return /*y + lineHeight - rect.y() + bottom*/nS;
}
