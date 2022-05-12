#include "StdAfx.h"
//#include "playermanager.h"

IMPLEMENT_SINGLETON(CPlayerManager);

CPlayerManager::CPlayerManager() : m_nNpcAreaCount(0), m_pPlayer(nullptr), m_pHostPlayer(nullptr)
{
	m_mapPlayer.rehash(MAX_USER_COUNT); // 2의 거듭제곱
	m_mapPlayer.clear();

	m_isNotSetupHost = true;
}

CPlayerManager::~CPlayerManager()
{
	if (nullptr != m_pPlayer)
	{
		delete[] m_pPlayer;
		m_pPlayer = nullptr;
	}
}

bool CPlayerManager::CreatePlayer(INITCONFIG &initConfig, DWORD dwMaxPlayer)
{
	m_pPlayer = new(nothrow)CPlayer[dwMaxPlayer];
	if (nullptr == m_pPlayer) return false;
	for (int i = 0; i < (int)dwMaxPlayer; i++)
	{
		initConfig.nIndex = i;
		if (m_pPlayer[i].CreateConnection(initConfig) == false)
			return false;
	}

	return true;
}

bool CPlayerManager::AddPlayer(CPlayer* pPlayer)
{
	auto player_it = m_mapPlayer.find(pPlayer->GetKey());
	if (player_it != m_mapPlayer.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CPlayerManager::AddPlayer() | PKey(%d)는 이미 m_mapPlayer에 있습니다.", pPlayer->GetKey());
		return false;
	}
	pPlayer->SetPKey(GeneratePrivateKey());

	m_mapPlayer.insert(pair< DWORD, CPlayer* >(pPlayer->GetKey(), pPlayer));
	return true;
}
bool CPlayerManager::RemovePlayer(CPlayer* pPlayer)
{
	unsigned int key = pPlayer->GetKey();
	if (0 == key)
		return false;

	auto player_it = m_mapPlayer.find(key);
	if (player_it == m_mapPlayer.end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CPlayerManager::RemovePlayer() | PKey(%u)는 m_mapPlayer에 없습니다. %p", key, pPlayer);
		return false;
	}

	CMonitorSRW::OwnerSRW lock(m_srwPlayer, TryLockExclusive);

	m_mapPlayer.unsafe_erase(key);
	return true;
}
CPlayer* CPlayerManager::FindPlayer(DWORD dwPkey)
{
	auto player_it = m_mapPlayer.find(dwPkey);
	if (player_it == m_mapPlayer.end())
	{
		//LOG(LOG_ERROR_NORMAL, "SYSTEM | CPlayerManager::FindPlayer() | PKey(%u)는 m_mapPlayer에 없습니다.", dwPkey);
		return nullptr;
	}
	return (CPlayer*)player_it->second;
}



void CPlayerManager::Send_TCP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize)
{
	for (auto player_it = m_mapPlayer.begin(); player_it != m_mapPlayer.end(); player_it++)
	{
		CPlayer* pWorldPlayer = (CPlayer*)player_it->second;
		char* pSendBuffer = pWorldPlayer->PrepareSendPacket(dwSize);
		if (nullptr == pSendBuffer)
			continue;
		CopyMemory(pSendBuffer, pRecvBuffer, dwSize);
		pWorldPlayer->SendPost(dwSize);
	}
}

void CPlayerManager::Send_UDP_RecvBufferFromServer(char* pRecvBuffer, DWORD dwSize)
{
	for (auto player_it = m_mapPlayer.begin(); player_it != m_mapPlayer.end(); player_it++)
	{
		CPlayer* pWorldPlayer = (CPlayer*)player_it->second;
		if (pWorldPlayer->GetIsConfirm() == false)
			continue;

		//char* pSendBuffer = pWorldPlayer->PrepareSendPacket(dwSize);
		//char* pSendBuffer = IocpGameServer()->UDP_PrepareSendPacket(dwSize);
		//if (NULL == pSendBuffer)
		//continue;
		//CopyMemory(pSendBuffer, pRecvBuffer, dwSize);
		//pWorldPlayer->SendPost(dwSize);
		//IocpGameServer()->UDP_SendPost(dwSize, (CConnection*)pWorldPlayer);
	}
}

void CPlayerManager::Send_TCP_WithPacketTypeToPlayer(CPlayer* _pPlayer, PacketType _eType)
{
	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(_eType));
	char* pSendBuffer = _pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pSendBuffer)
		return;
	tls_pSer->CopyBuffer(pSendBuffer);
	_pPlayer->SendPost(tls_pSer->GetCurBufSize());
}

void CPlayerManager::GS_CL_LoginPlayerInfo(CPlayer* pPlayer)
{
	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_LoginPlayerInfo));
	ser.Serialize(pPlayer->GetKey());

	for (auto it = m_mapPlayer.begin(); it != m_mapPlayer.end(); ++it)
	{
		CPlayer* pWorldPlayer = (CPlayer*)it->second;
		if (pWorldPlayer->m_bIsDummy || pWorldPlayer == pPlayer)
			continue;

		char* pLoginPlayerInfo = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
		if (nullptr == pLoginPlayerInfo)
			continue;
		ser.CopyBuffer(pLoginPlayerInfo);
		pWorldPlayer->SendPost(ser.GetCurBufSize());
	}
}

