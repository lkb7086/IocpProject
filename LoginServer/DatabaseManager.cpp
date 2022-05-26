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

void CDatabaseManager::ConfirmID_Req(CConnection* pConnection, char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char szPass[MAX_PASS_LENGTH]; memset(szPass, 0, sizeof(szPass));
	char result = 0;

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(szPass, sizeof(szPass));

	// sql인젝션 특수문자 들어가있으면
	char* pSearch = strchr(szID, ';');
	if (pSearch != nullptr)
		return;

	CSHA512 hash;
	const string& hash2 = hash.sha512(szPass);

	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "select serverid from user where id = '%s' and pass = '%s'", szID, hash2.c_str());

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

	/*
	// 중복 ID체크
	if (FindID(szID))
	{
		result = 2;
	}
	else
	{
		InsertID(szID);
	}
	*/

	if (result == 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pResult);
		if (row == nullptr)
			return;
		((CPlayer*)pConnection)->m_serverID = (char)atoi(row[0]);
		//((CPlayer*)pConnection)->m_serverID = 0;


		m_pSerializer->StartSerialize();
		m_pSerializer->Serialize(static_cast<packet_type>(PacketType::ConfirmIDGameServer_Req));
		m_pSerializer->Serialize(result);
		m_pSerializer->Serialize(((CPlayer*)pConnection)->GetLSKey());
		m_pSerializer->Serialize(szID);

		CConnection* pGameSerserConection = ConnectionManager()->GetServerConn(((CPlayer*)pConnection)->m_serverID);
		if (nullptr == pGameSerserConection)
			return;
		char* pBuffer = pGameSerserConection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
		if (nullptr == pBuffer)
			return;
		m_pSerializer->CopyBuffer(pBuffer);
		pGameSerserConection->SendPost(m_pSerializer->GetCurBufSize());
	}
	else
	{
		m_pSerializer->StartSerialize();
		m_pSerializer->Serialize(static_cast<packet_type>(PacketType::ConfirmID_Res));
		m_pSerializer->Serialize(result);
		char* pBuf = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
		if (nullptr == pBuf)
			return;
		m_pSerializer->CopyBuffer(pBuf);
		pConnection->SendPost(m_pSerializer->GetCurBufSize());
	}


	/*
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
	*/
}

void CDatabaseManager::JoinID_Req(CConnection* pConnection, char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char szPass[MAX_PASS_LENGTH]; memset(szPass, 0, sizeof(szPass));
	char result = 0;

	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(szPass, sizeof(szPass));

	// 빈 문자열이면
	if (strcmp(szID, "") == 0)
		return;

	// sql인젝션 특수문자 들어가있으면
	char* pSearch = strchr(szID, ';');
	if (pSearch != nullptr)
		return;

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
		CSHA512 hash;
		const string& hash2 = hash.sha512(szPass);


		// 겹치는 아이디가 없으니 insert
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO user (id, pass, date) VALUES ('%s', '%s', now());", szID, hash2.c_str());
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
	m_pSerializer->Serialize(szID);
	char* pWorldPlayerInfos = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pWorldPlayerInfos) return;
	m_pSerializer->CopyBuffer(pWorldPlayerInfos);
	pConnection->SendPost(m_pSerializer->GetCurBufSize());
}

void CDatabaseManager::ConfirmIDGameServer_Res(char* pMsg)
{
	unsigned char result = 0;
	unsigned long long key = 0;
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));

	m_pSerializer->StartDeserialize(pMsg);
	m_pSerializer->Deserialize(result);
	m_pSerializer->Deserialize(key);
	m_pSerializer->Deserialize(szID, sizeof(szID));

	CConnection* con = ConnectionManager()->FindConnection(key);
	if (con == nullptr)
		return;

	m_pSerializer->StartSerialize();
	m_pSerializer->Serialize(static_cast<packet_type>(PacketType::ConfirmID_Res));
	m_pSerializer->Serialize(result);
	m_pSerializer->Serialize(key);
	m_pSerializer->Serialize(szID);
	m_pSerializer->Serialize(((CPlayer*)con)->m_serverID);
	char* pBuf = con->PrepareSendPacket(m_pSerializer->GetCurBufSize());
	if (nullptr == pBuf)
		return;
	m_pSerializer->CopyBuffer(pBuf);
	con->SendPost(m_pSerializer->GetCurBufSize());
}


void CDatabaseManager::LogoutPlayerID_Not(char* pRecvedMsg)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));
	char serverID = -1;
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(szID, sizeof(szID));
	m_pSerializer->Deserialize(serverID);
	
	EraseID(szID);

	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "UPDATE user SET serverid = %d WHERE id = '%s'", serverID, szID);

	int	state = -1;
	state = mysql_query(m_pConnection, query);
	if (0 != state)
	{
		LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::LogoutPlayerID_Not() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
}

void CDatabaseManager::MoveServer_Not1(char* pRecvedMsg, DWORD size)
{
	char nextServerID = -1; char prevServerID = -1;
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(nextServerID);

	CConnection* pServerCon = ConnectionManager()->GetServerConn(nextServerID);
	if (pServerCon == nullptr)
		return;
	char* pBuffer = pServerCon->PrepareSendPacket(size);
	if (pBuffer == nullptr)
		return;
	memcpy_s(pBuffer, size, pRecvedMsg, size);
	pServerCon->SendPost(size);
}

void CDatabaseManager::MoveServer_Not2(char* pRecvedMsg, DWORD size)
{
	char prevServerID = -1;
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(prevServerID);

	CConnection* pServerCon = ConnectionManager()->GetServerConn(prevServerID);
	if (pServerCon == nullptr)
		return;
	char* pBuffer = pServerCon->PrepareSendPacket(size);
	if (pBuffer == nullptr)
		return;
	memcpy_s(pBuffer, size, pRecvedMsg, size);
	pServerCon->SendPost(size);
}




























/*
	wchar_t strUnicode[256] = { 0, };
	int nLen = MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), strUnicode, nLen);

	wprintf(L"Login: %s\n", strUnicode);
*/
