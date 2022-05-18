#pragma once

class CProcessPacket final
{
public:
	CProcessPacket() = delete;
	~CProcessPacket() = delete;
	CProcessPacket(const CProcessPacket&) = delete;
	CProcessPacket& operator=(const CProcessPacket&) = delete;
	CProcessPacket(CProcessPacket&&) = delete;
	CProcessPacket& operator=(CProcessPacket&&) = delete;

	// 월드 TCP유틸리티
	static void fnSendToWorldPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// 지역 TCP유틸리티
	static void fnSendToAreaPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// 서버 유틸리티
	static void fnSendToNS_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);      // 클라이언트에서 받은 버퍼를 NPC서버에 그대로 보낸다
	static void fnSendToDA_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);      // 클라이언트에서 받은 버퍼를 DA에 그대로 보낸다
	// DBqueue
	static void fnPushDBQueue(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);




	static void fnConfirmID_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg); // 로그인서버 <-> 게임서버
	static void fnStartLobby_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnCreateCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnDeleteCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnStartGame_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);




	// 로그인
	static void CL_GS_Login(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// NPC초기화
	static void CL_GS_CurNPCPosFromHost(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// 이동
	static void CL_GS_MovePlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// 아이템
	static void CL_GS_GetItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_DiscardItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_UseMedKit(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_UseVaccine(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_GetVictim(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_EquipGun(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_CL_CureDeadPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// 공격
	static void CL_GS_CL_PlayerAreaAttack(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_PlayerAttackToNPC(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_NPCAttackToPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	static void CL_GS_CL_Chat(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	// 좀비체크
	static void fnKeepAliveCn(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	static void fnStartLobby_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
};
