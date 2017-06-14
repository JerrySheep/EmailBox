#ifndef LOGINDLG_H
#define LOGINDLG_H

#include "logindlg.h"
#include <QDialog>
#include <widget.h>

namespace Ui {
class loginDlg;
}

class loginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit loginDlg(QWidget *parent = 0);
    ~loginDlg();

private slots:
    void on_LoginBtn_clicked();

signals:
    void sendUsrInfo(QString,QString);

private:
    Ui::loginDlg *ui;
};

#endif // LOGINDLG_H
