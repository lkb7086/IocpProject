#include "StdAfx.h"
//#include "player.h"

CPlayer::CPlayer()
{
	//srand(GetTickCount64());
	//srand(time(nullptr));
	InitPlayer();
}

CPlayer::~CPlayer()
{
}

//변수를 초기화 시킨다.
void CPlayer::InitPlayer()
{
	m_isConfirm = false;
	m_key = 0;

	m_pPrevMoveNode = nullptr;
	InitPrevMoveNode();

	ZeroMemory(m_szID, MAX_ID_LENGTH);
	ZeroMemory(m_szNickName, MAX_NICKNAME_LENGTH);

	//memset( m_byInActiveAreas , -1 , MAX_INACTIVE_AREAS );
	//memset( m_byActiveAreas , -1 , MAX_ACTIVE_AREAS );
	for (int i = 0; i < MAX_INACTIVE_AREAS; ++i)
		m_inActiveAreas[i] = -1;
	for (int i = 0; i < MAX_ACTIVE_AREAS; ++i)
		m_activeAreas[i] = -1;

	m_autoTargetedNPCKey = 0xffffffff;
	m_nLevel = 0;
	m_damage = 0;
	m_nHP = 100;
	m_nExp = 0;
	//m_nZone = 0xFFFF;
	m_nArea = -1;
	m_nMoney = 0;

	m_nAccox_UID = 0xffffffff;
	m_nUnit_UID = 0xffffffff;

	m_isInfection = false;
	m_bIsAutoAttack = false;
	m_bIsDummy = false;
	m_isAccept = false;
	m_isDead = false;
	m_isInitNPCInfo = false;
	m_nItemSlotsCnt = 0;
	m_equipGun = 0xFF;
	m_isHost = false;

	m_arrItem.fill(stItem());
}

void CPlayer::SetPlayInfo(char* pRecvedMsg)
{
	// pkey class area x y uid
	UINT PKey; // 더미데이터
	tls_pSer->StartDeserialize(pRecvedMsg);
	tls_pSer->Deserialize(PKey);
	tls_pSer->Deserialize(m_nClass);
	tls_pSer->Deserialize(m_nArea);
	tls_pSer->Deserialize(m_pos.x);
	tls_pSer->Deserialize(m_pos.y);
	tls_pSer->Deserialize(m_nUnit_UID);
	tls_pSer->Deserialize(m_nMoney);
	tls_pSer->Deserialize(m_nExp);

	//printf("%d %d %d %f %f %d\n", PKey, m_nClass, m_nArea, m_vPos.x, m_vPos.y, m_nUnit_UID);

	// 인벤토리
	tls_pSer->Deserialize(m_nItemSlotsCnt);
	if (0 > m_nItemSlotsCnt || MAX_INVEN_SLOT < m_nItemSlotsCnt)
		return;
	if (0 != m_nItemSlotsCnt)
	{
		unsigned __int64 nUID = 0; unsigned int nCode = 0; int nAmount = 0; unsigned short nSlot = 0;
		for (int i = 0; i < m_nItemSlotsCnt; i++)
		{
			tls_pSer->Deserialize(nUID); // uid
			tls_pSer->Deserialize(nCode); // code
			tls_pSer->Deserialize(nAmount); // amount
			tls_pSer->Deserialize(nSlot); // slot
			
			m_arrItem[nSlot].nUid = nUID;
			m_arrItem[nSlot].nCode = nCode;
			m_arrItem[nSlot].nAmount = nAmount;
			m_arrItem[nSlot].nSlot = nSlot;

			//ItemManager()->code(*this, nSlot);
		}
	}

	//sort(m_arrItem, m_arrItem + (MAX_INVEN_SLOT - 1));
	for (int i = 0; i < 25; i++)
	{
		//cout << m_arrItem[i].nUid << " ";
	}
	//cout << endl;

	//SetZone(0); // 임시
	SetArea(AreaManager()->GetPosToArea(this->m_pos));
	
	//문제
	//AreaManager()->AllPopNewArea(0);
	//AreaManager()->UpdateActiveAreas(this);
	//AreaManager()->AddPlayerToArea(this, m_nArea);
}