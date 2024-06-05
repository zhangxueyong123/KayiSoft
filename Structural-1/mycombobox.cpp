#include "mycombobox.h"
#include <iostream>
//static QString g_parentId;
#include "ctemplatemanage.h"
#include "contrl_center.h"

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
        if (str == "")
            return;  
        //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << __FUNCDNAME__;
        bool flag = ControlCenter::getInstance()->isCreatedFinish();
        if (!flag)
            return;

        if(str.isEmpty())
        {
            if(m_pChildCombobox != nullptr)
            {
                m_pChildCombobox->clear();
            }

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
       //一级节点
        int n_level = ControlCenter::getInstance()->getComboboxLevelForTitle(str);
        if(m_pChildCombobox != nullptr && n_level == 1)
        {
            //m_pCallBack("", "", m_strId, n_level, &isNewChild);
            SetChild(m_pChildCombobox, m_strId);
        }
        //二三级节点
        else if(m_pCallBack != nullptr && n_level != 1)
        {
            QString parentId = ControlCenter::getInstance()->getParentID(strId);
            if (strId != "" && parentId != "")
            {
                //先把之前的选择过的json保存下来
                ControlCenter::getInstance()->m_currentFirstId = parentId;
                ControlCenter::getInstance()->m_currentSecondId = strId;
            }
            m_pCallBack(str,strId, parentId, n_level, &isNewChild);
            return;
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
            m_strBodyPart = ControlCenter::getInstance()->m_bodyPart;
            auto bodyParttList = m_strBodyPart.split(",");
            //if(m_strBodyPart.isEmpty() || itName.strShow == m_strBodyPart)
            if(m_strBodyPart.isEmpty() || bodyParttList.contains(itName.strShow ))
            {
                auto secId = ControlCenter::getInstance()->m_defaultSecondId;
                secId = ControlCenter::getInstance()->m_isUseDefault ? secId : "";
                if(nDefulat == -1 && /*itName.bSelect*/ secId == "" ? itName.bSelect : secId == itName.strId)
                {
                    nDefulat  = nCul;
                }
                m_pChildCombobox->addItem(itName.strName,itName.strId);
                
                m_pChildCombobox->m_mapComboboxId[itName.strId] = itName;
                
                ++nCul;
            }

        }
        m_pChildCombobox->setToolTip();
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
    if(strDepartment.isEmpty())
    {
        return;
    }
    QMutex mutex;
    mutex.lock();

    m_strBodyPart = strBodyPart;
    this->clear();
    auto map = ControlCenter::getInstance()->getFirst();

    for (auto it : map)
    {
        auto itMap = m_mapComboboxId.find(it);
        if (itMap->second.strShow == strDepartment)
            addItem(itMap->second.strName, itMap->second.strId);
    }
    //int size = m_mapComboboxId.size();
    //for(auto itMap : m_mapComboboxId)
    //{
    //    if(itMap.second.strShow == strDepartment)
    //        addItem(itMap.second.strName, itMap.second.strId);
    // 
    //}
    //setToolTip();
    if(this->count() != 0)
    {
        setCurrentIndex(0);
    }
    else
    {
        setCurrentText("");
    }
    mutex.unlock();
}

void MyComboBox::SetBodyPart(const QString &strBodyPart)
{

}

QWidget *MyComboBox::GetPtr()
{
    return this;
}
//禁用鼠标事件
void MyComboBox::wheelEvent(QWheelEvent* e)
{
}
//设置提示
void MyComboBox::setToolTip()
{
    auto str = currentIndex();
    QStandardItemModel* model = new QStandardItemModel();
    QStandardItem* item = new QStandardItem();//设置下拉框选择提示
    for (int i = 0; i < count(); i++)//RegionVo.list插入下拉框的数据
    {
        
        item = new QStandardItem(this->itemText(i));//下拉框显示文本
        item->setData(this->itemText(i));//显示文本对应的值,根据自己项目需要选择是否设置
        item->setToolTip(this->itemText(i));//设置提示类容
        model->appendRow(item);//将item添加到model中
    }
    this->setModel(model);//下拉框设置model
    setCurrentIndex(str);
}
//暂时无用，用于公式功能
void MyComboBox::setFormula(const QString& strFormula)
{
    m_strFormula = strFormula;
}
//仅用于快照combobox
void MyComboBox::Tdisconnect()
{
    disconnect(CTemplateManage::GetSingle(), &CTemplateManage::signChangeComboboxItem, this, &MyComboBox::slotChangeComboboxItem);
}
//仅用于快照combobox
void MyComboBox::Tconnect()
{
    connect(CTemplateManage::GetSingle(), &CTemplateManage::signChangeComboboxItem, this, &MyComboBox::slotChangeComboboxItem);
}
//仅用于快照combobox
void MyComboBox::slotChangeComboboxItem(QString title,bool sw)
{
    //删除
    if (!sw)
    {
        int index = findText(title);  // 查找文本匹配的项的索引
        if (index != -1) {  // 确保找到了该项
            removeItem(index);  // 删除找到的项
        }
        setCurrentIndex(0);
        ControlCenter::getInstance()->m_currentTemplateName = "--";
    }
    else
    {
        int index = findText(title);  // 查找文本匹配的项的索引
        if (index == -1) {  // 确保找到了该项
            addItem(title);
        }
        
        if (currentText() != title)
        {
            setCurrentText(title);
            ControlCenter::getInstance()->m_currentTemplateName = title;
        }
           
    }
}


