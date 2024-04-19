#include "clickablelabel.h"
#include <iostream>
ClickableLabel::ClickableLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)

{
  //  QLabel::setStyleSheet("QLabel {background-color: transparent;color:rgb(255,0,0);}");
}

ClickableLabel::ClickableLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : QLabel("", parent, f)
    ,m_strText(text)
{
}

ClickableLabel::~ClickableLabel()
{

}

ClickableLabel * ClickableLabel::AddChildLabel(int nCul, const QString &strText, ParentClickCall call, bool &bCreate)
{
    if(nCul < 0)
    {
        return nullptr;
    }
    else if( nCul < m_vecChildLabel.size())
    {
        //  return nullptr;
        ClickableLabel *pLabel = m_vecChildLabel[nCul];
        pLabel->setText(strText);
        pLabel->setVisible(isVisible());
        bCreate = false;
        return pLabel;
    }
    //QLabel::setText("");
    bCreate = true;
    ClickableLabel *pLabel = new ClickableLabel(strText, this) ;
    pLabel->setText(strText);
   // pLabel->setStyleSheet("QLabel {background-color: rgb(255,0,0);}");
    pLabel->m_bEnableDowm = true;
    connect(pLabel, SIGNAL(clicked()), this, SLOT(labelIsClicked()));
    pLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    pLabel->setVisible(isVisible());
    pLabel->setToolTip(m_strToolTips);
    m_vecChildLabel.push_back(pLabel);
    if(call != nullptr)
    {
        m_pCall = call;
    }
    return pLabel;
    //  return nullptr;
}

void ClickableLabel::ShowAllChild(bool bShow)
{
     for(auto &itChild : m_vecChildLabel)
     {
           itChild->setVisible(bShow);
     }

}

void ClickableLabel::ClearChild()
{
    for(auto &itChild : m_vecChildLabel)
    {
        itChild->setParent(nullptr);
        delete itChild;
        itChild = nullptr;
    }
    m_vecChildLabel.clear();
}

void ClickableLabel::setTextSelf(const QString &strText)
{
    m_strText = strText;
}

void ClickableLabel::setVisible(bool visible)
{

    m_bVisable = visible;
    QLabel::setVisible(visible);
    if(!visible)
    {
        ShowAllChild(visible);
    }
    else
    {
        int i = 0;
        for(auto &itChild : m_vecChildLabel)
        {
            if(i <= m_nNowMax)
            {
                itChild->setVisible(visible);
            }
            else
            {
                break;
            }
            ++i;
        }
    }
}

bool ClickableLabel::isVisible()
{
    return m_bVisable;
}

void ClickableLabel::show()
{
    setVisible(true);
}

void ClickableLabel::hide()
{

    setVisible(false);
}

void ClickableLabel::setHidden(bool hidden)
{
    setVisible(!hidden);
}

void ClickableLabel::setToolTip(const QString &strToolTips)
{
    m_strToolTips = strToolTips;
    QLabel::setToolTip(strToolTips);
    for(auto &itChild : m_vecChildLabel)
    {
        itChild->setToolTip(strToolTips);
    }
}

void ClickableLabel::setToolTip(const QString &strToolTips)
{
    m_strToolTips = strToolTips;
    QLabel::setToolTip(strToolTips);
    for(auto &itChild : m_vecChildLabel)
    {
        itChild->setToolTip(strToolTips);
    }
}


void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_bEnableDowm)
    {
        Q_UNUSED(event);
        emit clicked();
    }
    else if(m_pCall != nullptr)
    {
        m_pCall(mapToParent(event->pos()));
    }
}

void ClickableLabel::labelIsClicked()
{
    emit clicked();
}
