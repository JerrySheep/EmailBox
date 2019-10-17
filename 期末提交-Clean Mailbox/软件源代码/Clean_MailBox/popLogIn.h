#ifndef POPLOGIN_H
#define POPLOGIN_H

#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include "base64.h"
#include <iostream>   
#include <cstdlib>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <base64.h>
#include <regex>
#include <QDebug>
#include <QSqlQuery>
#include <QProcess>
#include <QDir>
#include <QNetworkConfigurationManager>

using namespace std;
#define MAXBUF 200000
#define NORMAL      0
#define SPAM        1
#define WORK        2
#define FRIEND      3
#define UNCLASSIFY  4

#pragma comment(lib,"ws2_32.lib")  
//================================================  
bool InitSock();
void  pickUpInfo(char *s, string *from, string *subject, string *text, string *date);
bool logIn(string name, string pass);
int calEmailCount(char *s);


/*
 * 完成登录操作并将邮件存入数据库
 * name: 邮箱账户
 * pass: 邮箱密码
 *
*/
bool logIn(string name, string pass) {
    QHostInfo info = QHostInfo::fromName(QString("www.baidu.com"));
    qDebug() << "network state" << info.addresses().isEmpty();
    if(info.addresses().isEmpty())
        return false;

    char sendbuf[MAXBUF] = { 0 };
    char quit[] = "quit\t\n";  //
    char recvbuf[MAXBUF] = { 0 };
    int mailNumber;//邮件总数量
	InitSock();

	//163邮箱pop协议
	struct hostent *phost = gethostbyname("pop.163.com");
	SOCKET httpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == httpsock)
	{
        qDebug() << "socket error !" << endl;
        closesocket(httpsock);
        WSACleanup();
        return false;
	}
	SOCKADDR_IN sock_addr = { 0 };
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(110);
    qDebug() << endl <<1 ;
	memcpy(&sock_addr.sin_addr.S_un.S_addr, phost->h_addr_list[0], phost->h_length);
    qDebug() << 2;
    int res = connect(httpsock, (sockaddr *)&sock_addr, sizeof(sockaddr));
    qDebug() << 3;
	if (res == SOCKET_ERROR)
	{
        qDebug() << "connect error !";
		closesocket(httpsock);
		WSACleanup();
        return false;
	}
	recv(httpsock, recvbuf, MAXBUF, 0);
    qDebug() << "Client : connect successfully!";
	memset(recvbuf, 0, MAXBUF);

	//发送账号
    send(httpsock, name.c_str(), strlen(name.c_str()), 0);
	recv(httpsock, recvbuf, MAXBUF, 0);
    qDebug() << 3;
    qDebug() << "Client : send account";
	memset(recvbuf, 0, MAXBUF);

	//发送密码
	int numPass = send(httpsock, pass.c_str(), strlen(pass.c_str()), 0);
	recv(httpsock, recvbuf, MAXBUF, 0);
    qDebug() << "Client : send password" << QString::fromStdString(pass) <<numPass;
    if (recvbuf[0] == '+')
        qDebug() << "Password Success";
	else {
        qDebug() << "Password Failed";
        closesocket(httpsock);
        WSACleanup();
        return false;
	}
	memset(recvbuf, 0, MAXBUF);

    //返回总邮件大小
    send(httpsock, "stat\r\n", strlen("stat\r\n"), 0);
    recv(httpsock, recvbuf, MAXBUF, 0);
    mailNumber = calEmailCount(recvbuf);
    cout << recvbuf << endl;
    memset(recvbuf, 0, MAXBUF);

    string ret;
    ret = "retr 1\t\n";
    send(httpsock, ret.c_str(), strlen(ret.c_str()), 0);
    recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);
    memset(recvbuf, 0, MAXBUF);

    int len = name.length()-7;
    name = name.substr(5,len);

    //循环操作获取并存放邮件信息
    int count = 1;
    QSqlQuery query;
    while (count <= mailNumber) {        
        string from, subject, text, date;

        stringstream temp;
        temp << count;
        string Choice;
        Choice = temp.str();

        ret = "retr " + Choice + "\t\n";
        if (Choice != "1")
            send(httpsock, ret.c_str(), strlen(ret.c_str()), 0);
        Sleep(150);

        recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);
        pickUpInfo(recvbuf, &from, &subject, &text, &date);//获取邮件信息
        if(!from.empty() || !subject.empty() || !text.empty() || !date.empty()){
            query.prepare("select id from email where body=? and title=? and time=?");
            query.addBindValue(QString::fromLocal8Bit(text.c_str()));
            query.addBindValue(QString::fromLocal8Bit(subject.c_str()));
            query.addBindValue(QString::fromStdString(date));
            query.exec();
            if(query.next())
                qDebug() << "email has exist";
            else{
                //数据库操作
                query.prepare("insert into email values"
                           "(NULL,?, ?, ?, ?, ?, 4, 1)");
                query.addBindValue(QString::fromStdString(from));
                query.addBindValue(QString::fromStdString(name));
                query.addBindValue(QString::fromStdString(date));
                query.addBindValue(QString::fromLocal8Bit(subject.c_str()));
                query.addBindValue(QString::fromLocal8Bit(text.c_str()));
                query.exec();
                qDebug() << 4;
            }

            memset(recvbuf, 0, MAXBUF);
            count++;
        }
    }

    /*QSqlQuery q;
    q.exec("select * from email");
    while(q.next()){
        qDebug() << q.value(0).toString() << q.value(3).toString() << q.value(4).toString();
    }*/

	send(httpsock, quit, strlen(quit), 0);
	recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);
    memset(recvbuf, 0, MAXBUF);

	closesocket(httpsock);
	WSACleanup();
    return true;
}

