#pragma once

enum class PacketType : packet_type
{
	// �α��μ���
	ConfirmID_Req = 0,
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
	StartLogin_Not,



	// �̵�
	MovePlayer_Req,
	MovePlayer_Res,

	// �÷�
	ChangeColor_Req,
	ChangeColor_Res,
	// �����̵�
	MoveServer_Req,
	MoveServer_Res,
	PlayerInfoAndMoveLevel_Req,
	PlayerInfoAndMoveLevel_Res,





	// ����
	UpdateAreaForDeleteObject_Not,
	UpdateAreaForCreateObjectV_Not,
	UpdateAreaForCreateObject_Not,
	UpdateAreaForDeleteObjectV_Not,


	CL_GS_CL_Chat,




	// �������� �ְ�޴� Ÿ��
	ImServer_Not,
	ConfirmIDGameServer_Req,
	ConfirmIDGameServer_Res,
	StartLobby_Not,
	LogoutPlayerDB_Not,
	LogoutPlayerID_Not,
	MoveServer_Not1,
	MoveServer_Not2,

	// ���
	ServerTestPacket = 40000,
	Notice_Not,

	// GameDB�� Ŭ���̾�Ʈ�� �ְ�޴� Ÿ��
	CreateCharacter_Req = 50001,
	DeleteCharacter_Req,
	StartGame_Req,

	// Nosql
	Nosql_Not = 0xFFFF
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
	float posX;
	float posY;
	float posZ;
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