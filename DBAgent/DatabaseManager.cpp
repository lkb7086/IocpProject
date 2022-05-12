#include "stdafx.h"
//#include "DatabaseManager.h"
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
	printf("MySQL client version: %s\n", mysql_get_client_info());

	// 초기화
	mysql_init(&m_Conn);

	// DB 연결
	m_pConnection = mysql_real_connect(&m_Conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 3306, (char *)NULL, 0);
	if (m_pConnection == NULL)
	{
		printf("Mysql connection error : %s", mysql_error(&m_Conn));
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

void CDatabaseManager::DB_SearchUnit_Rq(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	UINT nUnit_UID = 0;
	UINT nAccox_UID = 0xFFFFFFFF;
	char szName[MAX_NICKNAME_LENGTH]; memset(szName, 0x00, sizeof(szName));
	char szID[MAX_ID_LENGTH]; memset(szID, 0x00, sizeof(szID));

	// 캐릭터 검사
	{
		MYSQL_RES* result = nullptr; MYSQL_ROW row = nullptr;
		int	nState = -1;
		char query[128]; memset(query, 0, sizeof(query));
		unsigned short userClass = 0xFFFF;

		UINT nPKey = 0;
		//char szID[MAX_ID_LENGTH]; memset(szID, 0x00, sizeof(szID));
		//char szName[MAX_NICKNAME_LENGTH]; memset(szName, 0x00, sizeof(szName));

		m_pSerializer->StartDeserialize(pRecvedMsg);
		m_pSerializer->Deserialize(nPKey);
		m_pSerializer->Deserialize(nAccox_UID);
		m_pSerializer->Deserialize(szID, sizeof(szID));
		m_pSerializer->Deserialize(szName, sizeof(szName));
		m_pSerializer->Deserialize(userClass);

		_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT class, area, x, y, uid, gold, exp FROM unit where accox_uid = %u and class = %u", nAccox_UID, userClass);

		nState = mysql_query(m_pConnection, query);
		if (0 != nState)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchUnit_Rq() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
			return;
		}

		result = mysql_store_result(m_pConnection);   // mysql_use_result
		CResultEraser eraser(result, m_pConnection);
		if (nullptr == result)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchUnit_Rq() | 1 mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
			return;
		}

		nState = mysql_num_rows(result);
		if (0 == nState)         // 디비에 캐릭터가 없으면
		{
			m_pSerializer->StartSerialize();
			m_pSerializer->Serialize(DA_GS_SearchUnit_Aq);
			m_pSerializer->Serialize(0);
			m_pSerializer->Serialize(nPKey);
			char *pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
			if (nullptr == pBuffer) return;
			m_pSerializer->CopyBuffer(pBuffer);
			pConnection->SendPost(m_pSerializer->GetCurBufSize());

			printf("DB_SearchUnit_Rq: 디비에 캐릭터가 없습니다. \n");
			return;
		}
		else if (0 < nState)    // 디비에 캐릭터가 있으면
		{
			row = mysql_fetch_row(result);
			if (nullptr == row)
			{
				printf("DB_SearchUnit_Rq: NULL == row error \n");
				return;
			}

			// 게임서버에 유닛정보 패킷을 보낸다
			m_pSerializer->StartSerialize();
			m_pSerializer->Serialize(DA_GS_SearchUnit_Aq);
			m_pSerializer->Serialize(nPKey);
			m_pSerializer->Serialize((unsigned char)atoi(row[0])); // class
			m_pSerializer->Serialize(atoi(row[1])); // area
			m_pSerializer->Serialize((float)atof(row[2])); // x
			m_pSerializer->Serialize((float)atof(row[3])); // y
			nUnit_UID = atoi(row[4]);
			m_pSerializer->Serialize(nUnit_UID); // uid
			m_pSerializer->Serialize(atoi(row[5])); // gold
			m_pSerializer->Serialize(atoi(row[6])); // exp
			/*
			char *pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
			if (nullptr == pBuffer) return;
			m_pSerializer->CopyBuffer(pBuffer);
			pConnection->SendPost(m_pSerializer->GetCurBufSize());*/
		}
	}

	wchar_t strUnicode[256] = { 0, };
	int nLen = MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), NULL, NULL);
	MultiByteToWideChar(CP_UTF8, 0, szID, strlen(szID), strUnicode, nLen);

	wprintf(L"Login: %s\n", strUnicode);

	// 인벤토리 검사
	MYSQL_RES* result = nullptr; MYSQL_ROW row = nullptr;
	int	nState = -1;
	char query[128]; memset(query, 0, sizeof(query));

	_snprintf_s(query, _countof(query), _TRUNCATE, "SELECT * FROM worlddb.inventory where unit_uid = %u limit %u", nUnit_UID, MAX_INVEN_SLOT);

	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchUnit_Rq() inven | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	result = mysql_store_result(m_pConnection);
	CResultEraser eraser(result, m_pConnection);
	if (nullptr == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchUnit_Rq() inven | 1 mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		return;
	}

	nState = mysql_num_rows(result);
	if (0 == nState)
	{
		m_pSerializer->Serialize((USHORT)0);
		char *pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
		if (nullptr == pBuffer) return;
		m_pSerializer->CopyBuffer(pBuffer);
		pConnection->SendPost(m_pSerializer->GetCurBufSize());
		return;
	}
	else if (0 < nState)
	{
		m_pSerializer->Serialize((USHORT)nState);

		while (nullptr != (row = mysql_fetch_row(result)))
		{
			int offset = 0; // 1번째는 unit_uid
			m_pSerializer->Serialize((unsigned long long)atoll(row[offset++])); offset++; // uid
			m_pSerializer->Serialize((unsigned int)atoi(row[offset++])); // code
			m_pSerializer->Serialize((int)atoi(row[offset++])); // amount
			m_pSerializer->Serialize((unsigned short)atoi(row[offset++])); // slot
		}

		char *pBuffer = pConnection->PrepareSendPacket(m_pSerializer->GetCurBufSize());
		if (nullptr == pBuffer) return;
		m_pSerializer->CopyBuffer(pBuffer);
		pConnection->SendPost(m_pSerializer->GetCurBufSize());
	}
}

