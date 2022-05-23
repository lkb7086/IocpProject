#pragma once

enum class PacketType : packet_type
{
	// �α��μ���
	ConfirmID_Req,
	ConfirmID_Res,
	JoinID_Req,
	JoinID_Res,
	// ���Ӽ���
	Accept_Not,
	StartLobby_Req,
	StartLobby_Res,	
	CreateCharacter_Res,
	DeleteCharacter_Res,
	StartGame_Res,












	// �α���
	CL_GS_Login,
	GS_CL_LoginInfo,
	GS_CL_WorldPlayerInfo,
	GS_CL_LoginPlayerInfo,
	// NPC�ʱ�ȭ
	GS_CL_NPCInfo,
	GS_CL_CurNPCPosFromHost,
	CL_GS_CurNPCPosFromHost,
	// NPC������Ʈ
	GS_CL_UpdateNPC,
	// �α׾ƿ�
	GS_CL_LogoutPlayer,
	// �̵�
	CL_GS_MovePlayer,
	GS_CL_MovePlayer,
	// ������
	GS_CL_InitItemInfo,
	GS_CL_VictimInfo,
	CL_GS_GetItem,
	GS_CL_GetItem,
	CL_GS_DiscardItem,
	GS_CL_DiscardItem,
	CL_GS_UseMedKit,
	CL_GS_UseVaccine,
	CL_GS_GetVictim,
	GS_CL_GetVictim,
	CL_GS_EquipGun,
	GS_CL_EquipGun,
	CL_GS_CL_CureDeadPlayer,
	// ����
	CL_GS_UpdateAreaForDeleteObject,
	UpdateAreaForCreateObjectV_Not,
	UpdateAreaForCreateObject_Not,
	CL_GS_UpdateAreaForDeleteObjectV,
	// ����
	GS_CL_PlayerAttackEffect,
	CL_GS_CL_PlayerAreaAttack,
	CL_GS_PlayerAttackToNPC,
	CL_GS_NPCAttackToPlayer,
	GS_CL_PlayerInfection,
	GS_CL_PlayerHP,
	GS_CL_PlayerDead,
	// ���̺�
	GS_CL_StartWave,

	GS_CL_DayAndNightTime,

	CL_GS_CL_Chat,




	// �������� �ְ�޴� Ÿ��
	ImServer_Not,
	ConfirmIDGameServer_Req,
	ConfirmIDGameServer_Res,
	StartLobby_Not,
	LogoutPlayerID_Not,

	ServerTestPacket = 40000,
	Notice_Not = 40001,

	// GameDB�� �ְ�޴� Ÿ��
	TestLogin_Rq = 50001,
	CreateCharacter_Req,
	DeleteCharacter_Req,
	StartGame_Req,



};






#pragma pack(push, 1) // ���� ���� ������ ���ÿ� push�ϰ� -> 1����Ʈ ���ķ� �ٲ۴� -> ���߿� pop�ؼ� ����(4����Ʈ) ���ķ� �ǵ�����
struct PacketHeader
{
	packet_length length;
	packet_type type;
};

// ��ƿ��Ƽ
struct stUtil_Empty : PacketHeader {};
struct stUtil_Char : PacketHeader { unsigned char nChar; };
struct stUtil_UChar : PacketHeader { unsigned short nUChar; };
struct stUtil_UShort : PacketHeader { unsigned short nUShort; };
struct stUtil_Integer : PacketHeader { int nInteger; };
struct stUtil_UInteger : PacketHeader { unsigned int nUInteger; };

struct stUtil_Int_Int : PacketHeader { int nInteger1; int nInteger2; };
struct stUtil_Int_UInt : PacketHeader { int nInteger; unsigned int nUInteger; };
struct stUtil_UInt_Int : PacketHeader { unsigned int nUInteger; int nInteger; };
struct stUtil_UInt_UShort : PacketHeader { unsigned int nUInteger; unsigned short nUShort; };
struct stUtil_UShort_UInt : PacketHeader { unsigned short nUShort; unsigned int nUInteger; };

struct stUtil_US_UI_UI : PacketHeader { unsigned short nUShort; unsigned int nUInteger1; unsigned int nUInteger2; };

struct stUtil_UI_UI_S : PacketHeader { unsigned int nUInteger1; unsigned int nUInteger2; short nShort; };

struct stUtil_UI_F_F : PacketHeader { unsigned int nUInteger; float nFloat1; float nFloat2; };










struct MovePlayer_Cn : PacketHeader
{
	unsigned int nPKey;
	float x;
	float y;
	float z;
};


struct MovePlayer_Sn : PacketHeader
{
	unsigned int key;
	float posX;
	float posY;
	float posZ;
};

struct stCL_GS_UpdateAreaForDeleteObject : PacketHeader
{
	unsigned int	uiPKey;
	int hp;
	float fPosX;
	float fPosY;
	float fPosZ;
};
#pragma pack(pop)