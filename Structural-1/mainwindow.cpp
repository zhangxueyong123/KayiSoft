﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qpainter.h"
#include "cnetworkaccessmanager.h"
#include "qpushbutton.h"
#include "qscrollarea.h"
#include <iostream>
#include <fstream>
#include <QtConcurrent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    vecData.push_back(QStringLiteral("报告诊断"));
    vecData.push_back(QStringLiteral("报告描述"));
    for (auto& itVec : vecData)
    {
        //创建数据显示页面 比之前多传输一个remarks
        pReport = new MyTextReportWidget(itVec, this);
        m_vecWidget.push_back(pReport);
        ui->verticalLayout->addWidget(pReport, 0);
    }
    m_vecWidget[1]->setText("sjahndajklh fjklhafljkh ikljbhnlcjkvnbkjwhnoilqwjiolf nxcklvbnkwjehl");

    loadStructuralWidget("", "", "");
    //测试gitee
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadStructuralWidget(QString firstId, QString secondId, QString json)
{
    CNetworkAccessManager* network = new CNetworkAccessManager;
    QString strAuthorization = "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJuYW1laWQiOiJhZDBiNGI0ZS1kZjViLTQ0ODMtYTZlZS02YjE4NjA2MTZmMDYiLCJ1bmlxdWVfbmFtZSI6IueuoeeQhuWRmCIsIndpbmFjY291bnRuYW1lIjoiYWRtaW4iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9tb2JpbGVwaG9uZSI6IjE1ODU3NzcyNTQwIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbENvZGUiOiI0NzAwMDU5MzAzMzAzMDIxMUExMDAxIiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1Ib3NwaXRhbE5hbWUiOiLmn5Dmn5DljLvnp5HlpKflrabpmYTlsZ7nrKzkuozljLvpmaIiLCJLYXlpQ2xvdWRDbGFpbVR5cGVzLUhvc3BpdGFsSWQiOiI5OTlGRkNGQzgyQ0E0QjNCOTY0NjNGNUYzNzE3NDVCMCIsIktheWlDbG91ZENsYWltVHlwZXMtRGVwYXJ0bWVudElkIjoiNGZjYmE0ZDktMmE3Yy00ZTZkLThlNDQtOTliZTRkNzY1YTk2IiwiS2F5aUNsb3VkQ2xhaW1UeXBlcy1EZXBhcnRtZW50TmFtZSI6IuaUvuWwhOenkSIsImh0dHA6Ly9zY2hlbWFzLm1pY3Jvc29mdC5jb20vd3MvMjAwOC8wNi9pZGVudGl0eS9jbGFpbXMvZXhwaXJhdGlvbiI6IjYvNS8yMDI0IDc6MDg6MTYgQU0iLCJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcy9oYXNoIjoiNzU4ZTAzNDYtYmQ5Ny00MWNmLWIyZTctMTVlOWUwNTY1NjJlIiwibmJmIjoxNzE3NTY0MDk2LCJleHAiOjE3MTc1NzEyOTYsImlhdCI6MTcxNzU2NDA5NiwiaXNzIjoiS2F5aUNsb3VkLlVzZXJSaWdodFNlcnZpY2UuV2ViQXBpLklzc3VlciIsImF1ZCI6IktheWlDbG91ZC5Vc2VyUmlnaHRTZXJ2aWNlLldlYkFwaS5BdWRpZW5jZSJ9.Dolh0lCRsyS6rYSCb8kpQvTU4XzX0pzuUmKuMcofCt8";
    network->setRawHeader("Authorization", strAuthorization.toLatin1());
    if (m_pExplotWidget == nullptr)
    {
        m_pExplotWidget = new ExplotWidget("https://hidos-ris.kayisoft.dev", network, this, eTemplateType_Null);
        ui->horizontalLayout->addWidget(m_pExplotWidget, 1);
    }

    m_pExplotWidget->setTextEdit(m_vecWidget[0]);
    m_pExplotWidget->setTextEdit(m_vecWidget[1]);
    m_pExplotWidget->SetDepartment("CT", QStringLiteral("头颈"));
    m_pExplotWidget->SetViewerType(eTemplateType_SRRa, false, "" ,"","", true);
}

void MainWindow::on_pushButton_clicked()
{
    if (count++ % 2 != 0)
    {
        //m_pExplotWidget->SetPatientPar("RIS", "47000593033030211A1001", "男", "10Y");
        m_pExplotWidget->SetDepartment("CT", QStringLiteral("胸部"));
        //m_pExplotWidget->SetDepartment("US", QStringLiteral("产科"));
    }
    else
    {
        //m_pExplotWidget->SetPatientPar("RIS", "47000593033030211A1001", "男", "6Y");
        m_pExplotWidget->SetDepartment("MRI", QStringLiteral("头颈"));
        //m_pExplotWidget->SetDepartment("US", QStringLiteral("产科"));
    }

    ui->verticalLayout->removeWidget(m_vecWidget[0]);
    ui->verticalLayout->removeWidget(m_vecWidget[1]);
    m_vecWidget.clear();
    for (auto& itVec : vecData)
    {
        //创建数据显示页面 比之前多传输一个remarks
        pReport = new MyTextReportWidget(itVec, this);
        m_vecWidget.push_back(pReport);
        ui->verticalLayout->addWidget(pReport, 0);
    }
    m_pExplotWidget->setTextEdit(m_vecWidget[0]);
    m_pExplotWidget->setTextEdit(m_vecWidget[1]);

    m_vecWidget[1]->setText("sjahndajklh fjklhafljkh ikljbhnlcjkvnbkjwhnoilqwjiolf nxcklvbnkwjehl");
    m_pExplotWidget->SetViewerType(eTemplateType_SRRa, false, "", "", "", true);
}

void MainWindow::on_pushButton_2_clicked()
{

}

void MainWindow::on_pushButton_3_clicked()
{
    //loadStructuralWidget("", "", "");
}

void MainWindow::on_checkBox_clicked(bool checked)
{
    m_pExplotWidget->showAllDepartment(checked);
}
