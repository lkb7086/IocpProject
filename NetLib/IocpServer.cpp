#include "Precompile.h"

unsigned int WINAPI CallWorkerThread(LPVOID p);
unsigned int WINAPI CallManagerThread(LPVOID p);
CIocpServer* CIocpServer::m_pIocpServer = nullptr;
bool  CIocpServer::UDPThreadFlag = true;

CIocpServer::CIocpServer() : m_hProcessThreadEvent(NULL), m_isProcessThreadUP(false), m_lpProcessPacket(nullptr), m_isGameServer(false)
{
	m_hProcessThreadEvent = CreateEvent
		(NULL,
		FALSE, // TRUE == 수동리셋, FALSE == 자동리셋
		FALSE, // TRUE == 신호 상태시작, FALSE == 비신호 상태시작
		NULL);
	if (NULL == m_hProcessThreadEvent) return;

	m_bWorkThreadFlag = true;
	m_bProcessThreadFlag = true;
	m_bUdpWorkThreadFlag = true;
	m_dwProcessThreadCount = 0;
	m_dwWorkerThreadCount = 0;
}

CIocpServer::~CIocpServer()
{
	if (NULL != m_hProcessThreadEvent)
	{
		CloseHandle(m_hProcessThreadEvent);
		m_hProcessThreadEvent = NULL;
	}
	if (nullptr != m_lpProcessPacket)
	{
		delete[] m_lpProcessPacket;
		m_lpProcessPacket = nullptr;
	}
	//WSACleanup();
}

unsigned int WINAPI CallUdpRecvFromThread(LPVOID p)
{
	srand((unsigned int)time(NULL));
	CIocpServer* pServerSock = (CIocpServer*)p;
	pServerSock->UDP_RecvFromThread();
	return 1;
}

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	srand((unsigned int)time(NULL));

	CIocpServer* pServerSock = (CIocpServer*)p;
	//pServerSock->OnInitIocpServer();
	pServerSock->WorkerThread();
	//pServerSock->OnClose_IocpServer();
	return 1;
}

unsigned int WINAPI CallProcessThread(LPVOID p)
{
	srand((unsigned int)time(NULL)); // 스레드마다 srand를 호출해야 한다

	CIocpServer* pServerSock = (CIocpServer*)p;
	pServerSock->OnInitIocpServer();
	pServerSock->ProcessThread();
	pServerSock->OnClose_IocpServer();
	return 1;
}

bool CIocpServer::ServerStart(INITCONFIG &initConfig)
{
	srand((unsigned int)time(NULL));

	m_usPort = initConfig.nServerPort;
	m_dwWorkerThreadCount = initConfig.nWorkerThreadCnt;
	m_dwProcessThreadCount = initConfig.nProcessThreadCnt;

	if (InitializeSocket() == false)
		return false;
	if (!CreateWorkerIOCP())
		return false;
	if (!CreateListenAndUdpSock())
		return false;
	if (!CreateWorkerThreads())
		return false;
	if (!CreateProcessThreads())
		return false;
	//if (!CreateListenAndUdpSock()) FIXED: 자리옮김
	//return false;

	initConfig.sockListener = GetListenSocket();
	
	if (nullptr != m_lpProcessPacket)
		delete[] m_lpProcessPacket;
	m_lpProcessPacket = new(nothrow) PROCESSPACKET[initConfig.nProcessPacketCnt];
	for (int i = 0; i < initConfig.nProcessPacketCnt; i++)
	{
		m_stackPacket.Push(&m_lpProcessPacket[i]);
	}

	m_dwProcessPacketCnt = initConfig.nProcessPacketCnt;

	return true;
}

// FIXME
void CIocpServer::ServerOff()
{
	puts("서버 종료시작");

	// HACK: UDP스레드 종료
	UDPThreadFlag = false;

	// 로그스레드 종료
	Log()->Destroy_LogThread();

	// 워커스레드 종료
	if (NULL != m_hWorkerIOCP)
	{
		m_bWorkThreadFlag = false;
		for (DWORD i = 0; i < m_dwWorkerThreadCount; i++)
		{
			// WorkerThread에 종료 메시지를 보낸다
			PostQueuedCompletionStatus(m_hWorkerIOCP, 0, 0, NULL);
		}
		WaitForMultipleObjects(MAX_WORKER_THREAD, m_hWorkerThread, TRUE, INFINITE);
		
		CloseHandle(m_hWorkerIOCP);
		m_hWorkerIOCP = NULL;
	}

	// 로직스레드 종료
	if (NULL != m_hProcessThreadEvent)
	{
		m_bProcessThreadFlag = false;
		for (DWORD i = 0; i < m_dwProcessThreadCount; i++)
			SetEvent(m_hProcessThreadEvent);
		WaitForMultipleObjects(MAX_PROCESS_THREAD, m_hProcessThread, TRUE, INFINITE);
	}

	///*
	// 모든핸들 close
	for (unsigned int i = 0; i < m_dwWorkerThreadCount; i++)
	{
		if (m_hWorkerThread[i] != NULL)
			CloseHandle(m_hWorkerThread[i]);
		m_hWorkerThread[i] = NULL;
	}

	for (DWORD i = 0; i < m_dwProcessThreadCount; i++)
	{
		if (m_hProcessThread[i] != NULL)
			CloseHandle(m_hProcessThread[i]);
		m_hProcessThread[i] = NULL;
	}

	for (DWORD i = 0; i < 1; i++)
	{
		if (m_hUdpRecvFromThread[i] != NULL)
			CloseHandle(m_hUdpRecvFromThread[i]);
		m_hUdpRecvFromThread[i] = NULL;
	}

	if (m_ListenSock != INVALID_SOCKET)
	{
		closesocket(m_ListenSock);
		m_ListenSock = INVALID_SOCKET;
	}

	WSACleanup();
	//*/

	return;
}

