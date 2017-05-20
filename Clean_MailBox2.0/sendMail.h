#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include "stdafx.h"  
#include <afx.h>
#include <afxtempl.h>
#pragma comment(lib,"WSOCK32")    
#pragma comment(lib, "ws2_32") 

using namespace std;

//邮件信息
struct sMailInfo{
	char*   m_pcUserName;//用户登录邮箱的名称    
	char*   m_pcUserPassWord;//用户登录邮箱的密码    
	char*   m_pcSenderName;//用户发送时显示的名称    
	char*   m_pcSender;//发送者的邮箱地址    
	char*   m_pcReceiver;//接收者的邮箱地址    
	char*   m_pcTitle;//邮箱标题    
	char*   m_pcBody;//邮件文本正文    
	char*   m_pcIPAddr;//服务器的IP    
	char*   m_pcIPName;//服务器的名称（IP与名称二选一，优先取名称）    
	sMailInfo() { memset(this, 0, sizeof(sMailInfo)); }
};

class CSendMail{
public:
	CSendMail(void);
	~CSendMail(void);

public:
	bool SendMail(sMailInfo &smailInfo);//发送邮件，需要在发送的时候初始化邮件信息    

protected:   
	void CharToBase64(char* pBuff64, char* pSrcBuff, int iLen);//把char类型转换成Base64类型    
	bool  CReateSocket(SOCKET &sock);//建立socket连接    
	bool Logon(SOCKET &sock);//登录邮箱，主要进行发邮件前的准备工作    

	bool SendHead(SOCKET &sock);//发送邮件头    
	bool SendTextBody(SOCKET &sock);//发送邮件文本正文      
	bool SendEnd(SOCKET &sock);//发送邮件结尾  

protected:
	char  m_cSendBuff[4096];//发送缓冲区    
	char  m_cReceiveBuff[1024];
	sMailInfo m_sMailInfo;
};

CSendMail::CSendMail(void){
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memset(m_cReceiveBuff, 0, sizeof(m_cReceiveBuff));
}

CSendMail::~CSendMail(void){
}

//把char类型转换成Base64类型
void CSendMail::CharToBase64(char* pBuff64, char* pSrcBuff, int iLen)
{
	//1   1   1   1   1   1   1   1    
	// 分配给pBuff64  ↑ 分配给pBuff64+1    
	//         point所在的位置    
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//base64所映射的字符表    
	int point;//每一个源字符拆分的位置，可取2,4,6；初始为2    
	point = 2;
	int i;
	int iIndex;//base64字符的索引    
	char n = 0;//上一个源字符的残留值    
	for (i = 0; i<iLen; i++)
	{
		if (point == 2)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3f;//取得pSrcBuff的高point位    
		}
		else if (point == 4)
		{
			iIndex = ((*pSrcBuff) >> point) & 0xf;//取得pSrcBuff的高point位    
		}
		else if (point == 6)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3;//取得pSrcBuff的高point位    
		}
		iIndex += n;//与pSrcBuff-1的低point结合组成Base64的索引    
		*pBuff64++ = Base64Encode[iIndex];//由索引表得到pBuff64    
		n = ((*pSrcBuff) << (6 - point));//计算源字符中的残留值    
		n = n & 0x3f;//确保n的最高两位为0    
		point += 2;//源字符的拆分位置上升2    
		if (point == 8)//如果拆分位置为8说明pSrcBuff有6位残留，可以组成一个完整的Base64字符，所以直接再组合一次    
		{
			iIndex = (*pSrcBuff) & 0x3f;//提取低6位，这就是索引了    
			*pBuff64++ = Base64Encode[iIndex];//    
			n = 0;//残留值为0    
			point = 2;//拆分位置设为2    
		}
		pSrcBuff++;

	}
	if (n != 0)
	{
		*pBuff64++ = Base64Encode[n];
	}
	if (iLen % 3 == 2)//如果源字符串长度不是3的倍数要用'='补全    
	{
		*pBuff64 = '=';
	}
	else if (iLen % 3 == 1)
	{
		*pBuff64++ = '=';
		*pBuff64 = '=';
	}
}

