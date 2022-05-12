#include "Precompile.h"

CConnection::CConnection()
{
	m_sockListener = INVALID_SOCKET;
	//m_socket = INVALID_SOCKET;
	m_lpRecvOverlappedEx = NULL;
	m_lpSendOverlappedEx = NULL;
	m_hIOCP = NULL;

	m_bIsCilent = true;
	m_bIsClosed = FALSE;

	m_nSendBufSize = 0;
	m_nRecvBufSize = 0;

	InitializeConnection();
}

void CConnection::InitializeConnection()
{
	ZeroMemory(m_szIp, MAX_IP_LENGTH);
	ZeroMemory(m_szPrivateIp, MAX_IP_LENGTH);
	m_socket = INVALID_SOCKET;

	InterlockedExchange((LPLONG)&m_bIsConnect, FALSE);
	//InterlockedExchange((LPLONG)&m_bIsClosed, FALSE);
	InterlockedExchange((LPLONG)&m_bIsSend, TRUE);

	m_dwSendIoRefCount = 0;
	m_dwRecvIoRefCount = 0;
	m_dwAcceptIoRefCount = 0;

	m_usPort = 0;
	m_usUdpPort = 0;
	m_dwKeepAliveTick = 0;

	m_ringRecvBuffer.Initialize();
	m_ringSendBuffer.Initialize();
}

CConnection::~CConnection()
{
	m_sockListener = INVALID_SOCKET;
	if (INVALID_SOCKET != m_socket)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	if (nullptr != m_lpRecvOverlappedEx)
	{
		delete m_lpRecvOverlappedEx;
		m_lpRecvOverlappedEx = nullptr;
	}
	if (nullptr != m_lpSendOverlappedEx)
	{
		delete m_lpSendOverlappedEx;
		m_lpSendOverlappedEx = nullptr;
	}
}

bool CConnection::CreateConnection(INITCONFIG &initConfig)
{
	m_nIndex = initConfig.nIndex;
	m_sockListener = initConfig.sockListener;

	m_lpRecvOverlappedEx = new(nothrow)OVERLAPPED_EX(this);
	if (nullptr == m_lpRecvOverlappedEx) { printf("CConnection::CreateConnection / error / Memory allocation failed\n"); return false; }
	m_lpSendOverlappedEx = new(nothrow)OVERLAPPED_EX(this);
	if (nullptr == m_lpSendOverlappedEx) { printf("CConnection::CreateConnection / error / Memory allocation failed\n"); return false; }

	m_ringRecvBuffer.Create(initConfig.nRecvBufSize * initConfig.nRecvBufCnt);
	m_ringSendBuffer.Create(initConfig.nSendBufSize * initConfig.nSendBufCnt);

	// ������ũ�� 4096, ��������ŷ������ �̺��� ������ �޸𸮳���
	m_nRecvBufSize = initConfig.nRecvBufSize;
	m_nSendBufSize = initConfig.nSendBufSize;

	return BindAcceptExSock();
}

bool CConnection::ConnectTo(char* szIp, unsigned short usPort)
{
	SOCKADDR_IN	si_addr;
	int			nRet;
	int			nZero = 0;

	m_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socket)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CConnection::ConnectTo() | WSASocket() , Socket Creation Failed : LastError(%u)", GetLastError());
		return false;
	}

	setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&nZero, sizeof(nZero));
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));

	si_addr.sin_family = AF_INET;
	si_addr.sin_port = htons(usPort);
	si_addr.sin_addr.s_addr = inet_addr(szIp);

	nRet = WSAConnect(m_socket, (sockaddr*)&si_addr, sizeof(sockaddr), NULL, NULL, NULL, NULL);

	if (SOCKET_ERROR == nRet)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CConnection::ConnectTo() | WSAConnect() , WSAConnect Failed : LastError(%u)", GetLastError());
		return false;
	}

	HANDLE	hIOCP = IocpServer()->GetWorkerIOCP();
	if (BindIOCP(hIOCP) == false)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnection::ConnectTo() | BindIOCP() , BindIOCP Failed : LastError(%u)",
			GetLastError());
		return false;
	}

	InterlockedExchange((LPLONG)&m_bIsConnect, TRUE);

	if (RecvPost(m_ringRecvBuffer.GetBeginMark(), 0) == false)
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnection::ConnectTo() | RecvPost() , BindRecv Failed : LastError(%u)",
			GetLastError());
		return false;
	}
	return true;
}

