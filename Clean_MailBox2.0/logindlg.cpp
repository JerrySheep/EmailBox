#include "logindlg.h"
#include "widget.h"
#include "ui_logindlg.h"
#include "popLogIn.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>

Widget *w;

loginDlg::loginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginDlg)
{
    ui->setupUi(this);
    QSqlQuery q;
    w = new Widget;
    q.exec("select * from user where last=1");
    if(q.next()){
        ui->usrlineEdit->setText(q.value(0).toString());
        ui->pwdlineEdit->setText(q.value(2).toString());
        ui->saveButton->setChecked(1);
    }
    connect(this,SIGNAL(sendUsrInfo(QString,QString)),w,SLOT(getUsrInfo(QString,QString)));
}

loginDlg::~loginDlg()
{
    delete ui;
    delete w;
}


/*
*/
void loginDlg::on_LoginBtn_clicked()
{
    string userId = ui->usrlineEdit->text().trimmed().toStdString();
    string userPswd = ui->pwdlineEdit->text().toStdString();

    //"13551365646@163.com","435390541yanhao"

    if(logIn(string("user ")+userId+string("\t\n"),
             string("pass ")+userPswd+string("\t\n"))){
       QSqlQuery query;
       query.exec("update user set last=0 where last=1");
       if(ui->saveButton->isChecked()){
           //判断是否之前已经登录过
           query.prepare("select * from user where id=?");
           query.addBindValue(QString::fromStdString(userId));
           query.exec();
           if(query.next()){
               //登录过则直接更新
               query.prepare("update user set last=1 where id=?");
               query.addBindValue(QString::fromStdString(userId));
               query.exec();
           }else{
               //未登陆过则插入
               query.prepare("insert into user values(?, NULL, ?, 1)");
               query.addBindValue(QString::fromStdString(userId));
               query.addBindValue(QString::fromStdString(userPswd));
               query.exec();
           }
           qDebug() << "account save succed!";
       }
       QSqlQuery q;
       q.prepare("select * from user");
       q.exec();
       while(q.next()){
           qDebug() << q.value(0).toString() << q.value(2).toString() << q.value(3).toInt();
       }

       emit sendUsrInfo(QString::fromStdString(userId), QString::fromStdString(userPswd));
       this->accept();
       w->show();
       //发送用户账号、密码至主界面
    }
    else{
        QMessageBox::warning(this,QStringLiteral("失败！"),QStringLiteral("请检查账号、密码以及网络连接"),QMessageBox::Yes);
        //ui->usrlineEdit->clear();//清空用户名输入框
        ui->pwdlineEdit->clear();//清空密码输入框
        ui->usrlineEdit->setFocus();//移动光标至用户名输入框
    }
}
