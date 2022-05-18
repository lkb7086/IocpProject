#include "StdAfx.h"
#include "processpacket.h"

void CProcessPacket::fnSendToWorldPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	PlayerManager()->Send_TCP_RecvBufferFromServer(pRecvedMsg, dwSize);
}

void CProcessPacket::fnSendToNS_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	CConnection* pNpcConn = IocpGameServer()->GetNpcServerConn(); if (NULL == pNpcConn) return;
	char* pSendBuffer = pNpcConn->PrepareSendPacket(dwSize); if (NULL == pSendBuffer) return;
	CopyMemory(pSendBuffer, pRecvedMsg, dwSize);
	pNpcConn->SendPost(dwSize);
}

void CProcessPacket::fnSendToDA_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	CConnection* pDAConn = IocpGameServer()->GetDbAgentConn();
	if (NULL == pDAConn)
		return;
	char* pSendBuffer = pDAConn->PrepareSendPacket(dwSize);
	if (NULL == pSendBuffer)
		return;
	CopyMemory(pSendBuffer, pRecvedMsg, dwSize);
	pDAConn->SendPost(dwSize);
}

void CProcessPacket::fnPushDBQueue(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->PushDBQueue(pPlayer, dwSize, pRecvedMsg);
}

void CProcessPacket::fnSendToAreaPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	AreaManager()->Send_TCP_RecvBufferFromServer(dwSize, pRecvedMsg);
}


void CProcessPacket::fnConfirmID_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
    IocpGameServer()->ConfirmID_Not(pPlayer, pRecvedMsg);

}

void CProcessPacket::fnStartLobby_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	IocpGameServer()->StartLobby_Req(pPlayer, pRecvedMsg);
}

void CProcessPacket::fnCreateCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->CreateCharacter_Req(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}

void CProcessPacket::fnDeleteCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DeleteCharacter_Req(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}

void CProcessPacket::fnStartGame_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->StartGame_Req(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}




void CProcessPacket::CL_GS_Login(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	if (!PlayerManager()->AddPlayer(pPlayer))
	{
		IocpGameServer()->CloseConnection(pPlayer);
		return;
	}

	CPlayer& player = *pPlayer;


	// uni: 로그인 플레이어 정보 -> 로그인 클라
	//PlayerManager()->GS_CL_LoginInfo(player);
	// uni: 월드 플레이어들 정보 -> 로그인 클라
	//PlayerManager()->GS_CL_WorldPlayerInfo(pPlayer);
	// bro: 로그인 플레이어 정보 -> 월드 플레이어들 클라
	//PlayerManager()->GS_CL_LoginPlayerInfo(pPlayer);



	///*
	// 영역
	player.m_pos.Zero();
	int nArea = AreaManager()->GetPosToArea(pPlayer->m_pos);
	player.SetArea(nArea);
	AreaManager()->UpdateActiveAreas(pPlayer);
	AreaManager()->AddPlayerToArea(pPlayer, pPlayer->GetArea());
	AreaManager()->Send_UpdateAreaForCreateObject(pPlayer);
	//*/


	pPlayer->SetIsConfirm(true);
	LOG(LOG_INFO_LOW, "ID (%u) Connected. / Current Players (%u)",
		pPlayer->GetKey(), PlayerManager()->GetPlayerCnt());
}

void CProcessPacket::CL_GS_CurNPCPosFromHost(CPlayer* _pPlayer, DWORD dwSize, char* _pRecvedMsg)
{
	NPCManager()->CL_GS_CurNPCPosFromHost(_pRecvedMsg);
}

void CProcessPacket::CL_GS_MovePlayer(CPlayer* _pPlayer, DWORD dwSize, char* _pRecvedMsg)
{
	//if (false == pPlayer->GetIsConfirm()) return; // UDP테스트중에는 닫는다


//#ifndef _DEBUG
	AreaManager()->Send_MovePlayerToActiveAreas(_pPlayer, _pRecvedMsg);
//#endif
	
	
	
	//PlayerManager()->Send_MovePlayer_Sn(nullptr,  dwSize, _pRecvedMsg);

	//while (InterlockedExchange((LPLONG)&s_bNpcServLock, TRUE) == TRUE)
	//Sleep(0);
	//InterlockedExchange((LPLONG)&s_bNpcServLock, FALSE);
}

void CProcessPacket::CL_GS_CL_PlayerAreaAttack(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	NPCManager()->CL_GS_CL_PlayerAreaAttack(pPlayer, dwSize, pRecvedMsg);
}

void CProcessPacket::CL_GS_PlayerAttackToNPC(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	NPCManager()->SuccessAttackToNPC_Cn(pPlayer, pRecvedMsg);
}

void CProcessPacket::CL_GS_NPCAttackToPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	AreaManager()->Send_SuccessNPCAttackToPlayer_Aq(pPlayer, pRecvedMsg);
}

void CProcessPacket::fnKeepAliveCn(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	bool isAuto = false;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->GetStream(&isAuto, sizeof(bool));

	pPlayer->m_bIsAutoAttack = isAuto;
	if (!pPlayer->m_bIsAutoAttack)
		pPlayer->SetAuto_TargetNPCKey(0xFFFFFFFF);

	pPlayer->SetKeepAliveTick(IocpGameServer()->GetServerTick());

	//printf("%d ", pPlayer->m_bIsAutoAttack);
	/* 서버에서는 필요 없다
	ULONGLONG nTick = GetTickCount64();
	int n = (nTick - pPlayer->m_nPingCount) - 3008;  // 겟틱으로 하지말것
	pPlayer->m_nPingCount = nTick;
	if (0 < n)
	printf("ping %d\n", n);
	*/

	//PlayerManager()->Send_한플레이어에게패킷타입으로보내는함수(pPlayer, GS_CL_KeepAlive_Aq);
}

void CProcessPacket::CL_GS_GetItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	ItemManager()->Recv_GetItem(pPlayer, pRecvedMsg);
}

void CProcessPacket::CL_GS_DiscardItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	PlayerManager()->CL_GS_DiscardItem(pPlayer, dwSize, pRecvedMsg);
}

void CProcessPacket::CL_GS_UseMedKit(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	pPlayer->SetHP(pPlayer->GetHP() + 20);
}

void CProcessPacket::CL_GS_UseVaccine(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	pPlayer->m_isInfection = false;
}

void CProcessPacket::CL_GS_GetVictim(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	ItemManager()->CL_GS_GetVictim(pRecvedMsg);
}

void CProcessPacket::CL_GS_EquipGun(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(pPlayer->m_equipGun);

	PlayerManager()->CL_GS_EquipGun(pPlayer, pRecvedMsg);
}

void CProcessPacket::CL_GS_CL_CureDeadPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	unsigned int playerKey = 0;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(playerKey);
	CPlayer* pCurePlayer = PlayerManager()->FindPlayer(playerKey);
	if (nullptr == pCurePlayer)
		return;
	pCurePlayer->SetHP(20);
	pCurePlayer->m_isDead = false;

	PlayerManager()->Send_TCP_RecvBufferFromServer(pRecvedMsg, dwSize);
}

void CProcessPacket::CL_GS_CL_Chat(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	PlayerManager()->Send_TCP_RecvBufferFromServer(pRecvedMsg, dwSize);
}

void CProcessPacket::fnStartLobby_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->StartLobby_Not(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}