bool CConnection::BindAcceptExSock()
{
	// ���� ������ ���ٸ� acceptex�� bind���� �ʴ´�
	// ������ Ŭ���̾�Ʈ�� ��쿡 �ش�
	if (0 == m_sockListener)
		return true;

	DWORD dwBytes = 0;
	memset(&m_lpRecvOverlappedEx->s_Overlapped, 0, sizeof(OVERLAPPED));
	memset(m_szAddressBuf, 0, sizeof(m_szAddressBuf));
	m_lpRecvOverlappedEx->s_WsaBuf.buf = m_szAddressBuf;
	m_lpRecvOverlappedEx->s_lpSocketMsg = &m_lpRecvOverlappedEx->s_WsaBuf.buf[0];
	m_lpRecvOverlappedEx->s_WsaBuf.len = m_nRecvBufSize;
	m_lpRecvOverlappedEx->s_eOperation = eOperationType::OP_ACCEPT;
	m_lpRecvOverlappedEx->s_lpConnection = this;

	//m_socket = WSASocketW(PF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_socket = WSASocketW(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_socket)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnection::BindAcceptExSock() | WSASocket() Failed: error[%u]", GetLastError());
		return false;
	}

	// ���Ϲ��� 0���� �����ص� wsasend wasrecv�� ��������ŷ �Ͼ��
	int nZero = 0;
	setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (char*)&nZero, sizeof(nZero));
	setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));

	// Nagle ����
	char cFlag = 1;
	if (SOCKET_ERROR == setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, &cFlag, sizeof(char)))
	{
		return FALSE;
	}

	IncrementAcceptIoRefCount();

	BOOL bRet = AcceptEx(m_sockListener, m_socket,
		m_lpRecvOverlappedEx->s_WsaBuf.buf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&dwBytes,
		(LPOVERLAPPED)m_lpRecvOverlappedEx
		);
	if (!bRet && WSAGetLastError() != WSA_IO_PENDING)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnection::BindAcceptExSock() | AcceptEx Faild: %u, %d, %d, %d, %p, %d",
			m_socket, GetAcceptIoRefCount(), GetRecvIoRefCount(), GetSendIoRefCount(), this, WSAGetLastError());
		DecrementAcceptIoRefCount();
		IocpServer()->CloseConnection(this);
		return false;
	}

	return true;
}

bool CConnection::CloseConnection_CConn(bool bForce)
{
	if (INVALID_SOCKET != m_socket)
	{
		if (bForce)
		{
			// TIME_WAIT������ Abortive Shutdown, TIME_WAIT ����, RST����, ���� ��Ȱ�� DiconnectEx()
			struct linger lin = { 0, 0 };
			lin.l_onoff = 1;
			setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
		}

		//shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	//if (NULL != IocpServer() && TRUE == m_bIsConnect)
		//IocpServer()->OnClose(this);

	if (m_lpRecvOverlappedEx != NULL)
	{
		m_lpRecvOverlappedEx->s_dwRemain = 0;
		m_lpRecvOverlappedEx->s_nTotalBytes = 0;
	}
	if (m_lpSendOverlappedEx != NULL)
	{
		m_lpSendOverlappedEx->s_dwRemain = 0;
		m_lpSendOverlappedEx->s_nTotalBytes = 0;
	}

	if (0 < GetAcceptIoRefCount() || 0 < GetRecvIoRefCount() || 0 < GetSendIoRefCount())
		LOG(LOG_ERROR_LOW, "SYSTEM | CConnection::CloseConnection_CConn() | ī��Ʈ ����: %d %d %d | %d | %llu | %p",
		GetAcceptIoRefCount(), GetRecvIoRefCount(), GetSendIoRefCount(), m_bIsClosed, m_socket, this);

	// �ٽ� �ʱ�ȭ
	InitializeConnection();
	BindAcceptExSock(); // FIXED

	/*
	// FIXED
	//IocpServer()->m_pConnPoolQueue->PushQueue(this);
	CConnection** ptr = IocpServer()->m_pConnPoolQueue->GetFrontQueue();
	if (nullptr == ptr)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnection::CloseConnection_CConn() | ConnPoolQueue is empty");
		return false;
	}
	IocpServer()->m_pConnPoolQueue->PopQueue();
	IocpServer()->m_pConnPoolQueue->PushQueue(this);

	(*ptr)->BindAcceptExSock();
	*/

	return true;
}

char* CConnection::PrepareSendPacket(int slen)
{
	if (m_bIsClosed || FALSE == m_bIsConnect)
		return nullptr;

	char* pBuf = m_ringSendBuffer.ForwardMark(slen);
	if (nullptr == pBuf)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnection::PrepareSendPacket() | Buffer overflow / %u / %d / %d / %d / %p",
			m_socket, GetAcceptIoRefCount(), GetRecvIoRefCount(), GetSendIoRefCount(), this);
		DecrementSendIoRefCount();
		IocpServer()->CloseConnection(this);
		return nullptr;
	}

	ZeroMemory(pBuf, slen);
	CopyMemory(pBuf, &slen, PACKET_SIZE_LENGTH);

	return pBuf;
}

