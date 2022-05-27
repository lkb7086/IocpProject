#include "StdAfx.h"
//#include "IocpGameServer.h"
#include "ProcessPacket.h"

#define BASE_SERVER _T("BASE_SERVER")
#define INIFILE_NAME _T(".//GameServer.ini")

IMPLEMENT_SINGLETON(CIocpGameServer);

HANDLE CIocpGameServer::m_hMainThreadEvent = NULL;

CIocpGameServer::CIocpGameServer() : m_pNpcServerConn(nullptr), m_pDbAgentConn(nullptr),
									 m_pLoginServerConn(nullptr), m_pNoSQLServerConn(nullptr)
{
	m_hMainThreadEvent = CreateEvent
		(NULL,
		FALSE, // TRUE == 수동리셋, FALSE == 자동리셋
		FALSE, // TRUE == 신호 상태시작, FALSE == 비신호 상태시작
		NULL);
	if (NULL == m_hMainThreadEvent)
		return;

	if (nullptr == m_pIocpServer)
		m_pIocpServer = this;

	m_pTickThread = new(nothrow)CTickThread;
	if (nullptr == m_pTickThread)
		return;
	
	m_secTick = 0;
	m_dayAndNightTime = 0;
	m_hostCount = 0;
	InitProcessFunc();
	m_serverID = -1;

	m_ringBuffer.Create(10240);
}

CIocpGameServer::~CIocpGameServer()
{
	if (nullptr != m_pNpcServerConn)
	{
		delete m_pNpcServerConn;
		m_pNpcServerConn = nullptr;
	}
	if (nullptr != m_pDbAgentConn)
	{
		delete m_pDbAgentConn;
		m_pDbAgentConn = nullptr;
	}
	if (nullptr != m_pLoginServerConn)
	{
		delete m_pLoginServerConn;
		m_pLoginServerConn = nullptr;
	}
	if (nullptr != m_pNoSQLServerConn)
	{
		delete m_pNoSQLServerConn;
		m_pNoSQLServerConn = nullptr;
	}
}

void CIocpGameServer::InitProcessFunc()
{
	mapPakect.insert(PACKET_PAIR(PacketType::ConfirmIDGameServer_Req, CProcessPacket::fnConfirmID_Not));
	mapPakect.insert(PACKET_PAIR(PacketType::StartLobby_Req, CProcessPacket::fnStartLobby_Req));
	mapPakect.insert(PACKET_PAIR(PacketType::CreateCharacter_Req, CProcessPacket::fnCreateCharacter_Req));
	mapPakect.insert(PACKET_PAIR(PacketType::DeleteCharacter_Req, CProcessPacket::fnDeleteCharacter_Req));
	mapPakect.insert(PACKET_PAIR(PacketType::StartGame_Req, CProcessPacket::fnStartGame_Req));
	
	// 이동
	mapPakect.insert(PACKET_PAIR(PacketType::MovePlayer_Req, CProcessPacket::fnMovePlayer_Req));
	mapPakect.insert(PACKET_PAIR(PacketType::ChangeColor_Req, CProcessPacket::fnChangeColor_Req));

	mapPakect.insert(PACKET_PAIR(PacketType::MoveServer_Req, CProcessPacket::fnMoveServer_Req));
	mapPakect.insert(PACKET_PAIR(PacketType::PlayerInfoAndMoveLevel_Req, CProcessPacket::fnPlayerInfoAndMoveLevel_Req));
	







	// 채팅
	mapPakect.insert(PACKET_PAIR(PacketType::CL_GS_CL_Chat, CProcessPacket::CL_GS_CL_Chat));








	mapPakect.insert(PACKET_PAIR(PacketType::StartLobby_Not, CProcessPacket::fnStartLobby_Not));
	mapPakect.insert(PACKET_PAIR(PacketType::StartLogin_Not, CProcessPacket::fnStartLogin_Not));
	mapPakect.insert(PACKET_PAIR(PacketType::MoveServer_Not1, CProcessPacket::fnMoveServer_Not1));
	mapPakect.insert(PACKET_PAIR(PacketType::MoveServer_Not2, CProcessPacket::fnMoveServer_Not2));
	
	


	mapPakect.insert(PACKET_PAIR(PacketType::ServerTestPacket, CProcessPacket::fnServerTestPacket));
	mapPakect.insert(PACKET_PAIR(PacketType::Notice_Not, CProcessPacket::fnSendToWorldPlayer_RecvBufferFromServer));
}

