#pragma once

class NETLIB_API CMonitor final
{
public:
	class NETLIB_API Owner final
	{
	public:
		explicit Owner(CMonitor& crit);
		~Owner();
		Owner(const Owner& rhs) = delete;
		Owner& operator=(const Owner& rhs) = delete;
		Owner(Owner&&) = delete;
		Owner& operator=(Owner&&) = delete;
	private:
		CMonitor& m_csSyncObject;
	};

	CMonitor();
	~CMonitor();
	CMonitor(const CMonitor& rhs) = delete;
	CMonitor& operator=(const CMonitor& rhs) = delete;
	CMonitor(CMonitor&&) = delete;
	CMonitor& operator=(CMonitor&&) = delete;
	
	void Enter();
	void Leave();

private:
	CRITICAL_SECTION m_csSyncObject;
};

/*
// 다른 스레드가 이미 들어갔으면 false 반환, 스핀락이라 대기상태로 되지 않음
BOOL CMonitor::TryEnter() { return TryEnterCriticalSection(&m_csSyncObject); }
*/

enum ESRWLock
{
	LockShared,
	LockExclusive,
	TryLockShared,
	TryLockExclusive
};

class NETLIB_API CMonitorSRW final
{
public:
	class NETLIB_API OwnerSRW final
	{
	public:
		explicit OwnerSRW(CMonitorSRW& crit, ESRWLock _eLockState);
		~OwnerSRW();
		OwnerSRW(const OwnerSRW& rhs) = delete;
		OwnerSRW& operator=(const OwnerSRW& rhs) = delete;
		OwnerSRW(OwnerSRW&&) = delete;
		OwnerSRW& operator=(OwnerSRW&&) = delete;
	private:
		CMonitorSRW& m_monitorObject;
		ESRWLock m_eLockState;
	};

	CMonitorSRW();
	~CMonitorSRW();
	CMonitorSRW(const CMonitorSRW& rhs) = delete;
	CMonitorSRW& operator=(const CMonitorSRW& rhs) = delete;
	CMonitorSRW(CMonitorSRW&&) = delete;
	CMonitorSRW& operator=(CMonitorSRW&&) = delete;

	void Enter(ESRWLock _eLockState);
	void Leave(ESRWLock _eLockState);

private:
	SRWLOCK m_srwSyncObject;
};

/*
SRWLOCK srwLock;
InitializeSRWLock(&srwLock);
AcquireSRWLockExclusive(&srwLock);
ReleaseSRWLockExclusive(&srwLock);

AcquireSRWLockShared(&srwLock);
ReleaseSRWLockShared(&srwLock);

//TryAcquireSRWLockExclusive(&srwLock);
//TryAcquireSRWLockExclusive(&srwLock);

//SleepConditionVariableSRW
*/