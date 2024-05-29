#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QDebug>
#include <QDialog>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QToolButton>

enum BtnType
{
    Modify, //新增、修改
    Delete, //删除
    Default //赞空
};

class MyPushButton:public QToolButton
{
    Q_OBJECT
public:
    MyPushButton(BtnType btntype,QWidget *parent = nullptr);
private:
    BtnType     m_btnType;
};

#endif // MYPUSHBUTTON_H
