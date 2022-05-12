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

	//�̹� ���ӵǾ� �ִ� �����̶��
	if (conn_it != m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnectionManager::AddConnection() | index[%d]�� �̹� m_mapConnection�� �ֽ��ϴ�.",
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
	//���ӵǾ� �ִ� ������ ���°��
	if (conn_it == m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnectionManager::RemoveConnection() | index[%d]�� m_mapConnection�� �����ϴ�.",
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
			printf("���Ӽ��� Ŀ�ؼ��� �����ݾ�\n");
			return;
		}
		LoginPlayerSv* pLogin = (LoginPlayerSv*)pConnection->PrepareSendPacket(sizeof(LoginPlayerSv));
		if (NULL == pLogin)
		{
			printf("���Ӽ��� Ŀ�ؼ��� �����ݾ�\n");
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
			printf("���Ӽ��� Ŀ�ؼ��� �����ݾ�\n");
			return;
		}
		LoginError* pError = (LoginError*)pConnection->PrepareSendPacket(sizeof(LoginError));
		if (NULL == pError)
		{
			printf("���Ӽ��� Ŀ�ؼ��� �����ݾ�\n");
			return;
		}
		//pError->s_sType = Error;
		pError->s_dwPKey = dwPkey;
		pConnection->SendPost(sizeof(LoginError));
	}
	*/
}