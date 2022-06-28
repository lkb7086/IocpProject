#include "stdafx.h"
//#include "connectionmanager.h"

IMPLEMENT_SINGLETON(CConnectionManager)

CConnectionManager::CConnectionManager() : m_pConnection(nullptr), m_pServerConn(nullptr)
{
}

CConnectionManager::~CConnectionManager()
{
	if (nullptr != m_pConnection)
	{
		delete[] m_pConnection;
		m_pConnection = nullptr;
	}

	if (nullptr != m_pServerConn)
	{
		delete[] m_pServerConn;
		m_pServerConn = nullptr;
	}
}

bool CConnectionManager::CreateConnection(INITCONFIG &initConfig, DWORD dwMaxConnection)
{
	// Ŭ���̾�Ʈ��
	m_pConnection = new(nothrow)CPlayer[dwMaxConnection];
	if (nullptr == m_pConnection) return false;
	for (int i = 0; i < (int)dwMaxConnection; i++)
	{
		initConfig.nIndex = i;
		if (m_pConnection[i].CreateConnection(initConfig) == false)
			return false;
	}

	/*
	// ������
	m_pServerConn = new(nothrow)CConnection[1];
	if (nullptr == m_pServerConn) return false;
	m_pServerConn->m_bIsCilent = false;
	initConfig.nIndex = 0;
	if (m_pServerConn->CreateConnection(initConfig) == false)
		return false;
		*/

	return true;
}

bool CConnectionManager::AddConnection(CConnection* pConnection)
{
	CONN_IT conn_it = m_mapConnection.find(((CPlayer*)pConnection)->GetLSKey());
	//�̹� ���ӵǾ� �ִ� �����̶��
	if (conn_it != m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CConnectionManager::AddConnection() | index[%d]�� �̹� m_mapConnection�� �ֽ��ϴ�.", pConnection->GetIndex());
		return false;
	}

	((CPlayer*)pConnection)->SetLSKey(IocpLoginServer()->GeneratePrivateKey());
	m_mapConnection.insert(CONN_PAIR(((CPlayer*)pConnection)->GetLSKey(), pConnection));

	return true;
}

bool CConnectionManager::RemoveConnection(CConnection* pConnection)
{
	CONN_IT conn_it = m_mapConnection.find(((CPlayer*)pConnection)->GetLSKey());
	//���ӵǾ� �ִ� ������ ���°��
	if (conn_it == m_mapConnection.end())
	{
		LOG(LOG_ERROR_LOW,
			"SYSTEM | CConnectionManager::RemoveConnection() | index[%d]�� m_mapConnection�� �����ϴ�. size: %u",
			pConnection->GetIndex(), m_mapConnection.size());
		return false;
	}
	m_mapConnection.erase(((CPlayer*)pConnection)->GetLSKey());
	return true;
}

CConnection* CConnectionManager::FindConnection(unsigned int _nLSPKey)
{
	CONN_IT conn_it = m_mapConnection.find(_nLSPKey);
	if (conn_it == m_mapConnection.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CConnectionManager::FindConnection() | LSKey(%d)�� m_mapConnection�� �����ϴ�.", _nLSPKey);
		return nullptr;
	}
	return (CConnection*)conn_it->second;
}