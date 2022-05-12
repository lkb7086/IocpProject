#include "StdAfx.h"
//#include "ItemManager.h"

IMPLEMENT_SINGLETON(CItemManager);

CItemManager::CItemManager() : m_generateItemKey(0)
{
	m_mapItem.clear();
	InitItem();
}

CItemManager::~CItemManager()
{
}

void CItemManager::InitItem()
{
	if (!m_mapItem.empty())
		return;

	srand((unsigned int)time(NULL));

	// 시작위치
	int itemCnt = 50;
	int scopeX = 5;
	int scopeZ = 5;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);
		Item item;
		item.x = 187.0f; item.y = 0.0f; item.z = -318.0f;
		if (0 == num)
			item.x += CMTRand::GetRand_uint64(0, scopeX);
		else
			item.x -= (CMTRand::GetRand_uint64(0, scopeX));
		num = rand() % 2;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if (0 == num)
			item.code = CMTRand::GetRand_uint64(6, 6);
		else
			item.code = CMTRand::GetRand_uint64(0, 0);

		InsertItem(GenerateItemKey(), item);
	}

	// 1도시
	itemCnt = 100;
	scopeX = 80;
	scopeZ = 80;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);;
		Item item;
		item.x = 450.0f; item.y = 0.0f; item.z = -310.0f;
		if (0 == num)
			item.x += rand() % scopeX;
		else
			item.x += -(rand() % scopeX);
		num = CMTRand::GetRand_uint64(0, 1);;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if(0 == num)
			item.code = CMTRand::GetRand_uint64(14, 15);
		else
			item.code = CMTRand::GetRand_uint64(6, 7);

		InsertItem(GenerateItemKey(), item);
	}

	// 1도시 경찰서
	itemCnt = 100;
	scopeX = 6;
	scopeZ = 6;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);;
		Item item;
		item.x = 435.9f; item.y = 0.0f; item.z = -351.2f;
		if (0 == num)
			item.x += rand() % scopeX;
		else
			item.x += -(rand() % scopeX);
		num = CMTRand::GetRand_uint64(0, 1);;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if (0 == num)
			item.code = CMTRand::GetRand_uint64(1, 2);
		else
			item.code = CMTRand::GetRand_uint64(7, 8);

		InsertItem(GenerateItemKey(), item);
	}

	// mcp도시
	itemCnt = 250;
	scopeX = 225;
	scopeZ = 180;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);;
		Item item;
		item.x = -625.0f; item.y = 0.0f; item.z = -330.0f;
		if (0 == num)
			item.x += rand() % scopeX;
		else
			item.x += -(rand() % scopeX);
		num = CMTRand::GetRand_uint64(0, 1);;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if (0 == num)
			item.code = CMTRand::GetRand_uint64(12, 13);
		else
			item.code = CMTRand::GetRand_uint64(6, 10);

		InsertItem(GenerateItemKey(), item);
	}

	// mcp총가게
	itemCnt = 150;
	scopeX = 2;
	scopeZ = 2;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);;
		Item item;
		item.x = -600.0f; item.y = 0.0f; item.z = -440.0f;
		if (0 == num)
			item.x += rand() % scopeX;
		else
			item.x += -(rand() % scopeX);
		num = CMTRand::GetRand_uint64(0, 1);;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if (0 == num)
			item.code = CMTRand::GetRand_uint64(1, 4);
		else
			item.code = CMTRand::GetRand_uint64(8, 10);

		InsertItem(GenerateItemKey(), item);
	}

	/*
	// 매트로
	itemCnt = 30;
	unsigned int num = CMTRand::GetRand_uint64(0, 9);
	num *= 100;
	if (CMTRand::GetBernoulliDist(0.5f)) // 가로기준
	{
		pos.x = num;
		pos.x -= 10;
		pos.x += CMTRand::GetRand_uint64(0, 20);

		pos.z = CMTRand::GetRand_uint64(0, 900);
	}
	else
	{
		pos.z = num;
		pos.z -= 10;
		pos.z += CMTRand::GetRand_uint64(0, 20);

		pos.x = CMTRand::GetRand_uint64(0, 900);
	}
	*/

	// 매트로 총가게
	itemCnt = 200;
	scopeX = 2;
	scopeZ = 2;
	for (int i = 0; i < itemCnt; i++)
	{
		int num = CMTRand::GetRand_uint64(0, 1);;
		Item item;
		item.x = 531.68f; item.y = 0.0f; item.z = 647.33f;
		if (0 == num)
			item.x += rand() % scopeX;
		else
			item.x += -(rand() % scopeX);
		num = CMTRand::GetRand_uint64(0, 1);;
		if (0 == num)
			item.z += rand() % scopeZ;
		else
			item.z += -(rand() % scopeZ);

		if (0 == num)
			item.code = CMTRand::GetRand_uint64(4, 5);
		else
			item.code = CMTRand::GetRand_uint64(8, 11);

		InsertItem(GenerateItemKey(), item);
	}

	// 생존자
	for (size_t i = 0; i < 3; i++)
	{
		Item item;
		switch (i)
		{
		case 0:
			item.code = i + 1;
			item.x = 345.0f; item.y = 0.0f; item.z = 645.0f;
			break;
		case 1:
			item.code = i + 1;
			item.x = -663.0f; item.y = 0.0f; item.z = 886.0f;
			break;
		case 2:
			item.code = i + 1;
			item.x = -741.0f; item.y = 0.0f; item.z = -349.0f;
			break;
		default:
			break;
		}
		m_listVictim.push_back(item);
	}
}

void CItemManager::GS_CL_InitItemInfo(CPlayer* pPlayer)
{
	// 아이템
	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_InitItemInfo));
	ser.Serialize((unsigned short)m_mapItem.size());
	for (auto it = m_mapItem.begin(); it != m_mapItem.end(); ++it)
	{
		Item& item = it->second;
		ser.Serialize(it->first); // 아이템키
		ser.Serialize(item.code);
		ser.Serialize(item.x);
		ser.Serialize(item.z);
	}
	char* pBuf = pPlayer->PrepareSendPacket(ser.GetCurBufSize());
	if (nullptr == pBuf)
		return;
	ser.CopyBuffer(pBuf);
	pPlayer->SendPost(ser.GetCurBufSize());
	pBuf = nullptr;


	// 생존자
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_VictimInfo));
	ser.Serialize((unsigned short)m_listVictim.size());
	for (auto it = m_listVictim.begin(); it != m_listVictim.end(); ++it)
	{
		Item& item = *it;
		ser.Serialize(item.code);
		ser.Serialize((char)item.flag);
		ser.Serialize(item.x);
		ser.Serialize(item.y);
		ser.Serialize(item.z);
	}
	pBuf = pPlayer->PrepareSendPacket(ser.GetCurBufSize());
	if (nullptr == pBuf)
		return;
	ser.CopyBuffer(pBuf);
	pPlayer->SendPost(ser.GetCurBufSize());
}