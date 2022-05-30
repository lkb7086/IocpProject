#pragma once

enum class PacketType : packet_type
{
	// 로그인서버
	ConfirmID_Req = 0,
	ConfirmID_Res,
	JoinID_Req,
	JoinID_Res,
	// 게임서버
	Accept_Not,
	StartLobby_Req,
	StartLobby_Res,	
	CreateCharacter_Res,
	DeleteCharacter_Res,
	StartGame_Res,



	// 이동
	MovePlayer_Req,
	MovePlayer_Res,

	// 컬러
	ChangeColor_Req,
	ChangeColor_Res,
	// 서버이동
	MoveServer_Req,
	MoveServer_Res,
	PlayerInfoAndMoveLevel_Req,
	PlayerInfoAndMoveLevel_Res,





	// 지역
	CL_GS_UpdateAreaForDeleteObject,
	UpdateAreaForCreateObjectV_Not,
	UpdateAreaForCreateObject_Not,
	CL_GS_UpdateAreaForDeleteObjectV,


	CL_GS_CL_Chat,




	// 서버끼리 주고받는 타입
	ImServer_Not,
	ConfirmIDGameServer_Req,
	ConfirmIDGameServer_Res,
	StartLobby_Not,
	StartLogin_Not,
	LogoutPlayerID_Not,
	MoveServer_Not1,
	MoveServer_Not2,

	// 툴
	ServerTestPacket = 40000,
	Notice_Not = 40001,

	// GameDB와 주고받는 타입
	TestLogin_Rq = 50001,
	CreateCharacter_Req,
	DeleteCharacter_Req,
	StartGame_Req,


	Nosql_Not = 0xFFFF
};






#pragma pack(push, 1) // 먼저 원본 정렬을 스택에 push하고 -> 1바이트 정렬로 바꾼다 -> 나중에 pop해서 원본(4바이트) 정렬로 되돌린다
struct PacketHeader
{
	packet_length length;
	packet_type type;
};

// 유틸리티
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