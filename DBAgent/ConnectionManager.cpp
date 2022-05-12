#include "StdAfx.h"
//#include "connectionmanager.h"

IMPLEMENT_SINGLETON(CConnectionManager)

CConnectionManager::CConnectionManager()
{
	m_pConnection = NULL;
}

CConnectionManager::~CConnectionManager()
{
	if (nullptr != m_pConnection)
	{
		delete[] m_pConnection;
		m_pConnection = nullptr;
	}
}

bool CConnectionManager::CreateConnection(INITCONFIG &initConfig, DWORD dwMaxConnection)
{
	m_pConnection = new(nothrow) CConnection[dwMaxConnection];
	if (nullptr == m_pConnection) return false;
	for (int i = 0; i < (int)dwMaxConnection; i++)
	{
		initConfig.nIndex = i;
		m_pConnection[i].m_bIsCilent = false;
		if (m_pConnection[i].CreateConnection(initConfig) == false)
			return false;
	}

	return true;
}

bool CConnectionManager::AddConnection(CConnection* pConnection)
{
	CMonitor::Owner lock(m_csConnection);
	CONN_IT conn_it;
	conn_it = m_mapConnection.find(pConnection);

	//이미 접속되어 있는 연결이라면
	if (conn_it != m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnectionManager::AddConnection() | index[%d]는 이미 m_mapConnection에 있습니다.",
			pConnection->GetIndex());
		return false;
	}

	m_mapConnection.insert(CONN_PAIR(pConnection, GetTickCount()));

	return true;
}

bool CConnectionManager::RemoveConnection(CConnection* pConnection)
{
	CMonitor::Owner lock(m_csConnection);
	CONN_IT conn_it;
	conn_it = m_mapConnection.find(pConnection);
	//접속되어 있는 연결이 없는경우
	if (conn_it == m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnectionManager::RemoveConnection() | index[%d]는 m_mapConnection에 없습니다.",
			pConnection->GetIndex());
		return false;
	}
	m_mapConnection.erase(pConnection);
	return true;
}







void CConnectionManager::TestLoginSu(DWORD dwPkey)
{
	/*
	CONN_IT conn_it;


	for (conn_it = m_mapConnection.begin(); conn_it != m_mapConnection.end(); conn_it++)
	{

		CConnection* pConnection = (CConnection*)conn_it->first;
		if (NULL == pConnection)
		{
			printf("게임서버 커넥션이 널이잖아\n");
			return;
		}
		LoginPlayerSv* pLogin = (LoginPlayerSv*)pConnection->PrepareSendPacket(sizeof(LoginPlayerSv));
		if (NULL == pLogin)
		{
			printf("게임서버 커넥션이 널이잖아\n");
			return;
		}


		//pLogin->s_sType = Suses;
		pLogin->s_dwPKey = dwPkey;
		pConnection->SendPost(sizeof(LoginPlayerSv));

	}
	*/

}

void CConnectionManager::erroremptysql(DWORD dwPkey)
{
	/*
	CONN_IT conn_it;


	for (conn_it = m_mapConnection.begin(); conn_it != m_mapConnection.end(); conn_it++)
	{

		CConnection* pConnection = (CConnection*)conn_it->first;
		if (NULL == pConnection)
		{
			printf("게임서버 커넥션이 널이잖아\n");
			return;
		}
		LoginError* pError = (LoginError*)pConnection->PrepareSendPacket(sizeof(LoginError));
		if (NULL == pError)
		{
			printf("게임서버 커넥션이 널이잖아\n");
			return;
		}
		//pError->s_sType = Error;
		pError->s_dwPKey = dwPkey;
		pConnection->SendPost(sizeof(LoginError));
	}
	*/
}