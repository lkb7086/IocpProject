#include "StdAfx.h"
#include "processpacket.h"

void CProcessPacket::fnSendToWorldPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	PlayerManager()->Send_TCP_RecvBufferFromServer(pRecvedMsg, dwSize);
}


void CProcessPacket::fnSendToLoginServer_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	CConnection* pCon = IocpGameServer()->GetLoginServerConn();
	if (nullptr == pCon)
		return;
	char* pBuffer = pCon->PrepareSendPacket(dwSize);
	if (nullptr == pBuffer)
		return;
	CopyMemory(pBuffer, pRecvedMsg, dwSize);
	pCon->SendPost(dwSize);
}



void CProcessPacket::fnSendToAreaPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	AreaManager()->Send_TCP_RecvBufferFromServer(dwSize, pRecvedMsg);
}


void CProcessPacket::fnConfirmIDGameServer_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
    IocpGameServer()->ConfirmIDGameServer_Req(pPlayer, pRecvedMsg);

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



void CProcessPacket::fnStartLogin_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	if (pPlayer->GetIsConfirm() == false)
	{
		PlayerManager()->AddPlayer(pPlayer);
		PlayerManager()->InitPlayerInfo(*pPlayer);

		pPlayer->SetKeepAliveTick(IocpGameServer()->GetServerTick());
		pPlayer->SetIsConfirm(true);
	}

	///*
	AreaManager()->AddPlayerToArea(pPlayer, AreaManager()->GetPosToArea(pPlayer->m_pos));
	AreaManager()->UpdateActiveAreas(pPlayer);
	AreaManager()->Send_UpdateAreaForCreateObject(pPlayer);
	//*/



	LOG(LOG_INFO_LOW, "ID (%u) Connected. / Current Players (%u)",
		pPlayer->GetKey(), PlayerManager()->GetPlayerCnt());

	/*
	char result = 0;
	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::StartGame_Res));
	tls_pSer->Serialize(result);
	tls_pSer->Serialize(pPlayer->GetNickName());
	char* pBuffer = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	pPlayer->SendPost(tls_pSer->GetCurBufSize());
	*/
}

void CProcessPacket::fnMoveServer_Not1(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	// map에 키랑 캐릭터정보들 넣는다
	char nextServerID = -1; char prevServerID = -1; unsigned int playerKey = 0;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(nextServerID);
	tls_pSer->Deserialize(prevServerID);
	tls_pSer->Deserialize(playerKey);

	unsigned int nextKey = PlayerManager()->MoveServer_Not1(pRecvedMsg);


	// 키랑 다음서버번호, 전서버번호 보낸다
	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::MoveServer_Not2));
	tls_pSer->Serialize(prevServerID);
	tls_pSer->Serialize(nextServerID);
	tls_pSer->Serialize(playerKey);
	tls_pSer->Serialize(nextKey);
	

	CConnection* pCon = IocpGameServer()->GetLoginServerConn();
	if (pCon == nullptr)
		return;
	char* pBuffer = pCon->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	pCon->SendPost(tls_pSer->GetCurBufSize());
}

void CProcessPacket::fnMoveServer_Not2(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	// nextID predID  playerKey nextPlayerKey
	char nextServerID = -1; char prevServerID = -1; unsigned int playerKey = 0; unsigned int nextPlayerKey = 0;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(prevServerID);
	tls_pSer->Deserialize(nextServerID);
	tls_pSer->Deserialize(playerKey);
	tls_pSer->Deserialize(nextPlayerKey);


	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::MoveServer_Res));
	tls_pSer->Serialize(nextServerID);
	tls_pSer->Serialize(nextPlayerKey);
	CPlayer* realPlayer = PlayerManager()->FindPlayer(playerKey);
	if (realPlayer == nullptr)
		return;
	realPlayer->m_isMoveServer = true;
	char* pBuffer = realPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	realPlayer->SendPost(tls_pSer->GetCurBufSize());
}

