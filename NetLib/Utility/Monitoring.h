#pragma once
//#include "../Precompile.h"
#include "psapi.h"
#define fixInRange(minimum, x, maximum)     min(maximum, max(x, minimum)) 

class NETLIB_API CMonitoring final : public CSingleton
{
	DECLEAR_SINGLETON(CMonitoring);
public:
	CMonitoring();
	~CMonitoring();

	double ProcessCPU_Usage();
	SIZE_T ProcessMemoryUsage();
	SIZE_T PhysyicMemoryUsage();

private:
	ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
	int numProcessors;
	HANDLE self;
};
CREATE_FUNCTION(CMonitoring, Monitoring)
