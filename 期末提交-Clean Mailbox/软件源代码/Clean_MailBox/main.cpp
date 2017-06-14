#include "widget.h"
#include "logindlg.h"
#include "database.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextCodec>

extern int id;
extern QString name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(":/2.qss");
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));

    if(!createConnection()){
        return 1;               //连接数据库
    }

    loginDlg *login = new loginDlg;

    login->show();
    return a.exec();
}
