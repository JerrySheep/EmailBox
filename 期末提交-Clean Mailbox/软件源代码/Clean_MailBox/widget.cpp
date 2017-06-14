#include "widget.h"
#include "ui_widget.h"
#include "logindlg.h"
//#include "popLogIn.h"
#include <QtCore>
#include <QTextCodec>
#include <QHBoxLayout>
#include <QSqlQuery>
#include <QDir>
#include <QMessageBox>
#include <QStyleFactory>
#include <QPropertyAnimation>
#include <string>

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
    //l1->resize(80,20);l1->resize(80,20);
    //l1->resize(60,20);
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
    //QTimer* timer2 = new QTimer(this);
    //关联计时器记满信号和相应的槽函数
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
    //connect(timer2,SIGNAL(timeout()),this,SLOT(updateEmail()));
    //定时器开始计时
    timer->start(1000);
    //timer2->start(3000);
    ui->timeLabel->setStyleSheet("font-size : 14px");

    //初始化按钮种类为sendBtn
    btnName = "inboxBtn";

    //初始化邮件种类选项
    ui->classifyComboBox->addItem(QStringLiteral("普通"));
    ui->classifyComboBox->addItem(QStringLiteral("垃圾"));
    ui->classifyComboBox->addItem(QStringLiteral("办公"));
    ui->classifyComboBox->addItem(QStringLiteral("好友"));

    //设置列表为不带control的多选（ExtendedSelection）
    ui->list2Play->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //设置软件图标
    this->setWindowIcon(QIcon(":images/email.png"));

    this->setAutoFillBackground(true);   // 这个属性一定要设置
    QPalette pal;
    pal.setBrush(QPalette::Window,QBrush(QPixmap(":/images/timg2.jpg").scaled( // 缩放背景图.
                             this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    //pal.setBrush(this->backgroundRole(), QBrush(QColor(0,0,0)));
    this->setPalette(pal);
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
    QHostInfo info = QHostInfo::fromName(QString("www.baidu.com"));
    if(info.addresses().isEmpty())
        ui->Networklabel->setText(QStringLiteral("请检查网络！"));
    else
        ui->Networklabel->setText("");
}

/*网络断开时显示信息*/
void Widget::networkCheck(QHostInfo host){
    if (host.error() != QHostInfo::NoError){
        ui->Networklabel->setText(QStringLiteral("请检查网络！"));
    }
    else{
        ui->Networklabel->setText("");
    }
}

/*void Widget::updateEmail(){
    string userId=id.toStdString();
    string userPswd = name.toStdString();

    if(!logIn(string("user ")+userId+string("\t\n"),
             string("pass ")+userPswd+string("\t\n"))){
        QMessageBox::warning(this,QStringLiteral("失败！"),
                             QStringLiteral("请检查网络连接"),QMessageBox::Yes);
    }

}*/

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

    //初始化所有邮件列表
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

    //创建垃圾文件夹spam
    if(!dir->exists("data/spam"))
        dir->mkdir("data/spam");
    else{
        dir->setCurrent("data/spam");
        dir->removeRecursively();
        dir->setCurrent("../..");
    }
    //创建垃圾邮件文件
    qry.prepare("select * from email where valid=0 and variety=1");
    qry.exec();
    count = 0;
    while(qry.next()){
        QFile data(QString("data/spam/")+QString::number(count)+QString(".txt"));
        if (data.open(QFile::WriteOnly | QIODevice::WriteOnly)) {
            QTextStream out(&data);
            out << qry.value(5).toString();
            count++;
        }
    }
    btnName = "inboxBtn";

    /*QStringList ls = QStyleFactory::keys();
    int num = ls.count();
    for(int i=0;i<num;i++){
        qDebug() << ls[i];
    }*/

    //开启分类器进程
    process.start("python.exe",QStringList() << "data/classify.py");
}

//分类进程结束后获取分类信息
void Widget::getResults(int exitCode, QProcess::ExitStatus exitStatus){
    QFile data(QString("temp/results.txt"));        //获取邮件分类结果
    data.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&data);

    QString str = in.readAll();
    qDebug() << "result.txt: " + str;
    qDebug() << process.readAllStandardOutput();
    int len = str.length();
    QSqlQuery qry;                                  //根据邮件分类结果更新邮件
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
    if(exitStatus == QProcess::NormalExit && len>2)
        QMessageBox::warning(this,QStringLiteral("提示"),
                         QStringLiteral("分类成功！"),QMessageBox::Yes);
    if(exitStatus == QProcess::CrashExit && len>2)
        QMessageBox::warning(this,QStringLiteral("提示"),
                         QStringLiteral("分类失败！"),QMessageBox::Yes);
    //将学习过的垃圾邮件标为已学习，避免重复学习
    qry.prepare("select * from email where valid=0 and variety=1");
    qry.exec();
    while(qry.next()){
        QSqlQuery query;
        query.prepare("update email set valid=-1 where id=?");
        query.addBindValue(qry.value(0).toInt());
    }
}

/*写信按钮*/
void Widget::on_sendBtn_clicked()
{
    emit sendUsrInfo(id, name, -1);//发送用户账户、密码和草稿ID（此处没有草稿）
    this->hide();
    edit->show();
    edit->exec();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(100);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
    this->show();
}

/*退出按钮*/
void Widget::on_pushButton_clicked()
{
    this->close();
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(100);
    animation->setStartValue(1);
    animation->setEndValue(0);
    animation->start();
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

void Widget::on_list2Play_clicked(const QModelIndex &index)
{

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
    else{
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
    QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("标记成功！"),QMessageBox::Yes);
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

void Widget::on_outboxBtn_clicked()
{
    clearList();
    QSqlQuery qry;
    qry.prepare("select * from email where sender=? and valid=1");
    qry.addBindValue(id);
    qry.exec();
    while(qry.next()){
        ListItem *item=new ListItem(ui->list2Play);
        item->init(qry.value(2).toString(),qry.value(4).toString(),qry.value(3).toString());

        QListWidgetItem *listItem1 = new QListWidgetItem(ui->list2Play);

        listItem1->setText(qry.value(0).toString());//隐蔽的保存邮件的内容
        listItem1->setTextColor("transparent");
        ui->list2Play->addItem(listItem1);
        ui->list2Play->setItemWidget(listItem1, item);
    }
    btnName = "ouboxBtn";
}