bool CIocpGameServer::GameServerStart()
{
	///*
	//if (-1 == GetINIString(m_szLogFileName, BASE_SERVER, _T("LOGFILE"), 100, INIFILE_NAME))
	//return false;
	//*/
	// 임시
	_tcsncpy_s(m_szLogFileName, _countof(m_szLogFileName), _T("GameServer"), _TRUNCATE);

	// 로그 초기화
	sLogConfig LogConfig;
	_tcsncpy_s(LogConfig.s_szLogFileName, _countof(LogConfig.s_szLogFileName), m_szLogFileName, _TRUNCATE);
	LogConfig.s_nLogInfoTypes[STORAGE_WINDOW] = LOG_ALL;
	LogConfig.s_nLogInfoTypes[STORAGE_FILE] = LOG_ERROR_ALL;
	LogConfig.s_hWnd = NULL;
	INIT_LOG(LogConfig);

	m_isGameServer = true;

	// 서버 정보 초기화
	INITCONFIG initConfig;
	int nMaxConnectionCnt = 0;



	if (-1 == (initConfig.nServerPort = GetINIInt(BASE_SERVER, _T("SERVER_PORT"), INIFILE_NAME)))
		return false;
	if (-1 == (m_serverID = GetINIInt(BASE_SERVER, _T("SERVER_ID"), INIFILE_NAME)))
		return false;
	


	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	// 임시
	initConfig.nProcessPacketCnt = MAX_USER_COUNT * 10;
	initConfig.nSendBufCnt = 20;
	initConfig.nRecvBufCnt = 10;
	initConfig.nSendBufSize = 4096;
	initConfig.nRecvBufSize = 4096;
	//initConfig.nServerPort = 59080;
	nMaxConnectionCnt = MAX_USER_COUNT;
	initConfig.nWorkerThreadCnt = (4 * 2) + 1;
	initConfig.nProcessThreadCnt = 1;
	//initConfig.connPoolSize = MAX_POOL_USER_COUNT; // 복붙

	DatabaseManager();
	AreaManager();
	if (false == CIocpServer::ServerStart(initConfig)) // 자리바꾸면 안됨
		return false;
	if (false == PlayerManager()->CreatePlayer(initConfig, nMaxConnectionCnt)) // 자리바꾸면 안됨
		return false;
	
	m_pTickThread->SetTickThreadType(TickThreadType::Common);
	m_pTickThread->CreateThread(UPDATE_TICK);
	m_pTickThread->Run();





	char name[32]; memset(name, 0, sizeof(name));
	char* ip = nullptr;
	PHOSTENT host = nullptr;
	if (gethostname(name, sizeof(name)) == 0)
	{
		if ((host = gethostbyname(name)) != nullptr)
		{
			ip = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
		}
	}

	printf("IP: %s\n", ip);
	printf("Port: %d\n", initConfig.nServerPort);
	printf("Server ID: %d\n", m_serverID);
	printf("Maximum number of connections: %d\n", nMaxConnectionCnt);
	puts("GameServer Start");
	puts("");

	return true;
}

void CIocpGameServer::OnInitIocpServer()
{
	tls_pSer = new(nothrow)CSerializer();
	if (nullptr == tls_pSer)
		return;
	tls_pEngine = new(nothrow) std::mt19937_64((std::random_device())());
	if (nullptr == tls_pEngine) return;
}