bool CIocpServer::InitializeSocket()
{
	for (DWORD i = 0; i < m_dwWorkerThreadCount; i++)
		m_hWorkerThread[i] = NULL;
	for (DWORD i = 0; i < m_dwProcessThreadCount; i++)
		m_hProcessThread[i] = NULL;
	for (DWORD i = 0; i < 1; i++)
		m_hUdpRecvFromThread[i] = NULL;

	m_hWorkerIOCP = NULL;
	//m_hProcessIOCP = NULL; // FIXED: IOCP QUEUE
	m_ListenSock = INVALID_SOCKET;
	m_UdpBlockSock = INVALID_SOCKET;

	WSADATA	WsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (nRet)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::InitializeSocket() | WSAStartup() Failed");
		return false;
	}
	return true;
}

// CPU개수를 파악하여 적당한 WorkerThread의 개수를 얻어온다 (cpu*2 + 1)
void CIocpServer::GetProperThreadsCount()
{
	/*
	SYSTEM_INFO		SystemInfo;
	DWORD			ProperCount = 0;
	DWORD			DispatcherCount = 0;
	GetSystemInfo(&SystemInfo);
	ProperCount = SystemInfo.dwNumberOfProcessors * 2 + 1;
	if (ProperCount > MAX_WORKER_THREAD)
		ProperCount = (DWORD)MAX_WORKER_THREAD;
	m_dwWorkerThreadCount = ProperCount;*/
}

bool CIocpServer::CreateListenAndUdpSock()
{
	SOCKADDR_IN	si_addr, si_udp_addr;
	int			nRet = 0;
	int			nZero = 0;
	char		cFlag = 1;
	int optVal = 0;
	int optLen = sizeof(int);
	int rcv_buf = 100000, snd_buf = 40000;  // HACK

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 리슨소켓
	m_ListenSock = WSASocketW(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_ListenSock)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateListenAndUdpSock() | ListenSocket Creation Failed : (%u)", GetLastError());
		return false;
	}
	// 소켓설정: SO_REUSEADDR, bind 호출전에 설정해야함, TIME_WAIT 파괴
	if (SOCKET_ERROR == setsockopt(m_ListenSock, SOL_SOCKET, SO_REUSEADDR, &cFlag, sizeof(cFlag)))
		return false;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// UDP 봉쇄동기소켓
	m_UdpBlockSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_UdpBlockSock == INVALID_SOCKET)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateListenAndUdpSock() | m_UdpBlockSock Creation Failed : (%u)", GetLastError());
		return false;
	}
	// 소켓설정: 버퍼 늘리기, TCP는 listen, connect() 함수 전에 크기를 변경하는 것이 권장
	setsockopt(m_UdpBlockSock, SOL_SOCKET, SO_RCVBUF, (char*)&rcv_buf, sizeof(rcv_buf));
	setsockopt(m_UdpBlockSock, SOL_SOCKET, SO_SNDBUF, (char*)&snd_buf, sizeof(snd_buf));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// bind: 리슨소켓
	memset(&si_addr, 0, sizeof(SOCKADDR_IN));
	si_addr.sin_family = AF_INET;
	si_addr.sin_port = htons(m_usPort);
	si_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //si_addr.sin_addr.s_addr = inet_addr("192.168.219.5");   // 명시적으로 아이피를 적을시

	nRet = ::bind(m_ListenSock, (struct sockaddr*) &si_addr, sizeof(si_addr));
	if (SOCKET_ERROR == nRet)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CIocpServer::CreateListenAndUdpSock() | m_ListenSock bind() Failed : (%u)",
			GetLastError());
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// bind: UDP 봉쇄동기소켓
	memset(&si_udp_addr, 0, sizeof(SOCKADDR_IN));
	si_udp_addr.sin_family = AF_INET;
	si_udp_addr.sin_port = htons(m_usPort - 2);
	si_udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	nRet = ::bind(m_UdpBlockSock, (SOCKADDR*)&si_udp_addr, sizeof(si_udp_addr));
	if (SOCKET_ERROR == nRet)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateListenAndUdpSock() | m_UdpBlockSock bind() Failed : (%u)", GetLastError());
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// listen
	nRet = listen(m_ListenSock, SOMAXCONN);
	if (SOCKET_ERROR == nRet)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CIocpServer::CreateListenAndUdpSock() | listen() Failed : (%u)",
			GetLastError());
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TCP Listen CreateIoCompletionPort
	HANDLE hIOCPHandle = NULL;
	hIOCPHandle = CreateIoCompletionPort((HANDLE)m_ListenSock,
		m_hWorkerIOCP, (DWORD)NULL, 0); // (DWORD)NULL 전달
	if (NULL == hIOCPHandle || m_hWorkerIOCP != hIOCPHandle)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CIocpServer::CreateListenAndUdpSock() | CreateIoCompletionPort() Failed : (%u)",
			GetLastError());
		return false;
	}

	return true;
}

