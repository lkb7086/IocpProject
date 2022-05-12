#pragma once
//#include "NPC.h"
//#include "Player.h"
#include "MovingNPC.h"
#include "NormalNPC.h"

/*
struct NPCInitData
{
	NPC_Type m_npcType; NPC_Code m_npcCode; UINT m_npcCnt; UCHAR m_teamNum; USHORT m_gruopPos;
	NPCInitData(NPC_Type npcType, NPC_Code npcCode, UINT npcCnt, UCHAR teamNum, USHORT gruopPos)
	{
		m_npcType = npcType; m_npcCode = npcCode; m_npcCnt = npcCnt; m_teamNum = teamNum; m_gruopPos = gruopPos;
	};
};
*/

class CNPCManager final : public CSingleton
{
	DECLEAR_SINGLETON(CNPCManager);
public:
	CNPCManager();
	~CNPCManager();

	bool CreateNpc(NPCInitData data);
	bool DestroyNpc();
	bool AddNpc(CNPC* pNpc);
	bool RemoveNpc(UINT dwNpcKey);
	CNPC* FindNpc(UINT dwNpcKey);
	inline int GetCnt() { return (int)m_mapNPC.size(); }

	void UpdateNpc();

	inline unsigned int GenerateNPCKey() { return ++m_generateNPCKey; }   // npc개인키 생성
	inline unsigned __int64 GenerateDropItemKey(unsigned __int32 _nType) // 드롭아이템 개인키 생성
	{
		m_generateDropItemKey++; if (0 == m_generateDropItemKey) return ++m_generateDropItemKey; else return m_generateDropItemKey;
	}

