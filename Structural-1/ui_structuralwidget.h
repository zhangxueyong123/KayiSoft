/********************************************************************************
** Form generated from reading UI file 'structuralwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STRUCTURALWIDGET_H
#define UI_STRUCTURALWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StructuralWidget
{
public:

    void setupUi(QWidget *StructuralWidget)
    {
        if (StructuralWidget->objectName().isEmpty())
            StructuralWidget->setObjectName(QString::fromUtf8("StructuralWidget"));
        StructuralWidget->resize(400, 300);

        retranslateUi(StructuralWidget);

        QMetaObject::connectSlotsByName(StructuralWidget);
    } // setupUi

    void retranslateUi(QWidget *StructuralWidget)
    {
        StructuralWidget->setWindowTitle(QCoreApplication::translate("StructuralWidget", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StructuralWidget: public Ui_StructuralWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STRUCTURALWIDGET_H
