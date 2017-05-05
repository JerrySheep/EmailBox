#include "logindlg.h"
#include "widget.h"
#include "ui_logindlg.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

loginDlg::loginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginDlg)
{
    ui->setupUi(this);
    w = new Widget;
    connect(this,SIGNAL(sendUsrInfo(int,QString)),w,SLOT(getUsrInfo(int,QString)));
}

loginDlg::~loginDlg()
{
    delete ui;
}


/*
*/
void loginDlg::on_LoginBtn_clicked()
{
    int userId = ui->usrlineEdit->text().trimmed().toInt();
    int userPswd = ui->pwdlineEdit->text().toInt();

    QSqlQuery query;
    query.prepare("select count(*) from user where id=? and pswd=?");
    query.addBindValue(userId);
    query.addBindValue(userPswd);
    query.exec();
    query.next();

    if(query.value(0).toInt() > 0){
       query.prepare("select * from user where id=? and pswd=?");
       query.addBindValue(userId);
       query.addBindValue(userPswd);
       query.exec();
       query.first();

       this->hide();
       w->show();
       emit sendUsrInfo(userId, query.value(1).toString());
    }
    else{
        QMessageBox::warning(this,tr("Warning"),tr("user name or password error!"),QMessageBox::Yes);
        ui->usrlineEdit->clear();//清空用户名输入框
        ui->pwdlineEdit->clear();//清空密码输入框
        ui->usrlineEdit->setFocus();//移动光标至用户名输入框
    }
}
