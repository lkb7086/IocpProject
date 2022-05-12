#include "stdafx.h"
//#include "ProcessPacket.h"
//#include "npcmanager.h"
#include "DetectNPC.h"
#include "NormalNPC.h"
#include "MovingNPC.h"

IMPLEMENT_SINGLETON(CNPCManager);

CNPCManager::CNPCManager() : m_hugeWave(0), m_smallWave(0)
{
	m_generateNPCKey = 0;
	m_generateDropItemKey = 0;

	//if (false == m_xmlRead.LoadFile("NPC_Info.xml")) // xml 파일 로드
		//puts("xml LoadFile 에러");
}

CNPCManager::~CNPCManager()
{
	m_xmlRead.Clear();
	m_mapNPC.clear();
	for (size_t i = 0; i < MAX_AREA; i++)
	{
		m_vecAreaNPC[i].clear();
	}
}

bool CNPCManager::CreateNpc(NPCInitData data)
{
	/*
	TiXmlElement* readRoot = m_xmlRead.FirstChildElement("NPC");
	TiXmlElement* sub = readRoot->FirstChildElement("monster1")->FirstChildElement("info");
	TiXmlAttribute *pAttrib = sub->FirstAttribute();
	while (pAttrib)
	{
	std::cout << pAttrib->Name() << "  " << pAttrib->Value() << std::endl;
	pAttrib = pAttrib->Next();
	}
	*/

	if (0 == data.m_npcCnt)
		return false;

	switch (data.m_npcType)
	{
	case NPC_Type::NORMAL_NPC:
	{
		for (int i = 0; i < data.m_npcCnt; i++)
		{
			CNPC* pNPC = new NormalNPC;
			pNPC->SetNpcInfo(data, m_xmlRead);
			m_vecAreaNPC[pNPC->m_area].push_back(pNPC);
			AddNpc(pNPC);
			m_listDelete.push_front(pNPC);
		}
	}
	break;
	case NPC_Type::DETECT_NPC:
	{
		for (int i = 0; i < data.m_npcCnt; i++)
		{
			CNPC* pNPC = new DetectNPC;
			pNPC->SetNpcInfo(data, m_xmlRead);
			m_vecAreaNPC[pNPC->m_area].push_back(pNPC);
			AddNpc(pNPC);
			m_listDelete.push_front(pNPC);
		}
	}
	break;
	case NPC_Type::MOVING_NPC:
	{
		for (int i = 0; i < data.m_npcCnt; i++)
		{
			CNPC* pNPC = new CMovingNPC;
			pNPC->SetNpcInfo(data, m_xmlRead);
			m_vecAreaNPC[pNPC->m_area].push_back(pNPC);
			AddNpc(pNPC);
			m_listDelete.push_front(pNPC);
		}
	}
	break;
	default:
		break;
	}

	return true;
}

bool CNPCManager::DestroyNpc()
{
	while (!m_listDelete.empty())
	{
		CNPC* pNPC = m_listDelete.front();
		if (nullptr != pNPC)
			delete pNPC;
		pNPC = nullptr;
		m_listDelete.pop_front();
	}

	m_mapNPC.clear();
	return true;
}

bool CNPCManager::AddNpc(CNPC* pNpc)
{
	auto npc_it = m_mapNPC.find(pNpc->GetKey());
	if (npc_it != m_mapNPC.end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CNpcManager::AddNpc() | NpcKey(%d)는 이미 m_mapNpc에 존재하고 있습니다.", pNpc->GetKey());
		return false;
	}
	m_mapNPC.insert(pair< UINT, CNPC* >(pNpc->GetKey(), pNpc));
	return true;
}

bool CNPCManager::RemoveNpc(UINT dwNpcKey)
{
	/*
	NPC_IT npc_it = m_mapNpc.find(dwNpcKey);
	if( npc_it == m_mapNpc.end() )
	{
	LOG( LOG_ERROR_LOW, "SYSTEM | CNpcManager::RemoveNpc() | NpcKey(%d)는 m_mapNpc에 존재하지 않습니다.", dwNpcKey );
	return false;
	}
	m_mapNpc.erase( npc_it );
	*/
	return true;
}

