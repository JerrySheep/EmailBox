#include "widget.h"
#include "ui_widget.h"
#include "logindlg.h"
#include <QtCore>
#include <QTextCodec>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QDir>
#include <QMessageBox>

#define NORMAL  0
#define SPAM    1
#define WORK    2
#define FRIEND  3

loginDlg *logindg;
QString btnName;

ListItem::ListItem(QWidget *parent):
    QWidget(parent){

}

ListItem::~ListItem(){
    delete l1;
    delete l2;
    delete l3;
    delete layout;
}

void ListItem::init(QString lb1, QString lb2, QString lb3){
    l1 = new QLabel(lb1);
    l2 = new QLabel(lb2);
    l3 = new QLabel(lb3);
    layout = new QHBoxLayout(this);
    layout->addWidget(l1); layout->addStretch(1);
    layout->addWidget(l2); layout->addStretch(1);
    layout->addWidget(l3);
    layout->setContentsMargins(5,0,0,5);
    this->resize(350,50);
}

/*
 * 初始化主界面： 设置信号与槽的连接，添加几种分类，初始化列表
 */
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    //创建子界面——编辑界面
    edit = new WrtDlg;
    //连接与子界面的信号和槽
    connect(this, SIGNAL(sendUsrInfo(QString,QString,int)),edit, SLOT(getUsrInfo(QString,QString,int)));
    connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)),this, SLOT(getResults(int,QProcess::ExitStatus)));
    ui->setupUi(this);
    //新建定时器
    QTimer* timer = new QTimer(this);
    //关联计时器记满信号和相应的槽函数
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
    //定时器开始计时
    timer->start(1000);
    ui->timeLabel->setStyleSheet("font-size : 14px");

    //初始化按钮种类为sendBtn
    btnName = "inboxBtn";

    //初始化邮件种类选项
    ui->classifyComboBox->addItem(QStringLiteral("普通"));
    ui->classifyComboBox->addItem(QStringLiteral("垃圾"));
    ui->classifyComboBox->addItem(QStringLiteral("工作"));
    ui->classifyComboBox->addItem(QStringLiteral("好友"));

    //设置列表为不带control的多选（ExtendedSelection）
    ui->list2Play->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

Widget::~Widget()
{
    delete ui;
    delete logindg;
    delete edit;
}

/*更新当前时间*/
void Widget::timerUpDate(){
    //获取系统当前时间
    QDateTime time = QDateTime::currentDateTime();
    //设置系统时间显示格式
    QString str = time.toString("hh:mm");
    //在标签上显示时间
    ui->timeLabel->setText(str);
}

/*从登录界面获取信息*/
void Widget::getUsrInfo(QString getId, QString getName){
    id = getId; name = getName;

    ui->helloLabel->setText("<"+id+">");
    ui->helloLabel->setStyleSheet("font-size : 12px");

    //创建缓冲文件夹temp
    QDir *dir = new QDir;
    if(!dir->exists("temp"))
        dir->mkdir("temp");
    else{
        dir->setCurrent("temp");
        dir->removeRecursively();
        dir->setCurrent("..");
    }

    //初始化列表
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where receiver=? and valid=1");
    qry.addBindValue(id);
    qry.exec();
    int count = 0;
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(1).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//将邮件的id存为透明文本
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
        if(qry.value(6).toInt() == 4){
            QFile data(QString("temp/")+QString::number(count)+QString(".txt"));
            if (data.open(QFile::WriteOnly | QIODevice::WriteOnly)) {
                QTextStream out(&data);
                out << qry.value(5).toString();
                count++;
            }
        }
    }
    btnName = "inboxBtn";

    //开起分类器进程
    process.start("python.exe",QStringList() << "data/classify.py");
}

void Widget::getResults(int exitCode, QProcess::ExitStatus exitStatus){
    QFile data(QString("temp/results.txt"));
    data.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&data);

    QString str = in.readAll();
    qDebug() << str;
    int len = str.length();
    QSqlQuery qry;
    for(int i=0; i<len; i+=2){
        qry.prepare("select id from email where receiver=? and valid=1 and variety=4");
        qry.addBindValue(id);
        qry.exec();
        qry.next();
        int emailId = qry.value(0).toInt();
        qDebug() << str[i] << emailId;
        if(str[i]=='1'){
            qry.prepare("update email set variety=1 where id=? and valid=1");
            qry.addBindValue(emailId);
            if(qry.exec())
                qDebug() << "success1";
        }
        if(str[i]=='0'){
            qry.prepare("update email set variety=0 where id=? and valid=1");
            qry.addBindValue(emailId);
            if(qry.exec())
                qDebug() << "sucess2";
        }
    }

}

