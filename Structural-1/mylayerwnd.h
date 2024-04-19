#ifndef MYLAYERWND_H
#define MYLAYERWND_H

#include <QWidget>
#include "qgroupbox.h"
#include "qboxlayout.h"
#include "mygroup.h"

//自定义
class MyLayerWnd
{

public:
    explicit MyLayerWnd(QWidget *parent = nullptr);
    ~MyLayerWnd();

private:
public:
    void SetTitle(const QString &strTitle);
    void ShowWithTable(bool bHasCombobox);
    void ShowWnd(bool bShow);
    bool m_bIsEmpty = true;
    MyGroup *m_pGroupBox = nullptr;
    QVBoxLayout  *m_pLayout = nullptr;
    void AddWnd( QWidget * wnd, int nPos = -1);
    void AddLayout( QLayout * layout, int nPos = -1);
};

#endif // MYLAYERWND_H
