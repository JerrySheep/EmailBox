#ifndef DATABASE_H
#define DATABASE_H

#include<QMessageBox>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QString>

#define NORMALEMAIL 0
#define SPAMEMAIL   1
#define WORKEMAIL   2
#define FRIENDEMAIL 3

#define VALID   1
#define INVALID 0

/* 建立数据库连接，并创建基本表录入数据
*  by: 张森
*  date: 2017.5.01
*/
static bool createConnection(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");
    if (!db.open()){
        QMessageBox::critical(0,QString::fromStdString("Cannot open database"),
                              QString::fromStdString("Unable to establish a database connection."),
                              QMessageBox::Cancel);
        return false;
    }
    QSqlQuery query;
    query.exec("create table user ("
               "id      varchar(40) not null, "
               "name    varchar(20), "
               "pswd    varchar(40) not null, "
               "last    int         not null,"
               "primary key(id))");
    query.exec("create table email ("
               "id          integer         primary key,"
               "sender      varchar(40)     not null, "
               "receiver    varchar(40)     not null, "
               "time        varchar(40)     not null, "
               "title       varchar(200)    not null, "
               "body        varchar(1000)   not null, "
               "variety     int             not null, "//0-norm;1-spam;2-work;3-friend;4-un
               "valid       int             not null)");
    query.exec("create table draft ("
               "id          integer         primary key,"
               "sender      varchar(40)     not null, "
               "receiver    varchar(40)     not null, "
               "title       varchar(200)    not null, "
               "body        varchar(1000)   not null)");
    //query.exec("insert into user values('13890458375@163.com', '张三', 'zs970612','1')");
    return true;
}

#endif // DATABASE_H
