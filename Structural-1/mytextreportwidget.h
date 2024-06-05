#ifndef MYTEXTREPORTWIDGET_H
#define MYTEXTREPORTWIDGET_H


#if defined(MYTEXTREPORTWIDGET_LIBRARY)
#  define MYTEXTREPORTWIDGETSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MYTEXTREPORTWIDGETSHARED_EXPORT Q_DECL_IMPORT
#endif

#include "qtextedit.h"
#include "structdef.h"
#include "qevent.h"
#include <map>
#include "qtimer.h"
//获取显示时的处理方式
enum eCopyState
{
    eCopyState_NoCopy = 0,      //不拷贝
    eCopyState_CopyDrawById,        //根据修改id拷贝
    eCopyState_CopyAll,     //拷贝全部
};
//自适应模式
enum eChangeDataMode
{
    eChangeDataMode_NoLimit,    //没有限制，根据实际数据来
    eChangeDataMode_LimitByMin = 1, //被setMinimumHeight限制
    eChangeDataMode_LimitByMax = 2, //被setMaximumHeight限制
    eChangeDataMode_LimitByMinAndMax = eChangeDataMode_LimitByMin | eChangeDataMode_LimitByMax,
};
//高亮显示
struct stColorShow
{
    QString strData; //高亮关键词
   // QStringList list;
    QColor color;   //颜色
    stColorShow()
    {

    }
    stColorShow(const QString &s, const QColor &c)
        :strData(s)
        ,color(c)
    {

    }
};
//高亮位置
struct stHighLight
{
    int highLightPos = 0;   //高亮左侧位置
    int highLightRight = 0; //高亮右侧为位置
    QColor color;  //高亮颜色
};
using listHighLight = std::vector<stHighLight>;
using listColorShow = std::vector<stColorShow>;
//自定义多行文本编辑器
class MYTEXTREPORTWIDGETSHARED_EXPORT MyTextReportWidget : public QTextEdit
{
    Q_OBJECT
public:
    explicit MyTextReportWidget(const QString &strRemarks, QWidget *parent = nullptr);
    void SetRemarks(const QString &strRemarks);
    //设置结构化数据
    void SetReportData(listReportData *pListData, const QString &strChangeId, eCopyState copy);
    //设置列表修改回调
    void SetTransListDataCallBack(UpdateDataCallBack callBack);
    //获取列表数据
    QString GetListData();
    //添加字符串数据
    void append(const QString &strData);
    void insertPlainText(const QString &strData);
    //清空
    void clear();
    //设置字符串数据
    void setPlainText(const QString &text);
    //获取结构化json
    QString GetReportKeyValueJson();
    //设置结构化json
    void SetReportStateByJson(const QString &strJson);

    eChangeDataMode GetChangeDataMode() {return m_ChangeDataMode;}

    //设置高度根据数据自适应
    // 参数 bool bFlg 是否自适应
    // 参数 bool eChangeDataMode mode  设置自适应限制
    void SetChangeHeightByData(bool bFlg, eChangeDataMode mode = eChangeDataMode_LimitByMin);
    bool GetChangeHeightByDataFlg(){return m_ChangeHeightByDataFlg;}
    //设置自适应范围
    void SetMinAndMaxLimit(int nMin, int nMax);
    //设置每行标号
    // 参数 bool bShow 是否显示标号
    // 参数 bool bDeletEmptyLine 是否删除空行
    // 参数 bool bSend 是否刷新界面，默认刷新
    void ChangeShowNumberState( bool bSend = true);
    bool SetShowNumber(bool bShow, bool bDeletEmptyLine, bool bSend = true);
   // bool GetShowNumberFlg(){return m_bShowNumberFlg;}
    //删除空行
    void DeleteEmptyLine();
    void EnableRecover(bool bEnable, int nMaxSize){m_bEnableReCover = bEnable;m_nMaxSize = nMaxSize;}

    //添加高亮列表
    void SetColorShowList(const QStringList &list,const QColor &color);
    void AddColorShow(const QStringList &list,const QColor &color);
    void AddColorShow(const QString &strData,const QColor &color);
    void ClearColorShow();
   // bool eventFilter(QObject *obj, QEvent *e);
    void ChangeHeightByData();

    void setLineSpacing(int space);

    QString GetRemarks()
    {
        return m_strRemarks;
    }
signals:
    void signal_clicked();

private:
    int m_lineSpacing = 0;
    bool m_bSendChange = false;
    QTimer *m_pTimer = nullptr;
    std::atomic_int m_timerTimes;
    void SendData();
    QStringList CheckColorShow(const QStringList &list,const QColor &color);
    void ReDrawList();
    void CopyList( listReportData *pListData,const QString &strChangeId, eCopyState copy);
    void ResetListLineState();
    //    void GetAllId();
    //    std::map<QString, int> m_AllId;
    //找到列表迭代器
    listReportData::iterator FindReportIt(int nPos, int *pCul);
    listColorShow m_ListColorShow;
    //找到变化位置
    int FindChangeLeft(const QString &strNow);
    listReportData m_listReportData;
    bool m_bEnableReCover = true;
    int m_nMaxSize = 10;
    bool m_bChangePos = true;
    std::list<listReportData> m_oldList;

    void AddOldList();
    void ClearOldList();
    UpdateDataCallBack m_TransListDataCallBack = nullptr;
    QString m_strChangeId;
    std::map<QString, QAction *> m_mapAction;
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e) ;
    void IsButtonChangePostion();
    void IsInputChangePostion();
    
    QString m_strOldData;
   // int m_nPos = 0;
    bool m_bStick = false;
    bool m_bSelectAll = false;
    int m_nSelectLeft =  -1;
    int m_nSelectRight = -1;
    int m_nSelectChangeLeft=  -1;
    int m_nSelectChangeRight = -1;
    int m_nNowSize = 0;
    bool m_bHasChange = false;
    bool m_ChangeHeightByDataFlg = false;
   // bool m_bShowNumberFlg = false;
    int m_nMinLimit = 0;
    int m_nMaxLimit = INT_MAX;
    int m_ClickTimes = 0;
    eChangeDataMode m_ChangeDataMode = eChangeDataMode_LimitByMin;
    bool AddPostionData(const QString &str, bool bChange);
    bool DeletePostionData(int nLeft, int nRight);
    void keyPressEvent(QKeyEvent *e);
    QString m_strRemarks;
    QString m_currentDrawPre = "";
    bool    m_isFirst = true;
public slots:
    void OnTimeOut();
    void OnMenuSelect();
    void PositionChanged();

};

#endif // MYTEXTREPORTWIDGET_H