/*写信按钮*/
void Widget::on_sendBtn_clicked()
{
    emit sendUsrInfo(id, name, -1);//发送用户账户、密码和草稿ID（此处没有草稿）
    this->hide();
    edit->show();
    edit->exec();
    this->show();
}

/*退出按钮*/
void Widget::on_pushButton_clicked()
{
    this->close();
    logindg = new loginDlg;
    logindg->show();
}

/*草稿箱*/
void Widget::on_contactBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select id,receiver,title from draft where sender=?");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(0).toString(),qry.value(1).toString(),qry.value(2).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存草稿的id
        listItem1->setTextColor("transparent");

        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }

    btnName = "draftBtn";
}

/*清除列表*/
void Widget::clearList(){
    QListWidgetItem *item;
    int num = ui->list2Play->count();
    for(int i=0; i<num; i++){
        item = ui->list2Play->takeItem(0);
        ui->list2Play->removeItemWidget(item);
        delete item;
    }
}

/*列表项双击事件*/
void Widget::on_list2Play_doubleClicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->list2Play->currentItem();
    if(btnName == "draftBtn"){
        ui->textEdit->clear();
        emit sendUsrInfo(id, name, item->text().toInt());//发送用户账户、密码和草稿ID（此处没有草稿）
        this->hide();
        edit->show();
        edit->exec();
        this->show();
    }
    else{
        int emailId = item->text().toInt();
        QSqlQuery qry;
        qry.prepare("select body from email where id=? and valid=1");
        qry.addBindValue(emailId);
        qry.exec();
        qry.next();
        ui->textEdit->setPlainText(qry.value(0).toString());
    }
}

/*收件箱*/
void Widget::on_inboxBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where receiver=? and valid=1");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(1).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存邮件的内容
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }
    btnName = "inboxBtn";
}

/*删除功能*/
void Widget::on_delBtn_clicked()
{
    QSqlQuery qry;
    QList<QListWidgetItem*> list = ui->list2Play->selectedItems();
    int count = list.count();
    if(btnName == "draftBtn"){
        int id;
        qry.prepare("delete from draft where id=?");
        for(int i=0; i<count; i++){
            ui->list2Play->takeItem(ui->list2Play->row(list.at(i)));
            id = list.at(i)->text().toInt();
            qry.addBindValue(id);
            qry.exec();
        }
    }
    if(btnName == "inboxBtn"){
        int emailId;
        qry.prepare("update email set valid=0 where id=? and valid=1");
        for(int i=0; i<count; i++){
            ui->list2Play->takeItem(ui->list2Play->row(list.at(i)));
            emailId = list.at(i)->text().toInt();
            qry.addBindValue(emailId);
            qry.exec();
        }
    }
}

/*标记功能*/
void Widget::on_pushButton_9_clicked()
{
    QList<QListWidgetItem*> list = ui->list2Play->selectedItems();
    int count = list.count();
    int type = ui->classifyComboBox->currentIndex();

    QSqlQuery qry;
    qry.prepare("update email set variety=? where id=? and valid=1");
    for(int i=0; i<count; i++){
        int emailId = list.at(i)->text().toInt();
        qry.addBindValue(type);
        qry.addBindValue(emailId);
        qry.exec();
    }
}

/*工作邮件*/
void Widget::on_workBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where variety=2 and valid=1 and receiver=?");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(1).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存邮件的内容
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }
    btnName = "workBtn";
}

/*垃圾邮件*/
void Widget::on_emailBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where variety=1 and valid=1 and receiver=?");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(1).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存邮件的内容
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }
    btnName = "spamBtn";
}

/*好友邮件*/
void Widget::on_friendBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where variety=3 and valid=1 and receiver=?");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(1).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存邮件的内容
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }
    btnName = "friendBtn";
}
