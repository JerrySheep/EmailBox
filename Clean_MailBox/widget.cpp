#include "widget.h"
#include "ui_widget.h"
#include <QtCore>
#include <QTextCodec>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    //创建子界面——编辑界面
    edit = new WrtDlg;
    //连接与子界面的信号和槽
    connect(this, SIGNAL(sendUsrInfo(int,QString)),edit, SLOT(getUsrInfo(int,QString)));

    ui->setupUi(this);
    //新建定时器
    QTimer* timer = new QTimer(this);
    //关联计时器记满信号和相应的槽函数
    connect(timer,SIGNAL(timeout()),this,SLOT(timerUpDate()));
    //定时器开始计时
    timer->start(1000);
    ui->timeLabel->setStyleSheet("font-size : 14px");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::timerUpDate(){
    //获取系统当前时间
    QDateTime time = QDateTime::currentDateTime();
    //设置系统时间显示格式
    QString str = time.toString("hh:mm");
    //在标签上显示时间
    ui->timeLabel->setText(str);
}

void Widget::getUsrInfo(int getId, QString getName){
    id = getId; name = getName;

    ui->helloLabel->setText("你好, "+name);
    ui->helloLabel->setStyleSheet("font-size : 16px");
}


void Widget::on_sendBtn_clicked()
{
    emit sendUsrInfo(id, name);
    this->hide();
    edit->show();
    edit->exec();
    this->show();
}
