#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <functional>
#include "structuraldata.h"
//点击回调
using ParentClickCall = std::function<void(const QPoint &pt)>;
using SetExclusiveCall = std::function<void(bool bExc)>;
//换行可点击label
class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    explicit ClickableLabel(const QString &text, QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel() override;
    //子控件列表
    std::vector<ClickableLabel *> m_vecChildLabel;
    //************************************
    // 方法:    AddChildLabel
    // 全称:  ClickableLabel::AddChildLabel
    // Access:    public static
    // 返回值:   ClickableLabel *
    // 描述: 添加子控件
    // 参数: int nCul 当前子控件需要，用于判断是获取还是新建
    // 参数: const QString &strText 子控件字符串
    // 参数: ParentClickCall call 子控件点击回调函数
    // 参数: bool &bCreate    输出是否为创建
    //************************************
    ClickableLabel * AddChildLabel(int nCul, const QString &strText, ParentClickCall call, bool &bCreate);
    //************************************
    // 方法:    ShowAllChild
    // 全称:  ClickableLabel::ShowAllChild
    // Access:    public static
    // 返回值:   void
    // 描述: 显示/隐藏全部子控件
    // 参数: bool bShow
    //************************************
    void ShowAllChild(bool bShow);
    //************************************
    // 方法:    ClearChild
    // 全称:  ClickableLabel::ClearChild
    // Access:    public static
    // 返回值:   void
    // 描述: 清空子控件
    //************************************
    void ClearChild();
    int m_nNowMax = 0;
    QString m_strText;
    //int m_nStart = 0;
    //************************************
    // 方法:    setTextSelf
    // 全称:  ClickableLabel::setTextSelf
    // Access:    public static
    // 返回值:   void
    // 描述: 清空子控件
    // 参数: const QString &strText 设置显示字符串
    //************************************
    void setTextSelf(const QString &strText);
    //************************************
    // 方法:    setVisible
    // 全称:  ClickableLabel::setVisible
    // Access:    public static
    // 返回值:   void
    // 描述: 显示/隐藏控件
    // 参数: bool visible
    //************************************
    void setVisible(bool visible);
    //************************************
    // 方法:    isVisible
    // 全称:  ClickableLabel::isVisible
    // Access:    public static
    // 返回值:   bool
    // 描述: 获取显示状态
    //************************************
    bool isVisible();
    void show();
    void hide();
    void setHidden(bool hidden);
    bool m_bEnableDowm = false;
    QWidget *m_pFriendWnd = nullptr;
    QRect m_rect;

    void setToolTip(const QString &strToolTips);
signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QString m_strToolTips;
    bool m_bVisable = false;
    ParentClickCall m_pCall = nullptr;
public slots:
    void labelIsClicked();
};
//包含控件和显示区域 用于校验点击事件
struct stClickCheck
{
    QWidget *pLabel = nullptr;
    QRect rect;
    stClickCheck()
    {

    }
    stClickCheck(const stClickCheck &click)
        :pLabel(click.pLabel)
        ,rect(click.rect)
    {

    }
    stClickCheck(QWidget *ptr, const QRect &r)
        :pLabel(ptr)
        ,rect(r)
    {

    }
    bool Check(const QPoint &pt)
    {
        return rect.contains(pt);
    }
    bool operator == ( QWidget *ptr) const
    {
        return pLabel == ptr;
    }
};

#endif // CLICKABLELABEL_H
