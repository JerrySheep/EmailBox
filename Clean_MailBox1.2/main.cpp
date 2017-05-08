#include "widget.h"
#include "logindlg.h"
#include "database.h"
#include <QApplication>
#include <QDebug>

extern int id;
extern QString name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(!createConnection()){
        qDebug() << "lianjiecuowu";
        return 1;               //连接数据库
    }

    loginDlg *login = new loginDlg;

    login->show();
    return a.exec();
}
