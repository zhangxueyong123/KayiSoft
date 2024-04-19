#ifndef MYGROUP_H
#define MYGROUP_H

#include <QGroupBox>
#include <QWidget>
#include "structdef.h"
//自定义group  用于构建自定义列表框
class MyGroup : public QWidget
{
    Q_OBJECT
public:
    QString m_strTitle;
    MyGroup(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    bool m_bIsTable = false;
     bool m_bHasCombobox = false;
    QColor m_bkColor = QColor(220,231,244);
};

#endif // MYGROUP_H
