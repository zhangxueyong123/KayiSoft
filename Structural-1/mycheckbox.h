#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include <QCheckBox>

#include "structdef.h"
#include "qlabel.h"
#include "clickablelabel.h"
#include "qboxlayout.h"
//自定义checkbox
class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    MyCheckBox(const QString &strId,const QString &strTitle,  const QString &strPre,
               const QString &strSuff,
               const QString &strDataTypeSummary,
               StateChangeCallBack callBack = nullptr, QWidget *parent = nullptr);
    //前缀及后缀控件
    ClickableLabel *m_pPreLabel = nullptr;
    ClickableLabel *m_pSuffLabel = nullptr;
    SetExclusiveCall m_ExclusiveCall = nullptr;
    bool m_bIsButton = true;
    QString m_strId;
    QString m_strTitle;
    bool m_bWait = true;
    //获取当前选中状态
    std::vector<stTableState> GetState();
    QAbstractButton *GetPtr();
    void SetExplain(const QString &strExplain);
    //设置显示字符串
    void setText(const QString &strTitle);
    QString text() const;
    //当前label控件
    ClickableLabel  *m_label = nullptr;

    QString m_strExplain;
    void setFormula(const QString& strFormula);
    QString m_strFormula;
public slots:
    void labelIsClicked();
protected:
    void resizeEvent(QResizeEvent *event) ;
private:
    QString m_strDataTypeSummary;
    //状态变化回调
    StateChangeCallBack m_pCallBack = nullptr;
    const int separation = 5;
    bool    isNewChild = true;
};

#endif // MYCHECKBOX_H
