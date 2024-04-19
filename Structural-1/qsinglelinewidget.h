#ifndef QSINGLELINEWIDGET_H
#define QSINGLELINEWIDGET_H

#include <QWidget>
//自定义列表当行widget  为了区分其他widget
class QSingleLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QSingleLineWidget(QWidget *parent = nullptr);
    bool m_bIsTable = false;
    QString m_strTitle;
    void *m_pWndSingle = nullptr;
signals:

};

#endif // QSINGLELINEWIDGET_H
