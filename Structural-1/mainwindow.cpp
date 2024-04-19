#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qpainter.h"
#include "cnetworkaccessmanager.h"
#include "qpushbutton.h"
#include "qscrollarea.h"
#include <iostream>
#include <fstream>
#include <QtConcurrent>

static std::chrono::time_point<std::chrono::high_resolution_clock> start;
static std::chrono::time_point<std::chrono::high_resolution_clock> end;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // TestText();
    auto s = clock();
    TestReport();
    auto e = clock();
     std::cout << "sumTime:" << e-s << std::endl;
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString strListJson;
QString strTreeJson;
QString strFirst;
QString strSecond;
void MainWindow::TestReport()
{
    CNetworkAccessManager* network = new CNetworkAccessManager;
    QString strAuthorization = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJuYW1laWQiOiJhZDBiNGI0ZS1kZjViLTQ0ODMtYTZlZS02YjE4NjA2MTZmMDYiLCJ1bmlxdWVfbmFtZSI6IueuoeeQhuWRmCIsIndpbmFjY291bnRuYW1lIjoiYWRtaW4iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9tb2JpbGVwaG9uZSI6IjE1ODU3NzcyNTQwIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbENvZGUiOiI0NzAwMDU5MzAzMzAzMDIxMUExMDAxIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbE5hbWUiOiLmn5Dmn5DljLvnp5HlpKflrabpmYTlsZ7nrKzkuozljLvpmaIiLCJLYXlpQ2xvdWRDbGFpbVR5cGVzLUhvc3BpdGFsSWQiOiI5OTlGRkNGQzgyQ0E0QjNCOTY0NjNGNUYzNzE3NDVCMCIsIktheWlDbG91ZENsYWltVHlwZXMtRGVwYXJ0bWVudElkIjoiNGZjYmE0ZDktMmE3Yy00ZTZkLThlNDQtOTliZTRkNzY1YTk2IiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1EZXBhcnRtZW50TmFtZSI6IuaUvuWwhOenkSIsImh0dHA6Ly9zY2hlbWFzLm1pY3Jvc29mdC5jb20vd3MvMjAwOC8wNi9pZGVudGl0eS9jbGFpbXMvZXhwaXJhdGlvbiI6IjQvMTgvMjAyNCAxMToxMzo0MCBBTSIsImh0dHA6Ly9zY2hlbWFzLnhtbHNvYXAub3JnL3dzLzIwMDUvMDUvaWRlbnRpdHkvY2xhaW1zL2hhc2giOiI1MGJlZjM2Zi1kZGZmLTQxZDItOGVjYi1hOGM0MTUyZWI4OTEiLCJuYmYiOjE3MTM0MzE2MjAsImV4cCI6MTcxMzQzODgyMCwiaWF0IjoxNzEzNDMxNjIwLCJpc3MiOiJLYXlpQ2xvdWQuVXNlclJpZ2h0U2VydmljZS5XZWJBcGkuSXNzdWVyIiwiYXVkIjoiS2F5aUNsb3VkLlVzZXJSaWdodFNlcnZpY2UuV2ViQXBpLkF1ZGllbmNlIn0.mv89c55Kue9z6WzjvrfvCZReT-QDT_nzCQkp4HYXAl0";




    network->setRawHeader("Authorization", strAuthorization.toLatin1());

    m_pStuctualWidget = new StructuralWidget("https://hidos-ris.kayisoft.dev",
                                             network, this, eTemplateType_Null);

    //http://192.168.0.156:8017/api/v1/reportdictionary/measureparameterlist?hospitalcode=47000593033030211A1001&systemcode=ES
    QString systemCode = "US";
    QString hospitalCode = "47000593033030211A1001";
    QString sex = QStringLiteral( "男");
    QString strAge;
    clock_t s = clock();
    m_pStuctualWidget->SetPatientPar(systemCode, hospitalCode,sex, strAge);
    clock_t e = clock();
    std::cout << "SetPatientPar:" << e -s << std::endl;
    showFullScreen ();
    //this->resize(1920, 1080);
    auto rect = this->rect();
    rect.setHeight(1080);
    m_pStuctualWidget->setMaximumWidth(368);
    //初始化界面
    //QFuture<void > end = QtConcurrent::run([=] {

    //    m_pStuctualWidget->InitStructuralData("CT", ("头颅"));
    //    });
    //while (!end.isFinished())
    //{
    //    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    //}

    m_pStuctualWidget->InitStructuralData(/*"", ("")*/);

    QPushButton *pBtnTest1 = new QPushButton("saveJson",this);
    QPushButton *pBtnTest2 = new QPushButton("setJson",this);
    QPushButton *pBtnTest3 = new QPushButton("test",this);
    QLineEdit* pEditTest = new QLineEdit(this);
    QPushButton *pBtnTest4 = new QPushButton("send template",this);
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->addWidget(ui->label);
    vLayout->addWidget(pBtnTest1);
    vLayout->addWidget(pBtnTest2);
    vLayout->addWidget(pBtnTest3);
    vLayout->addWidget(pEditTest);
    vLayout->addWidget(pBtnTest4);
    QHBoxLayout* hLayout = new QHBoxLayout;
    ui->centralwidget->setLayout(hLayout);
    ui->label->setWordWrap(true);
//    connect(m_pStuctualWidget, &StructuralWidget::measureParameterCheck, this, [=](const QStringList &measureParameterList)
//    {
//        QString str;
//        for(auto &itVec : measureParameterList)
//        {
//            str += itVec;
//        }
//        ui->label->setText(str);
//    });
    connect(m_pStuctualWidget, &StructuralWidget::explainParameterCheck, this, [&](const QStringList &explainParameterList)
    {
        QString str;
        for(auto &itVec : explainParameterList)
        {
            str += itVec;
        }
        ui->label->setText(str);
    });

    connect(pBtnTest1, &QPushButton::clicked, this, [=](bool bCheck)
    {
     //   m_pStuctualWidget->SetDepartment("MRI", QStringLiteral("腹部")/*, true*/);
        //获取保存的结构化json并保存
        QString strJson =m_pStuctualWidget->GetSaveReportJson();
        std::ofstream file("C:\\code\\json.txt");
        file << strJson.toStdString() << std::endl;
        file.close();
    });

    connect(pBtnTest2, &QPushButton::clicked, this, [=](bool bCheck)
    {
        m_vecWidget[1]->insertPlainText("[12|qwe]");
      //  m_vecWidget[1]->insertPlainText("12]");
       // m_vecWidget[0]->append("对方是否石帆胜丰");
//        m_pStuctualWidget->ClearCallBack();
//        m_pStuctualWidget->ResetTreeData();

//        m_pStuctualWidget->SetViewerType(eTemplateType_SRRa, false,strFirst, strSecond, strTreeJson, true);
//       // m_pStuctualWidget->ReloadTreeDataByJson(strFirst, strSecond, strTreeJson, true);
//        std::vector<QString> vecData = m_pStuctualWidget->GetDataTypeList();
////        if(vecData.empty())
////        {
////            vecData.push_back("");
////        }
//        auto itData = vecData.begin();
//        for(auto &itVec : m_vecWidget)
//        {
//            m_pStuctualWidget->SetCallBck(*itData,stCallBack{std::bind(&MyTextReportWidget::SetReportData, itVec,std::placeholders::_1, std::placeholders::_2, eCopyState_CopyDrawById),
//                                                           std::bind(&MyTextReportWidget::GetReportKeyValueJson, itVec)});
////            itVec->SetTransListDataCallBack(std::bind(&StructuralWidget::UpdateByReportData,
////                                                        m_pStuctualWidget,
////                                                        std::placeholders::_1));
//            //设置下listjson里的数据
//            itVec->SetReportStateByJson(strListJson);

//            ++itData;
//        }
//        QString strJson =m_pStuctualWidget->GetSaveReportJson();
//        std::ofstream file("d:\\4t.txt");
//        file << strJson.toStdString() << std::endl;
//        file.close();

    });

    connect(pBtnTest3, &QPushButton::clicked, this, [=](bool bCheck)
    {           

        for (auto it : m_pStuctualWidget->m_StructuralData.m_StructuralData.mapContext)
        {     
            for (auto itt : it.second.mapChild)
            {
                if (itt.second.strTitle == "T分期")                
                    int k = 0;          
                qDebug() << itt.second.strTitle;
            }      
            qDebug() << it.second.strTitle;
        }
    });

    connect(pBtnTest4, &QPushButton::clicked, this, [=](bool bCheck)
        {
           m_pStuctualWidget->sendTemplateData(pEditTest->text());

    
        });

    hLayout->addWidget(m_pStuctualWidget,1);

    QWidget *pReportWidget = new QWidget(this);
    QVBoxLayout* vLayout2 = new QVBoxLayout(pReportWidget);
    pReportWidget->setLayout(vLayout2);
    QScrollArea * scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(pReportWidget);

    hLayout->addWidget(scrollArea,1);
    hLayout->addLayout(vLayout,1);


    //读取测试json  该json为GetSaveReportJson返回完整json
    std::ifstream file("D:\\code\\json.txt", std::ios::binary);

    if(file.is_open())
    {
        //读取全部文件数据
        file.seekg(0, std::ios::end);
        int nLength = static_cast<int>(file.tellg());
        file.seekg(0, std::ios::beg);
        char *pBuffer = new char[nLength + 1];
        file.read(pBuffer, nLength);
        file.close();
        pBuffer[nLength] = '\0';
        QString strJson(pBuffer);
        delete []pBuffer;
        pBuffer = nullptr;


        QJsonParseError error;
        //解析json
        QJsonDocument jsonDocument = QJsonDocument::fromJson(strJson.toUtf8(),&error);

        if (error.error == QJsonParseError::NoError)
        {
            if (jsonDocument.isObject())
            {
                auto result = jsonDocument.toVariant().toMap();
                if(!result.empty())
                {
                    auto itFind = result.find("TreeJson");
                    //获取treejson  保存在2t.txt
                    if(itFind != result.end()  && !itFind->isNull() )
                    {
                        strTreeJson = itFind->toString();
                        std::ofstream outfile("d:\\2t.txt");
                        outfile << strTreeJson.toStdString() << std::endl;
                        outfile.close();
                    }
                     //ListJson
                    itFind = result.find("ReportJson");
                    if(itFind != result.end()  && !itFind->isNull())
                    {
                        strListJson = itFind->toString();
                        std::ofstream outfile("d:\\3t.txt");
                        outfile << strListJson.toStdString() << std::endl;
                        outfile.close();
                    }

                    //FirstId
                    itFind = result.find("FirstId");
                    if(itFind != result.end()  && !itFind->isNull())
                    {
                        strFirst = itFind->toString();
                    }

                    //SecondId
                    itFind = result.find("SecondId");
                    if(itFind != result.end()  && !itFind->isNull())
                    {
                        strSecond = itFind->toString();
                    }
                }
            }
        }
    }
    //切换显示页面

    s = clock();
   // start = std::chrono::high_resolution_clock::now();

  //  m_pStuctualWidget->SetDepartment("MRI", QStringLiteral("头颅")/*, true*/);
     std::vector<QString> vecData;
     vecData.push_back( QStringLiteral("报告诊断"));
     vecData.push_back( QStringLiteral("报告描述"));
     for(auto &itVec : vecData)
     {
         //创建数据显示页面 比之前多传输一个remarks
         MyTextReportWidget *pReport = new MyTextReportWidget(itVec, this);
         m_vecWidget.push_back(pReport);
         //设置回调 比之前多添加一个回调 并且设置remark
         m_pStuctualWidget->SetCallBck(itVec,stCallBack{std::bind(&MyTextReportWidget::SetReportData, pReport,std::placeholders::_1, std::placeholders::_2, eCopyState_CopyDrawById),
                                                        std::bind(&MyTextReportWidget::GetReportKeyValueJson, pReport)});
         pReport->SetTransListDataCallBack(std::bind(&StructuralWidget::UpdateByReportData,
                                                     m_pStuctualWidget,
                                                     std::placeholders::_1));
         vLayout2->addWidget(pReport);
         //设置下listjson里的数据
         pReport->SetReportStateByJson(strListJson);
     }
    m_pStuctualWidget->SetViewerType(eTemplateType_SRRa, false,strFirst, strSecond, "", true);
    e = clock();

  //
    std::cout << "SetViewerType:" << e -s << std::endl;
    //用treejson构建结构化列表

    s = clock();
     //报告数据
    //m_pStuctualWidget->ReloadTreeDataByJson(strFirst, strSecond, strTreeJson, true);

    e = clock();
    std::cout << "ReloadTreeDataByJson:" << e - s << std::endl;
    //m_pStuctualWidget->SetViewerType(eTemplateType_SRRa, false);
    //获取所有remarks类型  影像所见/诊断等
//    std::vector<QString> vecData = m_pStuctualWidget->GetDataTypeList();
//    if(vecData.empty())
//    {
//        vecData.push_back("");
//    }




    m_pStuctualWidget->templateRepaint = true;
}


//void MainWindow::TestText()
//{
//    m_pReportWidget = new MyTextReportWidget(this);
//    QPushButton *pBtnTest1 = new QPushButton("append",this);
//    m_pReportWidget->append(QStringLiteral("123[data1|data2|data3|data4]3232[data5|data6|data7|data8]gdf"));
//    connect(pBtnTest1, &QPushButton::clicked, this, [=](bool bCheck)
//    {
//        m_pReportWidget->insertPlainText(QStringLiteral(":[11|22|33|44][55|66|77|88]:"));
//    });
//   // SetReportData(&m_listReportData, "", eCopyState_NoCopy);
//    QHBoxLayout* hLayout = new QHBoxLayout;
//    ui->centralwidget->setLayout(hLayout);
//    hLayout->addWidget(m_pReportWidget,1);
//    hLayout->addWidget(pBtnTest1,1);
//}


