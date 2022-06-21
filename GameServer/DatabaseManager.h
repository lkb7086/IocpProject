#pragma once
//#include "stdafx.h"

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "1234"
#define DB_NAME "colorverse"

#include <concurrent_queue.h>
//#include "GameDefine.h"
#include "DBThread.h"




struct stPlayerInfo
{
	CPlayer* pPlayer;
	DWORD dwSize;
	char* pMsg;

	stPlayerInfo() { memset(this, 0, sizeof(stPlayerInfo)); }
	stPlayerInfo(CPlayer *_pPlayer, DWORD _dwSize, char *_pMsg) : pPlayer(_pPlayer), dwSize(_dwSize), pMsg(_pMsg)
	{
	}
};


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
	void StartLobby_Not(const stPlayerInfo& info);
	void CreateCharacter_Req(const stPlayerInfo& info);
	void DeleteCharacter_Req(const stPlayerInfo& info);
	void StartGame_Req(const stPlayerInfo& info);
	void LogoutPlayerDB_Not(const stPlayerInfo& info);


	// ppl queue
	void PushDBQueue(CPlayer *_pPlayer, DWORD _dwSize, char *_pMsg)
	{
		//if (false == _pPlayer->GetIsConfirm())
			//return;
		m_queueDB.push(stPlayerInfo(_pPlayer, _dwSize, _pMsg));
		m_pDBThread->SetEventDBThread();
	}

	void Destroy_DBThread()
	{
		m_pDBThread->DestroyThread();
	}

	void PopDBQueue()
	{
		stPlayerInfo info;
		while (m_queueDB.try_pop(info))
		{
			//if (0 == info.playerKey)
			//{
				//memset(&info, 0, sizeof(stPlayerInfo));
				//continue;
			//}

			if (nullptr != info.pPlayer && nullptr != info.pMsg)
			{
				int nType = -1;
				memcpy_s(&nType, sizeof(nType), info.pMsg + PACKET_SIZE_LENGTH, PACKET_TYPE_LENGTH);
				IocpGameServer()->OnRecv(info.pPlayer, info.dwSize, info.pMsg);
			}
			memset(&info, 0, sizeof(stPlayerInfo));
		}
	}

private:
	MYSQL *m_pConnection;
	MYSQL m_conn;
private:
	concurrent_queue<stPlayerInfo> m_queueDB;
	CDBThread* m_pDBThread;
	CRingBuffer m_ringBuffer;

	

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
