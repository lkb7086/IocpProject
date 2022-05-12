#include "stdafx.h"
//#include "IocpDBAgent.h"

IMPLEMENT_SINGLETON(CIocpDBAgent)

CIocpDBAgent::CIocpDBAgent()
{
	if (nullptr == m_pIocpServer)
		m_pIocpServer = this;
	m_pTickThread = new(nothrow) CTickThread;
	InitProcessFunc();
}

CIocpDBAgent::~CIocpDBAgent()
{
	if (nullptr != m_pTickThread)
	{
		m_pTickThread->DestroyThread();
		delete m_pTickThread;
		m_pTickThread = nullptr;
	}
}

void CIocpDBAgent::InitProcessFunc()
{
	un_mapPakect.insert(PACKET_PAIR(GS_DA_SearchUnit_Rq, CProcessPacket::fnSearchUnit_Rq));
	un_mapPakect.insert(PACKET_PAIR(GS_SV_LogoutPlayer_Cn, CProcessPacket::fnLogoutPlayer_Cn));
	un_mapPakect.insert(PACKET_PAIR(GS_DA_InsertDropItem_Cn, CProcessPacket::fnInsertDropItem_Cn));
	un_mapPakect.insert(PACKET_PAIR(GS_DA_MerseDropItem_Cn, CProcessPacket::fnMerseDropItem_Cn));
	un_mapPakect.insert(PACKET_PAIR(CL_DA_DeleteItem_Cn, CProcessPacket::fnDeleteItem_Cn));

	/*
	un_mapPakect.insert(PACKET_PAIR(DA_CL_ConnectToGS_Rq, CProcessPacket::fnConnectToGSRq));
	un_mapPakect.insert(PACKET_PAIR(CL_AllSearchMarket_Cn, CProcessPacket::fnCLAllSearchMarketCn));
	un_mapPakect.insert(PACKET_PAIR(CL_SearchTextFromMarket_Rq, CProcessPacket::fnCLSearchTextFromMarketRq));
	un_mapPakect.insert(PACKET_PAIR(CL_DA_SearchInven_Rq, CProcessPacket::fnCL_DA_SearchInven_Rq));
	un_mapPakect.insert(PACKET_PAIR(CL_DA_DoPurchaseItemInMarket_Rq, CProcessPacket::fnDoPurchaseItemInMarket));
	un_mapPakect.insert(PACKET_PAIR(CL_DA_DoInsertItemToMarket_Rq, CProcessPacket::fnCL_DA_DoInsertItemToMarket_Rq));
	un_mapPakect.insert(PACKET_PAIR(DA_GS_LogoutPlayer_Cn, CProcessPacket::fnDA_GS_LogoutPlayer_Cn));
	*/
}

void CIocpDBAgent::OnInitIocpServer(){}
void CIocpDBAgent::OnClose_IocpServer() {}

//client가 접속 수락이 되었을 때 호출되는 함수
bool CIocpDBAgent::OnAccept(CConnection *lpConnection)
{
	ConnectionManager()->AddConnection(lpConnection);
	LOG(LOG_INFO_LOW, "SYSTEM | CIocpDBAgent::OnAccept() | 게임서버 접속");
	//LOG(LOG_INFO_LOW, "SYSTEM | CIocpDBAgent::OnAccept() | IP[%s] Socket[%d] 접속 Cnt[%d]",
		//lpConnection->GetConnectionIp(), lpConnection->GetSocket(),
		//ConnectionManager()->GetConnectionCnt());

	return true;
}

//client에서 packet이 도착했을 때 순서 성 있게 처리되어지는 패킷처리
bool CIocpDBAgent::OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	__int32 nType = -1;
	CopyMemory(&nType, pRecvedMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);

	if (nType < 0 || nType > MAX_PROCESSFUNC)
	{
		unsigned __int32 nLength = 0xFFFFFFFF;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpDBAgent::OnRecv() | 정의되지 않은 패킷타입(%d), 길이(%u)", nType, nLength);
		return true;
	}

	PACKET_IT it = un_mapPakect.find(nType);
	if (un_mapPakect.end() != it)
		(*((*it).second))(lpConnection, dwSize, pRecvedMsg);
	else
	{
		unsigned __int32 nLength = 0xFFFFFFFF;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpDBAgent::OnRecv() | un_mapPakect.end() 정의되지 않은 패킷(%d), 길이(%u)", nType, nLength);
		if (lpConnection->m_bIsCilent)
		{
			CloseConnection(lpConnection);
		}
		return true;
	}

	return true;
}

//client에서 packet이 도착했을 때 순서 성 없이 곧바로 처리 되는 패킷처리
bool CIocpDBAgent::OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	/////////////////////////////////////////////////////////////////
	//패킷이 처리되면 return true; 처리 되지 않았다면 return false;
	return false;
}

void CIocpDBAgent::OnPrepareClose(CConnection* lpConnection)
{
	lpConnection->CloseConnection_CConn(true);
}

//client와 연결이 종료되었을 때 호출되는 함수
void CIocpDBAgent::OnClose(CConnection* lpConnection)
{
	ConnectionManager()->RemoveConnection(lpConnection);
	LOG(LOG_INFO_LOW, "SYSTEM | CIocpDBAgent::OnClose() | 게임서버 종료");
}
bool CIocpDBAgent::OnSystemMsg(CConnection* lpConnection, LPARAM msgType, WPARAM wParam)
{
	return true;
}

bool CIocpDBAgent::ServerStart()
{
	// 로그 초기화
	sLogConfig LogConfig;
	_tcsncpy_s(LogConfig.s_szLogFileName, _countof(LogConfig.s_szLogFileName), _T("DBAgent"), _TRUNCATE);
	LogConfig.s_nLogInfoTypes[STORAGE_WINDOW] = LOG_ALL;
	LogConfig.s_nLogInfoTypes[STORAGE_FILE] = LOG_ERROR_ALL;
	LogConfig.s_hWnd = NULL;
	INIT_LOG(LogConfig);

	// 서버정보 초기화
	INITCONFIG InitConfig;
	InitConfig.nServerPort = 9081;
	InitConfig.nRecvBufCnt = 100;
	InitConfig.nRecvBufSize = 1024;
	InitConfig.nProcessPacketCnt = 1000;
	InitConfig.nSendBufCnt = 50;
	InitConfig.nSendBufSize = 1024;
	InitConfig.nWorkerThreadCnt = 4 + 1;
	InitConfig.nProcessThreadCnt = 1;

	CIocpServer::ServerStart(InitConfig);
	ConnectionManager()->CreateConnection(InitConfig, 1);
	DatabaseManager();
	m_pTickThread->CreateThread(INFINITE);
	m_pTickThread->Run();

	LOG(LOG_INFO_LOW, "DBAgent Start");

	return true;
}