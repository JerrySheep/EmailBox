#include "wrtdlg.h"
#include "ui_wrtdlg.h"
#include "sendMail.h"
#include <thread>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>
#include <QTextCodec>
#include <QDateTime>
#include <QPropertyAnimation>

WrtDlg::WrtDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WrtDlg)
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("邮件编辑"));
    this->setWindowIcon(QIcon(":images/email.png"));

}

WrtDlg::~WrtDlg()
{
    delete ui;
}

/*
 * 作为槽函数从主界面获取当前信息，设置当前信息
 * 参数列表：
 * getId: 用户账户; getPswd: 用户密码; drfId: 草稿id
 */
void WrtDlg::getUsrInfo(QString getId, QString getPswd, int drfId){
    id = getId; pswd = getPswd; draftId = drfId;
    ui->id2PlayLabel->setText(QStringLiteral("发件人：")+"<"+id+">:");
    ui->id2PlayLabel->setStyleSheet("font-size : 12px");

    ui->RecieverEdit->clear();
    ui->emailEdit->clear();
    ui->titleEdit->clear();
    //若草稿id有效，则取出草稿，并显示
    if(draftId != -1){
        QSqlQuery qry;
        qry.prepare("select * from draft where id = ?");
        qry.addBindValue(draftId);
        qry.exec();
        qry.first();

        qDebug() << "draft ID: " << draftId << endl;

        ui->RecieverEdit->setText(qry.value(2).toString());
        ui->titleEdit->setText(qry.value(3).toString());
        ui->emailEdit->setPlainText(qry.value(4).toString());
    }
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(100);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
}

void WrtDlg::on_sendBtn_clicked()
{
    if(!ui->RecieverEdit->text().isEmpty() && !ui->titleEdit->text().isEmpty()
            && !ui->emailEdit->toPlainText().isEmpty()){

       sMailInfo sm;

       sm.m_pcUserName = strdup(id.toLatin1().data());
       sm.m_pcUserPassWord = strdup(pswd.toLatin1().data());//"435390541yanhao";// "发件人密码";
       sm.m_pcSenderName = strdup(id.toLatin1().data());//"13551365646@163.com";//"发件人（姓名）"
       sm.m_pcSender = strdup(id.toLatin1().data());//"13551365646@163.com";//发送者的邮箱地址
       sm.m_pcReceiver = strdup(ui->RecieverEdit->text().toLatin1().data());//"435390541@qq.com";//接收者的邮箱地址

       sm.m_pcTitle = strdup(ui->titleEdit->text().toStdString().c_str());
       QTextCodec *code = QTextCodec::codecForName("gb2312");
       sm.m_pcBody = strdup(code->fromUnicode(ui->emailEdit->toPlainText()).data());

       QString IPAddr = "";
       QString IPName = "smtp.163.com";
       sm.m_pcIPAddr = strdup(IPAddr.toLatin1().data());//服务器的IP可以留空
       sm.m_pcIPName = strdup(IPName.toLatin1().data());//服务器的名称（IP与名称二选一，优先取名称）

       cout <<sm.m_pcUserName << endl;
       cout <<sm.m_pcReceiver << endl;
       cout <<sm.m_pcSenderName << endl;
       cout <<sm.m_pcUserPassWord << endl;
       cout <<sm.m_pcTitle << endl;
       cout <<sm.m_pcBody << endl;
       qDebug() << ui->emailEdit->toPlainText();
       qDebug() << ui->emailEdit->toPlainText().toStdString().c_str();


       CSendMail SM;
       if(SM.SendMail(sm)){
           //获取当前系统时间
           QDateTime time = QDateTime::currentDateTime();
           QString date = time.toString("dd MM");
           QSqlQuery qry;
           qry.prepare("insert into email "
                       "values (NULL,?,?,?,?,?,0,1)");
           qry.addBindValue(id);
           qry.addBindValue(ui->RecieverEdit->text());
           qry.addBindValue(date);
           qry.addBindValue(ui->titleEdit->text());
           qry.addBindValue(ui->emailEdit->toPlainText());
           if(qry.exec())
               qDebug() << "保存发件箱成功";
           else
               qDebug() << "保存发件箱失败";
           QMessageBox::information(this,QStringLiteral("提示"),
                                QStringLiteral("发送成功！"),QMessageBox::Yes);
       }
       else{
           QMessageBox::warning(this,QStringLiteral("提示"),
                                QStringLiteral("发送失败，请检查网络"),QMessageBox::Yes);
       }
    }else{
        QMessageBox::warning(this,QStringLiteral("提示"),
                             QStringLiteral("请输入完整信息"),QMessageBox::Yes);
    }
}

void WrtDlg::on_SaveBtn_clicked()
{
    if(!ui->RecieverEdit->text().isEmpty() && !ui->titleEdit->text().isEmpty()
            && !ui->emailEdit->toPlainText().isEmpty()){
        QSqlQuery qry;
        if(draftId == -1){
            qry.prepare("insert into draft "
                        "values (NULL,?,?,?,?)");
            qry.addBindValue(id);
            qry.addBindValue(ui->RecieverEdit->text());
            qry.addBindValue(ui->titleEdit->text());
            qry.addBindValue(ui->emailEdit->toPlainText());

            if(qry.exec())
                QMessageBox::information(this,QStringLiteral("提示"),
                                         QStringLiteral("保存成功！"),QMessageBox::Yes);
            else
                QMessageBox::warning(this,QStringLiteral("提示"),
                                         QStringLiteral("保存失败！"),QMessageBox::Yes);
            /*qry.prepare("select * from draft");
            qry.exec();
            while(qry.next()){
                qDebug() << qry.value(0).toString() << qry.value(1).toString() << qry.value(2).toString();
            }*/
        }
        else{
            qry.prepare("update draft set sender=?,receiver=?,title=?,body=? "
                        "where id = ?");
            qry.addBindValue(id);
            qry.addBindValue(ui->RecieverEdit->text());
            qry.addBindValue(ui->titleEdit->text());
            qry.addBindValue(ui->emailEdit->toPlainText());
            qry.addBindValue(draftId);
            qDebug() << "draft Id:" << draftId;
            if(qry.exec())
                QMessageBox::information(this,QStringLiteral("提示"),
                                         QStringLiteral("保存成功！"),QMessageBox::Yes);
            else
                QMessageBox::warning(this,QStringLiteral("提示"),
                                         QStringLiteral("保存失败！"),QMessageBox::Yes);
        }
    }else{
        QMessageBox::warning(this,QStringLiteral("保存失败！"),
                             QStringLiteral("请输入完整信息"),QMessageBox::Yes);
    }
}

void WrtDlg::on_exitBtn_clicked()
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(100);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
    //Sleep(100);
    this->close();
}
