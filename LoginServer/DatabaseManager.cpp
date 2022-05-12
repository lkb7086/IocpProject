#include "stdafx.h"

IMPLEMENT_SINGLETON(CDatabaseManager)

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 생성자 라인
CDatabaseManager::CDatabaseManager() : m_pConnection(nullptr)
{
	m_pSerializer = new CSerializer();
	MySql_Init();
}
CDatabaseManager::~CDatabaseManager()
{
	MySql_Close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// MySQL 함수
void CDatabaseManager::MySql_Init()
{
	// 버전체크
	LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::MySql_Init() | MySQL client version: %s", mysql_get_client_info());

	// 초기화
	if (nullptr == mysql_init(&m_conn))
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::MySql_Init() | mysql_init() is NULL");
		return;
	}

	// DB 연결
	m_pConnection = mysql_real_connect(&m_conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)nullptr, 0);
	if (nullptr == m_pConnection)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::MySql_Init() | mysql_real_connect() is NULL: %s", mysql_error(&m_conn));
		return;
	}

	// 쿼리: 한글사용을위해추가
	mysql_query(m_pConnection, "set session character_set_connection=utf8;");
	mysql_query(m_pConnection, "set session character_set_results=utf8;");
	mysql_query(m_pConnection, "set session character_set_client=utf8;");
}
void CDatabaseManager::MySql_Close()
{
	// DB 연결닫기
	mysql_close(m_pConnection);
}
void CDatabaseManager::MySql_AutoTickUpdate()
{
}
void CDatabaseManager::MySql_QuickUpdate()
{
}

void CDatabaseManager::ConfirmID_Rq(CConnection* pConnection, char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char szPass[MAX_PASS_LENGTH]; memset(szPass, 0, sizeof(szPass));
	unsigned char result = 0;

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(szPass, sizeof(szPass));

	// sql 인젝션 검사, '있는지 확인할 것


	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "select id from user where id = '%s' and pass = '%s'", szID, szPass);

	int	state = -1;
	state = mysql_query(m_pConnection, query);
	if (0 != state)
	{
		LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::ConfirmID_Rq() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		result = 1;
	}

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	state = mysql_num_rows(pResult);
	if (state == 0)
	{
		result = 1;
	}
	else
	{
		// 중복 ID체크
		if (FindID(szID))
		{
			result = 2;
		}
		else
		{
			InsertID(szID);

			// 게임서버로 ID를 전달
			m_pSerializer->StartSerialize();
			m_pSerializer->Serialize(static_cast<packet_type>(PacketType::ConfirmID_Not));
			m_pSerializer->Serialize(szID);

			CConnection* pGameSerserConection = ConnectionManager()->GetServerConn();
			if (nullptr == pGameSerserConection) return;
			char* pBuffer = pGameSerserConection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
			if (nullptr != pBuffer)
			{
				m_pSerializer->CopyBuffer(pBuffer);
				pGameSerserConection->SendPost(m_pSerializer->GetCurBufSize());
				//return;
			}
			else
				LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::ConfirmID_Rq() | nullptr == pGameSerserConection");

			//m_pSerializer->CopyBuffer(pBuffer);
			//pGameSerserConection->SendPost(m_pSerializer->GetCurBufSize());
		}
	}


	// result: 1 = 아이디나 패스워드가 없음
    // result: 2 = 중복된 로그인 아이디
	m_pSerializer->StartSerialize();
	m_pSerializer->Serialize(static_cast<packet_type>(PacketType::ConfirmID_Res));
	m_pSerializer->Serialize(result);
	m_pSerializer->Serialize(szID);
	char* pWorldPlayerInfos = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pWorldPlayerInfos) return;
	m_pSerializer->CopyBuffer(pWorldPlayerInfos);
	pConnection->SendPost(m_pSerializer->GetCurBufSize());
}

void CDatabaseManager::JoinID_Req(CConnection* pConnection, char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char szPass[MAX_PASS_LENGTH]; memset(szPass, 0, sizeof(szPass));
	unsigned char result = 0;

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(szPass, sizeof(szPass));


	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "select id from user where id = '%s'", szID);

	int	state = -1;
	state = mysql_query(m_pConnection, query);
	if (0 != state)
	{
		LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::JoinID_Req() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	state = mysql_num_rows(pResult);
	if (state == 0)
	{
		// 겹치는 아이디가 없으니 insert
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO user (id, pass, date) VALUES ('%s', '%s', now());", szID, szPass);
		state = mysql_query(m_pConnection, query);
		if (0 != state)
		{
			LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::JoinID_Req() | 2 mysql_query() 에러: %s", mysql_error(m_pConnection));
			result = 2;
		}
	}
	else
	{
		result = 1;
	}


	// 클라이언트에 결과값 전달
	// result: 1 = 중복된 아이디
	m_pSerializer->StartSerialize();
	m_pSerializer->Serialize(static_cast<packet_type>(PacketType::JoinID_Res));
	m_pSerializer->Serialize(result);
	char* pWorldPlayerInfos = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pWorldPlayerInfos) return;
	m_pSerializer->CopyBuffer(pWorldPlayerInfos);
	pConnection->SendPost(m_pSerializer->GetCurBufSize());

}