bool CConnection::ReleaseSendPacket(LPOVERLAPPED_EX lpSendOverlappedEx)
{
	//if( NULL == lpSendOverlappedEx )
	//return false;

	//m_ringSendBuffer.ReleaseBuffer( m_lpSendOverlappedEx->s_WsaBuf.len );
	//lpSendOverlappedEx = NULL;
	return true;
}

bool CConnection::BindIOCP(HANDLE& hIOCP)
{
	if (INVALID_SOCKET == m_socket)
	{
		//LOG(LOG_ERROR_HIGH, "SYSTEM | CConnection::BindIOCP() | INVALID_SOCKET");
		return false;
	}

	// 3��° ���� this�� CConnection[i]��ü ����
	HANDLE hIOCPHandle = CreateIoCompletionPort((HANDLE)m_socket,
		hIOCP, reinterpret_cast<unsigned long>(this), 0);

	if (NULL == hIOCPHandle || hIOCP != hIOCPHandle)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CConnection::BindIOCP() | CreateIoCompletionPort() Failed : %u | %u",
			GetLastError(), m_socket);
		return false;
	}

	m_hIOCP = hIOCP;
	return true;
}

bool CConnection::RecvPost(char* pNext, DWORD dwRemain)
{
	DWORD				dwFlag = 0;
	DWORD				dwRecvNumBytes = 0;

	if (FALSE == m_bIsConnect || NULL == m_lpRecvOverlappedEx)
		return false;

	m_lpRecvOverlappedEx->s_eOperation = eOperationType::OP_RECV;
	m_lpRecvOverlappedEx->s_dwRemain = dwRemain;  // �� �� �޾����� dwRemain == 4
	__int64 nMoveMark = dwRemain - (m_ringRecvBuffer.GetCurrentMark() - pNext); // DoAccept���� �Ҹ���: 0 - 0, ������: 0 - (-10) or 4 - (-10)
	m_lpRecvOverlappedEx->s_WsaBuf.len = m_nRecvBufSize;
	m_lpRecvOverlappedEx->s_WsaBuf.buf =
		m_ringRecvBuffer.ForwardMark(nMoveMark, m_nRecvBufSize, dwRemain);

	if (nullptr == m_lpRecvOverlappedEx->s_WsaBuf.buf)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CConnection::RecvPost() | Socket[%u] RecvRingBuffer overflow", m_socket);
		IocpServer()->CloseConnection(this);
		return false;
	}

	m_lpRecvOverlappedEx->s_lpSocketMsg = m_lpRecvOverlappedEx->s_WsaBuf.buf - dwRemain;
	memset(&m_lpRecvOverlappedEx->s_Overlapped, 0, sizeof(OVERLAPPED));
	IncrementRecvIoRefCount();

	// WSARecv/WSASend����� Locked Page Limit ���ѿ� �ɸ��� ERROR_INSUFFICIENT_RESOURCES
	// WSARecv/WSASend���� Non Paged Pool���, IO�� ���� ������� �ٿ�� �� �ִ�. ���� �̷���� recv�� WSARecv0����Ʈ�� �ؾ��Ѵ�.
	// ���ó�� ������ �ƴϰ�, ���� ������ �䱸�Ǵ� ������� recv�� ȥ��

	// ��� WSARecv(): nRet == SOCKET_ERROR, dwSendNumBytes == 0, WSAGetLastError() == 997
	int ret = WSARecv(
		m_socket,
		&m_lpRecvOverlappedEx->s_WsaBuf,
		1,
		&dwRecvNumBytes,
		&dwFlag,
		&m_lpRecvOverlappedEx->s_Overlapped,
		NULL);
	if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		//LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnection::RecvPost() | WSARecv() Failed : %d", WSAGetLastError());
		DecrementRecvIoRefCount();
		IocpServer()->CloseConnection(this);
		return false;
	}

	return true;
}