bool CIocpServer::CreateProcessThreads()
{
	HANDLE	hThread = NULL;
	UINT	uiThreadId = 0;

	for (DWORD dwCount = 0; dwCount < m_dwProcessThreadCount; dwCount++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, &CallProcessThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (hThread == NULL)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateProcessThreads() | _beginthreadex() Failed : (%u)", GetLastError());
			return false;
		}
		m_hProcessThread[dwCount] = hThread;
		ResumeThread(hThread);

		SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
	}
	return true;
}

bool CIocpServer::CreateWorkerThreads()
{
	HANDLE	hThread = NULL;
	UINT	uiThreadId = 0;

	for (DWORD dwCount = 0; dwCount < m_dwWorkerThreadCount; dwCount++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (hThread == NULL)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateWorkerThreads() | _beginthreadex() Failed : (%u)", GetLastError());
			return false;
		}
		m_hWorkerThread[dwCount] = hThread;
		ResumeThread(hThread);
	}

	hThread = NULL;
	uiThreadId = 0;
	// UDP RECVFROM
	for (DWORD dwCount = 0; dwCount < 1; dwCount++)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, &CallUdpRecvFromThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (hThread == NULL)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::CreateUdpWorkerThreads() | _beginthreadex() Failed : (%u)", GetLastError());
			return false;
		}
		m_hUdpRecvFromThread[dwCount] = hThread;
		ResumeThread(hThread);
	}
	SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);

	return true;
}

bool CIocpServer::CreateWorkerIOCP()
{
	m_hWorkerIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hWorkerIOCP == NULL)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CIocpServer::CreateWorkerIOCP() | CreateIoCompletionPort() Failed : (%u)",
			GetLastError());
		return false;
	}

	return true;
}

