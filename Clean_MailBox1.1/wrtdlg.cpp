#include "wrtdlg.h"
#include "ui_wrtdlg.h"

WrtDlg::WrtDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WrtDlg)
{
    ui->setupUi(this);
    this->setWindowTitle("邮件编辑");
}

WrtDlg::~WrtDlg()
{
    delete ui;
}

void WrtDlg::getUsrInfo(int getId, QString getName){
    id = getId; name = getName;
    ui->id2PlayLabel->setText("发件人："+name+"<"+id+">:");
    ui->id2PlayLabel->setStyleSheet("font-size : 12px");
}