void CProcessPacket::fnMovePlayer_Req(CPlayer* _pPlayer, DWORD dwSize, char* _pRecvedMsg)
{
//#ifndef _DEBUG
	AreaManager()->Send_MovePlayerToActiveAreas(_pPlayer, _pRecvedMsg);
//#endif
	
	//while (InterlockedExchange((LPLONG)&s_bNpcServLock, TRUE) == TRUE)
	//Sleep(0);
	//InterlockedExchange((LPLONG)&s_bNpcServLock, FALSE);
}

void CProcessPacket::fnChangeColor_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(pPlayer->m_color.r);
	tls_pSer->Deserialize(pPlayer->m_color.g);
	tls_pSer->Deserialize(pPlayer->m_color.b);
	tls_pSer->Deserialize(pPlayer->m_color.a);
	tls_pSer->Deserialize(pPlayer->m_color.mode);

	AreaManager()->ChangeColor_Req(pPlayer);
}

void CProcessPacket::fnMoveServer_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	char nextServerID = -1;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(nextServerID);

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::MoveServer_Not1));
	tls_pSer->Serialize(nextServerID);
	tls_pSer->Serialize(IocpGameServer()->m_serverID);
	tls_pSer->Serialize(pPlayer->GetKey());

	// 캐릭터정보들 직렬화
	tls_pSer->Serialize(pPlayer->GetID());
	tls_pSer->Serialize(pPlayer->GetNickName());
	tls_pSer->Serialize(pPlayer->GetSpecies());
	tls_pSer->Serialize(pPlayer->GetGender());
	tls_pSer->Serialize(pPlayer->GetHeight());
	tls_pSer->Serialize(pPlayer->GetWidth());

	


	CConnection* pCon = IocpGameServer()->GetLoginServerConn();
	if (nullptr == pCon)
		return;
	char* pBuffer = pCon->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	pCon->SendPost(tls_pSer->GetCurBufSize());
}


void CProcessPacket::fnPlayerInfoAndMoveLevel_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	unsigned int nextPlayerKey = 0;
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(nextPlayerKey);
	pPlayer->SetPKey(nextPlayerKey);

	fnStartLogin_Not(pPlayer, dwSize, pRecvedMsg);

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::PlayerInfoAndMoveLevel_Res));

	char* pBuffer = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pBuffer)
		return;
	tls_pSer->CopyBuffer(pBuffer);
	pPlayer->SendPost(tls_pSer->GetCurBufSize());
}





void CProcessPacket::CL_GS_CL_PlayerAreaAttack(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	NPCManager()->CL_GS_CL_PlayerAreaAttack(pPlayer, dwSize, pRecvedMsg);
}

void CProcessPacket::CL_GS_PlayerAttackToNPC(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	NPCManager()->SuccessAttackToNPC_Cn(pPlayer, pRecvedMsg);
}

void CProcessPacket::fnKeepAliveCn(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
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

void CProcessPacket::CL_GS_UseMedKit(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	pPlayer->SetHP(pPlayer->GetHP() + 20);
}

void CProcessPacket::CL_GS_UseVaccine(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	pPlayer->m_isInfection = false;
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

void CProcessPacket::fnServerTestPacket(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	pPlayer->m_bIsDummy = true;
	pPlayer->m_pos = Vector3(CMTRand::GetRand_float(-3500.0f, 3500.0f), CMTRand::GetRand_float(-3500.0f, 3500.0f), 100.0f);
	fnStartLogin_Not(pPlayer, dwSize, pRecvedMsg);
	//fnImInWorld_Not(pPlayer, dwSize, pRecvedMsg);
}

void CProcessPacket::fnStartLobby_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->StartLobby_Not(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}

void CProcessPacket::fnLogoutPlayerDB_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->LogoutPlayerDB_Not(stPlayerInfo(pPlayer, dwSize, pRecvedMsg));
}
