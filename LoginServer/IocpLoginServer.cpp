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
	un_mapPakect.insert(PACKET_PAIR(PacketType::JoinID_Req, CProcessPacket::fnJoinID_Req));

	un_mapPakect.insert(PACKET_PAIR(PacketType::LogoutPlayerID_Not, CProcessPacket::LogoutPlayerID_Not));




	//un_mapPakect.insert(PACKET_PAIR(CL_LS_ConfirmID_Rq, CProcessPacket::fnConfirmID_Rq));
	//un_mapPakect.insert(PACKET_PAIR(CL_LS_SelectUnit_Rq, CProcessPacket::fnSelectUnit_Rq));
	//un_mapPakect.insert(PACKET_PAIR(GS_CL_SelectUnit_Aq, CProcessPacket::fnSelectUnit_Aq));
	//un_mapPakect.insert(PACKET_PAIR(GS_SV_LogoutPlayer_Cn, CProcessPacket::fnLogoutPlayer_Cn));

	//GS_CL_SelectUnit_Aq  ((CPlayer*)pConnection)->SetIsConfirm(true);*/
	//un_mapPakect.insert(PACKET_PAIR(CL_CreateUnit_Rq, CProcessPacket::fnCreateUnitRq));
	//un_mapPakect.insert(PACKET_PAIR(CL_DeleteUnit_Rq, CProcessPacket::fnDeleteUnitRq));

	un_mapPakect.insert(PACKET_PAIR(PacketType::ImServer_Not, CProcessPacket::fnImServer_Not));
}

void CIocpLoginServer::OnInitIocpServer(){}
void CIocpLoginServer::OnClose_IocpServer() {}

//client�� ���� ������ �Ǿ��� �� ȣ��Ǵ� �Լ�
bool CIocpLoginServer::OnAccept(CConnection *lpConnection)
{
	/*
	stUtil_Empty* pBuffer = (stUtil_Empty*)lpConnection->PrepareSendPacket(sizeof(stUtil_Empty));
	if (nullptr == pBuffer)
		return false;
	pBuffer->type = (packet_type)PacketType::GS_CL_Accept;
	lpConnection->SendPost(sizeof(stUtil_Empty));
	*/

	if (lpConnection->m_bIsCilent)
	{
		LOG(LOG_INFO_LOW, "SYSTEM | CIocpLoginServer::OnAccept() | Socket: %d", lpConnection->GetSocket());
	}

	return true;
}

//client���� packet�� �������� �� ���� �� �ְ� ó���Ǿ����� ��Ŷó��
bool CIocpLoginServer::OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	// Ŭ���̾�Ʈ�κ��� ���� ��Ŷ�� ��ȣȭ
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
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpLoginServer::OnRecv() | ���ǵ��� ���� ��ŶŸ��(%d), ����(%u)", nType, nLength);
		return true;
	}

	PACKET_IT it = un_mapPakect.find(static_cast<PacketType>(nType));
	if (un_mapPakect.end() != it)
		(*((*it).second))(lpConnection, dwSize, pRecvedMsg);
	else
	{
		packet_length nLength;
		CopyMemory(&nLength, pRecvedMsg, PACKET_SIZE_LENGTH);
		LOG(LOG_ERROR_CRITICAL, "SYSTEM | CIocpLoginServer::OnRecv() | un_mapPakect.end() ���ǵ��� ���� ��Ŷ(%d), ����(%u)", nType, nLength);
		if (lpConnection->m_bIsCilent)
		{
			CloseConnection(lpConnection);
		}
		return true;
	}

	return true;
}

//client���� packet�� �������� �� ���� �� ���� ��ٷ� ó�� �Ǵ� ��Ŷó��
bool CIocpLoginServer::OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg)
{
	/////////////////////////////////////////////////////////////////
	//��Ŷ�� ó���Ǹ� return true; ó�� ���� �ʾҴٸ� return false;
	return false;
}

void CIocpLoginServer::OnPrepareClose(CConnection* lpConnection)
{
	lpConnection->CloseConnection_CConn(true);
}

//client�� ������ ����Ǿ��� �� ȣ��Ǵ� �Լ�
void CIocpLoginServer::OnClose(CConnection* lpConnection)
{
	if (nullptr == lpConnection)
		return;
	//assert(lpConnection->m_bIsCilent); // ��������� ���õ�
	//exit(0);
	if (!lpConnection->m_bIsCilent)
	{ 
		puts("���Ӽ��� ����");
		lpConnection->m_bIsCilent = true;
		//exit(0);
		//system("pause");
		return;
	}

	if (((CPlayer*)lpConnection)->m_bIsNotSelectedUnit)
		DatabaseManager()->EraseID(((CPlayer*)lpConnection)->GetID());
	
	ConnectionManager()->RemoveConnection(lpConnection);

	((CPlayer*)lpConnection)->Init();

	LOG(LOG_INFO_LOW, "SYSTEM | CIocpLoginServer::OnClose() | Socket[%u] ���� ConnectionCnt %d",
		lpConnection->GetSocket(), ConnectionManager()->GetConnectionCnt());
}

bool CIocpLoginServer::OnSystemMsg(CConnection* lpConnection, LPARAM msgType, WPARAM wParam)
{
	return true;
}

bool CIocpLoginServer::ServerStart()
{
	// �α� �ʱ�ȭ
	sLogConfig LogConfig;
	_tcsncpy_s(LogConfig.s_szLogFileName, _countof(LogConfig.s_szLogFileName), _T("LoginServer"), _TRUNCATE);
	LogConfig.s_nLogInfoTypes[STORAGE_WINDOW] = LOG_ALL;
	LogConfig.s_nLogInfoTypes[STORAGE_FILE] = LOG_ERROR_ALL;
	LogConfig.s_hWnd = NULL;
	INIT_LOG(LogConfig);

	// ���� ���� �ʱ�ȭ
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
	//m_pTickThread->CreateThread(INFINITE);
	//m_pTickThread->Run();

	LOG(LOG_INFO_LOW, "Port: %u", InitConfig.nServerPort);
	LOG(LOG_INFO_LOW, "LoginServer Start");

	return true;
}


bool CIocpLoginServer::ConnectToGameServer()
{
	// �̹� �α��� ������ ������ �Ǿ��ִٸ� ���� ������ ���� �ٽ� �����Ѵ�
	if (nullptr != m_pGameServerConn)
	{
		CloseConnection(m_pGameServerConn);
		m_pGameServerConn = nullptr;
	}
	m_pGameServerConn = new(nothrow) CConnection;

	INITCONFIG initConfig;
	char		szIp[30] = "127.0.0.1";

	//������ DBAgent�� ���� ������ ���´�.
	if (-1 == (initConfig.nSendBufCnt = 2))
		return false;
	if (-1 == (initConfig.nRecvBufCnt = 2))
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
	LOG(LOG_INFO_NORMAL, "SYSTEM | CIocpLoginServer::ConnectToGameServer() | GameServer [%d]socket ���� ����"
		, m_pGameServerConn->GetSocket());

	return true;

}