#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char command[255 + 1] = { '\0', }; //Windows CMD ��ɾ ���� ���� �迭
	int lines = 10; //���� ����
	int cols = 150; //���� ����
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD ��ɹ��� �����ϴ� �Լ�

	//////////////////////////////////////////////////////////////////////////
	IocpDBAgent()->ServerStart();
	//////////////////////////////////////////////////////////////////////////

	WaitForSingleObject(GetCurrentThread(), INFINITE);

	system("pause");

	return 0;
}

