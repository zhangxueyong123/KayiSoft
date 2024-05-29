#ifndef CONTROLCENTER_H
#define CONTROLCENTER_H

#include <QObject>
#include <QMap>
#include <QVector>
#include <QString>
#include <QComboBox>
#include <QHBoxLayout>
#include "cnetworkaccessmanager.h"
#include "mytextreportwidget.h"

//#include "structuraldata.h"
enum eTemplateType
{
    eTemplateType_Null = 0,
    eTemplateType_SRRa = 1, //����
    eTemplateType_SRUS = 2, //����
    eTemplateType_SRES = 4, //�ڿ���
    eTemplateType_All = eTemplateType_SRRa | eTemplateType_SRUS | eTemplateType_SRES,
};

struct BothId
{
    BothId(QString f, QString s)
    {
        firstId = f;
        secondId = s;
    }
    QString firstId;
    QString secondId;
    bool operator<(const BothId& other) const
    {
        if (firstId == other.firstId)
            return secondId < other.secondId;
        return firstId < other.firstId;
    }
    bool operator=(const BothId& other) const
    {
        if (firstId == other.firstId && secondId == other.secondId)
            return 1;
        return 0;
    }
};

class ControlCenter : public QObject
{
    Q_OBJECT
public:
    static ControlCenter* getInstance();
    void addFirstData(const QString& key, QString value);
    void addSecondData(const QString& key, const QString& value, const QString& first);
    bool isCreatedFinish();
    int getComboboxLevel(QString id);
    int getComboboxLevelForTitle(QString title);
    QString getParentID(QString id);
    QString getParentIDByTitle(QString title);
    eTemplateType   m_type = eTemplateType_Null;
    void setCombobox(QComboBox*);
    void initCombobox();
    QMap<QString, QMap<QString, QString>> getDataSecondMap();
    QComboBox* getTemplateCombobBox();
    QString     getJsonFromID(BothId);
    void setBtnStyleSheet(QString modifyBtn, QString deleteBtn);
    QString getBtnStyleSheet();
    void setOpenedReportJson(BothId d,QString json);
    QString getOpenedReportJson(BothId d);
    bool        m_isUseDefault = true;
    bool        needLoadAll = true;
    bool        firstLoadFinish = false;//��һ�δ򿪸÷ݱ��棬�Ƿ��Ѿ���ʼ�����
    //û�����ò��źͲ�λʱ����Ĭ�ϵ�firstId��secondId�������˲��źͲ�λ�Ժ���ǲ���Id�͵�һ��ƥ�䵽�Ĳ�λid
    QString     m_defaultFirstId;
    QString     m_defaultSecondId;
    QString     m_department;
    QString     m_bodyPart;
    //��һ�ݵĵ�firstId��secondId
    QString     m_beforeFirstId;
    QString     m_beforeSecondId;
    //��ǰ��firstId��secondId
    QString     m_currentFirstId;
    QString     m_currentSecondId;
    //��ǰ����ģ������
    QString     m_currentTemplateName = "";
    //QMap<BothId, QString>                    jsonMap;
    QMap<BothId, QVariantMap>       jsonMap;
    
signals:
    void signSendTemplateData(QString, eTemplateType);
    void signDeleteTemplateData(QString, eTemplateType);
    void signStructuralWidgetBeUsed();
    void signSaveOpenedReport(QString, QString);
private:
    explicit ControlCenter(QObject* parent = nullptr);
    static ControlCenter* instance;
    //key:id   value:title
    QMap<QString, QString>                  dataFirstMap;
    //out-key:firstid    inner-key:secondid  inner-value:secondtitle
    QMap<QString, QMap<QString,QString>>    dataSecondMap;
    QComboBox* m_currentTemplatebox = nullptr;
    QString     m_modifyBtnImagPath;
    QString     m_deleteBtnImagPath;
    QMap<BothId, QString>           openedReportJson;
};

#endif // CONTROLCENTER_H