void CDatabaseManager::DB_LogoutPlayer_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	int	nState = -1;
	unsigned int nUnit_UID = 0;
	int nArea = -1; float x = 0.0f; float y = 0.0f; int nGold = 0; int nExp = 0;
	char query[255]; memset(query, 0, sizeof(query));
	
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(nUnit_UID);
	m_pSerializer->Deserialize(nArea);
	m_pSerializer->Deserialize(x);
	m_pSerializer->Deserialize(y);
	m_pSerializer->Deserialize(nGold);
	m_pSerializer->Deserialize(nExp);

	//cout << nUnit_UID << endl;
	_snprintf_s(query, _countof(query), _TRUNCATE, "update unit set area = %d, x = %f, y = %f, gold = %d, exp = %d where uid = %u",
		nArea, x, y, nGold, nExp, nUnit_UID);

	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LogoutPlayer_Cn() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	//if (-1 != nUnit_UID && 0 == mysql_affected_rows(m_pConnection))
		//LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_LogoutPlayer_Cn() | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));

	if (-1 != nUnit_UID)
		printf("Logout Player: UID %u\n", nUnit_UID);
}

void CDatabaseManager::DB_InsertDropItem_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	unsigned int nUnit_UID = 0xFFFFFFFF; unsigned int nType = 0xFFFFFFFF; unsigned short nSlot = 0xFFFF; int gold = 0;
	char query[256]; memset(query, 0, sizeof(query));
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(nUnit_UID);
	m_pSerializer->Deserialize(nType);
	m_pSerializer->Deserialize(nSlot);
	m_pSerializer->Deserialize(gold);

	_snprintf_s(query, _countof(query), _TRUNCATE, "INSERT INTO `inventory` (`unit_uid`, `code`, `amount`, slot) VALUES ('%u', '%u', '%d', '%d')",
		nUnit_UID, nType, 1, nSlot);
	int	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_InsertDropItem_Cn() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	if (0 == mysql_affected_rows(m_pConnection))
		LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_InsertDropItem_Cn() | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));

	if (0 > gold)
		return;
	memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "UPDATE unit SET gold = %d WHERE uid = %u",
		gold, nUnit_UID);
	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_InsertDropItem_Cn() 2 | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	if (0 == mysql_affected_rows(m_pConnection))
		LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_InsertDropItem_Cn() 2 | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));
}