void CDatabaseManager::LogoutPlayerID_Not(char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));

	if (FindID(szID))
	{
		EraseID(szID);
	}
}


















void CDatabaseManager::DB_ConfirmID_Rq(CConnection* pConnection, char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char szPass[MAX_PASS_LENGTH]; memset(szPass, 0, sizeof(szPass));

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(szPass, sizeof(szPass));

	// sql 인젝션 검사, '있는지 확인할 것

	// 중복 ID체크
	if (FindID(szID))
		return;

	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "select uid from account where id = '%s' and pass = '%s'", szID, szPass);

	int	nState = -1;
	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		IocpLoginServer()->CloseConnection(pConnection);
		return;
	}

	MYSQL_RES* result = nullptr;
	result = mysql_store_result(m_pConnection);   // mysql_use_result
	if (nullptr == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		IocpLoginServer()->CloseConnection(pConnection);
		return;
	}

	nState = mysql_num_rows(result);
	unsigned int nAccx_UID = 0xffffffff;

	if (0 == nState)     // 디비에 계정이 없으면
	{
		mysql_free_result(result);
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);

		//printf("DB_LoginCheck: 디비에 계정이나 비밀번호가 없습니다. \n");
		// 계정이 없으면 자동으로 만든다
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO account (id, pass, date) VALUES ('%s', '%s', now());", szID, szPass);
		nState = mysql_query(m_pConnection, query);

		// account uid 탐색
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT uid FROM account ORDER BY uid DESC LIMIT 1");
		nState = mysql_query(m_pConnection, query);
		result = mysql_store_result(m_pConnection);
		MYSQL_ROW row = nullptr;
		row = mysql_fetch_row(result);
		nAccx_UID = atoi(row[0]);
		mysql_free_result(result);
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);

		//int nRand = rand() % 2;
		int nRand = 0;

		// insert 캐릭터2개
		memset(query, 0, sizeof(query));
		//_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO unit (`accox_uid`, `name`, `class`, `area`, `x`, `y`, 'date') VALUES ('%u', '%s1', '%u', '41', '-35.0', '3.0', now());",
			//nAccx_UID, szID, 0);
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO unit VALUES (null, %u, '%s1', %u, 41, -8.5, -12.5, 0, 0, now());",
			nAccx_UID, szID, nRand);
		nState = mysql_query(m_pConnection, query);
		if (0 != nState) puts("1 캐릭터를 생성하지 못 하였다");

		/*
		while (true)
		{
			int nTemp = rand() % 2;
			if (nRand == nTemp)
				continue;
			nRand = nTemp;
			break;
		}
		*/
		nRand = 1;

		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO unit VALUES (null, %u, '%s2', %u, 41, -8.5, -12.5, 0, 0, now());",
			nAccx_UID, szID, nRand);
		nState = mysql_query(m_pConnection, query);
		if (0 != nState) puts("2 캐릭터를 생성하지 못 하였다");

		goto jump;
		return;
	}
	else if (0 < nState)    // 디비에 계정이 있으면
	{
		MYSQL_ROW row = nullptr;
		row = mysql_fetch_row(result);

		if (nullptr == row)
		{
			printf("DB_LoginCheck: nullptr == row 1 \n");
			return;
		}
		nAccx_UID = atoi(row[0]);

		row = nullptr;
		mysql_free_result(result);
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);

	jump:

		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "select name, class from unit where accox_uid = %u", nAccx_UID);

		nState = mysql_query(m_pConnection, query);
		if (0 != nState)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 2 mysql_query() 에러: %s", mysql_error(m_pConnection));
			IocpLoginServer()->CloseConnection(pConnection);
			return;
		}

		result = mysql_store_result(m_pConnection);
		CResultEraser eraser(result, m_pConnection);
		if (nullptr == result)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 2 mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
			IocpLoginServer()->CloseConnection(pConnection);
			return;
		}

		// iState이 0이면 캐릭터가 없는거다. 오류가 아니다.
		nState = mysql_num_rows(result);
		if (0 == nState)
		{
			m_pSerializer->StartSerialize();
			//m_pSerializer->Serialize(LS_CL_ConfirmID_Aq);
			m_pSerializer->Serialize(0);
			char* pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
			if (nullptr == pBuffer)
			{
				LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 PrepareSendPacket() is NULL");
				IocpLoginServer()->CloseConnection(pConnection);
				return;
			}
			m_pSerializer->CopyBuffer(pBuffer);
			pConnection->SendPost(m_pSerializer->GetCurBufSize());

			printf("DB_LoginCheck: 디비에 %s의 계정만 있고 캐릭터가 없습니다. \n", szID);
			goto jump_empty;
			return;
		}

		row = mysql_fetch_row(result);
		if (nullptr == row)
		{
			printf("DB_LoginCheck: nullptr == row 2 \n");
			return;
		}
		
		m_pSerializer->StartSerialize();
		//m_pSerializer->Serialize(LS_CL_ConfirmID_Aq);
		m_pSerializer->Serialize(nState);

		for (int i = 0; i < nState; i++)
		{
			m_pSerializer->Serialize(row[0]); // name
			m_pSerializer->Serialize((unsigned short)atoi(row[1])); // class
			row = mysql_fetch_row(result);
			if (nullptr == row)
				break;
		}

		char* pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
		if (nullptr == pBuffer)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 PrepareSendPacket() is NULL");
			IocpLoginServer()->CloseConnection(pConnection);
			return;
		}
		m_pSerializer->CopyBuffer(pBuffer);
		pConnection->SendPost(m_pSerializer->GetCurBufSize());
	}

