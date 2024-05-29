#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H


#include "qlineedit.h"

#include "structdef.h"
#include "qlabel.h"
#include "clickablelabel.h"
//自定义编辑框
class MyLineEdit :public QLineEdit
{
public:
    MyLineEdit(const QString &strId,const QString &strTitle,   const QString &strPre,const QString &strSuff,
               const QString &strDataTypeSummary,
               StateChangeCallBack callBack = nullptr, QWidget *parent = nullptr);

    void setChecked(bool bCheck) {};
    void SetExplain(const QString &strExplain);
    QString m_strExplain;
    //前缀后缀控件
    ClickableLabel *m_pPreLabel = nullptr;
    ClickableLabel *m_pSuffLabel = nullptr;
    ClickableLabel  *m_label = nullptr;
    SetExclusiveCall m_ExclusiveCall = nullptr;
    bool m_bIsButton = false;
    QString m_strId;
    bool m_bWait = true;
    //获取子控件状态
    std::vector<stTableState> GetChildState() ;
    QString m_strTitle;
    QWidget *GetPtr();
    //获取控件状态
    std::vector<stTableState> GetState();
    void setFormula(const QString& strFormula);
    QString     m_strFormula;
private:

    QString m_strDataTypeSummary;
    StateChangeCallBack m_pCallBack = nullptr;
    bool    isNewChild = true;
};



#endif // MYLINEEDIT_H
