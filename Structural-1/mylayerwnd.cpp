#include "mylayerwnd.h"
MyLayerWnd::MyLayerWnd(QWidget *parent)
{
    m_pGroupBox = new MyGroup();
    m_pLayout = new QVBoxLayout();
    m_pLayout->setContentsMargins(0,0,0,0);
    m_pLayout->setSpacing(0);
    m_pGroupBox->setLayout(m_pLayout);

}

MyLayerWnd::~MyLayerWnd()
{
}

void MyLayerWnd::SetTitle(const QString &strTitle)
{
    m_pGroupBox->m_strTitle = strTitle;
}

void MyLayerWnd::ShowWithTable(bool bHasCombobox)
{
    m_pGroupBox->m_bIsTable = true;
    m_pGroupBox->m_bHasCombobox = bHasCombobox;
}

void MyLayerWnd::ShowWnd(bool bShow)
{
    m_pGroupBox->setVisible(bShow);
}

void MyLayerWnd::AddWnd(QWidget *wnd, int nPos)
{
    if(nPos != -1)
        m_pLayout->insertWidget(nPos,wnd);
    else
        m_pLayout->addWidget(wnd);
    
    m_bIsEmpty = false;
}

void MyLayerWnd::AddLayout(QLayout *layout, int nPos)
{
    if (layout)
    {
        if(nPos != -1)
        {
            m_pLayout->insertLayout(nPos, layout);
        }
        else
        {
            m_pLayout->addLayout(layout);
        }
        m_bIsEmpty = false;
    }
}

