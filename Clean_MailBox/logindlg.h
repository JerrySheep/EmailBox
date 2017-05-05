#ifndef LOGINDLG_H
#define LOGINDLG_H

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
    void sendUsrInfo(int,QString);

private:
    Ui::loginDlg *ui;
    Widget *w;
};

#endif // LOGINDLG_H