void CIocpServer::WorkerThread()
{
	BOOL					bSuccess = false;
	LPOVERLAPPED			lpOverlapped = nullptr;
	CConnection*			lpConnection = nullptr;
	DWORD					dwIoSize = 0;

	while (m_bWorkThreadFlag)
	{
		bSuccess = false;
		dwIoSize = 0;
		lpConnection = nullptr;
		lpOverlapped = nullptr;

		// https://msdn.microsoft.com/ko-kr/library/windows/desktop/aa364986(v=vs.85).aspx
		// lpOverlapped == NULL -> buffer, lpConnection, dwIoSize의 값은 저장되지 않음
		// lpOverlapped == Not NULL -> 반환값 true or false / beffer에 값이 저장됨
		bSuccess = GetQueuedCompletionStatus(
			m_hWorkerIOCP,
			&dwIoSize,
			(PULONG_PTR)&lpConnection, // OP_ACCEPT: NULL == lpConnection, 나머지: Player[i]
			&lpOverlapped,             // OVERLAPPED_EX
			INFINITE);

		if (false == m_bWorkThreadFlag)
			break;

		if (nullptr == lpOverlapped)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::WorkerThread() | lpOverlapped is NULL %u", GetLastError());
			// 여기서 wsarecv를 안해도 계속 받아진다
			continue;
		}
		LPOVERLAPPED_EX lpOverlappedEx = (LPOVERLAPPED_EX)lpOverlapped;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*
		if (!bSuccess)
		{
		LOG(LOG_INFO_LOW, "SYSTEM | CIocpServer::WorkerThread() | GetLastError(%u), dwIoSize%d, lpConnection%d, lpOverlapped%d",
		GetLastError(), dwIoSize, lpConnection, lpOverlapped);
		}
		*/
		if (nullptr == lpConnection && eOperationType::OP_ACCEPT != lpOverlappedEx->s_eOperation)
			LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::WorkerThread() | NULL == lpConnection %u", GetLastError());
		
		/*
		if (!bSuccess && OP_ACCEPT == lpOverlappedEx->s_eOperation)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::WorkerThread() | AcceptEX error: %u / %d / %d / %d / %p / %d",
				((CConnection*)lpOverlappedEx->s_lpConnection)->GetSocket(),
				((CConnection*)lpOverlappedEx->s_lpConnection)->GetAcceptIoRefCount(),
				((CConnection*)lpOverlappedEx->s_lpConnection)->GetRecvIoRefCount(),
				((CConnection*)lpOverlappedEx->s_lpConnection)->GetSendIoRefCount(),
				lpOverlappedEx->s_lpConnection, WSAGetLastError());
		}
		*/
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// AcceptEX: bSuccess == true, dwIoSize == 0, lpConnection == NULL(설정값), lpOverlapped == NOT NULL
		// 정상종료: bSuccess == true, dwIoSize == 0, lpConnection와 lpOverlapped == NOT NULL
		// 비정상종료: bSuccess == false,  dwIoSize == 0, lpConnection와 lpOverlapped == NOT NULL / GetLastError() == 64(호스트다운)
		if (!bSuccess || (bSuccess && !dwIoSize && (eOperationType::OP_ACCEPT != lpOverlappedEx->s_eOperation)))
		{
			lpConnection = (CConnection*)lpOverlappedEx->s_lpConnection;
			if (lpConnection == nullptr)
			{
				LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::WorkerThread() | NULL == lpOverlappedEx->s_lpConnection %u", GetLastError());
				continue;
			}

			/*
			if (INVALID_SOCKET != lpConnection->GetSocket())
			{
				struct linger lin = { 1, 0 };
				setsockopt(lpConnection->GetSocket(), SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
				closesocket(lpConnection->GetSocket());
				lpConnection->SetSocket(INVALID_SOCKET);
			}
			*/

			if (eOperationType::OP_ACCEPT == lpOverlappedEx->s_eOperation && !bSuccess)
				InterlockedExchange((LPLONG)&lpConnection->m_bIsClosed, FALSE);

			// Overlapped I/O요청 되어있던 작업의 카운트를 줄인다
			if (lpOverlappedEx->s_eOperation == eOperationType::OP_ACCEPT)
				lpConnection->DecrementAcceptIoRefCount();
			else if (lpOverlappedEx->s_eOperation == eOperationType::OP_RECV)
				lpConnection->DecrementRecvIoRefCount();
			else if (lpOverlappedEx->s_eOperation == eOperationType::OP_SEND)
				lpConnection->DecrementSendIoRefCount();

			CloseConnection(lpConnection);
			continue;
		}

		switch (lpOverlappedEx->s_eOperation)
		{
		case eOperationType::OP_ACCEPT:
		{
			DoAccept(lpOverlappedEx);
			break;
		}
		case eOperationType::OP_RECV:
		{
			DoRecv(lpOverlappedEx, dwIoSize);
			break;
		}
		case eOperationType::OP_SEND:
		{
			DoSend(lpOverlappedEx, dwIoSize);
			break;
		}
		default:
			LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::WorkerThread() | switch default");
		}
	}
}

void CIocpServer::DoAccept(LPOVERLAPPED_EX lpOverlappedEx)
{
	SOCKADDR *lpLocalSockAddr = NULL, *lpRemoteSockAddr = NULL;
	int nLocalSockaddrLen = 0, nRemoteSockaddrLen = 0;

	CConnection* lpConnection = (CConnection*)lpOverlappedEx->s_lpConnection; //s_lpConnection == Player[i]
	if (lpConnection == NULL)
		return;

	lpConnection->m_ringRecvBuffer.Initialize(); // fixed
	lpConnection->m_ringSendBuffer.Initialize(); // fixed

	lpConnection->DecrementAcceptIoRefCount();

	// remote address
	GetAcceptExSockaddrs(lpConnection->m_szAddressBuf, 0, sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16, &lpLocalSockAddr, &nLocalSockaddrLen,
		&lpRemoteSockAddr, &nRemoteSockaddrLen);

	if (0 != nRemoteSockaddrLen)
	{
		lpConnection->SetConnectionIp(inet_ntoa(((SOCKADDR_IN*)lpRemoteSockAddr)->sin_addr));  // getsockname(SOCKET s, struct sockaddr* name,int* namelen)
		//printf("CIocpServer::DoAccept // TCP로받은 아이피주소: %s\n", inet_ntoa(((SOCKADDR_IN*)lpRemoteSockAddr)->sin_addr));
		lpConnection->SetConnectionPort(((SOCKADDR_IN*)lpRemoteSockAddr)->sin_port);
	}
	else
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::DoAccept() | GetAcceptExSockaddrs() Failed : (%u)", GetLastError());
		CloseConnection(lpConnection);
		return;
	}

	// bind Completion key & connection context
	if (lpConnection->BindIOCP(m_hWorkerIOCP) == false)
	{
		CloseConnection(lpConnection);
		return;
	}

	InterlockedExchange((LPLONG)&lpConnection->m_bIsClosed, FALSE);
	InterlockedExchange((LPLONG)&lpConnection->m_bIsConnect, TRUE);

	if (lpConnection->RecvPost(lpConnection->m_ringRecvBuffer.GetBeginMark(), 0) == false)
	{
		CloseConnection(lpConnection);
		return;
	}

	OnAccept(lpConnection);
}

