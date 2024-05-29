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
    //һ���ڵ㲻���� ������
    if (find == dataFirstMap.end())
    {
        QVector<QString> temp;
        dataFirstMap.insert(key, value);
    }
    //���ڣ�����
}

QMap<QString, QMap<QString, QString>> ControlCenter::getDataSecondMap()
{
    return dataSecondMap;
}

void ControlCenter::addSecondData(const QString& key, const QString& value, const QString& first)
{
    auto find = dataFirstMap.find(first);
    //һ���ڵ����
    if (find != dataFirstMap.end())
    {
        auto secondFfind = dataSecondMap.find(first);
        //�����ڣ�������
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

    // ���� dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  // �ⲿ QMap ��ֵ�����ڲ��� QMap

        // �����ڲ��� QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // �ڲ� QMap �ļ�
            QString innerValue = innerIt.value();  // �ڲ� QMap ��ֵ
            if (innerKey == id)
                return 2;
        }
    }

    return 3;
}
//����1��1���ڵ㣻����2��2���ڵ㣻����3������
int ControlCenter::getComboboxLevelForTitle(QString title)
{
    auto values = dataFirstMap.values();
    if (values.contains(title))
    {
        return 1;
    }
    // ���� dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  // �ⲿ QMap ��ֵ�����ڲ��� QMap

        // �����ڲ��� QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // �ڲ� QMap �ļ�
            QString innerValue = innerIt.value();  // �ڲ� QMap ��ֵ
            if (innerValue == title)
                return 2;
        }
    }
    return 3;
}

QString ControlCenter::getParentID(QString id)
{
    QString result = "";
    // ���� dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();  

        // �����ڲ��� QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // �ڲ� QMap �ļ�
            QString innerValue = innerIt.value();  // �ڲ� QMap ��ֵ
            if (innerKey == id)
                result = it.key();
        }
    }
    return result;
}
QString ControlCenter::getParentIDByTitle(QString title)
{
    QString targetFirstid = "";
    // ���� dataSecondMap
    for (auto it = dataSecondMap.begin(); it != dataSecondMap.end(); ++it)
    {
        QMap<QString, QString> innerMap = it.value();

        // �����ڲ��� QMap
        for (auto innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt)
        {
            QString innerKey = innerIt.key();  // �ڲ� QMap �ļ�
            QString innerValue = innerIt.value();  // �ڲ� QMap ��ֵ
            if (innerValue == title)
                targetFirstid = it.key();
        }
    }
    //auto it = dataFirstMap.find(targetFirstid);
    //return it.value();
    return targetFirstid;
}
