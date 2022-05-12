#pragma once

class NETLIB_API CThread
{
public:
	CThread();
	virtual ~CThread();
	
	bool	CreateThread( DWORD dwWaitTick );
	void	DestroyThread();
	void	Run();
	void	Stop();
	void	OnTickThread();

	virtual void	OnInitThread() = 0;
	virtual void	OnCloseThread() = 0;
	virtual void	OnProcess() = 0;

	
	inline HANDLE GetQuitEvent() { return m_hQuitEvent; }
	inline DWORD GetTickCount() { return m_dwTickCount; }
	inline bool	IsRun() { return m_bIsRun; }

protected:
	HANDLE	m_hThread;
	HANDLE	m_hQuitEvent;
	bool	m_bIsRun;
	DWORD	m_dwWaitTick;
	DWORD	m_dwTickCount;
};
