#include "mycombobox.h"
#include <iostream>
<<<<<<< HEAD
//static QString g_parentId;
=======
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c

QMap<QString, QMap<QString, comboboxInfo>>MyComboBox:: m_comboboxInfoMap;

MyComboBox::MyComboBox(const QString &strId, const QString &strTitle,const QString &strPre, const QString &strSuff,
                       const QString &strDataTypeSummary,
                       ComboboxChangeCallBack callBack, QWidget *parent)
    :m_strId(strId)
    ,m_strTitle(strTitle)
    ,m_strDataTypeSummary(strDataTypeSummary)
    ,m_pCallBack(callBack)
    ,QComboBox(parent)
{
    m_pPreLabel = new ClickableLabel(strPre, this);
    m_pSuffLabel = new ClickableLabel(strSuff, this);
    m_pPreLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_pSuffLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    //选中事件
    connect(this, &QComboBox::currentTextChanged,  [&](const QString &str)
    {
        if(str.isEmpty())
        {
            if(m_pChildCombobox != nullptr)
            {
                m_pChildCombobox->clear();
            }
<<<<<<< HEAD

=======
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
        }
        QString strId;
        for(auto &itMap : m_mapComboboxId)
        {
            if(itMap.second.strName == str)
            {
                strId = itMap.first;
                m_strId = strId;
            }      
        }
       
        if(m_pChildCombobox != nullptr)
        {
            SetChild(m_pChildCombobox, m_strId);
            m_pCallBack("", "", m_strId,level);
        }
        else
        {
            if(m_pCallBack != nullptr)
            {
                m_pCallBack(str,strId, "", level);
            }
        }

    });
}

void MyComboBox::SetExplain(const QString &strExplain)
{
    m_strExplain = strExplain;
}

void MyComboBox::SetChild(MyComboBox *pChild, const QString &strChildId)
{
    m_pChildCombobox = pChild;
    if(m_pChildCombobox == nullptr)
    {
        return;
    }
    //找到子控件id
    auto itFindNameList = m_mapNextNameAndId.find(strChildId);

    if (itFindNameList != m_mapNextNameAndId.end())
    {
        QMap<QString, comboboxInfo> m;
        
        for (auto it : itFindNameList->second)
        {
            comboboxInfo temp;
            temp.id = it.strId;
            temp.title = it.strName;
            m.insert(it.strId,temp);
        }
        m_comboboxInfoMap.insert(strChildId, m);
        //m_comboboxInfoMap.insert(std::pair(strChildId,std::pairitFindNameList.s)));
    }

    if(itFindNameList != m_mapNextNameAndId.end())
    {
        m_pChildCombobox->blockSignals(true);
        m_pChildCombobox->clear();
        int nDefulat = -1;
        int nCul = 0;
        //遍历子控件数据 构建子combobox
        for(auto &itName : itFindNameList->second)
        {
            //std::cout << itName.strShow.toStdString() << std::endl;
            if(m_strBodyPart.isEmpty() || itName.strShow == m_strBodyPart)
            {
                if(nDefulat == -1 && itName.bSelect)
                {
                    nDefulat  = nCul;
                }
                m_pChildCombobox->addItem(itName.strName,itName.strId);
                m_pChildCombobox->m_mapComboboxId[itName.strId] = itName;
<<<<<<< HEAD
                
=======
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
                ++nCul;
            }

        }
        if(nDefulat == -1)
        {
            nDefulat = 0;
        }
        m_pChildCombobox->blockSignals(false);
        if(/*!itFindNameList->second.empty()*/m_pChildCombobox->count() > 0)
        {
            m_pChildCombobox->setCurrentIndex(nDefulat);
            emit m_pChildCombobox->currentTextChanged(m_pChildCombobox->currentText());
        }

    }

}

std::vector<stTableState> MyComboBox::GetState()
{
    std::vector<stTableState> vec;
    for(auto &itMap : m_mapComboboxId)
    {
        QString strState = ((itMap.first == m_strId) ?  QStringLiteral("true") : QStringLiteral("false"));
        vec.push_back(stTableState(strState, itMap.first, this->isVisible(),false));
    }
    return vec;
}

void MyComboBox::SetDepartment(const QString &strDepartment, const QString &strBodyPart)
{
<<<<<<< HEAD
    if(strDepartment.isEmpty())
=======
    if(strDepartment.isEmpty() )
>>>>>>> e597408f3cb2168e95ad9886b485034f994e0b7c
    {
        return;
    }
    m_strBodyPart = strBodyPart;
    this->clear();
    for(auto &itMap : m_mapComboboxId)
    {
        if(itMap.second.strShow == strDepartment)
        {
            addItem(itMap.second.strName, itMap.second.strId);
//            auto itFindChild = m_mapNextNameAndId.find(itMap.second.strId);
//            if(itFindChild != m_mapNextNameAndId.end())
//            {
//                for(auto &itChild := itFindChild->second)
//                {
//                    itChild.strShow = strBodyPart;
//                }
//            }
            //strId = itMap.first;
        }
    }
    if(this->count() != 0)
    {
        setCurrentIndex(0);
    }
    else
    {
        setCurrentText("");
    }

}

void MyComboBox::SetBodyPart(const QString &strBodyPart)
{

}



QWidget *MyComboBox::GetPtr()
{
    return this;
}

