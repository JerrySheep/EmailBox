#ifndef WIDGET_H
#define WIDGET_H

#include "logindlg.h"
#include <QWidget>
#include "wrtdlg.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void timerUpDate();
    void getUsrInfo(QString id,QString name);

    void on_sendBtn_clicked();

    void on_pushButton_clicked();

signals:
    void sendUsrInfo(QString,QString);

private:
    Ui::Widget *ui;
    WrtDlg *edit;
    QString id;
    QString name;
};

#endif // WIDGET_H
