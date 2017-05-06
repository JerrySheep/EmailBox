#include "widget.h"
#include "logindlg.h"
#include "database.h"
#include <QApplication>

extern int id;
extern QString name;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loginDlg *login = new loginDlg;

    if(!createConnection())
        return 1;               //连接数据库

    login->show();
    return a.exec();
}
