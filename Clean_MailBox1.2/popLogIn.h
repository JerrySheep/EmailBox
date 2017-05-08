#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include "base64.h"
#include <iostream>   
#include <cstdlib>
#include <string>
#include <winsock2.h>
using namespace std;
#define MAXBUF 200000

#pragma comment(lib,"ws2_32.lib")  
//================================================  
bool InitSock();
void pickUpInfo(char *s);
bool logIn(string name, string pass);
void calEmailCount(char *s);

char sendbuf[MAXBUF] = { 0 };
char quit[] = "quit\t\n";  //    
char recvbuf[MAXBUF] = { 0 };
int mailNumber;//邮件总数量

bool logIn(string name, string pass) {
	InitSock();
	
	//163邮箱pop协议
	struct hostent *phost = gethostbyname("pop.163.com");

	SOCKET httpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == httpsock)
	{
        //cout << "socket error !" << endl;
        closesocket(httpsock);
        WSACleanup();
        return false;
	}

	SOCKADDR_IN sock_addr = { 0 };
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(110);

	memcpy(&sock_addr.sin_addr.S_un.S_addr, phost->h_addr_list[0], phost->h_length);

	int res = connect(httpsock, (sockaddr *)&sock_addr, sizeof(sockaddr));
	if (res == SOCKET_ERROR)
	{
        //cout << "connect error !" << endl;
		closesocket(httpsock);
		WSACleanup();
        return false;
	}
	recv(httpsock, recvbuf, MAXBUF, 0);
    //cout << "Client : connect successfully!"<< endl;
	memset(recvbuf, 0, MAXBUF);

	//发送账号
	int numUser = send(httpsock, name.c_str(), strlen(name.c_str()), 0);
	recv(httpsock, recvbuf, MAXBUF, 0);
    //cout << "Client : send account"<< endl;
	if (numUser == 26)
        ;//cout << "Account Success" << endl;
	else {
        //cout << "Account Failed" << endl;
        closesocket(httpsock);
        WSACleanup();
        return false;
	}
	memset(recvbuf, 0, MAXBUF);

	//发送密码
	int numPass = send(httpsock, pass.c_str(), strlen(pass.c_str()), 0);
	recv(httpsock, recvbuf, MAXBUF, 0);
    //cout << "Client : send password" << endl;
	if (numPass == 22)
        ;//cout << "Password Success" << endl;
	else {
        //cout << "Password Failed" << endl;
        closesocket(httpsock);
        WSACleanup();
        return false;
	}
	memset(recvbuf, 0, MAXBUF);

    /*//返回总邮件大小
	send(httpsock, "stat\r\n", strlen("stat\r\n"), 0);
	recv(httpsock, recvbuf, MAXBUF, 0);
	calEmailCount(recvbuf);
	memset(recvbuf, 0, MAXBUF);

	string Choice;
    //cout << "Which Email do you want： ";
	cin >> Choice;		

	string ret;
	ret = "retr " + Choice + "\t\n";
	send(httpsock, ret.c_str(), strlen(ret.c_str()), 0);
	recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);
	memset(recvbuf, 0, MAXBUF);
	recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);//recvbuf为信头
	pickUpInfo(recvbuf);

	send(httpsock, quit, strlen(quit), 0);
	recv(httpsock, (LPSTR)recvbuf, MAXBUF, 0);
    //cout << "Client : quit" << endl;
    memset(recvbuf, 0, MAXBUF);*/

	closesocket(httpsock);
    //system("pause");
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

/*//提取邮件发送者，主题，内容
void pickUpInfo(char *s) {
	bool judge = true;
	string from;
	string subject;
	string text;
	for (int i = 0; i < MAXBUF; i++) {
		if (s[i] == '<' && judge == true) {
			int j, k;
			for (j = i + 1;; j++) {
				if (s[j] == '>')
					break;
				else
					from += s[j];
			}
			i = j + 2;
			judge = false;
		}
		else if (s[i] == '>') {
			int j;
			for (j = i + 2;; j++) {
				if (s[j] == '\r')
					break;
				else
					subject += s[j];
			}
			break;
		}
	}

	string textTemp;
	for (int i = MAXBUF - 1;; i--) {
		if (s[i] == ':') {
			int j;
			for (j = i + 1;; j++) {
				if (s[j] == '4') {
					for (int k = j + 4;; k++) {
						if (s[k] == '-')
							break;
						else
							textTemp += s[k];
					}
					break;
				}
			}
			break;
		}
	}

	for (int i = 0; i < textTemp.length(); i++) {
		if (textTemp[i] != '\r' && textTemp[i] != '\n') {
			text += textTemp[i];
		}
	}

	text = base64_decode(text);
	cout << endl << from << endl;
	cout << subject << endl;
	cout << endl << text << endl;
}*/

//计算邮件总数
void calEmailCount(char *s) {
	string temp;
	for (int i = 4;; i++) {
		if (s[i] == ' ')
			break;
		else
			temp += s[i];
	}

	mailNumber = atoi(temp.c_str());
}
