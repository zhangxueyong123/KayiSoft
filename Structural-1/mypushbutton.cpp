#include "mypushbutton.h"
#include "contrl_center.h"
MyPushButton::MyPushButton( BtnType btntype,QWidget *parent):m_btnType(btntype)
{
    connect(this,&QToolButton::clicked,this,[=](){
        QString currentTemplateName = ControlCenter::getInstance()->m_currentTemplateName;
        qDebug() << "m_btnType = " << m_btnType << "currentTemplateName = " << currentTemplateName;
        
        if (m_btnType == BtnType::Modify)
        {
            ////弹出一个界面，输入快照名称，新增或者修改
            //QInputDialog dialog(this, Qt::Dialog | Qt::WindowCloseButtonHint);
            //dialog.setWindowIcon(QIcon());
            //dialog.setWindowTitle("输入快照名称");

            //QVBoxLayout* layout = new QVBoxLayout(&dialog);

            //QLabel* label = new QLabel("请输入名称：", &dialog);
            //layout->addWidget(label);

            //QLineEdit* lineEdit = new QLineEdit(&dialog);
            //layout->addWidget(lineEdit);
            //lineEdit->setPlaceholderText(currentTemplateName);

            //QPushButton* okButton = new QPushButton("确定", &dialog);
            //layout->addWidget(okButton);

            //connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

            //if (dialog.exec() == QDialog::Accepted) {
            //    QString name = lineEdit->text();
            //    if(name != "")
            //        ControlCenter::getInstance()->signSendTemplateData(name, ControlCenter::getInstance()->m_type);
            //}
            bool bRet = false;

            //单文本输入框 
            QString text = QInputDialog::getText(this,"输入快照名称" ,
                "请输入名称：", QLineEdit::Normal, currentTemplateName, &bRet, Qt::WindowCloseButtonHint);
            if (bRet && !text.isEmpty())
            {
                ControlCenter::getInstance()->signSendTemplateData(text, ControlCenter::getInstance()->m_type);
                qDebug() << "press ok text = " << text;
            }
            else
            {
                qDebug() << "press Cancel text = " << text;
            }

        }
        else if (m_btnType == BtnType::Delete)
        {
            //删除当前快照
            if (currentTemplateName == "--")
            {
                QMessageBox::information(this, "警告", "无法删除默认模板");
            }
            else
                ControlCenter::getInstance()->signDeleteTemplateData(currentTemplateName, ControlCenter::getInstance()->m_type);
        }
        else
        {

        }
    });
}
