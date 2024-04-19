#include "mylineedit.h"
#include "qvalidator.h"
MyLineEdit::MyLineEdit(const QString &strId, const QString &strTitle,  const QString &strPre,const QString &strSuff,
                       const QString &strDataTypeSummary,
                       StateChangeCallBack callBack , QWidget *parent )
    :m_strId(strId)
    ,m_strTitle(strTitle)
    ,m_strDataTypeSummary(strDataTypeSummary)
    ,m_pCallBack(callBack)
    ,QLineEdit(parent)
{
    m_pPreLabel = new ClickableLabel(strPre, this);
    m_pSuffLabel = new ClickableLabel(strSuff, this);
    m_pPreLabel->m_pFriendWnd = this;
    m_pSuffLabel->m_pFriendWnd = this;
    //修改回调
    connect(this, &QLineEdit::textChanged, [&](const QString &textChanged)
    {
        if(m_pCallBack != nullptr)
        {
            m_pCallBack(false,m_strId,textChanged ,m_bWait);
            m_bWait = true;
        }
    });
    if(strDataTypeSummary == "1")
    {
        setValidator(new QDoubleValidator(-10000, 10000,4,this)/*new QRegExpValidator(QRegExp("[0-9]+$"))*/);
    }
}

void MyLineEdit::SetExplain(const QString &strExplain)
{
    m_strExplain = strExplain;
}

std::vector<stTableState> MyLineEdit::GetChildState()
{
    return std::vector<stTableState>();
}

QWidget *MyLineEdit::GetPtr()
{
    return this;
}

std::vector<stTableState> MyLineEdit::GetState()
{
    std::vector<stTableState> vec;
    vec.push_back(stTableState(text(), m_strId, this->isVisible(),true));
    return vec;
}

