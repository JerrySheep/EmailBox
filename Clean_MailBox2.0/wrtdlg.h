#ifndef WRTDLG_H
#define WRTDLG_H

#include <QDialog>

namespace Ui {
class WrtDlg;
}

class WrtDlg : public QDialog
{
    Q_OBJECT

public:
    explicit WrtDlg(QWidget *parent = 0);
    ~WrtDlg();

private slots:
    void getUsrInfo(QString id,QString name, int draftId);


    void on_sendBtn_clicked();

    void on_SaveBtn_clicked();

private:
    Ui::WrtDlg *ui;
    QString id;
    QString pswd;
    int draftId;
};

#endif // WRTDLG_H
