#include "mycheckbox.h"
#include "qpainter.h"

#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
MyCheckBox::MyCheckBox(const QString &strId,const QString &strTitle,   const QString &strPre,const QString &strSuff,
                       const QString &strDataTypeSummary,
                       StateChangeCallBack callBack, QWidget *parent)
    :m_strId(strId)
    ,m_strTitle(strTitle)
    ,m_strDataTypeSummary(strDataTypeSummary)
    ,m_pCallBack(callBack)
    ,QCheckBox(parent)
{
    //前后缀
    m_pPreLabel = new ClickableLabel(strPre, this);
    m_pSuffLabel = new ClickableLabel(strSuff, this);
    m_pPreLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_pSuffLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    //当前文本控件
    m_label = new ClickableLabel(strTitle, this);
    QStyleOptionButton opt;
    initStyleOption(&opt);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    //设置当前文本的点击事件
    connect(m_label, SIGNAL(clicked()), this, SLOT(labelIsClicked()));

    //状态事件
    connect(this, &MyCheckBox::stateChanged, [&](int nState)
    {
        //回调
        if(m_pCallBack != nullptr)
        {
            m_pCallBack(Qt::Checked == nState,m_strId,m_strTitle,m_bWait, &isNewChild);
            m_bWait = true;
        }
    });
    //  this->setContentsMargins(0, 0, 0, 0);
}

std::vector<stTableState> MyCheckBox::GetState()
{
    std::vector<stTableState> vec;
    vec.push_back(stTableState((this->checkState() ? QStringLiteral("true") : QStringLiteral("false")), m_strId, this->isVisible(),false));
    return vec;
}


QAbstractButton *MyCheckBox::GetPtr()
{
    return this;
}

void MyCheckBox::SetExplain(const QString &strExplain)
{
    m_strExplain = strExplain;
    if(m_label != nullptr )
    {
        m_label->setToolTip(m_strExplain);
    }
}

void MyCheckBox::setText(const QString &strTitle)
{
    m_label->setTextSelf(strTitle);
}

QString MyCheckBox::text() const
{
    return m_label->text();
}


void MyCheckBox::labelIsClicked()
{
    setChecked(!isChecked());
}

void MyCheckBox::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGeometry();
}

void MyCheckBox::setFormula(const QString& strFormula)
{
    m_strFormula = strFormula;
}
