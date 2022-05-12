#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	char command[255 + 1] = { '\0', }; //Windows CMD 명령어를 담을 문자 배열
	int lines = 10; //세로 길이
	int cols = 150; //가로 길이
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD 명령문을 실행하는 함수

	//////////////////////////////////////////////////////////////////////////
	IocpDBAgent()->ServerStart();
	//////////////////////////////////////////////////////////////////////////

	WaitForSingleObject(GetCurrentThread(), INFINITE);

	system("pause");

	return 0;
}