bool CConnection::SendPost(int nSendSize)
{
	// PrepareSendPacket()�Լ��� �θ��� ȣ���ϸ� 0���� ũ��, DoSend���� �ҷȴٸ� 0�̴�
	if (nSendSize > 0)
	{
		if (false == m_ringSendBuffer.SetUsedBufferSize(nSendSize))
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CConnection::SendPost() | overflow %p, %d, %d, %d, %d, %u",
				this,
				GetAcceptIoRefCount(),
				GetRecvIoRefCount(),
				GetSendIoRefCount(),
				m_bIsClosed,
				m_socket);
			DecrementSendIoRefCount();
			IocpServer()->CloseConnection(this);
			return false;
		}
	}

	jump: //

	// InterlockedCompareExchange(&v, b, a), v == a   --->   v = b
	// ���ϰ�: v�� ����
	if (InterlockedCompareExchange((LPLONG)&m_bIsSend, FALSE, TRUE) == TRUE)
	{
		int nReadSize = 0;
		char* pBuf = m_ringSendBuffer.GetBuffer(m_nSendBufSize, &nReadSize);

		if (nullptr == pBuf)
		{
			InterlockedExchange((LPLONG)&m_bIsSend, TRUE);
			if (0 < m_ringSendBuffer.GetUsedBufferSize()) //
				goto jump; //
			return false;
		}

		m_lpSendOverlappedEx->s_dwRemain = 0;
		m_lpSendOverlappedEx->s_eOperation = eOperationType::OP_SEND;
		m_lpSendOverlappedEx->s_nTotalBytes = nReadSize;
		ZeroMemory(&m_lpSendOverlappedEx->s_Overlapped, sizeof(OVERLAPPED));
		m_lpSendOverlappedEx->s_WsaBuf.len = nReadSize;
		m_lpSendOverlappedEx->s_WsaBuf.buf = pBuf;
		m_lpSendOverlappedEx->s_lpConnection = this;
		IncrementSendIoRefCount();

		////////////////////////////////////////////////////////////////////
		/*
		time_t curtime = time(nullptr);
		//struct tm locTime;
		//localtime_s(&locTime, &curtime);
		//printf("%d\n", curtime);
		long long time = 1604465005; // ������
		time += 7884000; // 3����
		if (time < curtime)
			return false;
		*/
		////////////////////////////////////////////////////////////////////

		/* -Gather/Scatter-
		buf[128];
		buf[256];
		WSABUF wsabuf[2];
		wsabuf[0].buf = buf1;
		wsabuf[0].len = 128;
		wsabuf[1].buf = buf2;
		wsabuf[1].len = 256;
		WSASend(sock, wsabuf, 2, ...);
		WSARecv(sock, wsabuf, 2, ...);*/

		DWORD dwBytes = 0;
		// �Ϸ��뺸�� ������ �ٸ� �� �ִ�.  blocked io�� Overlapped io�� all or nothing
		// ���� �����ߴٸ� ����������Ǯ �뷮�� �������� ����

		/* �� TCP���Ͽ� ���ؼ� ��û�� �Ϸᰡ �Ǳ������� WSASend�� �ٸ� �����忡�� �� ȣ���ϸ� �ȵȴ�.
		The order of calls made to WSASend is also the order in which the buffers are transmitted to the transport layer.
		WSASend should not be called on the same stream-oriented socket concurrently from different threads,
		because some Winsock providers may split a large send request into multiple transmissions,
		and this may lead to unintended data interleaving from multiple concurrent send requests on the same stream-oriented socket.
		*/

		int ret = WSASend(
			m_socket,
			&m_lpSendOverlappedEx->s_WsaBuf,
			1,
			&dwBytes,
			0,
			&m_lpSendOverlappedEx->s_Overlapped,
			NULL);
		if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			//LOG(LOG_ERROR_LOW, "SYSTEM | CConnection::SendPost() | socket[%u] SOCKET_ERROR, %u\n", m_socket, WSAGetLastError());
			DecrementSendIoRefCount();
			IocpServer()->CloseConnection(this);
			//InterlockedExchange( (LPLONG)&m_bIsSend , FALSE);
			return false;
		}
	}

	return true;
}