#include "mygroup.h"
#include "qlayout.h"
#include "qpainter.h"
#include "qpen.h"
#include "structdef.h"
#include "qsinglelinewidget.h"
MyGroup::MyGroup(QWidget *parent)
    :QWidget(parent)
{
    this->setAttribute(Qt::WA_StyledBackground);

    this->setStyleSheet(QString("MyGroup{border:1px solid rgb(183,198,221);border-radius:4px;background:rgb(220,231,244);} QLabel{ color:rgba(0,0,0,0.7); }"));
}

void MyGroup::resizeEvent(QResizeEvent *event)
{
    update();
}
struct stLayerRect
{
    MyQRect<int> parentRect;
    
    std::vector<MyQRect<int>> layoutRect;
    QRect toQRect()
    {
        return parentRect.toQRect();
    }
    /*  1.如果layoutRect中矩形的数量不超过1，那么清空layoutRect并返回。
        2.遍历layoutRect向量中的每个矩形，除了最后一个矩形，每个矩形的右边界被设置为它自身的右边界和下一个矩形左边界的平均值。这样做的目的是让矩形之间平均分配空间。
        3.在遍历结束时，移除layoutRect中的最后一个矩形，因为在调整过程中它的位置已被前一个矩形的调整覆盖。
    */
    void MathLayoutRect()
    {
        if((int)layoutRect.size() <= 1)
        {
            layoutRect.clear();
            return;
        }
        auto itBegin = layoutRect.begin();
        auto itEnd = layoutRect.end();
        auto itNext = itBegin;
        ++itNext;
        if(itNext == itEnd)
        {
            return;
        }
        int nLeft =itBegin->right;
        while(itBegin != itEnd)
        {
            if(itNext == itEnd)
            {
                itBegin->left = nLeft;
            }
            else
            {
                itBegin->right = (itBegin->right + itNext->left)/2;
                nLeft =  itBegin->right;
            }
            ++itBegin;
            if(itNext == itEnd)
            {
                break;
            }
            ++itNext;
        }
        auto itLast = itEnd;
        --itLast;
        layoutRect.erase(itLast);
    }
};

void MyGroup::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    MyQRect<int> nowRect = this->rect();
    nowRect.top += 4;
    nowRect.bottom -= 4;
    painter.setPen(QPen(QColor(183,198,221)));
    //获取布局
    auto itLayout = this->layout();
    //如果要构建列表
    if(m_bIsTable)
    {
        //列表数据，调整下
        std::vector<stLayerRect> vecRect;
        if(itLayout != nullptr)
        {
            //遍历布局的子布局/widget
            int nCount = itLayout->count();
            //std::cout << "nCount = " << nCount << std::endl;
            for(int i = 0; i < nCount; ++i)
            {
                auto it = itLayout->itemAt(i);
                auto childWidget = it->widget();
                QLayout *childLayout = nullptr;
                //如果是widget
                if(childWidget != nullptr &&  childWidget->isVisible())
                {
                    std::string strName = typeid(*childWidget).name();
                    /*std::cout << "strName = " << strName << std::endl;*/
                    //自定义列表数据都用QSingleLineWidget封装
                    if((int)strName.find("QSingleLineWidget") < 0)
                    {
                        continue;
                    }
                    QSingleLineWidget *pSingleWidget = ( QSingleLineWidget *)childWidget;
                    //记录下列表子布局的尺寸
                    if(pSingleWidget->m_bIsTable)
                    {
                        MyQRect<int> parentRect = childWidget->geometry();
                        childLayout = childWidget->layout();
                        if(childLayout != nullptr /*&& childLayout->isVisible()*/)
                        {
                            int nChildCount = childLayout->count();
                            //std::cout << "nChildCount = " << nChildCount << std::endl;
                            for(int j = 0; j < nChildCount; ++j)
                            {
                                auto itTmp = childLayout->itemAt(j);
                                stLayerRect layerRect;
                                if(itTmp != nullptr && itTmp->widget() != nullptr && itTmp->widget()->isVisible())
                                {
                                    MyQRect<int> childRect = itTmp->geometry();
                                    childRect.left += parentRect.left;
                                    childRect.right += parentRect.left;
                                    childRect.top += parentRect.top;
                                    childRect.bottom += parentRect.top;
                                    layerRect.parentRect = childRect.toQRect();
                                    auto pNodeLayout = itTmp->widget()->layout();
                                    if(pNodeLayout != nullptr)
                                    {
                                        int nNodeCount = pNodeLayout->count();
                                        for(int k = 0; k < nNodeCount-1; ++k)
                                        {
                                            auto itNodeTmp = pNodeLayout->itemAt(k);
                                            MyQRect<int> nodeRect = itNodeTmp->geometry();
                                            nodeRect.left += parentRect.left;
                                            nodeRect.right += parentRect.left;
                                            nodeRect.top += parentRect.top;
                                            nodeRect.bottom += parentRect.top;
                                            layerRect.layoutRect.push_back(nodeRect.toQRect());
                                        }

                                        //layerRect.MathLayoutRect();

                                    }
                                vecRect.push_back(layerRect);
                            }
                        }
                    }
                }

            }
        }
        }
        int nSize = (int)vecRect.size();
        if(nSize <= 0)
        {
            return;
        }

        auto itBegin = vecRect.begin();
        auto itEnd =  vecRect.end();

        QColor colorDraw = QColor(250,240,225);
        //计算子布局位置，绘制列表格
        bool bFirst = true;
        if(!m_bHasCombobox)
        {
            colorDraw = Qt::white;
            bFirst = false;
            ++itBegin;
        }
        if(itBegin == itEnd)
        {
            return;
        }
        auto rectTmp = itBegin->parentRect;
        painter.setBrush(colorDraw);
        while(itBegin != itEnd)
        {
            rectTmp.bottom = itBegin->parentRect.bottom;

            painter.drawRect(rectTmp.toQRect());
            if(bFirst)
            {
                colorDraw = Qt::white;
                painter.setBrush(colorDraw);
                bFirst = false;
            }
            for(auto & itChild : itBegin->layoutRect)
            {
                painter.drawLine(QPoint(itChild.right, rectTmp.top), QPoint(itChild.right, rectTmp.bottom));
            }
            rectTmp.top = rectTmp.bottom;
            ++itBegin;

        }

    }

}

