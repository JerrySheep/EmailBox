#ifndef DATABASE_H
#define DATABASE_H

#include<QMessageBox>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QString>

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
    query.exec("create table user (id int primary key, "
               "name varchar(20), pswd int)");
    query.exec("insert into user values(0, '张三', 0)");
    query.exec("insert into user values(1, '李四', 1)");
    query.exec("insert into user values(2, '王五', 2)");
    query.exec("insert into user values(3, '徐六', 3)");
    query.exec("insert into user values(4, '陈七', 4)");
    return true;
}

#endif // DATABASE_H
