#pragma once
//#include "Player.h"
#include "NPC.h"

class CPlayerManager final : public CSingleton
{
	DECLEAR_SINGLETON(CPlayerManager);
public:
	CPlayerManager();
	~CPlayerManager();

	inline unsigned int GeneratePrivateKey() { m_generatePlayerKey++; if (0 == m_generatePlayerKey) return ++m_generatePlayerKey; else return m_generatePlayerKey; }
	bool CreatePlayer(INITCONFIG &initConfig, DWORD dwMaxPlayer);
	bool AddPlayer(CPlayer* pPlayer);
	bool RemovePlayer(CPlayer* pPlayer);
	CPlayer* FindPlayer(DWORD dwPkey);
	inline unsigned __int32 GetPlayerCnt() { return (unsigned __int32)m_mapPlayer.size(); }









	void Notice_Req()
	{

	}





	void GS_CL_CurNPCPosFromHost(CPlayer& player);

	void CL_GS_Chat(char* pMsg)
	{
		
	}

	void GS_CL_DayAndNightTime(char dayAndNightTime)
	{
		CSerializer& ser = *tls_pSer;
		ser.StartSerialize();
		ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_DayAndNightTime));
		ser.Serialize(dayAndNightTime);

		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			CPlayer* pWorldPlayer = (CPlayer*)it->second;
			char* pBuf = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			ser.CopyBuffer(pBuf);
			pWorldPlayer->SendPost(ser.GetCurBufSize());
		}
	}

	void CheckInfection()
	{
		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			CPlayer* pWorldPlayer = (CPlayer*)it->second;
			if (pWorldPlayer->m_isInfection && !pWorldPlayer->m_isDead)
			{
				pWorldPlayer->SetHP(pWorldPlayer->GetHP() - 10);

				CSerializer& ser = *tls_pSer;
				ser.StartSerialize();
				ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_PlayerHP));
				ser.Serialize(pWorldPlayer->GetHP());
				ser.Serialize((char)true);
				char* buf = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
				if (nullptr == buf)
					return;
				ser.CopyBuffer(buf);
				pWorldPlayer->SendPost(ser.GetCurBufSize());
			}
		}
	}

	void DoTargetWorldPlayer(CNPC* pDetectNPC)
	{
		if (0 == m_mapPlayer.size())
			return;
		unsigned int rand = CMTRand::GetRand_uint64(0, m_mapPlayer.size() - 1);
		unsigned int i = 0;
		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			if (i++ != rand)
				continue;

			CPlayer* pPlayer = (CPlayer*)it->second;
			if (nullptr == pPlayer || pPlayer->m_isDead)
			{
				continue;
			}
			pDetectNPC->SetTagetPlayerPKey(pPlayer->GetKey());
			pDetectNPC->SetEvent(Event::EVENT_PLAYER_APPEAR);
		}
	}

	void GS_CL_GetItem(CPlayer* pPlayer, unsigned short itemKey)
	{
		// 먹었다고 보낸다
		CSerializer& ser = *tls_pSer;
		ser.StartSerialize();
		ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_GetItem));
		ser.Serialize(itemKey);
		ser.Serialize(pPlayer->GetKey());

		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			CPlayer* pWorldPlayer = (CPlayer*)it->second;
			char* pBuf = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			ser.CopyBuffer(pBuf);
			pWorldPlayer->SendPost(ser.GetCurBufSize());
		}
	}

	void GS_CL_GetVictim(unsigned short code)
	{
		CSerializer& ser = *tls_pSer;
		ser.StartSerialize();
		ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_GetVictim));
		ser.Serialize(code);
		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			CPlayer* pPlayer = (CPlayer*)it->second;
			char* pBuf = pPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			ser.CopyBuffer(pBuf);
			pPlayer->SendPost(ser.GetCurBufSize());
		}
	}

	void GS_CL_PlayerDead(CPlayer* pDeadPlayer)
	{
		CSerializer& ser = *tls_pSer;
		ser.StartSerialize();
		ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_PlayerDead));
		ser.Serialize(pDeadPlayer->GetKey());

		for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
		{
			CPlayer* pPlayer = (CPlayer*)it->second;
			if (pPlayer == pDeadPlayer)
				continue;
			char* pBuf = pPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			ser.CopyBuffer(pBuf);
			pPlayer->SendPost(ser.GetCurBufSize());
		}
	}

	void CL_GS_DiscardItem(CPlayer* pPlayer, DWORD dwSize, char* pMsg);

	void Util_TCP_WithPacketTypeToAllPlayer(PacketType type)
	{
		stUtil_Empty packet;
		packet.length = PACKET_SIZE_LENGTH + PACKET_TYPE_LENGTH;
		packet.type = static_cast<packet_type>(type);
		for (auto it = m_mapPlayer.begin(); it != m_mapPlayer.end(); ++it)
		{
			CPlayer* pWorldPlayer = (CPlayer*)it->second;
			if (pWorldPlayer->GetIsConfirm() == false)
				continue;
			char* pBuffer = pWorldPlayer->PrepareSendPacket(packet.length);
			if (nullptr == pBuffer)
				continue;
			memcpy_s(pBuffer, packet.length, &packet, packet.length);
			pWorldPlayer->SendPost(packet.length);
		}
	}

	void CL_GS_EquipGun(CPlayer* pPlayer, char* pRecvedMsg)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::GS_CL_EquipGun));
		tls_pSer->Serialize(pPlayer->GetKey());
		tls_pSer->Serialize(pPlayer->m_equipGun);

		for (auto it = m_mapPlayer.begin(); it != m_mapPlayer.end(); ++it)
		{
			CPlayer* pWorldPlayer = (CPlayer*)it->second;
			if (pPlayer == pWorldPlayer)
				continue;
			char* pBuffer = pWorldPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
			if (nullptr == pBuffer)
				continue;
			tls_pSer->CopyBuffer(pBuffer);
			pWorldPlayer->SendPost(tls_pSer->GetCurBufSize());
		}
	}

	// 유틸리티
	void Send_TCP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize); // 플레이어에게 서버에서 받은 버퍼를 그대로 보낸다
	void Send_UDP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize); // 플레이어에게 서버에서 받은 버퍼를 그대로 보낸다
	void Send_TCP_WithPacketTypeToPlayer(CPlayer* _pPlayer, PacketType _eType); // 한 플레이어에게 패킷타입으로 보내는 함수
	// 로그인
	void GS_CL_LoginPlayerInfo(CPlayer* pPlayer);
	void GS_CL_WorldPlayerInfo(CPlayer* pPlayer);
	void GS_CL_LoginInfo(CPlayer& player);
	void Send_LogoutPlayer(CPlayer* pPlayer);
	// 공격
	void SuccessAttackToRemotePlayer_Rq(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	void Send_SuccessNPCAttackToPlayer_Aq(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	CPlayer* m_pHostPlayer;
	bool m_isNotSetupHost;
private:
	concurrent_unordered_map< DWORD, CPlayer* >		m_mapPlayer;
	//priority_queue <Rank, vector<Rank>, less<Rank> > rankMaxPriQueue;
	CPlayer*		m_pPlayer;

	CMonitorSRW		m_srwPlayer;

	int m_nNpcAreaCount;
	unsigned int m_generatePlayerKey;
};
CREATE_FUNCTION(CPlayerManager, PlayerManager);
