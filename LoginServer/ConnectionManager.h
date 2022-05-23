#pragma once
#include "Player.h"

class CConnectionManager final : public CSingleton
{
	DECLEAR_SINGLETON(CConnectionManager)
public:
	CConnectionManager();
	~CConnectionManager();

	typedef pair< unsigned long long, CConnection* >  CONN_PAIR;
	typedef map< unsigned long long, CConnection* >   CONN_MAP;
	typedef CONN_MAP::iterator	 CONN_IT;

	bool CreateConnection(INITCONFIG &initConfig, DWORD dwMaxConnection);
	bool AddConnection(CConnection* pConnection);
	bool RemoveConnection(CConnection* pConnection);
	CConnection* FindConnection(unsigned int _nLSPKey);

	inline int	GetConnectionCnt() { return (int)m_mapConnection.size(); }
	inline CConnection*  GetServerConn() { return m_pServerConn; }
	inline void SetServerConn(CConnection* serverConn) {  m_pServerConn = serverConn; }

	void AddServerCon(char key, CConnection* pConnection)
	{
		auto it = m_mapServerCon.find(key);
		if (it == m_mapServerCon.end())
		{
			m_mapServerCon.insert(pair<char, CConnection*>(key, pConnection));
		}
	}

	void RemoveServerCon(CConnection* pConnection)
	{
		auto it = m_mapServerCon.find(((CPlayer*)pConnection)->m_serverID);
		if (it != m_mapServerCon.end())
		{
			m_mapServerCon.erase(((CPlayer*)pConnection)->m_serverID);
		}
	}

	CConnection* GetServerConn(char serverKey)
	{
		auto it = m_mapServerCon.find(serverKey);
		if (it != m_mapServerCon.end())
		{
			return it->second;
		}
		else
			return nullptr;
	}

private:
	CONN_MAP		m_mapConnection;
	map<char, CConnection*> m_mapServerCon;
	CPlayer*	    m_pConnection;
	CConnection*	m_pServerConn;
	CMonitor		m_csConnection;

};
CREATE_FUNCTION(CConnectionManager, ConnectionManager)