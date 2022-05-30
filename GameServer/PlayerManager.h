#pragma once
//#include "Player.h"
#include "NPC.h"


struct PlayerInfo
{
	unsigned long long uid;
	char id[MAX_ID_LENGTH];
	char nickName[MAX_NICKNAME_LENGTH];
	char species;
	char gender;
	char height;
	char width;

	PlayerInfo()
	{
		memset(this, 0, sizeof(PlayerInfo));
	}
};



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
	

	void ErasePlayerInfo(unsigned int playerKey)
	{
		auto it = m_mapPlayerInfo.find(playerKey);
		if (it != m_mapPlayerInfo.end())
		{
			m_mapPlayerInfo.erase(playerKey);
		}
	}


	// 유틸리티
	void Send_TCP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize); // 플레이어에게 서버에서 받은 버퍼를 그대로 보낸다
	void Send_UDP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize); // 플레이어에게 서버에서 받은 버퍼를 그대로 보낸다
	void Send_TCP_WithPacketTypeToPlayer(CPlayer* _pPlayer, PacketType _eType); // 한 플레이어에게 패킷타입으로 보내는 함수


	void InitPlayerInfo(CPlayer& player)
	{
		auto it = m_mapPlayerInfo.find(player.GetKey());
		if (it == m_mapPlayerInfo.end())
			return;

	    PlayerInfo& playerInfo = it->second;
		player.SetUID(playerInfo.uid);
		player.SetID(playerInfo.id);
		player.SetNickName(playerInfo.nickName);
		player.SetSpecies (playerInfo.species);
		player.SetGender(playerInfo.gender);
		player.SetHeight(playerInfo.height);
		player.SetWidth(playerInfo.width);
		//printf("%u %s %s %d %d %d %d\n", playerInfo.uid, playerInfo.id, playerInfo.nickName, playerInfo.species, playerInfo.gender, playerInfo.height, playerInfo.width);
		player.m_pos = Vector3(CMTRand::GetRand_float(-30.0f, 30.0f), CMTRand::GetRand_float(-30.0f, 30.0f), CMTRand::GetRand_float(-30.0f, 30.0f));

		m_mapPlayerInfo.erase(player.GetKey());
	}

	unsigned int MoveServer_Not1(char* pMsg);




	CPlayer* m_pHostPlayer;
	bool m_isNotSetupHost;
private:
	concurrent_unordered_map< DWORD, CPlayer* >		m_mapPlayer;
	map<unsigned int, PlayerInfo> m_mapPlayerInfo;
	//priority_queue <Rank, vector<Rank>, less<Rank> > rankMaxPriQueue;
	CPlayer*		m_pPlayer;

	CMonitorSRW		m_srwPlayer;

	int m_nNpcAreaCount;
	unsigned int m_generatePlayerKey;



};
CREATE_FUNCTION(CPlayerManager, PlayerManager);
