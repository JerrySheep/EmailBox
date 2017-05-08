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
    query.exec("create table user (id varchar(40) primary key, "
               "name varchar(20), pswd varcahr(40), last char(1))");
    query.exec("insert into user values('1846182474@qq.com', '张三', '0','0')");
    return true;
}

#endif // DATABASE_H
