// LoginServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

HANDLE g_hMutex = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	char command[255 + 1] = { '\0', }; //Windows CMD ��ɾ ���� ���� �迭
	int lines = 10; //���� ����
	int cols = 150; //���� ����
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD ��ɹ��� �����ϴ� �Լ�

	g_hMutex = CreateMutex(NULL, FALSE, _T("Mutex_LoginServer"));
	if (NULL == g_hMutex)
		return 0;
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		puts("The login server is already running.");
		system("pause");
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	IocpLoginServer()->ServerStart();
	//////////////////////////////////////////////////////////////////////////

	/*
	char ServerName[255];
	gethostname(ServerName, 255);   // �����̸�(��ǻ���̸�)�� ������
	HOSTENT* Host = gethostbyname(ServerName);   // �̸����� IP�ּҸ� ������
	IN_ADDR Addr;

	memmove(&Addr, &Host->h_addr_list[2], 4);
	printf("�����̸� : %s, ���� IP�ּ� : %s\n", ServerName, inet_ntoa(Addr));
	*/

	WaitForSingleObject(GetCurrentThread(), INFINITE);

	system("pause");

	return 0;
}

