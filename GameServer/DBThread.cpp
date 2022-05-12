#include "stdafx.h"
#include "DBThread.h"

bool CDBThread::m_bQuitFlag = true;

unsigned int WINAPI CallDBThread(LPVOID p)
{
	srand((unsigned int)time(NULL));
	tls_pSer = new(nothrow)CSerializer();
	if (nullptr == tls_pSer) return 1;

	CDBThread* pDBThread = (CDBThread*)p;
	pDBThread->OnDBThread();

	if (nullptr != tls_pSer)
	{
		delete tls_pSer;
		tls_pSer = nullptr;
	}

	return 1;
}

CDBThread::CDBThread() : m_hThread(NULL), m_bDBThreadUP(false)
{
	m_hQuitEvent = CreateEvent
		(NULL,
		FALSE, // TRUE == 수동리셋, FALSE == 자동리셋
		FALSE, // TRUE == 신호 상태시작, FALSE == 비신호 상태시작
		NULL);
	if (NULL == m_hQuitEvent) return;

	CreateThread();
}

CDBThread::~CDBThread()
{
	if (NULL != m_hQuitEvent)
	{
		CloseHandle(m_hQuitEvent);
		m_hQuitEvent = NULL;
	}

	if (NULL != m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

bool CDBThread::CreateThread()
{
	unsigned int uiThreadId = 0;
	m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CallDBThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (NULL == m_hThread)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CDBThread::CreateThread() | Thread 생성 실패 : Error(%u)", GetLastError());
		return false;
	}
	ResumeThread(m_hThread);
	return true;
}

void CDBThread::OnDBThread()
{
	while (m_bQuitFlag)
	{
		m_bDBThreadUP = false;
		DWORD dwRet = WaitForSingleObject(m_hQuitEvent, 1);
		m_bDBThreadUP = true;
		if (WAIT_OBJECT_0 == dwRet)
		{
			if (false == m_bQuitFlag) break;
			DatabaseManager()->PopDBQueue();
		}
		else if (WAIT_FAILED == dwRet)
			break;
	}
}

void CDBThread::DestroyThread()
{
	m_bQuitFlag = false;
	SetEvent(m_hQuitEvent);
	WaitForSingleObject(m_hThread, INFINITE);
}