//建立socket连接
bool CSendMail::CReateSocket(SOCKET &sock)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return false;
	}
	if (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return false;
	}
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock == INVALID_SOCKET)
	{
		return false;
	}

	sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(25);//发邮件一般都是25端口    
	if (m_sMailInfo.m_pcIPName == "")
	{
		servaddr.sin_addr.s_addr = inet_addr(m_sMailInfo.m_pcIPAddr);//直接使用IP地址    
	}
	else
	{
		struct hostent *hp = gethostbyname(m_sMailInfo.m_pcIPName);//使用名称    
		if (hp == NULL)
		{
			DWORD dwErrCode = GetLastError();
			return false;
		}
		servaddr.sin_addr.s_addr = *(int*)(*hp->h_addr_list);
	}


	int ret = connect(sock, (sockaddr*)&servaddr, sizeof(servaddr));//建立连接    
	if (ret == SOCKET_ERROR)
	{
		DWORD dwErr = GetLastError();
		return false;
	}

	return true;
}

//登录邮箱，主要进行发邮件前的准备工作
bool CSendMail::Logon(SOCKET &sock)
{
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "HELO []\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//开始会话    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '5' || m_cReceiveBuff[2] != '0')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "AUTH LOGIN\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//请求登录    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	CharToBase64(m_cSendBuff, m_sMailInfo.m_pcUserName, strlen(m_sMailInfo.m_pcUserName));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//发送用户名    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	CharToBase64(m_cSendBuff, m_sMailInfo.m_pcUserPassWord, strlen(m_sMailInfo.m_pcUserPassWord));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//发送用户密码    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '5')
	{
		cout << m_cReceiveBuff[0] << m_cReceiveBuff[1] << m_cReceiveBuff[2] << endl;
		return false;
	}
	return true;//登录成功    
}

//发送邮件头
bool CSendMail::SendHead(SOCKET &sock)
{
	int rt;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "MAIL FROM:<%s>\r\n", m_sMailInfo.m_pcSender);
	rt = send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);

	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "RCPT TO:<%s>\r\n", m_sMailInfo.m_pcReceiver);
	rt = send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memcpy(m_cSendBuff, "DATA\r\n", strlen("DATA\r\n"));
	rt = send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "From:\"%s\"<%s>\r\n", m_sMailInfo.m_pcSenderName, m_sMailInfo.m_pcSender);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "To:\"INVT.COM.CN\"<%s>\r\n", m_sMailInfo.m_pcReceiver);
	sprintf_s(&m_cSendBuff[strlen(m_cSendBuff)], 150, "Subject:%s\r\nMime-Version: 1.0\r\nContent-Type: multipart/mixed;   boundary=\"INVT\"\r\n\r\n", m_sMailInfo.m_pcTitle);
	rt = send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}

	return true;
}

//发送邮件正文
bool CSendMail::SendTextBody(SOCKET &sock)
{
	int rt;
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "--INVT\r\nContent-Type: text/plain;\r\n  charset=\"gb2312\"\r\n\r\n%s\r\n\r\n", m_sMailInfo.m_pcBody);
	rt = send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);
	if (rt != strlen(m_cSendBuff))
	{
		return false;
	}
	else
	{
		return true;
	}
}

//发送邮件结尾
bool CSendMail::SendEnd(SOCKET &sock)
{
	sprintf_s(m_cSendBuff, "--INVT--\r\n.\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);

	sprintf_s(m_cSendBuff, "QUIT\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);
	Sleep(500);
	closesocket(sock);
	WSACleanup();
	return true;
}

//发送邮件，需要在发送的时候初始化邮件信息
bool CSendMail::SendMail(sMailInfo &smailInfo)
{
	memcpy(&m_sMailInfo, &smailInfo, sizeof(smailInfo));
	if (m_sMailInfo.m_pcBody == NULL
		|| m_sMailInfo.m_pcIPAddr == NULL
		|| m_sMailInfo.m_pcIPName == NULL
		|| m_sMailInfo.m_pcReceiver == NULL
		|| m_sMailInfo.m_pcSender == NULL
		|| m_sMailInfo.m_pcSenderName == NULL
		|| m_sMailInfo.m_pcTitle == NULL
		|| m_sMailInfo.m_pcUserName == NULL
		|| m_sMailInfo.m_pcUserPassWord == NULL)
	{
		return false;
	}
	SOCKET sock;
	if (!CReateSocket(sock))//建立连接    
	{
		return false;
	}

	if (!Logon(sock))//登录邮箱    
	{
		return false;
	}

	if (!SendHead(sock))//发送邮件头    
	{
		return false;
	}

	if (!SendTextBody(sock))//发送邮件文本部分    
	{
		return false;
	}

	if (!SendEnd(sock))//结束邮件，并关闭sock    
	{
		return false;
	}

	return true;
}