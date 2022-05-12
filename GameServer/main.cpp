#include "stdafx.h"
//#pragma comment( lib, "NetLib.lib" )
#define _CRT_SECURE_NO_WARNINGS
HANDLE g_hMutex = NULL;

void OnCreateNpc();

int _tmain(int argc, _TCHAR* argv[])
{
	char command[256] = { '\0', }; //Windows CMD ��ɾ ���� ���� �迭
	int lines = 30; //���� ����
	int cols = 150; //���� ����
	_snprintf_s(command, _countof(command), _TRUNCATE, "mode con: lines=%d cols=%d", lines, cols);
	system(command); //Windows CMD ��ɹ��� �����ϴ� �Լ�

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

	puts("���� ����");
	system("pause");

	return 0;
}

void OnCreateNpc()
{
	int num = 20;
	// ����
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
	// �ұԸ�(��ã��)
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
	// ��Ը�
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
ExecuteUpdateArea ����
NPC�� ExecuteUpdateArea�ȿ��� ���������� ����

��������Ŀ��Ʈ����: �ϴܿ����ϰ� �ٷ� ������� ��Ŷ������(�״��������������� ���� �����������͸� �����ִ´�)

DBmanger�� ��������� �������ϳ�
DB������ ForwardMark�� ���� �ȴ�. char* p = forward(len);

Ŭ�� ���̺귯���� �׽�Ʈ�����������

AREA_SECTOR_CNT
AREA_SECTOR_LINE
�ٽ����ϱ�


!zone�� �������� ��������, ����ä�ο����̵�







#����
���̵�(PK) ���


#ĳ���͵�
ĳ���̸�(PK) ���̵�(FK)
ĳ���̸� ���̵�


#�κ��丮
UID ĳ���̸�(FK)



Ǯ��ĵ�˻�



#���װ��ɼ�: DB�����忡�� ƽ������� ������ �� �����۰� �������� �� ����, Getbuffer()�� �ؾ��ϴ���
iocpserver ���μ����޴ºκп��� swtich����Ʈ������ OnRecvȣ���ϰ� ����
�ݵ�� tls_ser�� �� ��
m_setSERVER.erase(szID) ƽ�����忡�� �ð�ī��Ʈ�Ѵ��� �����?
*/