void CIocpGameServer::OnClose_IocpServer()
{
	if (nullptr != tls_pSer)
	{
		delete tls_pSer;
		tls_pSer = nullptr;
	}
	if (nullptr != tls_pEngine)
	{
		delete tls_pEngine;
		tls_pEngine = nullptr;
	}
}

bool CIocpGameServer::OnAccept(CConnection* lpConnection)
{
	//puts("OnAccept");

	

	((CPlayer*)lpConnection)->InitPlayer();
	((CPlayer*)lpConnection)->m_isAccept = true;
	lpConnection->SetKeepAliveTick(IocpGameServer()->GetServerTick());

	// 추가
	auto it = m_setConn.find(lpConnection->GetIndex());
	if (m_setConn.end() != it)
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::OnAccept() | Socket[%d]은 SET에 이미 추가되어 있다", lpConnection->GetSocket());
		CIocpServer::CloseConnection(lpConnection);
		return true;
	}
	m_setConn.insert(pair<int, CConnection*>(lpConnection->GetIndex(), lpConnection));

	/*
	// 로그인허락 패킷전송
	stUtil_Empty* pBuffer = (stUtil_Empty*)lpConnection->PrepareSendPacket(sizeof(stUtil_Empty));
	if (nullptr == pBuffer)
		return false;
	pBuffer->type = (packet_type)PacketType::Accept_Not;
	lpConnection->SendPost(sizeof(stUtil_Empty));
	*/

	return true;
}

bool CIocpGameServer::OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	CPlayer* pPlayer = (CPlayer*)lpConnection;

	packet_type type;
	CopyMemory(&type, pRecvedMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);
	if (type < 0 || type > MAX_PROCESSFUNC)
	{
		packet_length length;
		CopyMemory(&length, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpGameServer::OnRecv() | 잘못된 패킷(%d), 길이(%d)", type, length);
		if (lpConnection->m_bIsCilent)
		{
			CIocpServer::CloseConnection(lpConnection);
		}
		return true;
	}

	auto it = mapPakect.find(static_cast<PacketType>(type));
	if (mapPakect.end() != it)
	{
		(*((*it).second))(pPlayer, dwSize, pRecvedMsg);
		return true;
	}
	else
	{
		packet_length length;
		CopyMemory(&length, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpGameServer::OnRecv() | 정의되지 않은 패킷(%d), 길이(%d)", type, length);
		if (lpConnection->m_bIsCilent)
		{
			CIocpServer::CloseConnection(lpConnection);
		}
		return true;
	}
}

void CIocpGameServer::UDP_OnRecv(DWORD dwSize, char* pRecvedMsg)
{
	//CalcXor(pRecvedMsg, 0, dwSize);

	__int32 nType = -1;
	CopyMemory(&nType, pRecvedMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);
	if (nType < 0 || nType > MAX_PROCESSFUNC)
	{
		unsigned __int32 nLength = 0xFFFFFFFF;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpGameServer::UDP_OnRecv() | 잘못된 패킷(%d), 길이(%u)", nType, nLength);
		return;
	}

	/*
	// 임시
	if (nType == CL_GS_Shutdown_Cn)
	{
		SetEvent(CIocpGameServer::m_hMainThreadEvent);
		return;
	}*/

	CPlayer* pPlayer = PlayerManager()->FindPlayer(((stUtil_UInteger*)pRecvedMsg)->nUInteger);
	if (nullptr == pPlayer || !pPlayer->GetIsConfirm())
		return;

	PACKET_IT it = mapPakect.find(static_cast<PacketType>(nType));
	if (mapPakect.end() != it)
	{
		(*((*it).second))(pPlayer, dwSize, pRecvedMsg);
	}
	else
	{
		unsigned __int32 nLength = 0xFFFFFFFF;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpGameServer::UDP_OnRecv() | 정의되지 않은 패킷(%d), 길이(%u)", nType, nLength);
	}
}

bool CIocpGameServer::OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	// 패킷이 처리되었다면 return true;
	// 처리되지 않았다면 return false;

	//if (OnRecv(lpConnection, dwSize, pRecvedMsg))
	//return true;

	// 클라이언트로부터 받은 패킷만 복호화
	if (lpConnection->m_bIsCilent)
	{
		//CalcXor(pRecvedMsg, 0, dwSize);
	}

	packet_type packetType;
	CopyMemory(&packetType, pRecvedMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);

	if (packetType == static_cast<packet_type>(PacketType::MovePlayer_Req))
		CProcessPacket::fnMovePlayer_Req(static_cast<CPlayer*>(lpConnection), dwSize, pRecvedMsg);
	else if(packetType > 50000)
		DatabaseManager()->PushDBQueue((CPlayer*)lpConnection, dwSize, pRecvedMsg);
	else
		m_pTickThread->Enqueue_PacketQ((CPlayer*)lpConnection, dwSize, pRecvedMsg);
	
	return true; // 게임서버는 반드시 true
}

