#include "precompile.h"

unsigned int WINAPI CallTickThread(LPVOID p)
{
	srand((unsigned int)time(NULL));
	CThread *pTickThread = (CThread*)p;
	pTickThread->OnInitThread();
	pTickThread->OnTickThread();

	return 1;
}

CThread::CThread()
{
	m_hThread = NULL;
	m_bIsRun = false;
	m_dwWaitTick = 0;
	m_dwTickCount = 0;
	m_hQuitEvent = CreateEvent
		(NULL,
		TRUE, // TRUE == 수동리셋, FALSE == 자동리셋
		FALSE, // TRUE == 신호 상태시작, FALSE == 비신호 상태시작
		NULL);
	if (NULL == m_hQuitEvent) return;
}

CThread::~CThread()
{
	if (NULL != m_hQuitEvent)
	{
		CloseHandle( m_hQuitEvent );
		m_hQuitEvent = NULL;
	}
	if (NULL != m_hThread)
	{
		CloseHandle( m_hThread );
		m_hThread = NULL;
	}
}

bool CThread::CreateThread(DWORD dwWaitTick)
{
	m_dwWaitTick = dwWaitTick;

	unsigned int uiThreadId = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CallTickThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (NULL == m_hThread)
	{
		LOG(LOG_ERROR_NORMAL, " SYSTEM | CThread::CreateTickThread() | TickThread 생성 실패 : Error(%u) ", GetLastError() );	
		return false;
	}

	//m_dwWaitTick = dwWaitTick;
	return true;
}

void CThread::Run()
{
	if( false == m_bIsRun )
	{
		m_bIsRun = true;
		ResumeThread( m_hThread );
	}
}

void CThread::Stop()
{
	if( true == m_bIsRun )
	{
		m_bIsRun = false;
		SuspendThread( m_hThread );
	}
}

void CThread::OnTickThread()
{
	while( true )
	{
		DWORD dwRet = WaitForSingleObject( m_hQuitEvent, m_dwWaitTick );

		if( WAIT_TIMEOUT == dwRet )
		{
			m_dwTickCount++;
			OnProcess();
		}
		else if (WAIT_OBJECT_0 == dwRet)
		{
			break;
		}
	}

	OnCloseThread();
}

void CThread::DestroyThread()
{
	Run();
	SetEvent( m_hQuitEvent );
	WaitForSingleObject( m_hThread , INFINITE );
}