void CIocpServer::DoRecv(LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize)
{
	CConnection* lpConnection = (CConnection*)lpOverlappedEx->s_lpConnection;
	if (lpConnection == nullptr)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::DoRecv() | lpConnection == NULL");
		return;
	}

	// fixed
	if (dwIoSize > 0)
		lpConnection->m_ringRecvBuffer.SetUsedRecvBufferSize(dwIoSize);

	lpConnection->DecrementRecvIoRefCount();

	int nMsgSize = 0;
	int nRemain = 0;
	char *pCurrent = nullptr;
	char *pNext = nullptr;

	// 정상적인 경우라면 8바이트의 한 패킷을 받았다고 가정
	// 비정상적인 경우라면 8바이트의 한 패킷 + 여러패킷      or       덜 보내진 4바이트 패킷
	nRemain = lpOverlappedEx->s_dwRemain; // s_dwRemain가 처음에는 0이다, 다 못 받아서 다시 받을 때는 값이 있다
	lpOverlappedEx->s_WsaBuf.buf = lpOverlappedEx->s_lpSocketMsg;
	lpOverlappedEx->s_dwRemain += dwIoSize;  // dwIoSize == 8 or 8+ or 4
	if (lpOverlappedEx->s_dwRemain >= PACKET_SIZE_LENGTH)
		CopyMemory(&nMsgSize, &(lpOverlappedEx->s_WsaBuf.buf[0]), PACKET_SIZE_LENGTH);
	else
		nMsgSize = 0;

	// 잘못된 패킷이면
	if (nMsgSize <= 0 || nMsgSize > lpConnection->m_ringRecvBuffer.GetBufferSize())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::DoRecv() | nMsgSize %d 위 arrived wrong packet : (%u)", nMsgSize, GetLastError());

		// TODO: 서버간 다운방지
		if (false == lpConnection->m_bIsCilent)
		{
			lpConnection->RecvPost(lpConnection->m_ringRecvBuffer.GetBeginMark(), 0);
			return;
		}

		CloseConnection(lpConnection);
		return;
	}
	lpOverlappedEx->s_nTotalBytes = nMsgSize;

	// 패킷을 다 못받았다면
	if ((lpOverlappedEx->s_dwRemain < ((DWORD)nMsgSize)))
	{
		nRemain = lpOverlappedEx->s_dwRemain;
		pNext = lpOverlappedEx->s_WsaBuf.buf;
	}
	else	// 하나 이상의 패킷 데이터를 모두 받았다면
	{
		pCurrent = &(lpOverlappedEx->s_WsaBuf.buf[0]);
		int dwCurrentSize = nMsgSize;  // nMsgSize == 패킷헤더에 명시된 길이값
		nRemain = lpOverlappedEx->s_dwRemain;  // s_dwRemain == 8 or 8+
		if (ProcessPacket(eOperationType::OP_RELEASE_TCP_RECVBUF, lpConnection, pCurrent, dwCurrentSize) == false)
		{
			CloseConnection(lpConnection);
			return;
		}
		nRemain -= dwCurrentSize;  // 정상적이라면 nRemain == 0
		pNext = pCurrent + dwCurrentSize;

		// 패킷이 여러개가 뭉쳐서 온 경우라면 반복문 속 if에 들어간다
		while (true)
		{
			if (nRemain >= PACKET_SIZE_LENGTH)
			{
				CopyMemory(&nMsgSize, pNext, PACKET_SIZE_LENGTH);
				dwCurrentSize = nMsgSize;

				// 잘못된 패킷이면
				if (nMsgSize <= 0 || nMsgSize > lpConnection->m_ringRecvBuffer.GetBufferSize())
				{
					LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::DoRecv() | nMsgSize %d 아래 arrived wrong packet : (%u)", nMsgSize, GetLastError());

					// TODO: 서버간 다운방지
					if (false == lpConnection->m_bIsCilent)
					{
						lpConnection->RecvPost(lpConnection->m_ringRecvBuffer.GetBeginMark(), 0);
						return;
					}

					CloseConnection(lpConnection);
					return;
				}

				lpOverlappedEx->s_nTotalBytes = dwCurrentSize;
				if (nRemain >= dwCurrentSize)  // 패킷이 더 뭉쳐서 왔다면 마지막 패킷까지 계속 반복
				{
					if (ProcessPacket(eOperationType::OP_RELEASE_TCP_RECVBUF, lpConnection, pNext, dwCurrentSize) == false)
					{
						// 패킷 무시당하고 처리는 못하는 상황. 종료하는게 낫다
						CloseConnection(lpConnection);
						return;
					}
					nRemain -= dwCurrentSize;
					pNext += dwCurrentSize;
				}
				else
					break;
			}
			else
				break;
		}
	}

	lpConnection->RecvPost(pNext, nRemain);
}

