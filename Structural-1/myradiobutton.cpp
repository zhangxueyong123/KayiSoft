#include "myradiobutton.h"
#include "qboxlayout.h"
#include "qpainter.h"
#include <QStyle>
#include <QStyleOptionButton>
#include <QStylePainter>
MyRadioButton::MyRadioButton(const QString &strId, const QString &strTitle,  const QString &strPre,const QString &strSuff,
                             const QString &strDataTypeSummary,
                             StateChangeCallBack callBack, QWidget *parent)
    :m_strId(strId)
    ,m_pCallBack(callBack)
    ,m_strDataTypeSummary(strDataTypeSummary)
    ,m_strTitle(strTitle)
      ,QRadioButton(parent)
{
    m_pPreLabel = new ClickableLabel(strPre, this);
    m_pSuffLabel = new ClickableLabel(strSuff, this);
    m_pPreLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_pSuffLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

    //当前文本控件
    m_label = new ClickableLabel(strTitle, this);
    QStyleOptionButton opt;
    initStyleOption(&opt);
    int indicatorW = style()->pixelMetric(QStyle::PixelMetric::PM_IndicatorWidth, &opt, this);
    m_label->setIndent(indicatorW+separation);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(m_label, SIGNAL(clicked()), this, SLOT(click()));
    //修改回调
    connect(this, &QRadioButton::toggled, [&](bool bCheck)
    {
        if(m_pCallBack != nullptr && (!m_bExclusive || bCheck))
        {
            m_pCallBack(bCheck,m_strId,m_strTitle ,m_bWait);
            m_bWait = true;
        }
    });
}

void MyRadioButton::SetExplain(const QString &strExplain)
{
    m_strExplain = strExplain;
}


std::vector<stTableState>  MyRadioButton::GetState()
{
    std::vector<stTableState> vec;
    vec.push_back(stTableState((isChecked() ? QStringLiteral("true") : QStringLiteral("false")), m_strId, this->isVisible(),false));
    return vec;
}

QAbstractButton *MyRadioButton::GetPtr()
{
    return this;
}

void MyRadioButton::setText(const QString &strTitle)
{
    m_label->setTextSelf(strTitle);
}

QString MyRadioButton::text() const
{
    return m_label->text();
}

//void MyRadioButton::labelIsClicked()
//{

//}

//void MyRadioButton::labelIsClicked()
//{
//    emit click();
//    setChecked(!isChecked());
//}

void MyRadioButton::click()
{
    if(isChecked())
    {
        m_bExclusive = false;
        if(m_ExclusiveCall != nullptr)
        {
            m_ExclusiveCall(false);
        }
        setChecked(false);
        if(m_ExclusiveCall != nullptr)
        {
            m_ExclusiveCall(true);
        }
        m_bExclusive = true;
    }
    else
    {
        setChecked(true);
    }
}

void MyRadioButton::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateGeometry();
}


