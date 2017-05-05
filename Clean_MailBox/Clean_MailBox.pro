#-------------------------------------------------
#
# Project created by QtCreator 2017-04-23T20:31:22
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Clean_MailBox
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    logindlg.cpp \
    wrtdlg.cpp

HEADERS  += widget.h \
    logindlg.h \
    database.h \
    wrtdlg.h

FORMS    += widget.ui \
    logindlg.ui \
    wrtdlg.ui

RESOURCES += \
    imags.qrc