void CIocpGameServer::OnPrepareClose(CConnection* lpConnection)
{
	CPlayer* pPlayer = static_cast<CPlayer*>(lpConnection);
	m_pTickThread->Enqueue_PacketQ(pPlayer, 0, nullptr);
}

void CIocpGameServer::OnClose(CConnection* lpConnection)
{
	CPlayer* pPlayer = static_cast<CPlayer*>(lpConnection);

	auto it = m_setConn.find(lpConnection->GetIndex());
	if (m_setConn.end() == it)
	{
		//LOG(LOG_ERROR_LOW, "SYSTEM | CIocpGameServer::OnClose() | m_setConn에 삭제하려는 세션이 없다");
	}
	else
	{
		CMonitorSRW::OwnerSRW lock(m_srwConn, LockExclusive);
		m_setConn.unsafe_erase(lpConnection->GetIndex());
	}

	if (!lpConnection->m_bIsCilent)
	{
		puts("서버종료");
		return;
	}

	//puts("연결해제");

	if (!pPlayer->m_isMoveServer)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::LogoutPlayerID_Not));
		tls_pSer->Serialize(pPlayer->GetID());
		tls_pSer->Serialize(m_serverID);
		CConnection* pLoginConn = GetLoginServerConn();
		char* pSendBuffer = pLoginConn->PrepareSendPacket(tls_pSer->GetCurBufSize());
		if (nullptr == pSendBuffer)
			return;
		tls_pSer->CopyBuffer(pSendBuffer);
		pLoginConn->SendPost(tls_pSer->GetCurBufSize());
	}

	AreaManager()->RemovePlayerFromArea(pPlayer, pPlayer->GetArea());
	PlayerManager()->RemovePlayer(pPlayer);
	PlayerManager()->ErasePlayerInfo(pPlayer->GetKey());

	//PlayerManager()->Send_LogoutPlayer(pPlayer);

	LOG(LOG_INFO_LOW, "ID (%u) Disconnected. / Current Players (%u)",
		pPlayer->GetKey(), PlayerManager()->GetPlayerCnt());

	pPlayer->InitPlayer();
}