jump_empty:
	((CPlayer*)pConnection)->SetID(szID);
	((CPlayer*)pConnection)->SetAccox_UID(nAccx_UID);
	// ID 중복방지를 위해 추가
	InsertID(szID);

	wchar_t strUnicode[256] = { 0, };
	int nLen = MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), strUnicode, nLen);
	
	wprintf(L"Login: %s\n", strUnicode);
}

void CDatabaseManager::DB_SelectUnit_Rq(CConnection* pConnection, char* pRecvedMsg)
{
	char szName[MAX_NICKNAME_LENGTH]; memset(szName, 0x00, sizeof(szName));
	USHORT userClass = 0xFFFF;
	UINT LSPkey = IocpLoginServer()->GeneratePrivateKey();
	((CPlayer*)pConnection)->SetLSKey(LSPkey);
	((CPlayer*)pConnection)->m_bIsNotSelectedUnit = false;
	ConnectionManager()->AddConnection(pConnection);

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szName, sizeof(szName));
	m_pSerializer->Deserialize(userClass);

	m_pSerializer->StartSerialize();
	//m_pSerializer->Serialize(LS_GS_MoveServer_Rq);
	m_pSerializer->Serialize(LSPkey);
	m_pSerializer->Serialize(((CPlayer*)pConnection)->GetAccox_UID());
	m_pSerializer->Serialize(szName);
	m_pSerializer->Serialize(userClass);

	CConnection* pGS_Conn = ConnectionManager()->GetServerConn();
	if (nullptr == pGS_Conn) return;
	char* pGS_Buffer = pGS_Conn->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pGS_Buffer)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SelectUnit_Rq() | nullptr == pGS_Buffer");
		return;
	}
	m_pSerializer->CopyBuffer(pGS_Buffer);
	pGS_Conn->SendPost(m_pSerializer->GetCurBufSize());

	/*



	//
	m_pSerializer->StartSerialize();
	m_pSerializer->Serialize(LS_CL_SelectUnit_Aq);
	m_pSerializer->Serialize(LSPkey);
	
	char* pCL_Buffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pCL_Buffer)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SelectUnit_Rq() | nullptr == pCL_Buffer");
		return;
	}
	m_pSerializer->CopyBuffer(pCL_Buffer);
	pConnection->SendPost(m_pSerializer->GetCurBufSize());

	((CPlayer*)pConnection)->SetIsConfirm(true);*/
}

/*
void CDatabaseManager::DB_CreateUnit(CConnection* pConnection, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	CreateUnit_Rq* pCreate = (CreateUnit_Rq*)pRecvedMsg;

	CDatabaseManager::IT_USERCHA it = DatabaseManager()->m_unmapUserCha.find(pConnection);
	CharacterInfo* pCharac = it->second;

	// 캐릭터를 만드는 쿼리, 여기서는 간단하게하고, 나머지 설정은 프로시저로
	// 쿼리 매개변수: 계정, 닉네임, 클래스
	sprintf(query, "call CreateUnit('%s', '%s', %d);",
		pCharac[0].s_szId, pCreate->s_szNick, pCreate->s_iClass);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_CreateUnit() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		IocpLoginServer()->CloseConnection(pConnection);
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		return;
	}


	while (mysql_more_results(m_pConnection))
		mysql_next_result(m_pConnection);
}

void CDatabaseManager::DB_DeleteUnit(CConnection* pConnection, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	CreateUnit_Rq* pDelete = (CreateUnit_Rq*)pRecvedMsg;

	CDatabaseManager::IT_USERCHA it = DatabaseManager()->m_unmapUserCha.find(pConnection);
	CharacterInfo* pCharac = it->second;

	// 캐릭터를 만드는 쿼리, 여기서는 간단하게하고, 나머지 설정은 프로시저로
	// 쿼리 매개변수: 계정, 닉네임, 클래스
	sprintf(query, "delete from testcharacters where userid = '%s' and name = '%s'",
		pCharac[0].s_szId, pDelete->s_szNick);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_CreateUnit() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		IocpLoginServer()->CloseConnection(pConnection);
		return;
	}
}
*/