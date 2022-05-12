#include "Precompile.h"

CMonitor::CMonitor()
{
	//InitializeCriticalSection(&m_csSyncObject);
	
	if(false == ::InitializeCriticalSectionAndSpinCount(&m_csSyncObject, 4000))
		LOG(LOG_ERROR_HIGH, "SYSTEM | CMonitor::CMonitor(void) | InitializeCriticalSectionAndSpinCount false");

	// 스핀횟수를 중간에 변경함
	//SetCriticalSectionSpinCount(&m_csSyncObject, 1000)
}

CMonitor::~CMonitor()
{
	::DeleteCriticalSection( &m_csSyncObject );
}

void CMonitor::Enter()
{
	::EnterCriticalSection(&m_csSyncObject);
}

void CMonitor::Leave()
{
    ::LeaveCriticalSection(&m_csSyncObject);
}

CMonitor::Owner::Owner(CMonitor &crit)
   : m_csSyncObject(crit)
{
	m_csSyncObject.Enter();
}

CMonitor::Owner::~Owner()
{
   m_csSyncObject.Leave();
}

//---------------------------------------------------------------------------------------------------------
CMonitorSRW::CMonitorSRW()
{
	InitializeSRWLock(&m_srwSyncObject);
}

CMonitorSRW::~CMonitorSRW() {}

void CMonitorSRW::Enter(ESRWLock _eLockState)
{
	switch (_eLockState)
	{
	case LockShared:
		AcquireSRWLockShared(&m_srwSyncObject);
		break;
	case LockExclusive:
		AcquireSRWLockExclusive(&m_srwSyncObject);
		break;
	case TryLockShared:
		while (!TryAcquireSRWLockShared(&m_srwSyncObject)) {};   // if the current thread could not acquire the lock, the return value is zero
		break;
	case TryLockExclusive:
		while (!TryAcquireSRWLockExclusive(&m_srwSyncObject)) {};
		break;
	default:
		break;
	}
}

void CMonitorSRW::Leave(ESRWLock _eLockState)
{
	switch (_eLockState)
	{
	case LockShared:
		ReleaseSRWLockShared(&m_srwSyncObject);
		break;
	case LockExclusive:
		ReleaseSRWLockExclusive(&m_srwSyncObject);
		break;
	case TryLockShared:
		ReleaseSRWLockShared(&m_srwSyncObject);
		break;
	case TryLockExclusive:
		ReleaseSRWLockExclusive(&m_srwSyncObject);
		break;
	default:
		break;
	}
}

CMonitorSRW::OwnerSRW::OwnerSRW(CMonitorSRW &monitor, ESRWLock _eLockState)
	: m_monitorObject(monitor), m_eLockState(_eLockState)
{
	m_monitorObject.Enter(_eLockState);
}

CMonitorSRW::OwnerSRW::~OwnerSRW()
{
	m_monitorObject.Leave(m_eLockState);
}