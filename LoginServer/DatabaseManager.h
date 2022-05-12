#pragma once

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "1234"
#define DB_NAME "account"

//#include "StdAfx.h"

class CDatabaseManager final : public CSingleton
{
	DECLEAR_SINGLETON(CDatabaseManager)
public:
	CDatabaseManager();
	~CDatabaseManager();

	//typedef unordered_map<CConnection*, CharacterInfo*> UNMAP_USERCHA;
	//typedef UNMAP_USERCHA::iterator	  IT_USERCHA;
	//typedef pair< CConnection*, CharacterInfo*>  PAIR_USERCHA;
	//UNMAP_USERCHA m_unmapUserCha;

	// MySQL �Լ�
	void MySql_Init();
	void MySql_Close();
	void MySql_AutoTickUpdate();
	void MySql_QuickUpdate();
	// DB�Լ�

	void ConfirmID_Rq(CConnection* pConnection, char* pRecvedMsg);
	void JoinID_Req(CConnection* pConnection, char* pRecvedMsg);
	void LogoutPlayerID_Not(char* pRecvedMsg);


	void DB_ConfirmID_Rq(CConnection* pConnection, char* pRecvedMsg);
	void DB_SelectUnit_Rq(CConnection* pConnection, char* pRecvedMsg);
	//void DB_CreateUnit(CConnection* pConnection, char* pRecvedMsg);
	//void DB_DeleteUnit(CConnection* pConnection, char* pRecvedMsg);
	void Recv_LogoutPlayer_Cn(CConnection* pConnection, char* pRecvedMsg)
	{
		char szID[MAX_ID_LENGTH]; memset(szID, 0x00, sizeof(szID));
		m_pSerializer->StartDeserialize(pRecvedMsg);
		m_pSerializer->Deserialize(szID, sizeof(szID));

		EraseID(szID);
	}

	void InsertID(char _szID[])
	{
		// ID �ߺ������� ���� �߰�
		if (m_mapUserID.end() == m_mapUserID.find(_szID))
			m_mapUserID.insert(_szID);
	}

	void EraseID(char _szID[])
	{
		USERID_IT it = m_mapUserID.find(_szID);
		if (m_mapUserID.end() != it)
		{
			m_mapUserID.erase(it);

			wchar_t strUnicode[256] = { 0, };
			int nLen = MultiByteToWideChar(CP_UTF8, 0, _szID, strlen(_szID), NULL, NULL);
			MultiByteToWideChar(CP_UTF8, 0, _szID, strlen(_szID), strUnicode, nLen);

			wprintf(L"Logout: %s\n", strUnicode);
		}
		//else
			//printf("error: Logout�� �� ���̵� ����°��� �����ߴ�: %s\n", _szID);
	}

	bool FindID(char _szID[])
	{
		// �ߺ� IDüũ
		if (m_mapUserID.end() != m_mapUserID.find(_szID))
		{
			//LOG(LOG_ERROR_LOW, "FindID: �ߺ��� [%s] ���̵� �α��εǾ���. size:%d \n", _szID, m_mapUserID.size());
			// Ŭ�󿡰� ������Ŷ
			//IocpLoginServer()->CloseConnection(pConnection);
			return true;
		}
		return false;
	}

private:
	MYSQL *m_pConnection;
	MYSQL m_conn;

	CSerializer *m_pSerializer;

	typedef set<string>	  USERID_MAP;
	typedef USERID_MAP::iterator	  USERID_IT;
	USERID_MAP m_mapUserID;
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