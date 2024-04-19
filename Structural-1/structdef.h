#ifndef STRUCTDEF_H
#define STRUCTDEF_H

#include "qrect.h"
#include <atomic>
#include <future>
#include "qstring.h"
//状态变化回调
using StateChangeCallBack = std::function<void(bool bCheck, const QString &strId, const QString &strTitle, bool bWait)>;
using ComboboxChangeCallBack = std::function<void(const QString &strSelect, const QString &strId ,const QString& parentId, const int& level)>;

//Rect, 因为QRect不好用  它设计为可以使用不同的数据类型来创建矩形的维度（例如int、float等）
template<class T>
struct MyQRect
{
    T left = 0;
    T right = 0;
    T top = 0;
    T bottom = 0;
    MyQRect()
    {

    }

    MyQRect(T l, T t, T r, T b)
        :left(l)
        ,right(r)
        ,top(t)
        ,bottom(b)

    {

    }
    MyQRect(const MyQRect & data)
        :left(data.left)
        ,right(data.right)
        ,top(data.top)
        ,bottom(data.bottom)
    {

    }
    MyQRect(const QRect & data)
        :left(data.left())
        ,right(data.right())
        ,top(data.top())
        ,bottom(data.bottom())
    {

    }


    T Width()
    {
        return right - left;
    }
    T Height()
    {
        return bottom - top;
    }

    T width()
    {
        return right - left;
    }
    T height()
    {
        return bottom - top;
    }
    T x()
    {
        return left;
    }
    T y()
    {
        return top;
    }
    void SetWidth(T w)
    {
        right = left + w;
    }
    QRect toQRect()
     {
        return QRect(left, top, right - left, bottom - top);
    }

};
//报告列表显示数据集
struct stSingleDrawData
{
    QString strDrawPre; //前缀
    QString strDrawData; //文本
    QString strDrawSuff; //后缀
    QString strDrawSuffAdd; //后缀符号
    QString strDrawDeleteBefer;
    int nDrawDeleteBefer = -1;
    std::vector<QString> vecData; //可选项文本
    std::vector<QString> vecId; //可选项id
    QString strRemarks/* = QStringLiteral("影像所见")*/; //文本类型
    QString strParentId;  //父类id
    bool bEdit = false;
    bool bMulti = false;
    bool bMultiFirst = false;
    bool bIsEndEdit = false;
    bool bReset = false;
    bool bSingleLineFirst = false;
    bool bIsDelete = false;
    bool bChangDir = false;
    bool bHasChild= false;
    bool bIsEnd = false;
    stSingleDrawData(const QString & pre, const QString &data, const QString &suff,const QString &remarks, const std::vector<QString>&vec,
                     const std::vector<QString>&Id
                     , bool edit, bool multi, bool multiFirst, bool changDir, bool hasChild)
        :strDrawPre(pre)
         ,strDrawData(data)
        ,strDrawSuff(suff)
        ,strRemarks(remarks)
        ,vecData(vec)
        ,vecId(Id)
        ,bEdit(edit)
        ,bMulti(multi)
        ,bMultiFirst(multiFirst)
        ,bChangDir(changDir)
        ,bHasChild(hasChild)
    {

    }
    stSingleDrawData()
    {

    }

};
const int MAXDRAWSIZE = 4;
struct stReportData
{
    stSingleDrawData drawData; //当行文本内容
    QString strRemarks;
    QString strPreId;
    QString strId;  //当前id
    int nLeft[MAXDRAWSIZE];
    int nRight[MAXDRAWSIZE];
    bool operator == (const QString &str) const
    {
        return strId == str;
    }
    void InitEmpty(const QString &strData, int nAddPos)
    {
       int nDataSize = strData.size();
       this->nLeft[0] = nAddPos;
       this->nRight[0] = this->nLeft[0] + nDataSize;
       for(int i = 1; i < MAXDRAWSIZE; ++i)
       {
           this->nLeft[i] =  this->nLeft[0];
           this->nRight[i] =  this->nRight[0];
       }
       this->drawData.strDrawPre = strData;
    }
};

using listReportData = std::vector<stReportData>;
using UpdateDataCallBack = std::function<void(listReportData *pReportData, const QString &strChangId)>;
using GetJsonCallBack = std::function<QString()>;
using GetStringListParameterCallBack = std::function<void(QStringList *)>;
using GetMeasureParameterCallBack = GetStringListParameterCallBack;
using GetExplainParameterCallBack = GetStringListParameterCallBack;

using GetStringParameterCallBack = std::function<void(QString*,QString*,QString*)>;
using GetTemplateDataCallBack = GetStringParameterCallBack;

using CheckCallBack = GetJsonCallBack;
struct stCallBack
{
    UpdateDataCallBack updateDataCallBack = nullptr;
    GetJsonCallBack getJsonCallBack = nullptr;
};

struct stKeyEx
{
    QString strLabelEx;
};
//有序map的通用key
struct stTableKey
{
    QString strKey;
    int nSort = -1;
    stKeyEx keyEx;
    stTableKey(const QString &key, int sort)
        :strKey(key)
        ,nSort(sort)
    {

    }
    stTableKey(const stTableKey &key)
        :strKey(key.strKey)
        ,nSort(key.nSort)
        ,keyEx(key.keyEx)
    {

    }
    bool operator == (const QString &str) const
    {
        return strKey == str;
    }
    bool operator == (const stTableKey &key) const
    {
        return strKey == key.strKey ;
    }
    bool operator < (const stTableKey &key) const
    {
        return nSort < key.nSort;
    }
};
#endif // STRUCTDEF_H
