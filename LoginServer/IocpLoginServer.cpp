#include "stdafx.h"
//#include "IocpLoginServer.h"
#include "processpacket.h"

IMPLEMENT_SINGLETON(CIocpLoginServer)

CIocpLoginServer::CIocpLoginServer() : m_nPrivateKey(0)
{
	if (nullptr == m_pIocpServer)
		m_pIocpServer = this;
	m_pTickThread = new(nothrow) CTickThread;
	if (nullptr == m_pTickThread) return;
	m_pGameServerConn = nullptr;
	m_pNoSQLServerConn = nullptr;
	InitProcessFunc();
}

CIocpLoginServer::~CIocpLoginServer()
{
	if (nullptr != m_pTickThread)
	{
		m_pTickThread->DestroyThread();
		delete m_pTickThread;
		m_pTickThread = nullptr;
	}

	if (nullptr != m_pGameServerConn)
	{
		delete m_pGameServerConn;
		m_pGameServerConn = nullptr;
	}
}

void CIocpLoginServer::InitProcessFunc()
{
	un_mapPakect.insert(PACKET_PAIR(PacketType::ConfirmID_Req, CProcessPacket::fnConfirmID_Req));
	un_mapPakect.insert(PACKET_PAIR(PacketType::ConfirmIDGameServer_Res, CProcessPacket::fnConfirmIDGameServer_Res));

	un_mapPakect.insert(PACKET_PAIR(PacketType::JoinID_Req, CProcessPacket::fnJoinID_Req));

	un_mapPakect.insert(PACKET_PAIR(PacketType::LogoutPlayerID_Not, CProcessPacket::fnLogoutPlayerID_Not));

	un_mapPakect.insert(PACKET_PAIR(PacketType::MoveServer_Not1, CProcessPacket::fnMoveServer_Not1));
	un_mapPakect.insert(PACKET_PAIR(PacketType::MoveServer_Not2, CProcessPacket::fnMoveServer_Not2));


	un_mapPakect.insert(PACKET_PAIR(PacketType::ImServer_Not, CProcessPacket::fnImServer_Not));



	un_mapPakect.insert(PACKET_PAIR(PacketType::Nosql_Not, CProcessPacket::fnNosql_Not));
}

void CIocpLoginServer::OnInitIocpServer(){}
void CIocpLoginServer::OnClose_IocpServer() {}

//client가 접속 수락이 되었을 때 호출되는 함수
bool CIocpLoginServer::OnAccept(CConnection *lpConnection)
{
	/*
	stUtil_Empty* pBuffer = (stUtil_Empty*)lpConnection->PrepareSendPacket(sizeof(stUtil_Empty));
	if (nullptr == pBuffer)
		return false;
	pBuffer->type = (packet_type)PacketType::GS_CL_Accept;
	lpConnection->SendPost(sizeof(stUtil_Empty));
	*/


	ConnectionManager()->AddConnection(lpConnection);

	LOG(LOG_INFO_LOW, "SYSTEM | CIocpLoginServer::OnAccept() | Socket: %d", lpConnection->GetSocket());
	
	return true;
}

//client에서 packet이 도착했을 때 순서 성 있게 처리되어지는 패킷처리
bool CIocpLoginServer::OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	// 클라이언트로부터 받은 패킷만 복호화
	if (lpConnection->m_bIsCilent)
	{
		//CalcXor(pRecvedMsg, 0, dwSize);
	}

	packet_type nType;
	CopyMemory(&nType, pRecvedMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);

	if (nType < 0 || nType > MAX_PROCESSFUNC)
	{
		packet_length nLength;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpLoginServer::OnRecv() | 정의되지 않은 패킷타입(%d), 길이(%u)", nType, nLength);
		return true;
	}

	PACKET_IT it = un_mapPakect.find(static_cast<PacketType>(nType));
	if (un_mapPakect.end() != it)
		(*((*it).second))(lpConnection, dwSize, pRecvedMsg);
	else
	{
		packet_length nLength;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpLoginServer::OnRecv() | un_mapPakect.end() 정의되지 않은 패킷(%d), 길이(%u)", nType, nLength);
		if (lpConnection->m_bIsCilent)
		{
			CloseConnection(lpConnection);
		}
		return true;
	}

	return true;
}

//client에서 packet이 도착했을 때 순서 성 없이 곧바로 처리 되는 패킷처리
bool CIocpLoginServer::OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	/////////////////////////////////////////////////////////////////
	//패킷이 처리되면 return true; 처리 되지 않았다면 return false;
	return false;
}

void CIocpLoginServer::OnPrepareClose(CConnection* lpConnection)
{
	lpConnection->CloseConnection_CConn(true);
}