void CPlayerManager::GS_CL_WorldPlayerInfo(CPlayer* pPlayer)
{
	unsigned int uiPlayerCnt = (unsigned int)m_mapPlayer.size() - 1; // 자신은 뺀다
	if (0 == uiPlayerCnt) return;
	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::GS_CL_WorldPlayerInfo));
	tls_pSer->Serialize(uiPlayerCnt);
	for (auto player_it = m_mapPlayer.begin(); player_it != m_mapPlayer.end(); ++player_it)
	{
		CPlayer* pWorldPlayer = (CPlayer*)player_it->second;
		if (nullptr == pWorldPlayer || pWorldPlayer == pPlayer)
			continue;
		tls_pSer->Serialize(pWorldPlayer->GetKey());
		tls_pSer->Serialize(pWorldPlayer->m_equipGun);

		tls_pSer->Serialize((char)pWorldPlayer->m_isDead);

		tls_pSer->Serialize(pWorldPlayer->m_pos.x);
		tls_pSer->Serialize(pWorldPlayer->m_pos.y);
		tls_pSer->Serialize(pWorldPlayer->m_pos.z);
	}
	//printf("패킷용량: %d\n", tls_pSer->GetCurBufSize());
	char* pWorldPlayerInfos = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pWorldPlayerInfos) return;
	tls_pSer->CopyBuffer(pWorldPlayerInfos);
	pPlayer->SendPost(tls_pSer->GetCurBufSize());
}


void CPlayerManager::GS_CL_LoginInfo(CPlayer& player)
{
	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_LoginInfo));
	ser.Serialize(player.GetKey());
	ser.Serialize(IocpGameServer()->m_dayAndNightTime);

	char* pBuf = player.PrepareSendPacket(ser.GetCurBufSize());
	if (nullptr == pBuf)
		return;
	ser.CopyBuffer(pBuf);
	player.SendPost(ser.GetCurBufSize());
}

void CPlayerManager::Send_LogoutPlayer(CPlayer* pPlayer)
{
	if (0 == pPlayer->GetKey())
		return;

	for (auto player_it = m_mapPlayer.begin(); player_it != m_mapPlayer.end(); player_it++)
	{
		CPlayer* pWorldPlayer = (CPlayer*)player_it->second;
		if (pWorldPlayer->m_bIsDummy || pWorldPlayer == pPlayer)
			continue;
		stUtil_UInteger* pLogout = (stUtil_UInteger*)pWorldPlayer->PrepareSendPacket(sizeof(stUtil_UInteger));
		if (nullptr == pLogout)
			continue;
		pLogout->type = static_cast<packet_type>(PacketType::GS_CL_LogoutPlayer);
		pLogout->nUInteger = pPlayer->GetKey();
		pWorldPlayer->SendPost(sizeof(stUtil_UInteger));
	}
}

void CPlayerManager::GS_CL_CurNPCPosFromHost(CPlayer& player)
{
	NPCManager()->m_vecNPCInfoPlayer.push_back(&player);

	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_CurNPCPosFromHost));

	for (auto it = m_mapPlayer.begin(); m_mapPlayer.end() != it; ++it)
	{
		CPlayer* pWorldPlayer = (CPlayer*)it->second;
		if (pWorldPlayer->m_isHost)
		{
			char* pBuf = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == pBuf)
				return;
			ser.CopyBuffer(pBuf);
			pWorldPlayer->SendPost(ser.GetCurBufSize());
			return;
		}
	}
}

void CPlayerManager::CL_GS_DiscardItem(CPlayer* pPlayer, DWORD dwSize, char* pMsg)
{
	unsigned short itemCode = 0xFFFF;
	Vector3 discardPos;
	tls_pSer->StartDeserialize(pMsg);
	tls_pSer->Deserialize(itemCode);
	tls_pSer->Deserialize(discardPos.x);
	tls_pSer->Deserialize(discardPos.y);
	tls_pSer->Deserialize(discardPos.z);

	Item item;
	unsigned short itemKey = ItemManager()->GenerateItemKey();
	item.code = itemCode;
	item.x = discardPos.x; item.y = discardPos.y; item.z = discardPos.z;
	ItemManager()->InsertItem(itemKey, item);

	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_DiscardItem));
	ser.Serialize(itemKey);
	ser.Serialize(itemCode);
	ser.Serialize(item.x);
	ser.Serialize(item.y);
	ser.Serialize(item.z);
	for (auto it = m_mapPlayer.begin(); it != m_mapPlayer.end(); ++it)
	{
		CPlayer* pWorldPlayer = (CPlayer*)it->second;
		char* pBuf = pWorldPlayer->PrepareSendPacket(ser.GetCurBufSize());
		if (nullptr == pBuf)
			continue;
		ser.CopyBuffer(pBuf);
		pWorldPlayer->SendPost(ser.GetCurBufSize());
	}
}