#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mytextreportwidget.h"
#include "structuralwidget.h"
#include "qlineedit.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void TestReport();
    void Test();
   // void TestText();
    QString m_strJson;
private:
    StructuralWidget *m_pStuctualWidget = nullptr;
    std::vector<MyTextReportWidget *> m_vecWidget;
    //MyTextReportWidget *m_pReportWidget = nullptr;
    //MyTextReportWidget *m_pReportWidget2 = nullptr;
  //  QTextEdit *m_pReportWidget3 = nullptr;
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