void CDatabaseManager::DB_MerseDropItem_Cn(CConnection *_pConn, DWORD _dwSize, char *_pRecvMsg)
{
	unsigned __int32 nUnit_UID = 0xFFFFFFFF; int nAmount = 0; unsigned short nSlot = 0xFFFF; int gold = 0;
	char query[256]; memset(query, 0, sizeof(query));
	m_pSerializer->StartDeserialize(_pRecvMsg);
	m_pSerializer->Deserialize(nUnit_UID);
	m_pSerializer->Deserialize(nAmount);
	m_pSerializer->Deserialize(nSlot);
	m_pSerializer->Deserialize(gold);

	//printf("병합 %d %d\n", nAmount, nSlot);

	_snprintf_s(query, _countof(query), _TRUNCATE, "UPDATE inventory SET amount = %d WHERE unit_uid = %u AND slot = %u",
		nAmount, nUnit_UID, nSlot);
	int	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_MerseDropItem_Cn() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	if (0 == mysql_affected_rows(m_pConnection))
		LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_MerseDropItem_Cn() | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));

	if (0 > gold)
		return;
	memset(query, 0, sizeof(query));
	_snprintf_s(query, _countof(query), _TRUNCATE, "UPDATE unit SET gold = %d WHERE uid = %u",
		gold, nUnit_UID);
	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_MerseDropItem_Cn() 2 | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	if (0 == mysql_affected_rows(m_pConnection))
		LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_MerseDropItem_Cn() 2 | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));
}

void CDatabaseManager::DB_DeleteItem_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	unsigned int nUid = 0xFFFFFFFF; char query[256]; memset(query, 0, sizeof(query));
	m_pSerializer->StartDeserialize(pRecvedMsg);
	m_pSerializer->Deserialize(nUid);

	_snprintf_s(query, _countof(query), _TRUNCATE, "DELETE FROM inventory WHERE unit_uid = %d",
		nUid);
	int	nState = mysql_query(m_pConnection, query);
	if (0 != nState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_DeleteItem_Cn() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
	if (0 == mysql_affected_rows(m_pConnection))
		LOG(LOG_ERROR_LOW, "SYSTEM | CDatabaseManager::DB_DeleteItem_Cn() | 0 == mysql_affected_rows() 에러: %s", mysql_error(m_pConnection));
}

