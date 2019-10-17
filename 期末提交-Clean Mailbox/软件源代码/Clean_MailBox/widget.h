#ifndef WIDGET_H
#define WIDGET_H

#include "logindlg.h"
#include <QWidget>
#include "wrtdlg.h"
#include <string>
#include <QLabel>
#include <QHBoxLayout>
#include <QProcess>
#include <QHostInfo>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void clearList();

private slots:
    void timerUpDate();
    void getUsrInfo(QString id,QString name);
    void getResults(int, QProcess::ExitStatus);
    void networkCheck(QHostInfo host);
    //void updateEmail();
    void on_sendBtn_clicked();
    void on_pushButton_clicked();

    void on_contactBtn_clicked();

    void on_list2Play_doubleClicked(const QModelIndex &index);

    void on_inboxBtn_clicked();

    void on_delBtn_clicked();

    void on_pushButton_9_clicked();

    void on_workBtn_clicked();

    void on_emailBtn_clicked();

    void on_friendBtn_clicked();

    void on_outboxBtn_clicked();

    void on_list2Play_clicked(const QModelIndex &index);

signals:
    void sendUsrInfo(QString,QString,int);

private:
    Ui::Widget *ui;
    QProcess process;
    WrtDlg *edit;
    QString id;
    QString name;
};


//用于列表显示
class ListItem : public QWidget
{
    Q_OBJECT
public:
    explicit ListItem(QWidget *parent = 0);
    ~ListItem();
    void init(QString, QString, QString);
public:
    QLabel *l1;  //
    QLabel *l2;  //
    QLabel *l3;  //
    QHBoxLayout *layout;

signals:

public slots:

};



#endif // WIDGET_H
