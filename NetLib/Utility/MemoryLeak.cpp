#include "../Precompile.h"
#include "MemoryLeak.h"

CMemoryLeak::CMemoryLeak()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	/*
	// Send all reports to STDOUT
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);*/

	//_CrtSetBreakAlloc(235);

	//_CrtSetAllocHook(); // ?
#endif
}


CMemoryLeak::~CMemoryLeak()
{
#ifdef _DEBUG
	// 메모리를 체크합니다.
	//_ASSERTE(_CrtCheckMemory());
#endif
}
