#include "wrtdlg.h"
#include "ui_wrtdlg.h"

WrtDlg::WrtDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WrtDlg)
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("邮件编辑"));
}

WrtDlg::~WrtDlg()
{
    delete ui;
}

void WrtDlg::getUsrInfo(QString getId, QString getName){
    id = getId; name = getName;
    ui->id2PlayLabel->setText(QStringLiteral("发件人：")+"<"+id+">:");
    ui->id2PlayLabel->setStyleSheet("font-size : 12px");
}
