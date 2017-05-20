#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include "stdafx.h"  
#include <afx.h>
#include <afxtempl.h>
#pragma comment(lib,"WSOCK32")    
#pragma comment(lib, "ws2_32") 

using namespace std;

//�ʼ���Ϣ
struct sMailInfo{
	char*   m_pcUserName;//�û���¼���������    
	char*   m_pcUserPassWord;//�û���¼���������    
	char*   m_pcSenderName;//�û�����ʱ��ʾ������    
	char*   m_pcSender;//�����ߵ������ַ    
	char*   m_pcReceiver;//�����ߵ������ַ    
	char*   m_pcTitle;//�������    
	char*   m_pcBody;//�ʼ��ı�����    
	char*   m_pcIPAddr;//��������IP    
	char*   m_pcIPName;//�����������ƣ�IP�����ƶ�ѡһ������ȡ���ƣ�    
	sMailInfo() { memset(this, 0, sizeof(sMailInfo)); }
};

class CSendMail{
public:
	CSendMail(void);
	~CSendMail(void);

public:
	bool SendMail(sMailInfo &smailInfo);//�����ʼ�����Ҫ�ڷ��͵�ʱ���ʼ���ʼ���Ϣ    

protected:   
	void CharToBase64(char* pBuff64, char* pSrcBuff, int iLen);//��char����ת����Base64����    
	bool  CReateSocket(SOCKET &sock);//����socket����    
	bool Logon(SOCKET &sock);//��¼���䣬��Ҫ���з��ʼ�ǰ��׼������    

	bool SendHead(SOCKET &sock);//�����ʼ�ͷ    
	bool SendTextBody(SOCKET &sock);//�����ʼ��ı�����      
	bool SendEnd(SOCKET &sock);//�����ʼ���β  

protected:
	char  m_cSendBuff[4096];//���ͻ�����    
	char  m_cReceiveBuff[1024];
	sMailInfo m_sMailInfo;
};

CSendMail::CSendMail(void){
	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	memset(m_cReceiveBuff, 0, sizeof(m_cReceiveBuff));
}

CSendMail::~CSendMail(void){
}

//��char����ת����Base64����
void CSendMail::CharToBase64(char* pBuff64, char* pSrcBuff, int iLen)
{
	//1   1   1   1   1   1   1   1    
	// �����pBuff64  �� �����pBuff64+1    
	//         point���ڵ�λ��    
	static char Base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//base64��ӳ����ַ���    
	int point;//ÿһ��Դ�ַ���ֵ�λ�ã���ȡ2,4,6����ʼΪ2    
	point = 2;
	int i;
	int iIndex;//base64�ַ�������    
	char n = 0;//��һ��Դ�ַ��Ĳ���ֵ    
	for (i = 0; i<iLen; i++)
	{
		if (point == 2)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3f;//ȡ��pSrcBuff�ĸ�pointλ    
		}
		else if (point == 4)
		{
			iIndex = ((*pSrcBuff) >> point) & 0xf;//ȡ��pSrcBuff�ĸ�pointλ    
		}
		else if (point == 6)
		{
			iIndex = ((*pSrcBuff) >> point) & 0x3;//ȡ��pSrcBuff�ĸ�pointλ    
		}
		iIndex += n;//��pSrcBuff-1�ĵ�point������Base64������    
		*pBuff64++ = Base64Encode[iIndex];//��������õ�pBuff64    
		n = ((*pSrcBuff) << (6 - point));//����Դ�ַ��еĲ���ֵ    
		n = n & 0x3f;//ȷ��n�������λΪ0    
		point += 2;//Դ�ַ��Ĳ��λ������2    
		if (point == 8)//������λ��Ϊ8˵��pSrcBuff��6λ�������������һ��������Base64�ַ�������ֱ�������һ��    
		{
			iIndex = (*pSrcBuff) & 0x3f;//��ȡ��6λ�������������    
			*pBuff64++ = Base64Encode[iIndex];//    
			n = 0;//����ֵΪ0    
			point = 2;//���λ����Ϊ2    
		}
		pSrcBuff++;

	}
	if (n != 0)
	{
		*pBuff64++ = Base64Encode[n];
	}
	if (iLen % 3 == 2)//���Դ�ַ������Ȳ���3�ı���Ҫ��'='��ȫ    
	{
		*pBuff64 = '=';
	}
	else if (iLen % 3 == 1)
	{
		*pBuff64++ = '=';
		*pBuff64 = '=';
	}
}

//����socket����
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
	servaddr.sin_port = htons(25);//���ʼ�һ�㶼��25�˿�    
	if (m_sMailInfo.m_pcIPName == "")
	{
		servaddr.sin_addr.s_addr = inet_addr(m_sMailInfo.m_pcIPAddr);//ֱ��ʹ��IP��ַ    
	}
	else
	{
		struct hostent *hp = gethostbyname(m_sMailInfo.m_pcIPName);//ʹ������    
		if (hp == NULL)
		{
			DWORD dwErrCode = GetLastError();
			return false;
		}
		servaddr.sin_addr.s_addr = *(int*)(*hp->h_addr_list);
	}


	int ret = connect(sock, (sockaddr*)&servaddr, sizeof(servaddr));//��������    
	if (ret == SOCKET_ERROR)
	{
		DWORD dwErr = GetLastError();
		return false;
	}

	return true;
}

//��¼���䣬��Ҫ���з��ʼ�ǰ��׼������
bool CSendMail::Logon(SOCKET &sock)
{
	recv(sock, m_cReceiveBuff, 1024, 0);

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "HELO []\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//��ʼ�Ự    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '5' || m_cReceiveBuff[2] != '0')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	sprintf_s(m_cSendBuff, "AUTH LOGIN\r\n");
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//�����¼    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	CharToBase64(m_cSendBuff, m_sMailInfo.m_pcUserName, strlen(m_sMailInfo.m_pcUserName));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//�����û���    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '3' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '4')
	{
		return false;
	}

	memset(m_cSendBuff, 0, sizeof(m_cSendBuff));
	CharToBase64(m_cSendBuff, m_sMailInfo.m_pcUserPassWord, strlen(m_sMailInfo.m_pcUserPassWord));
	m_cSendBuff[strlen(m_cSendBuff)] = '\r';
	m_cSendBuff[strlen(m_cSendBuff)] = '\n';
	send(sock, m_cSendBuff, strlen(m_cSendBuff), 0);//�����û�����    
	recv(sock, m_cReceiveBuff, 1024, 0);
	if (m_cReceiveBuff[0] != '2' || m_cReceiveBuff[1] != '3' || m_cReceiveBuff[2] != '5')
	{
		cout << m_cReceiveBuff[0] << m_cReceiveBuff[1] << m_cReceiveBuff[2] << endl;
		return false;
	}
	return true;//��¼�ɹ�    
}

//�����ʼ�ͷ
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

//�����ʼ�����
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

//�����ʼ���β
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

//�����ʼ�����Ҫ�ڷ��͵�ʱ���ʼ���ʼ���Ϣ
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
	if (!CReateSocket(sock))//��������    
	{
		return false;
	}

	if (!Logon(sock))//��¼����    
	{
		return false;
	}

	if (!SendHead(sock))//�����ʼ�ͷ    
	{
		return false;
	}

	if (!SendTextBody(sock))//�����ʼ��ı�����    
	{
		return false;
	}

	if (!SendEnd(sock))//�����ʼ������ر�sock    
	{
		return false;
	}

	return true;
}