//创建socket连接
bool InitSock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2) {
		WSACleanup();
		return false;
	}
	return true;
}

//计算邮件总数
int calEmailCount(char *s) {
	string temp;
	for (int i = 4;; i++) {
		if (s[i] == ' ')
			break;
		else
			temp += s[i];
	}

    return atoi(temp.c_str());
}

//用正则表达式提取邮件信息
void pickUpInfo(char *s, string *from, string *subject, string *text, string *date) {
    string searchTemp;
    searchTemp = s;

    regex fromPattern("([0-9A-Za-z\\-_.]+)@([0-9a-z]+\\.[a-z]{2,3}(\\.[a-z]{2})?)");
    regex datePattern("([A-Za-z]+\\,\\s)([0-9]{1,2}\\s)([A-Za-z]+\\s)([0-9]{4}\\s)([0-9]{2}\\:[0-9]{2}\\:[0-9]{2})");
    regex subjectPattern("Subject:(\\s)(\\w|\\=?.)+");
    regex textPattern("base64(\\s)+([0-9A-Za-z\\=/+\\s]+)");
    regex chinesePattern("=?gb18030?B?");

    smatch match;
    //邮件来源提取
    regex_search(searchTemp, match, fromPattern);
    *from = match[0].str();

    //邮件主题提取
    regex_search(searchTemp, match, subjectPattern);
    *subject = match[0].str();
    string subjectTemp = match[0].str();

    if (regex_search(subjectTemp, match, chinesePattern)) {
        *subject = subjectTemp.substr(21, subjectTemp.length() - 23);
        char pickSubject[1000];
        base64_decode(*subject, pickSubject);
        *subject = pickSubject;
        *subject = "Subject: " + *subject;
    }

    int len;
    if((*subject).length()>9){
        len = (*subject).length() - 9;
        *subject = (*subject).substr(9,len);
    }
    //邮件内容提取
    regex_search(searchTemp, match, textPattern);
    string textTemp = match[0].str();
    for (int i = 10; i < textTemp.length(); i++) {
        if(textTemp[i] != '\r' && textTemp[i] != '\n')
             *text += textTemp[i];
    }
    char pickText[1000];
    base64_decode(*text, pickText);
    *text = pickText;

    //邮件时间提取
    regex_search(searchTemp, match, datePattern);
    *date = match[0].str();
    if((*date).length() > 19){
        len = (*date).length()-19;
        *date = (*date).substr(5,len);
    }
}

#endif