bool CIocpGameServer::OnSystemMsg(CConnection* lpConnection, LPARAM lParam, WPARAM wParam)
{
	CPlayer* pPlayer = (CPlayer*)lpConnection;
	if (nullptr == pPlayer)
		return true;
	GameServerSystemMsg typeSystemMsg = (GameServerSystemMsg)wParam;

	switch (typeSystemMsg)
	{
	case GameServerSystemMsg::SYSTEM_UPDATE_NPCPOS:
	{
		//AreaManager()->Send_UpdateNPC_VSn(nullptr, 1, nullptr);
		break;
	}
	case GameServerSystemMsg::SYSTEM_UPDATE_AUTOPLAYER:
	{
		//PlayerManager()->Send_AutoAttack_Sn(pPlayer);
		break;
	}
	case GameServerSystemMsg::SYSTEM_MSG:
	{
		pPlayer->CloseConnection_CConn(true);
		break;
	}
	case GameServerSystemMsg::SYSTEM_UPDATE_AREA:
	{
		//AreaManager()->ExecuteUpdateArea(pPlayer);
		break;
	}
	default:
		LOG(LOG_ERROR_LOW, "SYSTEM | CIocpGameServer::OnSystemMsg() | OnSystemMsg default");
		break;
	}

	return true;
}

void CIocpGameServer::ServerOffTest()
{
	//DatabaseManager()->Destroy_DBThread();

	if (nullptr != m_pTickThread)
	{
		m_pTickThread->DestroyThread();
		delete m_pTickThread;
		m_pTickThread = nullptr;
	}
}

void CIocpGameServer::ProcessSystemMsg(CPlayer* pPlayer, LPARAM lParam, WPARAM wParam)
{
	LPPROCESSPACKET lpProcessPacket = GetProcessPacket(eOperationType::OP_SYSTEM, lParam, wParam);
	if (nullptr == lpProcessPacket)
		return;
	lpProcessPacket->s_lpConnection = pPlayer;
	lpProcessPacket->s_dwCurrentSize = 0;

	PushProcessQueue(lpProcessPacket);

	/*
	LPPROCESSPACKET lpProcessPacket = GetProcessPacket(OP_SYSTEM, lParam, wParam);
	if (nullptr == lpProcessPacket)
		return;

	if (0 == PostQueuedCompletionStatus(m_hProcessIOCP, 0, (ULONG_PTR)pPlayer, (LPOVERLAPPED)lpProcessPacket))
	{
		ClearProcessPacket(lpProcessPacket);
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CIocpGameServer::ProcessSystemMsg() | PostQueuedCompletionStatus Failed : [%u]",
			GetLastError());
	}*/
}

int CIocpGameServer::GetINIString(TCHAR* szOutStr, TCHAR* szAppName, TCHAR* szKey, int nSize, TCHAR* szFileName)
{
	// GetPrivateProfileString(섹션, 키, 디폴트값, 값을 읽어올 버퍼, 버퍼크기, INI파일경로)
	int ret = GetPrivateProfileString(szAppName, szKey, _T(""), szOutStr, nSize, szFileName);
	if (0 == ret)
	{
		TCHAR szTemp[300];
		_sntprintf_s(szTemp, _countof(szTemp), _TRUNCATE, _T("[%s]Config File에 [%s]-[%s]항목은 존재하지 않습니다."), szFileName, szAppName, szKey);
		return -1;
	}
	return 0;
}

int	CIocpGameServer::GetINIInt(TCHAR* szAppName, TCHAR* szKey, TCHAR* szFileName)
{
	int ret = GetPrivateProfileInt(szAppName, szKey, -1, szFileName);
	if (ret < 0)
	{
		TCHAR szTemp[300];
		_sntprintf_s(szTemp, _countof(szTemp), _TRUNCATE, _T("[%s]Config File에 [%s]-[%s]항목은 존재하지 않습니다."), szFileName, szAppName, szKey);
	}
	return ret;
}

