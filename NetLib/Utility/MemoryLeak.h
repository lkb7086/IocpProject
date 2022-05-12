#pragma once

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include "crtdbg.h"
#endif

class CMemoryLeak
{
public:
	CMemoryLeak();
	~CMemoryLeak();
};