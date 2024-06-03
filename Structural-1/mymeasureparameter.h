#ifndef MYMEASUREPARAMETER_H
#define MYMEASUREPARAMETER_H

#include <QObject>
#include <QJsonObject>
#include "qwidget.h"
//#include "CNetWork.h"
#include "mycheckbox.h"
#include "myradiobutton.h"
#include "mylineedit.h"
typedef struct
{
    QString fieldName;
    double normalMin = -1.;
    double normalMax = -1.;
    QJsonObject jsonInfo;
    QString formula;
    QStringList dependentParameters;
    QString warning;
    QString parameterBigreminder;
    QString parameterSmallreminder;
    QString parameterNormalreminder;
    int parameterId;
    bool  operator == (int nId) const
    {
        return (this->parameterId == nId);
    }
} MY_MEASURE_PARAMETER_INFO;

class MyMeasureParameter
{

public:
    MyMeasureParameter(const  MyMeasureParameter &par)
    {
        m_hashMeasureParameterInfo = par.m_hashMeasureParameterInfo;
        m_url = par.m_url;
        m_network = par.m_network;
    }
    MyMeasureParameter(const QString &strUrl, CNetworkAccessManager *network, QObject *parent = nullptr);

    void getMeasureParameters(const QString & tableName, const QString & systemCode, const QString & hospitalCode, const QString & sex, const QString & strAge);
    //  void
    MY_MEASURE_PARAMETER_INFO GetMeasureParameter( int nCode);
    std::map<int, MY_MEASURE_PARAMETER_INFO> CheckNeedMath(const QString &fieldName);
    std::map<int, MY_MEASURE_PARAMETER_INFO> CheckNeedMath(int nId);
    int GetID(const QString & fieldName);
private:
    QStringList getMatchingStrings(QString str, QString pattern);

private:
    QHash<int, MY_MEASURE_PARAMETER_INFO> m_hashMeasureParameterInfo;
    QString m_url;
    CNetworkAccessManager* m_network;
};

#endif // MyMeasureParameter_H
struct stMeasureParameterWidget
{
    QWidget * pWidget = nullptr;
    QString strId;
    stMeasureParameterWidget(QWidget * pW, const QString &id)
        :pWidget(pW),
          strId(id)

    {

    }
   // bool bIsEdit = false;
};
enum eMeasureParameterWidgetState
{
    eMeasureParameterWidgetState_NoFind = 0,
    eMeasureParameterWidgetState_Small ,
    eMeasureParameterWidgetState_Normal ,
    eMeasureParameterWidgetState_Big ,

};

class MyMeasureParameterManage /*: public QObject*/
{
public:
    void AddWidget( int nId, stMeasureParameterWidget par);
    void ClearWidget();
    void AddMeasureParameters(const QString &tableName);
    void ClearMeasureParameters();
    void ClearAll();
    void SetCheckPar(const QString & systemCode, const QString & hospitalCode, const QString & sex, const QString & strAge);
    static MyMeasureParameterManage* GetSingle(const QString &strUrl, CNetworkAccessManager *network, QObject *parent = nullptr);
    static MyMeasureParameterManage* GetSingle();
    QString CheckWidget(const QString &strId, const QString &strText);
    void ChangeWidgetValue(const  QString &strId);

private:
    MyMeasureParameterManage(){}
    MyMeasureParameterManage(const QString &strUrl, CNetworkAccessManager *network, QObject *parent = nullptr);
    int GetID(const QString & fieldName);
    bool m_bInit = false;
    static MyMeasureParameterManage* m_pSinglePtr ;
    std::map<int, std::vector<stMeasureParameterWidget>> m_mapWidget; //ParameterId
    std::map<QString, MyMeasureParameter*> m_mapMeasureParameter; //table:
    QString m_strUrl;
    CNetworkAccessManager *m_pNetwork = nullptr;
    QObject *m_pParent = nullptr;
    QString m_systemCode;
    QString  m_hospitalCode;
    QString m_sex;
    QString m_strAge;
};
