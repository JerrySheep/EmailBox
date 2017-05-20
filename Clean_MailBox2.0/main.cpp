#include "widget.h"
#include "logindlg.h"
#include "database.h"
#include <QApplication>
#include <QDebug>
#include <QTextCodec>

extern int id;
extern QString name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GB2312"));
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
    if(!createConnection()){
        return 1;               //连接数据库
    }

    loginDlg *login = new loginDlg;

    login->show();
    return a.exec();
}
