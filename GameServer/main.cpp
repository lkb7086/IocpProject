#include "stdafx.h"
//#pragma comment( lib, "NetLib.lib" )
#define _CRT_SECURE_NO_WARNINGS
HANDLE g_hMutex = NULL;

void OnCreateNpc();

int _tmain(int argc, _TCHAR* argv[])
{
	char command[256] = { '\0', }; //Windows CMD 명령어를 담을 문자 배열
	int lines = 30; //세로 길이
	int cols = 150; //가로 길이
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD 명령문을 실행하는 함수

	g_hMutex = CreateMutex(NULL, FALSE, _T("Mutex_GameServer"));
	if (NULL == g_hMutex)
		return 0;
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		puts("The game server is already running.");
		system("pause");
		return 0;
	}

	tls_pEngine = new(nothrow) std::mt19937_64((std::random_device())());
	if (nullptr == tls_pEngine)
		return 0;


	/*
	for (size_t i = 0; i < 100; i++)
	{
		cout << CMTRand::GetRand_uint64(2) << " ";
	}
	system("pause");
	return 0;
	*/

	//////////////////////////////////////////////////////////////////////////
	//_CrtSetBreakAlloc(16289);
	srand((unsigned int)time(NULL));

	//OnCreateNpc();
	IocpGameServer()->GameServerStart();
	IocpGameServer()->ConnectToLoginServer();
	//////////////////////////////////////////////////////////////////////////

	/*
	printf("yesterday %ws\n", Clock()->yesterday().c_str());
	printf("today %ws\n", Clock()->today().c_str());
	printf("tomorrow %ws\n", Clock()->tomorrow().c_str());
	printf("today is %d week of the day\n", Clock()->todayOfTheWeek());
	printf("system = %llu\n", Clock()->systemTick());
	printf("nowTime %ws\n", Clock()->nowTime().c_str());
	printf("MilliSec %ws\n", Clock()->nowTimeWithMilliSec().c_str());
	*/
	
	WaitForSingleObject(CIocpGameServer::m_hMainThreadEvent, INFINITE);

	IocpGameServer()->ServerOffTest();
	IocpGameServer()->ServerOff();
	NPCManager()->DestroyNpc();
	CSingleton::ReleaseAll();

	if (nullptr != tls_pEngine)
	{
		delete tls_pEngine;
		tls_pEngine = nullptr;
	}

	puts("서버 종료");
	system("pause");

	return 0;
}

void OnCreateNpc()
{
	int num = 20;
	// 고정
	///*
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::ZOMBIE1, num*2, 1, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::ZOMBIE2, num / 2, 1, 0));

	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::ZOMBIE2, num, 3, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::GHOUL, num, 3, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::SLIDER, num, 3, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::FLY, num, 3, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::FAT, num, 3, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::SPIDER, 2, 3, 0));

	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::SLIDER, num*3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::FLY, num, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::SPIDER, num, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::DETECT_NPC, NPC_Code::TROLL, num, 4, 0));
	//*/


	int num2 = 1;
	// 소규모(길찾기)
	///*
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::NORMAL_NPC, NPC_Code::ZOMBIE1, num2, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::NORMAL_NPC, NPC_Code::ZOMBIE2, num2, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::NORMAL_NPC, NPC_Code::GHOUL, num2, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::NORMAL_NPC, NPC_Code::SLIDER, num2, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::NORMAL_NPC, NPC_Code::FAT, num2, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::SPIDER, num2, 0, 0));
	//*/

	int num3 = 40;
	///*
	// 대규모
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::ZOMBIE1, num3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::ZOMBIE2, num3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::GHOUL, num3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::SLIDER, num3 / 3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::FAT, num3, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::FLY, num3, 0, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::SPIDER, num3 / 2, 4, 0));
	NPCManager()->CreateNpc(NPCInitData(NPC_Type::MOVING_NPC, NPC_Code::TROLL, 1, 4, 0));
	//*/
}






/*
SetArea(-1);
ExecuteUpdateArea 실행
NPC도 ExecuteUpdateArea안에서 마찬가지로 구현

서버끼리커넥트문제: 일단연결하고 바로 서버라고 패킷보낸다(그다음받은곳에서는 따로 관리할포인터를 만들어서넣는다)

DBmanger에 멤버변수로 링버퍼하나
DB링버퍼 ForwardMark만 쓰면 된다. char* p = forward(len);

클라 라이브러리는 테스트월드버전으로

AREA_SECTOR_CNT
AREA_SECTOR_LINE
다시정하기


!zone이 여러개면 복잡해짐, 서버채널월드이동







#인증
아이디(PK) 비번


#캐릭터들
캐릭이름(PK) 아이디(FK)
캐릭이름 아이디


#인벤토리
UID 캐릭이름(FK)



풀스캔검사



#버그가능성: DB스레드에서 틱스레드로 보내줄 때 링버퍼가 문제생길 수 있음, Getbuffer()를 해야하는지
iocpserver 프로세스받는부분에서 swtich디폴트값으로 OnRecv호출하고 있음
반드시 tls_ser로 쓸 것
m_setSERVER.erase(szID) 틱스레드에서 시간카운트한다음 지울것?
*/