CNPC* CNPCManager::FindNpc(UINT dwNpcKey)
{
	auto npc_it = m_mapNPC.find(dwNpcKey);
	if (npc_it == m_mapNPC.end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CNpcManager::FindNpc() | NpcKey(%d)는 m_mapNpc에 존재하지 않습니다.", dwNpcKey);
		return nullptr;
	}
	return (CNPC*)npc_it->second;
}

void CNPCManager::UpdateNpc()
{
	for (auto it = m_mapNPC.begin(); m_mapNPC.end() != it; ++it)
	{
		CNPC* pNPC = it->second;
		pNPC->OnProcess();
		// 디버깅
		if(pNPC->m_updatePacketRefCount < 0)
			LOG(LOG_ERROR_LOW, "SYSTEM | CNPCManager::UpdateNpc() | m_updatePacketRefCount %d", pNPC->m_updatePacketRefCount);
	}
}

bool CNPCManager::GatherVBuffer_NpcInfo(int area)
{
	size_t size = m_vecAreaNPC[area].size();
	if (0 == size)
		return false;

	// 카운팅 검사
	unsigned short fixedSize = 0;
	for (size_t i = 0; i < size; i++)
	{
		CNPC& npc = *m_vecAreaNPC[area].at(i);
		if (npc.m_updatePacketRefCount > 0)
		{
			fixedSize++;
		}
	}

	if (fixedSize == 0)
		return false;

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::GS_CL_UpdateNPC));
	tls_pSer->Serialize(fixedSize);
	for (size_t i = 0; i < size; i++)
	{
		CNPC& npc = *m_vecAreaNPC[area].at(i);
		if (npc.m_updatePacketRefCount > 0)
		{
			tls_pSer->Serialize(npc.GetTagetPlayerPKey());
			tls_pSer->Serialize(npc.GetKey());
			tls_pSer->SetStream(&npc.m_bIsDead, sizeof(bool));
			tls_pSer->Serialize(npc.GetMaxHP());
			tls_pSer->Serialize(npc.m_pos.x);
			tls_pSer->Serialize(npc.m_pos.y);
			tls_pSer->Serialize(npc.m_pos.z);

			npc.m_updatePacketRefCount--;
			fixedSize--; // 디버깅
		}
	}

	// 디버깅
	if(0 != fixedSize)
		LOG(LOG_ERROR_LOW, "SYSTEM | CNPCManager::GatherVBuffer_NpcInfo() | if(0!= fixedSize) %u", fixedSize);

	return true;
}

void CNPCManager::SuccessAttackToNPC_Cn(CPlayer* pPlayer, char* pRecvedMsg)
{
	unsigned int pAttackedNPCKey = 0;
	unsigned short gunDamage = 0;
	Vector3 rotation;
	bool isPlayer = false; // 공격한 객체가 생존자인지 플레이어인지
	CSerializer& ser = *tls_pSer;

	ser.StartDeserialize(pRecvedMsg);
	ser.GetStream(&isPlayer, sizeof(bool));
	ser.Deserialize(pAttackedNPCKey);
	ser.Deserialize(gunDamage);
	ser.Deserialize(rotation.x);
	ser.Deserialize(rotation.y);
	ser.Deserialize(rotation.z);

	// 일단 공격했다고 플레이어들에게 알린다
	AreaManager()->Send_PlayerAttackEffect(pPlayer, pAttackedNPCKey, gunDamage, rotation, isPlayer);

	auto it = m_mapNPC.find(pAttackedNPCKey);
	if (m_mapNPC.end() == it)
		return;
	CNPC* pAttackedNPC = (CNPC*)it->second;
	if (nullptr == pAttackedNPC)
		return;
	if (pAttackedNPC->GetIsDead())
		return;

	pAttackedNPC->SetHp(pAttackedNPC->GetHp() - gunDamage);
	// 이번에 공격받고 NPC가 죽었으면
	if (0 >= pAttackedNPC->GetHp())
	{
		pAttackedNPC->SetIsDead(true);
		return;
	}

	if (isPlayer)
	{
		// 타겟이 없으면
		if (0 == pAttackedNPC->GetTagetPlayerPKey())
		{
			pAttackedNPC->SetEvent(Event::EVENT_PLAYER_ATTACK);
			pAttackedNPC->SetTagetPlayerPKey(pPlayer->GetKey());
		}
	}
}