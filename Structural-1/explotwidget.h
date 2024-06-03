#ifndef EXPLOTWIDGET_H
#define EXPLOTWIDGET_H

#if defined(EXPLOTWIDGET_LIBRARY)
#  define EXPLOTWIDGETSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXPLOTWIDGETSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <QWidget>
#include <QString>
#include "mytextreportwidget.h"
#include "structuralwidget.h"

#ifndef ETEMPLATETYPE
#define ETEMPLATETYPE
enum eTemplateType
{
    eTemplateType_Null = 0,
    eTemplateType_SRRa = 1, //放射
    eTemplateType_SRUS = 2, //超声
    eTemplateType_SRES = 4, //内窥镜
    eTemplateType_All = eTemplateType_SRRa | eTemplateType_SRUS | eTemplateType_SRES,
};
#endif


class EXPLOTWIDGETSHARED_EXPORT ExplotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ExplotWidget(const QString &strUrl,
                              QString token,
                              QWidget *parent = nullptr,
                              eTemplateType eNowTemplateType = eTemplateType_Null,
                              eVersionType eApiVersion = eVersionType_V1);
   
    ~ExplotWidget();
    //设置病人相关参数
    void SetPatientPar(const QString & systemCode, const QString & hospitalCode, const QString & sex, const QString & strAge);

    bool SetViewerType(eTemplateType eViewerType, bool bReLoad, const QString &strFirstId, const QString &strSecondId, const QString &strJson, bool bSingle);

    void setTextEdit(QPointer<MyTextReportWidget>vec);

    void SetDepartment(const QString& strDepartment, const QString& strBodyPart);

    QString GetReportKeyValueJson();

    void ReloadTreeDataByNet();

    void SetReportStateByJson(const QString& strJson);

    void ClearCallBack();

    void ResetTreeData();

    QJsonObject GetSaveReportPar();

    void setBtnStyleSheet(QString modifyBtn, QString deleteBtn);

    void showAllDepartment(bool sw);

    void updataToken(QString token);
private:
    void reloadStructuralWidget(QString firstId, QString secondId, QString json);

    void setEditCallBack();

    void connectAll();

    void initStructuralWidget(QString firstId, QString secondId, QString json);
signals:
    void measureParameterCheck(const QStringList& measureParameterList);

    void explainParameterCheck(const QStringList& explainParameterList);
private:
    StructuralWidget* m_pStuctualWidget[2];
    QString m_systemCode = "US";
    QString m_hospitalCode = "47000593033030211A1001";
    QString m_sex = QStringLiteral("男");
    QString m_strAge;
    QString m_beforeDepartment;
    QString m_beforeBodyPart;
    QString m_strUrl;
    CNetworkAccessManager *m_network;
    QMap<QString,QPointer<MyTextReportWidget>> m_textEdit;
    QHBoxLayout * layout;

    bool isFirst = true;
    eTemplateType m_eViewerType;
    int     m_clu = 0;
    QTimer timer;
//    static int count;
    QString  m_token;
    bool m_updateDataCallBackEnable = false;
};

#endif // EXPLOTWIDGET_H