/*
void CDatabaseManager::DB_AllSearchMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	CLAllSearchMarketCn* pAllSearch = (CLAllSearchMarketCn*)pRecvedMsg;

	// 마켓 전체검색 쿼리
	sprintf(query, "SELECT * FROM market");

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	result = mysql_store_result(m_pConnection);   // HACK: mysql_use_result
	if (NULL == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_LoginCheck() | 1 mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		mysql_free_result(result);
		return;
	}

	iState = mysql_num_rows(result);
	if (0 == iState)     // 디비에 검색된 아이템이 없으면
	{
		printf("DB_AllSearchMarket: 거래소에 등록된 아이템이 없습니다. \n");
		mysql_free_result(result);
		return;
	}
	else if (0 != iState)    // 디비에 아이템이 있으면
	{
		row = mysql_fetch_row(result);
		if (NULL == row)
		{
			printf("DB_AllSearchMarket: NULL == row error \n");
			return;
		}

		// 가변패킷을 설정
		m_vBuffer.Init();
		m_vBuffer.Serialize(DA_AllSearchMarket_Sn);  // 타입
		m_vBuffer.Serialize(pAllSearch->uiPKey);  // 플레이어 Key
		m_vBuffer.Serialize(iState);  // 검색된 레코드 수

		for (int i = 0; i < iState; i++)
		{
			// 인덱스, 계정, 아이템이름, 등록한날짜, 가격
			m_vBuffer.Serialize(atoi(row[0]));
			m_vBuffer.SetString(row[1]);
			m_vBuffer.SetString(row[2]);
			m_vBuffer.SetString(row[3]);   // 컬럼에 null이 있으면 포인터 터진다, 조심
			m_vBuffer.Serialize(atoi(row[4]));

			row = mysql_fetch_row(result);
			if (NULL == row)
				break;
		}

		char* pBuffer = pConnection->PrepareSendPacket(m_vBuffer.GetCurBufSize());
		if (NULL == pBuffer)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_AllSearchMarket() | PrepareSendPacket() is NULL");
			mysql_free_result(result);
			return;
		}

		m_vBuffer.CopyBuffer(pBuffer);
		pConnection->SendPost(m_vBuffer.GetCurBufSize());
	}

	mysql_free_result(result);
}

void CDatabaseManager::DB_SearchTextFromMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	CLSearchTextFromMarketRq* pTextSearch = (CLSearchTextFromMarketRq*)pRecvedMsg;

	// 마켓 부분검색 쿼리
	sprintf(query, "SELECT * FROM market WHERE itemname LIKE '%s'", pTextSearch->szItemName);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchTextFromMarket() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}

	result = mysql_store_result(m_pConnection);
	if (NULL == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchTextFromMarket() | mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		mysql_free_result(result);
		return;
	}

	iState = mysql_num_rows(result);
	if (0 == iState)     // 디비에 검색된 아이템이 없으면
	{
		printf("DB_SearchTextFromMarket: 거래소에 등록된 아이템이 없습니다. \n");
		mysql_free_result(result);
		return;
	}
	else if (0 != iState)    // 디비에 아이템이 있으면
	{
		row = mysql_fetch_row(result);
		if (NULL == row)
		{
			printf("DB_SearchTextFromMarket: NULL == row error \n");
			return;
		}

		// 가변패킷을 설정
		m_vBuffer.Init();
		m_vBuffer.Serialize(DA_SearchTextFromMarket_Aq);  // 타입
		m_vBuffer.Serialize(pTextSearch->uiPKey);  // 플레이어 Key
		m_vBuffer.Serialize(iState);  // 검색된 레코드 수

		for (int i = 0; i < iState; i++)
		{
			// 인덱스, 계정, 아이템이름, 등록한날짜, 가격
			m_vBuffer.Serialize(atoi(row[0]));
			m_vBuffer.SetString(row[1]);
			m_vBuffer.SetString(row[2]);
			m_vBuffer.SetString(row[3]);   // 컬럼에 null이 있으면 포인터 터진다, 조심
			m_vBuffer.Serialize(atoi(row[4]));

			row = mysql_fetch_row(result);
			if (NULL == row)
				break;
		}

		char* pBuffer = pConnection->PrepareSendPacket(m_vBuffer.GetCurBufSize());
		if (NULL == pBuffer)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchTextFromMarket() | PrepareSendPacket() is NULL");
			mysql_free_result(result);
			return;
		}

		m_vBuffer.CopyBuffer(pBuffer);
		pConnection->SendPost(m_vBuffer.GetCurBufSize());
	}

	mysql_free_result(result);
}

void CDatabaseManager::DB_SearchInven(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	CLSearchTextFromMarketRq* pSearch = (CLSearchTextFromMarketRq*)pRecvedMsg;

	// 인벤토리 검색
	sprintf(query, "SELECT Itemindex, itemname FROM useritem where name = '%s'", pSearch->szItemName);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchInven() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		mysql_free_result(result);
		return;
	}

	result = mysql_store_result(m_pConnection);
	if (NULL == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchInven() | mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		mysql_free_result(result);
		return;
	}

	iState = mysql_num_rows(result);
	if (0 == iState)
	{
		printf("DB_SearchInven: 인벤토리에 등록된 아이템이 없습니다. \n");
		mysql_free_result(result);
		return;
	}
	else if (0 != iState)    // 디비에 아이템이 있으면
	{
		row = mysql_fetch_row(result);
		if (NULL == row)
		{
			printf("DB_SearchInven: NULL == row error \n");
			mysql_free_result(result);
			return;
		}

		// 가변패킷을 설정
		m_vBuffer.Init();
		m_vBuffer.Serialize(DA_CL_SearchInven_VAq);  // 타입
		m_vBuffer.Serialize(pSearch->uiPKey);  // 플레이어 Key
		m_vBuffer.Serialize(iState);  // 검색된 레코드 수

		for (int i = 0; i < iState; i++)
		{
			// 인덱스, 계정, 아이템이름, 등록한날짜, 가격
			m_vBuffer.Serialize(atoi(row[0]));
			m_vBuffer.SetString(row[1]);

			row = mysql_fetch_row(result);
			if (NULL == row)
				break;
		}

		char* pBuffer = pConnection->PrepareSendPacket(m_vBuffer.GetCurBufSize());
		if (NULL == pBuffer)
		{
			LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_SearchInven() | PrepareSendPacket() is NULL");
			mysql_free_result(result);
			return;
		}

		m_vBuffer.CopyBuffer(pBuffer);
		pConnection->SendPost(m_vBuffer.GetCurBufSize());
	}

	mysql_free_result(result);
}

void CDatabaseManager::DB_DoPurchaseItemInMarket(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	MYSQL_RES* result = NULL;
	MYSQL_ROW row = NULL;
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	char szUserId[20]; memset(szUserId, 0, sizeof(szUserId));
	unsigned int uiSellerPKey = 0; int iSellerCoinFromDB = 0;  int iPurchaserCoin = 0;
	stCL_DA_DoPurchaseItemInMarket_Rq* pPurchase = (stCL_DA_DoPurchaseItemInMarket_Rq*)pRecvedMsg;
	

	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 거래소에서 구입
	sprintf(query, "call PurchaseItemInMarket('%s', %d, %d);",
		pPurchase->szNickName, pPurchase->iItemIndex, pPurchase->iCost);
	
	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_DoPurchaseItemInMarket() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		mysql_free_result(result);
		return;
	}

	result = mysql_store_result(m_pConnection);
	if (NULL == result)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_DoPurchaseItemInMarket() | mysql_store_result() is NULL: %s", mysql_error(m_pConnection));
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		mysql_free_result(result);
		return;
	}

	iState = mysql_num_rows(result);
	if (0 == iState)     // 디비에 검색된 레코드가 없으면
	{
		printf("DB_DoPurchaseItemInMarket: 0 == iState \n");
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		mysql_free_result(result);
		return;
	}
	else if (0 != iState)
	{
		row = mysql_fetch_row(result);
		if (NULL == row)
		{
			printf("DB_DoPurchaseItemInMarket: NULL == row error \n");
			while (mysql_more_results(m_pConnection))
				mysql_next_result(m_pConnection);
			mysql_free_result(result);
			return;
		}
		strncpy_s(szUserId, _countof(szUserId), row[0], _TRUNCATE);  // 문자열을 복사해놓은다
		iSellerCoinFromDB = atoi(row[1]);
		iPurchaserCoin = atoi(row[2]);
		printf("구입 테스트 아이디:  %s      ,     %s         , 판매아이템돈: %d\n", row[0], szUserId, iSellerCoinFromDB);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 판매한 사람이 접속중이라면
	USERID_IT it = m_unmapUserId.find(szUserId);
	if (m_unmapUserId.end() != it)
		uiSellerPKey = it->second;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	m_vBuffer.Init();
	m_vBuffer.Serialize(DA_CL_DoPurchaseItemInMarket_Aq);  // 타입
	m_vBuffer.Serialize(pPurchase->uiPKey);  // 구입 플레이어 Key
	m_vBuffer.Serialize(uiSellerPKey);       // 판매 플레이어 Key
	m_vBuffer.Serialize(iSellerCoinFromDB);
	m_vBuffer.Serialize(pPurchase->iListIndex);
	m_vBuffer.Serialize(iPurchaserCoin);

	char* pSendBuffer = pConnection->PrepareSendPacket(m_vBuffer.GetCurBufSize());
	if (NULL == pSendBuffer)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_DoPurchaseItemInMarket() | PrepareSendPacket() is NULL");
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		mysql_free_result(result);
		return;
	}
	m_vBuffer.CopyBuffer(pSendBuffer);
	pConnection->SendPost(m_vBuffer.GetCurBufSize());


	while (mysql_more_results(m_pConnection))
		mysql_next_result(m_pConnection);
	mysql_free_result(result);
}

void CDatabaseManager::DB_CL_DA_DoInsertItemToMarket_Rq(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	stCL_DA_DoInsertItemToMarket_Rq* pInsert = (stCL_DA_DoInsertItemToMarket_Rq*)pRecvedMsg;

	printf("거래소에 등록?       %s    %d      %d\n", pInsert->szNickName, pInsert->iItemIndex, pInsert->iSellCost);


	// 인벤토리 검색
	sprintf(query, "call InsertItemToMarket('%s', %d, %d);",
		pInsert->szNickName, pInsert->iItemIndex, pInsert->iSellCost);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_CL_DA_DoInsertItemToMarket_Rq() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		while (mysql_more_results(m_pConnection))
			mysql_next_result(m_pConnection);
		return;
	}

	while (mysql_more_results(m_pConnection))
		mysql_next_result(m_pConnection);
}

void CDatabaseManager::DB_DA_GS_LogoutPlayer_Cn(CConnection* pConnection, DWORD dwSize, char* pRecvedMsg)
{
	int	iState = -1;
	char query[255]; memset(query, 0, sizeof(query));
	stDA_GS_LogoutPlayer_Cn* pLogout = (stDA_GS_LogoutPlayer_Cn*)pRecvedMsg;

	// 인벤토리 검색
	sprintf(query, "update testcharacters set POS = %d, X = %f, Y = %f, Z = %f where USERID = '%s' and NAME = '%s' ",
		pLogout->uiArea, pLogout->fPosX, pLogout->fPosY, pLogout->fPosZ, pLogout->szUserId, pLogout->szNick);

	iState = mysql_query(m_pConnection, query);
	if (0 != iState)
	{
		LOG(LOG_ERROR_HIGH, "SYSTEM | CDatabaseManager::DB_CL_DA_DoInsertItemToMarket_Rq() | mysql_query() 에러: %s", mysql_error(m_pConnection));
		return;
	}
}



*/