void CIocpServer::DoSend(LPOVERLAPPED_EX lpOverlappedEx, DWORD dwIoSize)
{
	CConnection* lpConnection = (CConnection*)lpOverlappedEx->s_lpConnection;
	if (lpConnection == nullptr)
		return;
	lpConnection->DecrementSendIoRefCount();
	lpOverlappedEx->s_dwRemain += dwIoSize;

	// 만약 모든 메시지를 다 보내지 못했다면 남은거 다시 보낸다
	if ((DWORD)lpOverlappedEx->s_nTotalBytes > lpOverlappedEx->s_dwRemain)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::DoSend() | WSASend 재실행 / 보낼때 사이즈: %d  /  완료받은 사이즈: %u", lpOverlappedEx->s_nTotalBytes, lpOverlappedEx->s_dwRemain);
		DWORD dwFlag = 0;
		DWORD dwSendNumBytes = 0;
		lpOverlappedEx->s_WsaBuf.buf += dwIoSize;
		lpOverlappedEx->s_WsaBuf.len -= dwIoSize;
		memset(&lpOverlappedEx->s_Overlapped, 0, sizeof(OVERLAPPED));
		lpConnection->IncrementSendIoRefCount();

		int nRet = WSASend(lpConnection->GetSocket(),
			&(lpOverlappedEx->s_WsaBuf),
			1,
			&dwSendNumBytes,
			dwFlag,
			&(lpOverlappedEx->s_Overlapped),
			NULL);
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			lpConnection->DecrementSendIoRefCount();
			LOG(LOG_ERROR_NORMAL, "SYSTEM | CIocpServer::DoSend() | WSASend Failed : (%u)", GetLastError());
			CloseConnection(lpConnection);
			return;
		}
	}
	else // 한 메시지를 다 보냈다면 (일반적인 상황)
	{
		lpConnection->m_ringSendBuffer.ReleaseBuffer(lpOverlappedEx->s_nTotalBytes);
		InterlockedExchange((LPLONG)&lpConnection->m_bIsSend, TRUE);
		lpConnection->SendPost(0);   // 보낼메시지가 또 있는지 본다, m_nUsedBufferSize의 값이 남아있는지
	}
}

LPPROCESSPACKET CIocpServer::GetProcessPacket(eOperationType operationType, LPARAM lParam, WPARAM wParam)
{
	LPPROCESSPACKET lpProcessPacket = m_stackPacket.Pop();
	if (nullptr == lpProcessPacket)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::GetProcessPacket() | lpProcessPacket == NULL / Stack is Empty");
		OnCrush();
		return nullptr;
	}

	lpProcessPacket->s_eOperationType = operationType;
	lpProcessPacket->s_lParam = lParam;
	lpProcessPacket->s_wParam = wParam;
	return lpProcessPacket;
}

void CIocpServer::ClearProcessPacket(LPPROCESSPACKET lpProcessPacket)
{
	lpProcessPacket->Init();
	m_stackPacket.Push(lpProcessPacket);
}

bool CIocpServer::ProcessPacket(eOperationType releaseOperationType, CConnection* lpConnection, char* pCurrent, DWORD dwCurrentSize)
{
	//int nUseBufSize = lpConnection->m_ringRecvBuffer.GetUsedBufferSize();

	if (!OnRecvImmediately(lpConnection, dwCurrentSize, pCurrent))
	{
		LPPROCESSPACKET lpProcessPacket = GetProcessPacket(eOperationType::OP_RECVPACKET, (LPARAM)pCurrent, (WPARAM)releaseOperationType);
		if (nullptr == lpProcessPacket)
			return false;
		lpProcessPacket->s_lpConnection = lpConnection;
		lpProcessPacket->s_dwCurrentSize = dwCurrentSize;

		m_queueProcess.Push(lpProcessPacket);
		if (false == m_isProcessThreadUP)
			SetEvent(m_hProcessThreadEvent);
	}
	else if (eOperationType::OP_RELEASE_TCP_RECVBUF == releaseOperationType)
		lpConnection->m_ringRecvBuffer.ReleaseRecvBuffer(dwCurrentSize);

	return true;
}

