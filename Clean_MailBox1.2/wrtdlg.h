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
    void getUsrInfo(QString id,QString name);


private:
    Ui::WrtDlg *ui;
    QString id;
    QString name;
};

#endif // WRTDLG_H
