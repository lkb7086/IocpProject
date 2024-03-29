// LoginServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "../NetLib/RSA.h"

HANDLE g_hMutex = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	char command[255 + 1] = { '\0', }; //Windows CMD 명령어를 담을 문자 배열
	int lines = 10; //세로 길이
	int cols = 150; //가로 길이
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD 명령문을 실행하는 함수

	g_hMutex = CreateMutex(NULL, FALSE, _T("Mutex_LoginServer"));
	if (NULL == g_hMutex)
		return 0;
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		puts("The login server is already running.");
		system("pause");
		return 0;
	}

	//char p[256]; strcpy_s(p, "opalopalopal");
	//CRSA rsa(p, 12);
	//rsa.Encrypt();
	//rsa.Decrypt();

	//////////////////////////////////////////////////////////////////////////
	IocpLoginServer()->ServerStart();
	//IocpLoginServer()->ConnectToNoSQLServer();
	//////////////////////////////////////////////////////////////////////////

	/*
	char ServerName[255];
	gethostname(ServerName, 255);   // 서버이름(컴퓨터이름)을 가져옴
	HOSTENT* Host = gethostbyname(ServerName);   // 이름으로 IP주소를 가져옴
	IN_ADDR Addr;

	memmove(&Addr, &Host->h_addr_list[2], 4);
	printf("서버이름 : %s, 서버 IP주소 : %s\n", ServerName, inet_ntoa(Addr));
	*/

	WaitForSingleObject(GetCurrentThread(), INFINITE);

	system("pause");

	return 0;
}