	void GS_CL_NPCInfo(CPlayer* pPlayer)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::GS_CL_NPCInfo));
		tls_pSer->Serialize((unsigned short)m_mapNPC.size());
		for (auto it = m_mapNPC.begin(); it != m_mapNPC.end(); it++)
		{
			CNPC* pNpc = (CNPC*)it->second;
			tls_pSer->Serialize(pNpc->GetKey());
			tls_pSer->Serialize((unsigned short)pNpc->GetNPC_Code());
			tls_pSer->Serialize((unsigned char)pNpc->GetNPC_Type());
			tls_pSer->SetStream(&pNpc->m_bIsDead, sizeof(bool));
			tls_pSer->Serialize(pNpc->GetMaxHP());
			tls_pSer->Serialize(pNpc->GetTagetPlayerPKey());
			tls_pSer->Serialize(pNpc->m_nativePos.x);
			tls_pSer->Serialize(pNpc->m_nativePos.y);
			tls_pSer->Serialize(pNpc->m_nativePos.z);
			tls_pSer->Serialize(pNpc->m_pos.x);
			tls_pSer->Serialize(pNpc->m_pos.y);
			tls_pSer->Serialize(pNpc->m_pos.z);
		}

		char* pBuf = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
		if (nullptr == pBuf) return;
		tls_pSer->CopyBuffer(pBuf);
		pPlayer->SendPost(tls_pSer->GetCurBufSize());

		pPlayer->m_isInitNPCInfo = true;
	}

	void CL_GS_CurNPCPosFromHost(char* pRecvedMsg)
	{
		CSerializer& ser = *tls_pSer;
		unsigned short npcCnt = 0;
		unsigned int npcKey = 0;
		Vector3 tempPos;

		ser.StartDeserialize(pRecvedMsg);
		ser.Deserialize(npcCnt);
		for (size_t i = 0; i < npcCnt; i++)
		{
			ser.Deserialize(npcKey);
			ser.Deserialize(tempPos.x);
			ser.Deserialize(tempPos.y);
			ser.Deserialize(tempPos.z);

			CNPC* pNPC = FindNpc(npcKey);
			if (nullptr == pNPC)
				continue;
			pNPC->m_tempPosFromHost = tempPos;

			npcKey = 0;
			memset(&tempPos, 0, sizeof(Vector3));
		}

		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::GS_CL_NPCInfo));
		tls_pSer->Serialize((unsigned short)m_mapNPC.size());
		for (auto it = m_mapNPC.begin(); it != m_mapNPC.end(); it++)
		{
			CNPC* pNpc = (CNPC*)it->second;
			tls_pSer->Serialize(pNpc->GetKey());
			tls_pSer->Serialize((unsigned short)pNpc->GetNPC_Code());
			tls_pSer->Serialize((unsigned char)pNpc->GetNPC_Type());
			tls_pSer->SetStream(&pNpc->m_bIsDead, sizeof(bool));
			tls_pSer->Serialize(pNpc->GetMaxHP());
			tls_pSer->Serialize(pNpc->GetTagetPlayerPKey());
			tls_pSer->Serialize(pNpc->m_nativePos.x);
			tls_pSer->Serialize(pNpc->m_nativePos.y);
			tls_pSer->Serialize(pNpc->m_nativePos.z);
			tls_pSer->Serialize(pNpc->m_tempPosFromHost.x);
			tls_pSer->Serialize(pNpc->m_tempPosFromHost.y);
			tls_pSer->Serialize(pNpc->m_tempPosFromHost.z);
		}

		while (0 < m_vecNPCInfoPlayer.size())
		{
			CPlayer& player = *m_vecNPCInfoPlayer.back();
			m_vecNPCInfoPlayer.pop_back();

			char* pBuf = player.PrepareSendPacket(tls_pSer->GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			tls_pSer->CopyBuffer(pBuf);
			player.SendPost(tls_pSer->GetCurBufSize());

			player.m_isInitNPCInfo = true;
		}
	}

	bool GatherVBuffer_NpcInfo(int area);
	void SuccessAttackToNPC_Cn(CPlayer* pPlayer, char* pRecvedMsg);

	void DoHugeWave()
	{
		m_hugeWave += 1;
		for (auto it = m_mapNPC.begin(); m_mapNPC.end() != it; ++it)
		{
			CNPC& npc = *it->second;
			if (NPC_Type::MOVING_NPC != npc.GetNPC_Type() || m_hugeWave <= (int)npc.GetNPC_Code())
				continue;
			if (npc.GetIsDead())
				npc.RaiseNPC();
		}
		PlayerManager()->Util_TCP_WithPacketTypeToAllPlayer(PacketType::GS_CL_StartWave);
	}
	
	void DoSmallWave()
	{
		m_smallWave = 3;
		for (auto it = m_mapNPC.begin(); m_mapNPC.end() != it; ++it)
		{
			CNPC& npc = *it->second;
			if (NPC_Type::NORMAL_NPC != npc.GetNPC_Type() || m_smallWave <= (int)npc.GetNPC_Code())
				continue;
			NormalNPC* pNormalNPC = dynamic_cast<NormalNPC*>(&npc);
			if (nullptr == pNormalNPC)
				continue;
			pNormalNPC->m_smallStartWaveFlag = true;
		}
	}
	
	void CL_GS_CL_PlayerAreaAttack(CPlayer* pPlayer, DWORD dwSize, char* pMsg)
	{
		CSerializer& ser = *tls_pSer;
		unsigned short cnt = 0; unsigned int playerKey = 0;
		unsigned short damage = 0; unsigned int npcKey = 0;
		ser.StartDeserialize(pMsg);
		ser.Deserialize(cnt);
		ser.Deserialize(playerKey);
		ser.Deserialize(damage);

		for (size_t i = 0; i < cnt; i++)
		{
			ser.Deserialize(npcKey);
			CNPC* pGetHitNPC = FindNpc(npcKey);
			if (nullptr == pGetHitNPC || pGetHitNPC->GetIsDead())
				continue;
			pGetHitNPC->SetHp(pGetHitNPC->GetHp() - damage);
			if (0 >= pGetHitNPC->GetHp())
			{
				pGetHitNPC->SetIsDead(true);
				continue;
			}
			if (0 == pGetHitNPC->GetTagetPlayerPKey())
			{
				pGetHitNPC->SetEvent(Event::EVENT_PLAYER_ATTACK);
				pGetHitNPC->SetTagetPlayerPKey(playerKey);
			}
		}

		AreaManager()->RelayWithoutPlayer(pPlayer, dwSize, pMsg);
	}

	void DoDetectNPC(CMovingNPC& npc)
	{
		/*
		for (size_t i = 0; i < m_vecAreaNpc[zone][npcArea].size(); i++)
		{
			CNPC& targetNPC = *m_vecAreaNpc[zone][npcArea].at(i);
			if (targetNPC.m_teamNum == npc.m_teamNum || targetNPC.GetIsDead() || (&targetNPC == &npc))
				continue;
			float distance =
				sqrt(pow(targetNPC.m_pos.x - npc.m_pos.x, 2) +
					pow(targetNPC.m_pos.y - npc.m_pos.y, 2));
			if (DEFAULT_DETECT_DISTANCE < distance)
			{
				continue;
			}

			npc.m_targetNPC = &targetNPC;
			npc.SetTagetPlayerPKey(0);
			npc.SetEvent(Event::EVENT_PLAYER_APPEAR);
			break;
		}
		*/
	}

	int m_hugeWave;
	int m_smallWave;

	std::vector<CPlayer*> m_vecNPCInfoPlayer;
private:
	TiXmlDocument m_xmlRead;

	std::map< UINT, CNPC* > m_mapNPC;
	std::vector<CNPC*> m_vecAreaNPC[MAX_AREA];
	std::forward_list<CNPC*> m_listDelete;

	unsigned int m_generateNPCKey;
	unsigned __int64 m_generateDropItemKey;
};
CREATE_FUNCTION(CNPCManager, NPCManager);