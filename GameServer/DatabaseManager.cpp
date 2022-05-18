#include "stdafx.h"
//#include "DatabaseManager.h"

IMPLEMENT_SINGLETON(CDatabaseManager)

CDatabaseManager::CDatabaseManager() : m_pConnection(nullptr), m_pDBThread(nullptr)
{
	MySql_Init();

	m_ringBuffer.Create(10240);

	m_pDBThread = new(nothrow)CDBThread();
	if (nullptr == m_pDBThread) return;
}
CDatabaseManager::~CDatabaseManager()
{
	if (nullptr != m_pDBThread)
	{
		delete m_pDBThread;
		m_pDBThread = nullptr;
	}

	MySql_Close();
}

void CDatabaseManager::MySql_Init()
{
	///*
	// 버전체크
	//LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::MySql_Init() | MySQL client version: %s", mysql_get_client_info());

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
	//
}
void CDatabaseManager::MySql_Close()
{
	// DB 연결닫기
	///*
	if (nullptr != m_pConnection)
	{
		mysql_close(m_pConnection);
		m_pConnection = nullptr;
	}
	//
}
void CDatabaseManager::MySql_AutoTickUpdate()
{
}
void CDatabaseManager::MySql_QuickUpdate()
{
}




void CDatabaseManager::StartLobby_Not(const stPlayerInfo& info)
{
	char szID[MAX_ID_LENGTH]; memset(szID, 0, sizeof(szID));

	tls_pSer->StartDeserialize(info.pMsg);
	tls_pSer->Deserialize(szID, sizeof(szID));

	// 디비에서 캐릭터들을 참조해서 뽑아내서 보내준다
	char query[256]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "select * from coloverse.character where id = '%s'", szID);

	int state = mysql_query(m_pConnection, query);

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	if (0 != state)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::StartLobby_Not() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	state = mysql_num_rows(pResult);
	if (0 == state)
	{
		// 캐릭터가 아무것도 없으면 아무것도 안한다
	}
	else if (state > 0)
	{
		MYSQL_ROW row = mysql_fetch_row(pResult);
		if (row == nullptr)
			return;

		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::StartLobby_Res));
		tls_pSer->Serialize((char)state);

		int index = 0;
		for (int i = 0; i < state; i++)
		{
			// uid(bigint) id(varchar45) name(varchar45) index(tinyint) species(tinyint) gender(tinyint)
			index++;
			index++;
			tls_pSer->Serialize(row[index++]);
			tls_pSer->Serialize((char)atoi(row[index++]));
			tls_pSer->Serialize((char)atoi(row[index++]));
			tls_pSer->Serialize((char)atoi(row[index]));
			row = mysql_fetch_row(pResult);
			if (nullptr == row)
				break;
			index = 0;
		}

		char* pBuffer = info.pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
		if (nullptr == pBuffer)
			return;		
		tls_pSer->CopyBuffer(pBuffer);
		info.pPlayer->SendPost(tls_pSer->GetCurBufSize());
	}

	//CResultEraser eraser2(result, m_pConnection);
}

void CDatabaseManager::CreateCharacter_Req(const stPlayerInfo& info)
{
	char name[MAX_NICKNAME_LENGTH]; memset(name, 0, sizeof(name));
	char index = 0;
	char species = 0;
	char gender = 0;
	char result = 0;

	tls_pSer->StartDeserialize(info.pMsg);
	tls_pSer->Deserialize(name, sizeof(name));
	tls_pSer->Deserialize(index);
	tls_pSer->Deserialize(species);
	tls_pSer->Deserialize(gender);

	char query[128]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT * FROM character WHERE name = '%s'", name);

	int state = mysql_query(m_pConnection, query);

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	if (0 != state)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::CreateCharacter_Req() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::CreateCharacter_Res));

	state = mysql_num_rows(pResult);
	if (0 == state)
	{
		// 캐릭터가 있으면 실패
		result = 1;
		tls_pSer->Serialize((char)result);
	}
	else
	{
		// 캐릭터가 없으면 INSERT
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO character VALUES (NULL, '%s', '%s', %d, %d, %d)", info.pPlayer->GetID(), name, index, species, gender);
		state = mysql_query(m_pConnection, query);
		if (0 != state)
		{
			LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::CreateCharacter_Req() | 2 mysql_query() 에러: %s", mysql_error(m_pConnection));
			return;
		}

		tls_pSer->Serialize(result);
		tls_pSer->Serialize(name);
		tls_pSer->Serialize(index);
		tls_pSer->Serialize(species);
		tls_pSer->Serialize(gender);
	}

	char* pBuffer = info.pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	info.pPlayer->SendPost(tls_pSer->GetCurBufSize());
}

void CDatabaseManager::DeleteCharacter_Req(const stPlayerInfo& info)
{
	char name[MAX_NICKNAME_LENGTH]; memset(name, 0, sizeof(name));
	char result = 0;

	tls_pSer->StartDeserialize(info.pMsg);
	tls_pSer->Deserialize(name, sizeof(name));

	char query[128]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT * FROM character WHERE name = '%s'", name);

	int state = mysql_query(m_pConnection, query);

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	if (0 != state)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DeleteCharacter_Req() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::DeleteCharacter_Res));

	state = mysql_num_rows(pResult);
	if (state == 0)
	{
		// 삭제할 캐릭터가 없어서 실패
		result = 1;
		tls_pSer->Serialize(result);
	}
	else
	{
		// 삭제
		memset(query, 0, sizeof(query));
		_snprintf_s(query, _countof(query), _TRUNCATE, "DELETE FROM character WHERE name = '%s'", name);
		state = mysql_query(m_pConnection, query);
		if (0 != state)
		{
			LOG(LOG_INFO_LOW, "SYSTEM | CDatabaseManager::DeleteCharacter_Req() | 2 mysql_query() 에러: %s", mysql_error(m_pConnection));
			return;
		}

		tls_pSer->Serialize(result);
		tls_pSer->Serialize(name);
	}

	char* pBuffer = info.pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	info.pPlayer->SendPost(tls_pSer->GetCurBufSize());
}

void CDatabaseManager::StartGame_Req(const stPlayerInfo& info)
{
	char name[MAX_NICKNAME_LENGTH]; memset(name, 0, sizeof(name));
	char result = 0;

	tls_pSer->StartDeserialize(info.pMsg);
	tls_pSer->Deserialize(name, sizeof(name));

	char query[128]; memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT * FROM character WHERE name = '%s'", name);

	int state = mysql_query(m_pConnection, query);

	MYSQL_RES* pResult = nullptr;
	pResult = mysql_store_result(m_pConnection);
	CResultEraser eraser(pResult, m_pConnection);

	if (0 != state)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::StartGame_Req() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::StartGame_Res));

	state = mysql_num_rows(pResult);
	if (state == 0)
	{
		// 시작할 캐릭터가 없어서 실패
		result = 1;
		tls_pSer->Serialize(result);
	}
	else
	{
		MYSQL_ROW row = mysql_fetch_row(pResult);
		if (row == nullptr)
			return;

		tls_pSer->Serialize(result);
		int index = 0;
		index++;
		index++;
		tls_pSer->Serialize(row[index++]);
	}

	char* pBuffer = info.pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	info.pPlayer->SendPost(tls_pSer->GetCurBufSize());
}

