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
	// Overlapped I/O ��û�� ���� ����
	LPOVERLAPPED_EX		m_lpRecvOverlappedEx;
	LPOVERLAPPED_EX		m_lpSendOverlappedEx;
	// Ŭ���̾�Ʈ�� ������ �ۼ����� ���� �� ����
	CRingBuffer		m_ringRecvBuffer;
	CRingBuffer		m_ringSendBuffer;
	// Ŭ���̾�Ʈ �ּҸ� �ޱ����� ����
	char			m_szAddressBuf[1024];
	// Ŭ���̾�Ʈ�� ���� ���ᰡ �Ǿ����� ����
	BOOL			m_bIsClosed;
	// Ŭ���̾�Ʈ�� ������ �Ǿ��ִ��� ����
	BOOL			m_bIsConnect;
	// ���� Overlapped I/O ���� �۾��� �ϰ� �ִ��� ����
	BOOL			m_bIsSend;
	// TODO
	bool			m_bIsCilent;
private:
	SOCKET		m_sockListener;
	// �������� Ŭ���̾�Ʈ����
	SOCKET		m_socket;
	// �ѹ��� ������ �� �ִ� �������� �ִ� ũ��
	int			m_nRecvBufSize;
	// �ѹ��� �۽��� �� �ִ� �������� �ִ� ũ��
	int			m_nSendBufSize;
	// ������ ip
	BYTE		m_szIp[MAX_IP_LENGTH];
	BYTE		m_szPrivateIp[MAX_IP_LENGTH];
	USHORT		m_usPort;
	USHORT		m_usUdpPort;
	// ���ǹ迭�� �ε���
	int			m_nIndex;

	CMonitor	m_csConnection;
	HANDLE		m_hIOCP;
	// Overlapped i/o�۾��� ��û�� ����
	int		m_dwSendIoRefCount;
	int		m_dwRecvIoRefCount;
	int		m_dwAcceptIoRefCount;

	// TEST
	DWORD   m_dwKeepAliveTick; // KeepAlive_Cn��Ŷ�� ���� ������ ���� ƽ
};