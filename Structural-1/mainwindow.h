#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mytextreportwidget.h"
#include "structuralwidget.h"
#include "qlineedit.h"
#include "explotwidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public slots:
    void on_pushButton_clicked();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void TestReport();
   // void TestText();

    void loadStructuralWidget(QString firstId,QString secondId,QString json);
    QString m_strJson;
private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_checkBox_clicked(bool checked);

private:
    MyTextReportWidget* pReport;
    std::vector<QString> vecData;
    ExplotWidget *m_pExplotWidget = nullptr;
    std::vector<MyTextReportWidget *> m_vecWidget;
    //MyTextReportWidget *m_pReportWidget = nullptr;
    //MyTextReportWidget *m_pReportWidget2 = nullptr;
  //  QTextEdit *m_pReportWidget3 = nullptr;
    Ui::MainWindow *ui;
    int m_cul = 0;
    QString systemCode = "US";
    QString hospitalCode = "47000593033030211A1001";
    QString sex = QStringLiteral("男");
    QString strAge;
    int count = 0;
  
};
#endif // MAINWINDOW_H
