#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include "qcombobox.h"

#include "structdef.h"
#include "qlabel.h"
#include "clickablelabel.h"
#include <qobject.h>
#include <qmutex.h>
#include <QStandardItemModel>
struct ComboboxInfo
{
    QString title;
    QString id;
};
//static QMap<ComboboxInfo, ComboboxInfo> g_comboBoxMap;

//combobox数据
struct stNameAndId
{
    QString strId;
    QString strName;
    QString strOther;
    QString strShow;
    bool bSelect = false;
};

struct comboboxInfo
{
    QString id;
    QString title;
    //QString parId;
};




using MapNameAndIdList =  std::map<QString,std::vector<stNameAndId>> ;
//自定义combobox类
class MyComboBox : public QComboBox
{
    Q_OBJECT
public:
    MyComboBox(const QString &strId,
               const QString &strTitle,
               const QString &strPre,
               const QString &strSuff,
               const QString &strDataTypeSummary,
               ComboboxChangeCallBack callBack = nullptr, QWidget *parent = nullptr);

    //combobox的id映射
    std::map<QString, stNameAndId> m_mapComboboxId;
    void setChecked(bool bCheck) {}
     //前缀及后缀控件
    ClickableLabel *m_pPreLabel = nullptr;
    ClickableLabel *m_pSuffLabel = nullptr;
    SetExclusiveCall m_ExclusiveCall = nullptr;
    bool m_bWait = true;
    //标注并非按钮控件
    bool m_bIsButton = false;
    void setText(const QString & strText) {}
    void SetExplain(const QString &strExplain);
    QString m_strExplain;
    //设置子控件显示
    void SetChild(MyComboBox *pChild, const QString &strChildId);
    QString m_strId;
    
    //获取选中状态
    std::vector<stTableState> GetState();
    //关联子控件
    MyComboBox *m_pChildCombobox = nullptr;
    //子控件的数据
    MapNameAndIdList m_mapNextNameAndId;
    void SetDepartment(const QString &strDepartment, const QString & strBodyPart);
    void SetBodyPart( const QString & strBodyPart);
    QString m_strTitle;
    //当前label， 为空
    ClickableLabel  *m_label = nullptr;
    QWidget *GetPtr();
    int     level = -1;
    
    //禁止滚轮事件
    void wheelEvent(QWheelEvent* e);

    void setToolTip();
    void setFormula(const QString& strFormula);
    void Tdisconnect();
    void Tconnect();
    QString m_strFormula;

protected:
    virtual void mousePressEvent(QMouseEvent* e);  //添加鼠标点击事件

signals:
    void clicked();  //自定义点击信号，在mousePressEvent事件发生时触发

public slots:

    void slotChangeComboboxItem(QString title,bool sw);

private:
    QString m_strDataTypeSummary;
    QString m_strBodyPart;
    ComboboxChangeCallBack m_pCallBack = nullptr;
    bool    isNewChild = true;
    QMutex     m_mutex;
};
#endif // MYCOMBOBOX_H
