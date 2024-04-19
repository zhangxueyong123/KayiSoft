#ifndef MYCOMBOBOX_H
#define MYCOMBOBOX_H

#include "qcombobox.h"

#include "structdef.h"
#include "qlabel.h"
#include "clickablelabel.h"
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

<<<<<<< HEAD
struct comboboxInfo
{
    QString id;
    QString title;
    //QString parId;
};




=======
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
using MapNameAndIdList =  std::map<QString,std::vector<stNameAndId>> ;
//自定义combobox类
class MyComboBox : public QComboBox
{
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
    static QMap<QString, QMap<QString, comboboxInfo>> m_comboboxInfoMap;
    static QString getFirstId(QString SecondId)
    {
        QString firstid = "";
        
        //for (auto it : m_comboboxInfoMap)
        //{
        //    if (it.find(SecondId) != it.end())
        //    {
        //        auto index = it.find(SecondId);
        //        QString s = index.key();
        //    }
        //}
        auto keys = m_comboboxInfoMap.keys();
        auto values = m_comboboxInfoMap.values();
        for (int i = 0; i < keys.size(); i++)
        {
            if (values[i].find(SecondId) != values[i].end())
                firstid = keys[i];
        }
        return firstid;
    }
private:
    QString m_strDataTypeSummary;
    QString m_strBodyPart;
    ComboboxChangeCallBack m_pCallBack = nullptr;
};
#endif // MYCOMBOBOX_H
