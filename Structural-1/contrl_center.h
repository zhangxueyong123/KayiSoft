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
    eTemplateType_SRRa = 1, //放射
    eTemplateType_SRUS = 2, //超声
    eTemplateType_SRES = 4, //内窥镜
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
    bool        firstLoadFinish = false;//第一次打开该份报告，是否已经初始化完成
    //没有设置部门和部位时就是默认的firstId和secondId，设置了部门和部位以后就是部门Id和第一个匹配到的部位id
    QString     m_defaultFirstId;
    QString     m_defaultSecondId;
    QString     m_department;
    QString     m_bodyPart;
    //上一份的的firstId和secondId
    QString     m_beforeFirstId;
    QString     m_beforeSecondId;
    //当前的firstId和secondId
    QString     m_currentFirstId;
    QString     m_currentSecondId;
    //当前快照模板名称
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
