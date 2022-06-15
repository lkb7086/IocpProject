#pragma once
#include "Serializer.h"

class NETLIB_API CIocpServer
{
public:
	CIocpServer();
	virtual ~CIocpServer();
	CIocpServer(const CIocpServer& rhs) = delete;
	CIocpServer& operator=(const CIocpServer& rhs) = delete;

	virtual	void	OnInitIocpServer() = 0;
	virtual	void	OnClose_IocpServer() = 0;
	virtual	bool	OnAccept(CConnection *lpConnection) = 0;
	virtual	bool	OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) = 0;
	virtual	bool	OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) = 0;
	virtual	void	OnPrepareClose(CConnection* lpConnection) = 0;
	virtual	void	OnClose(CConnection* lpConnection) = 0;
	virtual bool	OnSystemMsg(CConnection* lpConnection, LPARAM dwMsgType, WPARAM lParam) = 0;
	virtual	void	UDP_OnRecv(DWORD dwSize, char* pRecvedMsg) = 0;

	bool	InitializeSocket();
	void	WorkerThread();
	void    UDP_RecvFromThread();
	void	ProcessThread();
	bool	CloseConnection(CConnection* lpConnection);
	bool	ProcessPacket(eOperationType operationType, CConnection* lpConnection, char* pCurrent, DWORD dwCurrentSize);
	bool	ServerStart( INITCONFIG &initConfig );
	void	ServerOff();

	SOCKET			GetListenSocket() { return m_ListenSock; }
	unsigned short	GetServerPort() { return m_usPort; }
	char*			GetServerIp() { return m_szIp; }
	inline HANDLE	GetWorkerIOCP() { return m_hWorkerIOCP; }

	void	DoAccept( LPOVERLAPPED_EX lpOverlappedEx );
	void	DoRecv( LPOVERLAPPED_EX lpOverlappedEx , DWORD dwIoSize );
	void	DoSend( LPOVERLAPPED_EX lpOverlappedEx , DWORD dwIoSize );

	LPPROCESSPACKET	GetProcessPacket( eOperationType operationType, LPARAM lParam, WPARAM wParam );
	void	ClearProcessPacket(LPPROCESSPACKET lpProcessPacke);
	// UDP
	char* UDP_unsafe_PrepareSendPacket(int slen); // unsafe: UDP스레드에서만 사용
	bool UDP_unsafe_SendPost(int nSendSize, CConnection* lpConnection);

	static CIocpServer* GetIocpServer() { return CIocpServer::m_pIocpServer; }
	static CIocpServer* m_pIocpServer;
	static bool UDPThreadFlag;
	
protected:
	// 공사중
	inline void PushProcessQueue(LPPROCESSPACKET massege)
	{
		m_queueProcess.Push(massege);
		if (false == m_isProcessThreadUP)
			SetEvent(m_hProcessThreadEvent);
	}
	
	bool				m_isGameServer;
	
private:
	char                m_arrUDPRecvBuffer[1024];
	char                m_arrUDPSendBuffer[1024];
	CMonitor		    m_csCloseSocket;

	bool				CreateProcessThreads();
	bool				CreateWorkerThreads();
	void				GetProperThreadsCount();
	bool				CreateWorkerIOCP();
	//bool				CreateProcessIOCP(); // FIXED: IOCP QUEUE
	bool				CreateListenAndUdpSock();

	// process thread queue
	MPMCQueue<PROCESSPACKET> m_queueProcess;
	HANDLE m_hProcessThreadEvent;
	bool m_isProcessThreadUP;

	// packet stack
	MPMCStack<PROCESSPACKET> m_stackPacket;

private:
	// common
	SOCKET				m_UdpBlockSock;
	SOCKET				m_ListenSock;

	HANDLE				m_hWorkerIOCP;
	//HANDLE			m_hProcessIOCP; // FIXED: IOCP QUEUE
	
	HANDLE              m_hUdpRecvFromThread[1];
	HANDLE				m_hWorkerThread[ MAX_WORKER_THREAD ];
	HANDLE				m_hProcessThread[ MAX_PROCESS_THREAD ];

	unsigned short		m_usPort;
	char				m_szIp[MAX_IP_LENGTH];

	DWORD				m_dwTimeTick;
	DWORD				m_dwWorkerThreadCount;
	DWORD				m_dwProcessThreadCount;

	bool				m_bWorkThreadFlag;
	bool				m_bUdpWorkThreadFlag;
	bool				m_bProcessThreadFlag;

	LPPROCESSPACKET		m_lpProcessPacket;
	DWORD				m_dwProcessPacketCnt;
};

inline CIocpServer* IocpServer()
{
	return CIocpServer::GetIocpServer();
}

