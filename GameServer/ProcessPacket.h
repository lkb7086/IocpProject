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

	// ���� TCP��ƿ��Ƽ
	static void fnSendToWorldPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// ���� TCP��ƿ��Ƽ
	static void fnSendToAreaPlayer_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// ���� ��ƿ��Ƽ
	static void fnSendToLoginServer_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);




	static void fnConfirmID_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg); // �α��μ��� <-> ���Ӽ���
	static void fnStartLobby_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnCreateCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnDeleteCharacter_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnStartGame_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);



	static void fnMovePlayer_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnChangeColor_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnMoveServer_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnPlayerInfoAndMoveLevel_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	



	// �α���
	static void fnStartLobby_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnStartLogin_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnMoveServer_Not1(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void fnMoveServer_Not2(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	




	// NPC�ʱ�ȭ
	static void CL_GS_CurNPCPosFromHost(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	// ������
	static void CL_GS_GetItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_DiscardItem(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_UseMedKit(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_UseVaccine(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_GetVictim(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_EquipGun(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_CL_CureDeadPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// ����
	static void CL_GS_CL_PlayerAreaAttack(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_PlayerAttackToNPC(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	static void CL_GS_NPCAttackToPlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	static void CL_GS_CL_Chat(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	// ����üũ
	static void fnKeepAliveCn(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);

	static void fnServerTestPacket(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
};
