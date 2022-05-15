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


private:
	CONN_MAP		m_mapConnection;
	CPlayer*	    m_pConnection;
	CConnection*	m_pServerConn;
	CMonitor		m_csConnection;

};
CREATE_FUNCTION(CConnectionManager, ConnectionManager)