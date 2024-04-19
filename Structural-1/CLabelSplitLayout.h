

#ifndef CFLOWLAYOUT_H
#define CFLOWLAYOUT_H


#include <QLayout>
#include <QRect>
#include <QStyle>
#include "clickablelabel.h"
//换行排版layout  适配label换行效果
class CLabelSplitLayout : public QLayout
{
public:
    explicit CLabelSplitLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit CLabelSplitLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~CLabelSplitLayout();
    void addItem(QLayoutItem *item) Q_DECL_OVERRIDE;
    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;
    bool hasHeightForWidth() const Q_DECL_OVERRIDE;
    int heightForWidth(int) const Q_DECL_OVERRIDE;
    int count() const Q_DECL_OVERRIDE;
    QLayoutItem *itemAt(int index) const Q_DECL_OVERRIDE;
    QSize minimumSize() const Q_DECL_OVERRIDE;
    void setGeometry(const QRect &rect) Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QLayoutItem *takeAt(int index) Q_DECL_OVERRIDE;

    std::vector<stClickCheck> m_vecClick;
    std::vector<stClickCheck> m_vecControl;
   void ClickCall(const QPoint &pt);
public:
private:
   int m_nHeight = 0;
   //主要布局实现
    int doLayout(const QRect &rect, bool testOnly) ;
   //int smartSpacing(QStyle::PixelMetrizc pm) const;

    QList<QLayoutItem *> itemList;
    QStringList         m_signleLineSelect;
    int m_hSpace;
    int m_vSpace;
};

#endif // CLabelSplitLayout
