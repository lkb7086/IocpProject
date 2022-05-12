#pragma once
//#include "StdAfx.h"

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "1234"
#define DB_NAME "worlddb"

#define MAX_DEFAULT_ITEM 100
#define MAXUSERLEVEL 10

class CDatabaseManager final : public CSingleton
{
	DECLEAR_SINGLETON(CDatabaseManager)
public:
	CDatabaseManager();
    ~CDatabaseManager();

	// MySQL 함수
	void MySql_Init();
	void MySql_Close();
	void MySql_AutoTickUpdate();
	void MySql_QuickUpdate();
	// DB함수
	void DB_SearchUnit_Rq(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_LogoutPlayer_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_InsertDropItem_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_MerseDropItem_Cn(CConnection *_pConn, DWORD _dwSize, char *_pRecvMsg);
	void DB_DeleteItem_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	/*
	void DB_AllSearchMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);  // 플레이어가 거래소의 전체검색을 요청하는 함수
	void DB_SearchTextFromMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);   // 플레이어가 거래소의 문자열이나 부분문자열 검색을 요청하는 함수
	void DB_SearchInven(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_DoPurchaseItemInMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_CL_DA_DoInsertItemToMarket_Rq(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	void DB_DA_GS_LogoutPlayer_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg);
	*/
private:
	typedef unordered_map<string, unsigned int>	  UN_MAP_USERID;
	typedef UN_MAP_USERID::iterator	  USERID_IT;
	UN_MAP_USERID m_unmapUserID;
	CSerializer  *m_pSerializer;

	// MySQL 변수
	MYSQL *m_pConnection;
	MYSQL m_Conn;
private:
	class CResultEraser final
	{
	public:
		explicit CResultEraser(MYSQL_RES *_pResult, MYSQL *_pConnection) : m_pResult(_pResult), m_pConnection(_pConnection)
		{
		}
		~CResultEraser()
		{
			if (nullptr != m_pResult)
				mysql_free_result(m_pResult);
			while (mysql_more_results(m_pConnection))
				mysql_next_result(m_pConnection);
		}
	private:
		MYSQL_RES *m_pResult;
		MYSQL *m_pConnection;
		CResultEraser(const CResultEraser &rhs);
		CResultEraser& operator=(const CResultEraser &rhs);
	};
};
CREATE_FUNCTION(CDatabaseManager, DatabaseManager);