bool CIocpServer::CloseConnection(CConnection* lpConnection)
{
	///*
	{
		CMonitor::Owner lock(m_csCloseSocket);
		if (INVALID_SOCKET != lpConnection->GetSocket())
		{
			struct linger lin = { 1, 0 };
			setsockopt(lpConnection->GetSocket(), SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
			closesocket(lpConnection->GetSocket());
			lpConnection->SetSocket(INVALID_SOCKET);
		}
	}
	//*/

	// 레퍼런스 카운트가 남아있다면 소켓을 끊고 iocp에서 completion될때가지 기다려야한다
	// 카운트가 다 떨어지고나서 accept와 초기화를 시작해야 한다
	if (lpConnection->GetAcceptIoRefCount() > 0 ||
		lpConnection->GetRecvIoRefCount() > 0 ||
		lpConnection->GetSendIoRefCount() > 0)
	{
		// 무한루프가 발생한다면 여기서 무한루프 방지 카운팅을 한다, n번이상이면 컨텐츠도 종료처리 해준다

		//CMonitor::Owner lock(m_csCloseSocket);

		/*
		if (INVALID_SOCKET != lpConnection->GetSocket())
		{
			struct linger lin = { 1, 0 };
			setsockopt(lpConnection->GetSocket(), SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
			closesocket(lpConnection->GetSocket());
			lpConnection->SetSocket(INVALID_SOCKET);
		}
		*/
		return true;
	}

	if (InterlockedCompareExchange((LPLONG)&lpConnection->m_bIsClosed, TRUE, FALSE) == FALSE)
	{
		if(m_isGameServer)
			OnPrepareClose(lpConnection);
		else
		{
			LPPROCESSPACKET lpProcessPacket = GetProcessPacket(eOperationType::OP_CLOSE, NULL, NULL);
			if (NULL == lpProcessPacket)
				return false;
			lpProcessPacket->s_lpConnection = lpConnection;
			lpProcessPacket->s_dwCurrentSize = 0;

			m_queueProcess.Push(lpProcessPacket);
			if (false == m_isProcessThreadUP)
				SetEvent(m_hProcessThreadEvent);
		}
	}

	return true;
}

void CIocpServer::ProcessThread()
{
	CConnection*			lpConnection = nullptr;
	DWORD					dwIoSize = 0;
	LPPROCESSPACKET			pProcessPacketEX = nullptr;
	size_t spinCount = 0;

	while (m_bProcessThreadFlag)
	{
		/* 2019 0525버전부터 수정됨
		// FIXED: IOCP QUEUE
		bSuccess = GetQueuedCompletionStatus(
		m_hProcessIOCP,
		&dwIoSize,
		(PULONG_PTR)&lpConnection,
		(LPOVERLAPPED*)&lpProcessPacket,
		INFINITE);
		*/

		if (false == m_bProcessThreadFlag)
			break;

		if (0 == m_queueProcess.GetSize())// FIXED: ProcessThread
		{// FIXED: ProcessThread
			m_isProcessThreadUP = false;
			DWORD ret = WaitForSingleObject(m_hProcessThreadEvent, 1);
			m_isProcessThreadUP = true;
			if (WAIT_FAILED == ret)
			{
				LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | WAIT_FAILED == ret");
				break;
			}
		}// FIXED: ProcessThread

		while (0 < m_queueProcess.GetSize())
		{
			pProcessPacketEX = nullptr;
			pProcessPacketEX = m_queueProcess.Pop();
			if (nullptr == pProcessPacketEX)
			{
				/*
				if (FALSE == SwitchToThread())
				{
					LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | FALSE == SwitchToThread()");
					Sleep(0);
				}
				*/

				if (10 < ++spinCount)
				{
					if (FALSE == SwitchToThread())
					{
						LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | FALSE == SwitchToThread()");
						Sleep(0);
					}
				}
				else
				{
					YieldProcessor(); // TEST
					LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | YieldProcessor");
				}

				continue;// FIXED: ProcessThread
			}

			spinCount = 0;
			lpConnection = nullptr;
			lpConnection = (CConnection*)pProcessPacketEX->s_lpConnection;
			dwIoSize = pProcessPacketEX->s_dwCurrentSize;

			switch (pProcessPacketEX->s_eOperationType)
			{
			case eOperationType::OP_CLOSE:
			{
				//OnPrepareClose(lpConnection);
				IocpServer()->OnClose(lpConnection); // TODO
				lpConnection->CloseConnection_CConn(true); // 매개변수 == true: linger 강제종료
				break;
			}
			case eOperationType::OP_RECVPACKET:
			{
				if (NULL == pProcessPacketEX->s_lParam)
				{
					LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | NULL == lpProcessPacket->s_lParam");
					break;
				}

				if (false == OnRecv(lpConnection, dwIoSize, (char*)pProcessPacketEX->s_lParam)) // lpProcessPacket->s_lParam은 recv링버퍼의 해당 패킷 첫위치
				{
					LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | false == OnRecv()");
					if (eOperationType::OP_RELEASE_TCP_RECVBUF == static_cast<eOperationType>(pProcessPacketEX->s_wParam))
						lpConnection->m_ringRecvBuffer.ReleaseRecvBuffer(dwIoSize);
					break;
				}

				if (eOperationType::OP_RELEASE_TCP_RECVBUF == static_cast<eOperationType>(pProcessPacketEX->s_wParam))
					lpConnection->m_ringRecvBuffer.ReleaseRecvBuffer(dwIoSize);
				break;
			}
			case eOperationType::OP_SYSTEM:
			{
				OnSystemMsg(lpConnection, pProcessPacketEX->s_lParam, pProcessPacketEX->s_wParam);
				break;
			}
			default:
			{
				LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::ProcessThread() | 패킷무시됨 / 번호: %d  %d  %d ", pProcessPacketEX->s_eOperationType, pProcessPacketEX->s_wParam, lpConnection);
				break;
			}
			}
			ClearProcessPacket(pProcessPacketEX);
		}
	}
}

char* CIocpServer::UDP_unsafe_PrepareSendPacket(int slen)
{
	ZeroMemory(m_arrUDPSendBuffer, slen);
	CopyMemory(m_arrUDPSendBuffer, &slen, PACKET_SIZE_LENGTH);
	return m_arrUDPSendBuffer;
}

bool CIocpServer::UDP_unsafe_SendPost(int nSendSize, CConnection* lpConnection)
{
	SOCKADDR_IN udpAddr; ZeroMemory(&udpAddr, sizeof(SOCKADDR_IN));
	udpAddr.sin_family = AF_INET;
	udpAddr.sin_addr.s_addr = inet_addr((const char*)lpConnection->GetConnectionIp());
	udpAddr.sin_port = htons(lpConnection->UDP_GetConnectionPort());
	int nResult = sendto(m_UdpBlockSock, m_arrUDPSendBuffer, nSendSize, 0, (SOCKADDR*)&udpAddr, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == nResult)
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpServer::UDP_SendPost() | sendto failed with error: %d", WSAGetLastError());

	return true;
}

void CIocpServer::UDP_RecvFromThread()
{
	SOCKADDR_IN clntAdr; memset(&clntAdr, 0, sizeof(clntAdr));
	int clntAdrSz = sizeof(clntAdr);

	// HACK: 접근할 수 없는 코드라 서버종료시 flag 필요
	while (UDPThreadFlag)
	{
		memset(&clntAdr, 0, sizeof(clntAdr));
		int strLen = recvfrom(m_UdpBlockSock, m_arrUDPRecvBuffer, 1024, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
		if (false == UDPThreadFlag) break;
		if (SOCKET_ERROR == strLen)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::UDP_RecvFromThread() | SOCKET_ERROR / WSAGetLastError: %d", WSAGetLastError());
			continue;
		}
		if (0 == strLen)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::UDP_RecvFromThread() | 0 == strLen");
			continue;
		}

		/*
		// 테스트라인--------------------------------------------------------------------------------------
		//printf("유디피길이: %d\n", strLen);
		char* szAddr = inet_ntoa(clntAdr.sin_addr);
		string strUdpAddr = szAddr; // real
		cout << "CIocpServer::UDP_RecvFromThread() : UDP로 받은주소 cout: " << strUdpAddr << endl;
		USHORT usUdpPort = ntohs(clntAdr.sin_port); // not real
		printf("CIocpServer::UDP_RecvFromThread() : UDP로 받은포트 cout: %d\n", usUdpPort);
		// 테스트라인--------------------------------------------------------------------------------------
		*/


		/*
		// 모바일 환경(아이피계속 바뀌는 경우)일경우 아예 패킷에 PKey를 담아서, 플레이어맵에서 찾도록 한다
		// 더불어 피시에서도 잘 안될시 이방법으로
		UDP_IT udpCon_it;
		{
		CMonitor::Owner lock(m_csUdpMap);
		udpCon_it = m_unMapUdpCon.find(strUdpAddr);
		}

		if (udpCon_it == m_unMapUdpCon.end())
		{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::UDP_RecvFromThread() | udpCon_it is NULL");
		continue;
		}
		CConnection* lpConnection = (CConnection*)udpCon_it->second;
		if (NULL == lpConnection)
		{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::UDP_RecvFromThread() | lpConnection is NULL");
		continue;
		}
		*/

		// 테스트
		//lpConnection->UDP_SetConnectionIp(strUdpAddr.c_str);
		//lpConnection->UDP_SetConnectionIp(szAddr);
		//lpConnection->UDP_SetConnectionPort(usUdpPort);
		// 테스트

		//OnRecv(NULL, strLen, m_arrUdpBuffer);
		UDP_OnRecv(strLen, m_arrUDPRecvBuffer);

		// 프로세스 쓰레드한테 부탁형식으로 하려면 링버퍼랑 밑의 코드를 사용함
		//if (false == ProcessPacket(OP_SEND, lpConnection, m_arrUdpBuffer, strLen))   // OP_SEND을 준이유: udp버퍼에 링버퍼 필요없다, 릴리즈할필요도 없고
		//LOG(LOG_ERROR_HIGH, "SYSTEM | CIocpServer::UdpWorkerThread() | ProcessPacket() 실패");
	}

	closesocket(m_UdpBlockSock);
	m_UdpBlockSock = INVALID_SOCKET;
}