bool CIocpGameServer::ConnectToNpcServer()
{
	if (nullptr != m_pNpcServerConn)
	{
		CIocpServer::CloseConnection(m_pNpcServerConn);
		delete m_pNpcServerConn;
		m_pNpcServerConn = nullptr;
	}
	m_pNpcServerConn = new(nothrow)CConnection;
	m_pNpcServerConn->m_bIsCilent = false;

	INITCONFIG initConfig;
	char		szIp[30]; memset(szIp, 0, sizeof(char) * 30);

	/*
	if( -1 == ( initConfig.nSendBufCnt = GetINIInt( CONNECT_NPCSERVER  , "SEND_BUFFER_CNT"  , INIFILE_NAME ) ) )
	return false;
	if( -1 == ( initConfig.nRecvBufCnt = GetINIInt( CONNECT_NPCSERVER  , "RECV_BUFFER_CNT"  , INIFILE_NAME ) ) )
	return false;
	if( -1 == ( initConfig.nSendBufSize = GetINIInt( CONNECT_NPCSERVER  , "SEND_BUFFER_SIZE"  , INIFILE_NAME ) ) )
	return false;
	if( -1 == ( initConfig.nRecvBufSize = GetINIInt( CONNECT_NPCSERVER  , "RECV_BUFFER_SIZE"  , INIFILE_NAME ) ) )
	return false;
	if( -1 == GetINIString( szIp , CONNECT_NPCSERVER  , "CONNECT_IP"  ,  100 , INIFILE_NAME ) )
	return false;
	if( -1 == ( initConfig.nServerPort = GetINIInt( CONNECT_NPCSERVER  , "CONNECT_PORT"  , INIFILE_NAME ) ) )
	return false;
	*/

	// 임시
	initConfig.nSendBufCnt = 100;
	initConfig.nRecvBufCnt = 400;
	initConfig.nSendBufSize = 4096;
	initConfig.nRecvBufSize = 4096;
	strncpy_s(szIp, _countof(szIp), "127.0.0.1", _TRUNCATE);
	initConfig.nServerPort = 8100;
	// 임시


	m_pNpcServerConn->SetConnectionIp(szIp);
	m_pNpcServerConn->CreateConnection(initConfig);
	if (m_pNpcServerConn->ConnectTo(szIp, initConfig.nServerPort) == false)
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::ConnectToNpcServer() | NpcServer Connect Failed");
		return false;
	}

	//LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToNpcServer() | [NpcServerConn_Connection] Connect Ip : %s", szIp);
	//LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToNpcServer() | [NpcServerConn_Connection] Connect Port : %d", initConfig.nServerPort);
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToNpcServer() | NpcServer [%d]socket 연결 성공", m_pNpcServerConn->GetSocket());

	return true;

}

bool CIocpGameServer::ConnectToDBAgent()
{
	// 이미 디비 서버와 연결이 되어있다면 전에 연결을 끊고 다시 연결한다
	if (nullptr != m_pDbAgentConn)
	{
		CIocpServer::CloseConnection(m_pDbAgentConn);
		delete m_pDbAgentConn;
		m_pDbAgentConn = nullptr;
	}
	m_pDbAgentConn = new(nothrow)CConnection;
	m_pDbAgentConn->m_bIsCilent = false;

	INITCONFIG initConfig;
	char		szIp[30] = "127.0.0.1";

	if (-1 == (initConfig.nSendBufCnt = 10))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 40))
		return false;
	if (-1 == (initConfig.nSendBufSize = 4096))
		return false;
	if (-1 == (initConfig.nRecvBufSize = 4096))
		return false;

	if (-1 == (initConfig.nServerPort = 9081))
		return false;

	m_pDbAgentConn->SetConnectionIp(szIp);
	m_pDbAgentConn->CreateConnection(initConfig);
	if (m_pDbAgentConn->ConnectTo(szIp, initConfig.nServerPort) == false)
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::ConnectToDBAgent() | DBAgent Connect Failed");
		return false;
	}
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToDBAgent() | DBAgent [%d]socket 연결 성공"
		, m_pDbAgentConn->GetSocket());

	return true;

}