//client와 연결이 종료되었을 때 호출되는 함수
void CIocpLoginServer::OnClose(CConnection* lpConnection)
{
	if (nullptr == lpConnection)
		return;

	ConnectionManager()->RemoveConnection(lpConnection);

	if (!lpConnection->m_bIsCilent)
	{ 
		//puts("게임서버 종료");
		ConnectionManager()->RemoveServerCon(lpConnection);
		lpConnection->m_bIsCilent = true;
	}
	
	((CPlayer*)lpConnection)->Init();

	LOG(LOG_INFO_LOW, "SYSTEM | CIocpLoginServer::OnClose() | 종료 ConnectionCnt %d", ConnectionManager()->GetConnectionCnt());
}

bool CIocpLoginServer::OnSystemMsg(CConnection* lpConnection, LPARAM msgType, WPARAM wParam)
{
	return true;
}

bool CIocpLoginServer::ServerStart()
{
	// 로그 초기화
	sLogConfig LogConfig;
	_tcsncpy_s(LogConfig.s_szLogFileName, _countof(LogConfig.s_szLogFileName), _T("LoginServer"), _TRUNCATE);
	LogConfig.s_nLogInfoTypes[STORAGE_WINDOW] = LOG_ALL;
	LogConfig.s_nLogInfoTypes[STORAGE_FILE] = LOG_ERROR_ALL;
	LogConfig.s_hWnd = NULL;
	INIT_LOG(LogConfig);

	// 서버 정보 초기화
	INITCONFIG InitConfig;
	InitConfig.nServerPort = 9082;
	InitConfig.nRecvBufCnt = 10;
	InitConfig.nRecvBufSize = 1024;
	InitConfig.nProcessPacketCnt = 3000;
	InitConfig.nSendBufCnt = 20;
	InitConfig.nSendBufSize = 1024;
	InitConfig.nWorkerThreadCnt = (4 * 2) + 1;
	InitConfig.nProcessThreadCnt = 1;

	CIocpServer::ServerStart(InitConfig);
	ConnectionManager()->CreateConnection(InitConfig, MAX_USER_COUNT);
	DatabaseManager();
	
	//m_pTickThread->SetTickThreadType(TickThreadType::Common);
	m_pTickThread->CreateThread(UPDATE_TICK);
	m_pTickThread->Run();

	LOG(LOG_INFO_LOW, "Port: %u", InitConfig.nServerPort);
	LOG(LOG_INFO_LOW, "LoginServer Start");

	return true;
}


bool CIocpLoginServer::ConnectToGameServer()
{
	// 이미 로그인 서버와 연결이 되어있다면 전에 연결을 끊고 다시 연결한다
	if (nullptr != m_pGameServerConn)
	{
		CloseConnection(m_pGameServerConn);
		m_pGameServerConn = nullptr;
	}
	m_pGameServerConn = new(nothrow) CConnection;

	INITCONFIG initConfig;
	char		szIp[30] = "127.0.0.1";

	//접속할 DBAgent에 대한 정보를 얻어온다.
	if (-1 == (initConfig.nSendBufCnt = 20))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 10))
		return false;
	if (-1 == (initConfig.nSendBufSize = 1024))
		return false;
	if (-1 == (initConfig.nRecvBufSize = 1024))
		return false;

	if (-1 == (initConfig.nServerPort = 59080))
		return false;

	m_pGameServerConn->SetConnectionIp(szIp);
	m_pGameServerConn->CreateConnection(initConfig);
	if (m_pGameServerConn->ConnectTo(szIp, initConfig.nServerPort) == false)
	{
		LOG(LOG_ERROR_LOW, "CIocpLoginServer::ConnectToGameServer() | GameServer Connect Failed");
		return false;
	}
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpLoginServer::ConnectToGameServer() | GameServer [%d]socket 연결 성공"
		, m_pGameServerConn->GetSocket());

	return true;

}

bool CIocpLoginServer::ConnectToNoSQLServer()
{
	if (nullptr != m_pNoSQLServerConn)
	{
		CIocpServer::CloseConnection(m_pNoSQLServerConn);
		delete m_pNoSQLServerConn;
		m_pNoSQLServerConn = nullptr;
	}
	m_pNoSQLServerConn = new(nothrow)CConnection;
	m_pNoSQLServerConn->m_bIsCilent = false;

	INITCONFIG initConfig;
	char		szIp[30] = "127.0.0.1";

	if (-1 == (initConfig.nSendBufCnt = 40))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 20))
		return false;
	if (-1 == (initConfig.nSendBufSize = 4096))
		return false;
	if (-1 == (initConfig.nRecvBufSize = 4096))
		return false;

	if (-1 == (initConfig.nServerPort = 9999))
		return false;

	m_pNoSQLServerConn->SetConnectionIp(szIp);
	m_pNoSQLServerConn->CreateConnection(initConfig);
	if (m_pNoSQLServerConn->ConnectTo(szIp, initConfig.nServerPort) == false)
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::ConnectToNoSQLServer() | NoSQLServer Connect Failed");
		return false;
	}
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToNoSQLServer() | NoSQLServer [%d]socket 연결 성공"
		, m_pNoSQLServerConn->GetSocket());

	return true;
}