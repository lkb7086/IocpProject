#include "StdAfx.h"
//#include "playermanager.h"

IMPLEMENT_SINGLETON(CPlayerManager);

CPlayerManager::CPlayerManager() : m_nNpcAreaCount(0), m_pPlayer(nullptr), m_pHostPlayer(nullptr)
{
	m_mapPlayer.rehash(MAX_USER_COUNT); // 2의 거듭제곱
	m_mapPlayer.clear();

	m_generatePlayerKey = 0;
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
		//LOG(LOG_ERROR_NORMAL, "SYSTEM | CPlayerManager::AddPlayer() | PKey(%d)는 이미 m_mapPlayer에 있습니다.", pPlayer->GetKey());
		return false;
	}

	if (pPlayer->GetKey() == 0)
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
	for (auto player_it = m_mapPlayer.begin(); player_it != m_mapPlayer.end(); ++player_it)
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


unsigned int CPlayerManager::MoveServer_Not1(char* pMsg)
{
	char nextServerID = -1; char prevServerID = -1;
	unsigned int key = GeneratePrivateKey();
	PlayerInfo playerInfo;
	tls_pSer->StartDeserialize(pMsg);

	
	m_mapPlayerInfo.insert(pair<unsigned int, PlayerInfo>(key, playerInfo));
	return key;
}