bool CIocpGameServer::ConnectToLoginServer()
{
	// 이미 로그인 서버와 연결이 되어있다면 전에 연결을 끊고 다시 연결한다
	if (nullptr != m_pLoginServerConn)
	{
		CIocpServer::CloseConnection(m_pLoginServerConn);
		delete m_pLoginServerConn;
		m_pLoginServerConn = nullptr;
	}
	m_pLoginServerConn = new(nothrow)CConnection;
	if (m_pLoginServerConn == nullptr)
		return false;
	m_pLoginServerConn->m_bIsCilent = false;

	INITCONFIG initConfig;
	char		szIp[30] = "127.0.0.1";

	if (-1 == (initConfig.nSendBufCnt = 20))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 10))
		return false;
	if (-1 == (initConfig.nSendBufSize = 4096))
		return false;
	if (-1 == (initConfig.nRecvBufSize = 4096))
		return false;

	if (-1 == (initConfig.nServerPort = 9082))
		return false;

	m_pLoginServerConn->SetConnectionIp(szIp);
	m_pLoginServerConn->CreateConnection(initConfig);
	if (m_pLoginServerConn->ConnectTo(szIp, initConfig.nServerPort) == false)
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::ConnectToLoginServer() | LoginServer Connect Failed");
		return false;
	}
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpGameServer::ConnectToLoginServer() | LoginServer [%d]socket 연결 성공"
		, m_pLoginServerConn->GetSocket());


	stUtil_Char* pBuffer = (stUtil_Char*)m_pLoginServerConn->PrepareSendPacket(sizeof(stUtil_Char));
	if (nullptr == pBuffer)
		return false;
	pBuffer->type = (packet_type)PacketType::ImServer_Not;
	pBuffer->nChar = m_serverID;
	m_pLoginServerConn->SendPost(sizeof(stUtil_Char));

	return true;
}

bool CIocpGameServer::ConnectToNoSQLServer()
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

	if (-1 == (initConfig.nSendBufCnt = 10))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 40))
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

void CIocpGameServer::ConfirmID_Not(CPlayer* pPlayer, char* pRecvedMsg)
{
	unsigned char result = 0;
	unsigned long long key = 0;
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));

	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(result);
	tls_pSer->Deserialize(key);
	tls_pSer->Deserialize(szID, sizeof(szID));

	if (m_mapSERVER.end() == m_mapSERVER.find(key))
	{
		m_mapSERVER.insert(key);

		CSerializer& ser = *tls_pSer;
		ser.StartSerialize();
		ser.Serialize(static_cast<packet_type>(PacketType::ConfirmIDGameServer_Res));
		ser.Serialize(result);
		ser.Serialize(key);
		ser.Serialize(szID);

		char* pBuf = pPlayer->PrepareSendPacket(ser.GetCurBufSize());
		if (nullptr == pBuf)
			return;
		ser.CopyBuffer(pBuf);
		pPlayer->SendPost(ser.GetCurBufSize());
	}
	else
	{
		m_mapSERVER.erase(key);
		LOG(LOG_ERROR_LOW, "CIocpGameServer::Recv_LSKey_Cn() | m_setSERVER 중복");
	}
}

void CIocpGameServer::StartLobby_Req(CPlayer* pPlayer, char* pRecvedMsg)
{
	unsigned long long serverKey = 0;
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));

	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(serverKey);
	tls_pSer->Deserialize(szID, sizeof(szID));

	if (m_mapSERVER.find(serverKey) == m_mapSERVER.end())
	{
		LOG(LOG_ERROR_LOW, "CIocpGameServer::StartLobby_Req() | m_setSERVER.find(serverKey) == m_setSERVER.end()");
		return;
	}
	else
	{
		m_mapSERVER.erase(serverKey);

		pPlayer->SetID(szID);


		// DB에서 다수의 캐릭터들을 확인
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::StartLobby_Not));
		tls_pSer->Serialize(szID);

		char* p = m_ringBuffer.ForwardMark(tls_pSer->GetCurBufSize());
		tls_pSer->CopyBuffer(p);
		DatabaseManager()->PushDBQueue(pPlayer, tls_pSer->GetCurBufSize(), p);
	}
}