#pragma once
#include "RingBuffer.h"

class NETLIB_API CConnection
{
public:
	CConnection();
	virtual ~CConnection();
	CConnection(const CConnection& rhs) = delete;
	CConnection& operator=(const CConnection& rhs) = delete;
public:
	void 	InitializeConnection();

	bool 	CreateConnection(INITCONFIG &initConfig);
	bool 	CloseConnection_CConn(bool bForce = false);
	bool	ConnectTo(char* szIp, unsigned short usPort);
	bool 	BindIOCP(HANDLE& hIOCP);

	bool 	RecvPost(char* pNext, DWORD dwRemain);
	bool 	SendPost(int nSendSize);

	void 	SetSocket(SOCKET socket) { m_socket = socket; }
	SOCKET 	GetSocket() { return m_socket; }

	bool 	BindAcceptExSock();

	char* 	PrepareSendPacket(int slen);

	bool 	ReleaseRecvPacket();
	bool 	ReleaseSendPacket(LPOVERLAPPED_EX lpSendOverlappedEx = NULL);

	inline void  SetConnectionIp(char* ip) { memcpy(m_szIp, ip, MAX_IP_LENGTH); }
	inline void  SetConnectionPrivateIp(char* ip) { memcpy(m_szPrivateIp, ip, MAX_IP_LENGTH); }

	inline void    SetConnectionPort(USHORT port) { m_usPort = port; }
	inline USHORT  GetConnectionPort() { return m_usPort; }
	inline void    UDP_SetConnectionPort(USHORT port) { m_usUdpPort = port; }
	inline USHORT  UDP_GetConnectionPort() { return m_usUdpPort; }

	inline BYTE*  GetConnectionPrivateIp() { return m_szPrivateIp; }
	inline BYTE*  GetConnectionIp() { return m_szIp; }

	inline int  GetIndex() { return m_nIndex; }
	inline int  GetRecvBufSize() { return m_nRecvBufSize; }
	inline int  GetSendBufSize() { return m_nSendBufSize; }
	inline int  GetRecvIoRefCount() { return m_dwRecvIoRefCount; }
	inline int  GetSendIoRefCount() { return m_dwSendIoRefCount; }
	inline int  GetAcceptIoRefCount() { return m_dwAcceptIoRefCount; }

	inline void  IncrementRecvIoRefCount()
	{
		InterlockedIncrement((LPLONG)&m_dwRecvIoRefCount);
	}
	inline void  IncrementSendIoRefCount()
	{
		InterlockedIncrement((LPLONG)&m_dwSendIoRefCount);
	}
	inline void  IncrementAcceptIoRefCount()
	{
		InterlockedIncrement((LPLONG)&m_dwAcceptIoRefCount);
	}

	inline void  DecrementRecvIoRefCount()
	{
		InterlockedDecrement((LPLONG)&m_dwRecvIoRefCount);
	}
	inline void  DecrementSendIoRefCount()
	{
		InterlockedDecrement((LPLONG)&m_dwSendIoRefCount);
	}
	inline void  DecrementAcceptIoRefCount()
	{
		InterlockedDecrement((LPLONG)&m_dwAcceptIoRefCount);
	}

	// TEST
	inline DWORD GetKeepAliveTick() { return m_dwKeepAliveTick; }
	inline void SetKeepAliveTick(DWORD dwKeepAliveTick) { m_dwKeepAliveTick = dwKeepAliveTick; }

public:
	// Overlapped I/O 요청을 위한 변수
	LPOVERLAPPED_EX		m_lpRecvOverlappedEx;
	LPOVERLAPPED_EX		m_lpSendOverlappedEx;
	// 클라이언트와 데이터 송수신을 위한 링 버퍼
	CRingBuffer		m_ringRecvBuffer;
	CRingBuffer		m_ringSendBuffer;
	// 클라이언트 주소를 받기위한 버퍼
	char			m_szAddressBuf[1024];
	// 클라이언트와 연결 종료가 되었는지 여부
	BOOL			m_bIsClosed;
	// 클라이언트와 연결이 되어있는지 여부
	BOOL			m_bIsConnect;
	// 현재 Overlapped I/O 전송 작업을 하고 있는지 여부
	BOOL			m_bIsSend;
	// TODO
	bool			m_bIsCilent;
private:
	SOCKET		m_sockListener;
	// 서버측의 클라이언트소켓
	SOCKET		m_socket;
	// 한번에 수신할 수 있는 데이터의 최대 크기
	int			m_nRecvBufSize;
	// 한번에 송신할 수 있는 데이터의 최대 크기
	int			m_nSendBufSize;
	// 세션의 ip
	BYTE		m_szIp[MAX_IP_LENGTH];
	BYTE		m_szPrivateIp[MAX_IP_LENGTH];
	USHORT		m_usPort;
	USHORT		m_usUdpPort;
	// 세션배열의 인덱스
	int			m_nIndex;

	CMonitor	m_csConnection;
	HANDLE		m_hIOCP;
	// Overlapped i/o작업을 요청한 개수
	int		m_dwSendIoRefCount;
	int		m_dwRecvIoRefCount;
	int		m_dwAcceptIoRefCount;

	// TEST
	DWORD   m_dwKeepAliveTick; // KeepAlive_Cn패킷을 받은 시점의 서버 틱
};