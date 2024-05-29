#include "contrl_center.h"

ControlCenter* ControlCenter::instance = nullptr;

QComboBox* ControlCenter::getTemplateCombobBox()
{
    return m_currentTemplatebox;
}

QString ControlCenter::getJsonFromID(BothId b)
{
    QString str = "";
    //auto it = jsonMap.find(b);
    //if (it != jsonMap.end())
    //{
    //    str = it.value();
    //}
    return str;
}

void ControlCenter::setBtnStyleSheet(QString modifyBtn, QString deleteBtn)
{
    m_modifyBtnImagPath = modifyBtn;
    m_deleteBtnImagPath = deleteBtn;
}

void ControlCenter::initCombobox()
{
    m_currentTemplatebox->setCurrentIndex(0);
}

void ControlCenter::setCombobox(QComboBox*com)
{
    m_currentTemplatebox = com;
}

QString ControlCenter::getBtnStyleSheet()
{
    return m_modifyBtnImagPath + ";" + m_deleteBtnImagPath;
}

void ControlCenter::setOpenedReportJson(BothId d, QString json)
{
    auto findit = openedReportJson.find(d);
    if (findit == openedReportJson.end())
    {
        openedReportJson.insert(d, json);
    }
    else
    {
        findit.value() = json;
    }
}

QString ControlCenter::getOpenedReportJson(BothId d)
{
    auto findit = openedReportJson.find(d);
    if (findit == openedReportJson.end())
    {
        return "";
    }
    else
    {
        return findit.value();
    }
}

ControlCenter::ControlCenter(QObject* parent) : QObject(parent)
{
}

ControlCenter* ControlCenter::getInstance()
{
    if (!instance)
    {
        instance = new ControlCenter();
    }
    return instance;
}

void ControlCenter::addFirstData(const QString& key,QString value)
{
    auto find = dataFirstMap.find(key);
    //一级节点不存在 ，创建
    if (find == dataFirstMap.end())
    {
        QVector<QString> temp;
        dataFirstMap.insert(key, value);
    }
    //存在，忽视
}

QMap<QString, QMap<QString, QString>> ControlCenter::getDataSecondMap()
{
    return dataSecondMap;
}

void ControlCenter::addSecondData(const QString& key, const QString& value, const QString& first)
{
    auto find = dataFirstMap.find(first);
    //一级节点存在
    if (find != dataFirstMap.end())
    {
        auto secondFfind = dataSecondMap.find(first);
        //不存在，先新增
        if (secondFfind == dataSecondMap.end())
        {
            QMap<QString, QString> temp;
            dataSecondMap.insert(first, temp);
            secondFfind = dataSecondMap.find(first);
        }
        secondFfind.value().insert(key, value);
  
        int k = 0;
    }
}



bool ControlCenter::isCreatedFinish()
{
    if (dataFirstMap.size() > 0)
        return true;
    else
        return false;
}

int ControlCenter::getComboboxLevel(QString id)
{
    auto keys = dataFirstMap.keys();
    if (keys.contains(id))
    {
        return 1;
    }

    // 遍历 dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  // 外部 QMap 的值，即内部的 QMap

        // 遍历内部的 QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // 内部 QMap 的键
            QString innerValue = innerIt.value();  // 内部 QMap 的值
            if (innerKey == id)
                return 2;
        }
    }

    return 3;
}
//返回1，1级节点；返回2，2级节点；返回3，不定
int ControlCenter::getComboboxLevelForTitle(QString title)
{
    auto values = dataFirstMap.values();
    if (values.contains(title))
    {
        return 1;
    }
    // 遍历 dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  // 外部 QMap 的值，即内部的 QMap

        // 遍历内部的 QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // 内部 QMap 的键
            QString innerValue = innerIt.value();  // 内部 QMap 的值
            if (innerValue == title)
                return 2;
        }
    }
    return 3;
}

QString ControlCenter::getParentID(QString id)
{
    QString result = "";
    // 遍历 dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  

        // 遍历内部的 QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // 内部 QMap 的键
            QString innerValue = innerIt.value();  // 内部 QMap 的值
            if (innerKey == id)
                result = it.key();
        }
    }
    return result;
}
QString ControlCenter::getParentIDByTitle(QString title)
{
    QString targetFirstid = "";
    // 遍历 dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();

        // 遍历内部的 QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // 内部 QMap 的键
            QString innerValue = innerIt.value();  // 内部 QMap 的值
            if (innerValue == title)
                targetFirstid = it.key();
        }
    }
    //auto it = dataFirstMap.find(targetFirstid);
    //return it.value();
    return targetFirstid;
}
