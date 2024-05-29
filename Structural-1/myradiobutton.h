#ifndef MYRADIOBUTTON_H
#define MYRADIOBUTTON_H

#include <QRadioButton>

#include "structdef.h"
#include "qlabel.h"
#include "clickablelabel.h"
#include "qboxlayout.h"
#include <qdatetime.h>
//自定义单选框
class MyRadioButton : public QRadioButton
{
    Q_OBJECT
public:
    MyRadioButton(const QString &strId,const QString &strTitle, const QString &strPre,const QString &strSuff,
                  const QString &strDataTypeSummary,
                  StateChangeCallBack callBack = nullptr, QWidget *parent = nullptr);
    //前缀后缀控件
    ClickableLabel *m_pPreLabel = nullptr;
    ClickableLabel *m_pSuffLabel = nullptr;
    SetExclusiveCall m_ExclusiveCall = nullptr;
    bool m_bIsButton = true;
    QString m_strId;
    QString m_strTitle;
    bool m_bWait = true;
    void SetExplain(const QString &strExplain);
    QString m_strExplain;
    std::vector<stTableState> GetState();
    QAbstractButton *GetPtr();
    //获取状态
    std::vector<stTableState> GetChildState() ;
    void setText(const QString &strTitle);
    QString text() const;
   // QSize sizeHint() const ;

    void setFormula(const QString& strFormula);

    ClickableLabel  *m_label = nullptr;
    QString     m_strFormula;
//public slots:
//    void labelIsClicked();
public Q_SLOTS:
    void click();
protected:
    void resizeEvent(QResizeEvent *event) ;
private:
    bool m_bExclusive = true;
    QString m_strDataTypeSummary;
    StateChangeCallBack m_pCallBack = nullptr;
    const int separation = 5;
    bool    isNewChild = true; 
};

#endif // MYRADIOBUTTON_H
