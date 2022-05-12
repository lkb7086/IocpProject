#pragma once

class CConnectionManager final : public CSingleton
{
	DECLEAR_SINGLETON(CConnectionManager)
public:
	CConnectionManager();
    ~CConnectionManager();

	typedef pair< CConnection*, DWORD >  CONN_PAIR;
	typedef map< CConnection*, DWORD >   CONN_MAP;
	typedef CONN_MAP::iterator	 CONN_IT;

	bool CreateConnection(INITCONFIG &initConfig, DWORD dwMaxConnection);
	bool AddConnection(CConnection* pConnection);
	bool RemoveConnection(CConnection* pConnection);
	void TestLoginSu(DWORD dwPkey); //
	void erroremptysql(DWORD dwPkey);
	inline int __fastcall	GetConnectionCnt() { return (int)m_mapConnection.size(); }
protected:
	CONN_MAP		m_mapConnection;
	CConnection*	m_pConnection;
	CMonitor		m_csConnection;
};
CREATE_FUNCTION(CConnectionManager, ConnectionManager)