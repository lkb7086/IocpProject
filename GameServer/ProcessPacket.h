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
	static void fnTCP_World_SendToCL_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);  // �������� ���� ���۸� CL�� �״�� ������
	// ���� TCP��ƿ��Ƽ
	static void fnTCP_AREA_SendToCL_RecvBufferFromServer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg); // �������� ���� ���۸� CL�� �״�� ������
	// ���� ��ƿ��Ƽ
	static void fnSendToNS_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);      // Ŭ���̾�Ʈ���� ���� ���۸� NPC������ �״�� ������
	static void fnSendToDA_RecvBufferFromClient(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);      // Ŭ���̾�Ʈ���� ���� ���۸� DA�� �״�� ������
	// DBqueue
	static void fnPushDBQueue(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);




	static void fnConfirmID_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg); // �α��μ��� <-> ���Ӽ���
	static void fnStartLobby_Req(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);




	// �α���
	static void CL_GS_Login(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// NPC�ʱ�ȭ
	static void CL_GS_CurNPCPosFromHost(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	// �̵�
	static void CL_GS_MovePlayer(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
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

	static void fnStartLobby_Not(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	

	static void TestLogin_Rq(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
};