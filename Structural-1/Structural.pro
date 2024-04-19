QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets \
                                        network  \
                                        qml\


CONFIG += c++17


msvc:{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CLabelSplitLayout.cpp \
   # HttpConnect.cpp \
    clickablelabel.cpp \
#   cmeasuredata.cpp \
    ctemplatemanage.cpp \
    main.cpp \
    mainwindow.cpp \
    mycheckbox.cpp \
    mycombobox.cpp \
    mygroup.cpp \
    mylayerwnd.cpp \
    mylineedit.cpp \
    mymeasureparameter.cpp \
    myradiobutton.cpp \
    mytextreportwidget.cpp \
    qsinglelinewidget.cpp \
    structuraldata.cpp \
    structuralwidget.cpp \
    wndsingle.cpp \
    CLabelSplitLayout.cpp \
    CNetWork.cpp \
    cnetworkaccessmanager.cpp

HEADERS += \
    CLabelSplitLayout.h \
   # CThredWork.h \
   # HttpConnect.h \
    clickablelabel.h \
    cmeasuredata.h \
    ctemplatemanage.h \
    mainwindow.h \
    mycheckbox.h \
    mycombobox.h \
    mygroup.h \
    mylayerwnd.h \
    mylineedit.h \
    mymeasureparameter.h \
    myradiobutton.h \
    mytextreportwidget.h \
    qsinglelinewidget.h \
    structdef.h \
    structuraldata.h \
    structuralwidget.h \
    wndsingle.h \
    CLabelSplitLayout.h \
 #   CNetWork.h \
    cnetworkaccessmanager.h

#LIBS +=-L../../lib \

#LIBS += -llibcurl_imp \

FORMS += \
    mainwindow.ui \
    structuralwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
