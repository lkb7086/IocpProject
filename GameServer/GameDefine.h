#pragma once

const int MAX_ID_LENGTH = 45;
const int MAX_PASS_LENGTH = 45;
const int MAX_NICKNAME_LENGTH = 45;

const int MAX_LOGFILENAME_LENGTH = 128;
const int SERVER_TICK = 1000;
const int UPDATE_TICK = 16;
const int MAX_PROCESSFUNC = 0xFFFF;
const int MAX_USER_COUNT = 3000;
const int KEEPALIVE_TICK = 60 * 30;

const int AREA_SECTOR_CNT = 100; // 한 행or열의 섹터갯수
const int AREA_SECTOR_LINE = 100; // 한 섹터의 크기(디폴트 10.0f)
const int MAX_AREA = AREA_SECTOR_CNT * AREA_SECTOR_CNT;
const int AREA_HALF_POS = AREA_SECTOR_LINE * AREA_SECTOR_CNT / 2; // 전체 지역 좌표의 반

const int MAX_ACTIVE_AREAS = 9;
const int MAX_INACTIVE_AREAS = 9;

const unsigned short MAX_INVEN_SLOT = 25;




const float DEFAULT_DETECT_DISTANCE = 20.0f;
const float DEFAULT_DETECT_DISTANCE_SQR = 400.0f;
const int DEFAULT_REGEN_TIME = 3000;



enum EDirection : int
{
	DIR_LEFTUP = 0,
	DIR_UP,
	DIR_RIGHTUP,
	DIR_LEFT,
	DIR_CENTER,
	DIR_RIGHT,
	DIR_LEFTDOWN,
	DIR_DOWN,
	DIR_RIGHTDOWN
};

enum class NPC_Type : int
{
	NONE_NPC = 0,
	NORMAL_NPC,
	DETECT_NPC,
	MOVING_NPC
};

enum class NPC_Code : int
{
	ZOMBIE1,
	ZOMBIE2,
	GHOUL,
	SLIDER,
	FLY,
	FAT,
	SPIDER,
	TROLL
};

enum class ItemCode : int
{
	// 총
	Rifle,
	Shotgun,
	SniperRifle,
	FlameThrower,
	RocketLauncher,
	// 총알
	RifleBullet,
	ShotgunBullet,
	SniperRifleBullet,
	FlameThrowerBullet,
	RocketLauncherBullet,
	// 약품
	MedKit,
	MaxCount
};