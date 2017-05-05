#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <wrtdlg.h>

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
    void getUsrInfo(int id,QString name);

    void on_sendBtn_clicked();

signals:
    void sendUsrInfo(int,QString);

private:
    Ui::Widget *ui;
    WrtDlg *edit;
    int id;
    QString name;
};

#endif // WIDGET_H
