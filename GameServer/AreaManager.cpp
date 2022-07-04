#include "StdAfx.h"
//#include "areamanager.h"

IMPLEMENT_SINGLETON(CAreaManager);

CAreaManager::CAreaManager() {}
CAreaManager::~CAreaManager()
{
	for (size_t j = 0; j < MAX_AREA; j++)
	{
		m_mapArea[j].clear();
	}
}

void CAreaManager::Send_TCP_RecvBufferFromServer(DWORD dwSize, char* pRecvBuffer)
{
	/*
	stUtil_UInteger *recv = (stUtil_UInteger*)pRecvBuffer;
	CPlayer *pPlayer = PlayerManager()->FindPlayer(recv->nUInteger);
	if (nullptr == pPlayer) return;

	int* pActiveAreas = pPlayer->GetActiveAreas();
	USHORT nZone = pPlayer->GetZone();

	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		int byActiveArea = pActiveAreas[i];
		if (0 > byActiveArea || byActiveArea >= MAX_AREA)
			continue;
		for (auto area_it = m_mapArea[nZone][byActiveArea].begin(); area_it != m_mapArea[nZone][byActiveArea].end(); ++area_it)
		{
			CPlayer* pAreaPlayer = (CPlayer*)*area_it;
			if (pAreaPlayer->GetIsConfirm() == false)
				continue;
			char* pSendBuffer = (char*)pAreaPlayer->PrepareSendPacket(dwSize);
			if (nullptr == pSendBuffer)
				continue;
			CopyMemory(pSendBuffer, pRecvBuffer, dwSize);
			pAreaPlayer->SendPost(dwSize);
		}
	}*/
}

bool CAreaManager::AddPlayerToArea(CPlayer* pPlayer, int area)
{
	if (0 > area || MAX_AREA <= area)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::AddPlayerToArea() | Wrong Area : %d", area);
		return false;
	}

	//CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive); // 없어도 된다, 왜냐하면 삭제를 이스레드가 하기 때문, 근데 여기는 원래 없어도됨

	auto it = m_mapArea[area].find(pPlayer);
	if (it != m_mapArea[area].end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::AddPlayerToArea() | PKey[%d]는 이미 m_mapArea[%d]에 있습니다.",
			pPlayer->GetKey(), area);
		return false;
	}
	pPlayer->SetArea(area);
	m_mapArea[area].insert(pPlayer);
	return true;
}

bool CAreaManager::RemovePlayerFromArea(CPlayer* pPlayer, int byArea)
{
	if (0 > byArea || MAX_AREA <= byArea)
	{
		//LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::RemovePlayerFromArea() | Wrong : %u / %d", nZone, byArea);
		return false;
	}

	CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive);

	auto area_it = m_mapArea[byArea].find(pPlayer);
	if (area_it == m_mapArea[byArea].end())
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CAreaManager::RemovePlayerFromArea() | PKey[%d]는 m_mapArea[%d]에 존재하지 않습니다.",
			pPlayer->GetKey(), byArea);
		return false;
	}

	m_mapArea[byArea].unsafe_erase(pPlayer);
	pPlayer->SetArea(-1);

	return true;
}

bool CAreaManager::PrepareUpdateArea(CPlayer* pPlayer, int newArea)
{
	if (0 > newArea || MAX_AREA <= newArea)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CAreaManager::PrepareUpdateArea() | PKey[%d]는 m_mapArea[%d]", pPlayer->GetKey(), newArea);
		return false;
	}

	int oldArea = pPlayer->GetArea();
	if (oldArea == newArea) // 지역이 바뀌지 않았다면
	{
		return false;
	}

	bool result = RemovePlayerFromArea(pPlayer, oldArea);
	if (false == result)
	{
		//return false;
	}
	
	result = AddPlayerToArea(pPlayer, newArea);
	if (false == result)
	{
		return false;
	}

	UpdateActiveAreas(pPlayer);

	return true;
}

void CAreaManager::UpdateActiveAreas(CPlayer* pPlayer)
{
	// 현재 플레이어가 속한 영역과 주위의 8개의 영역이 플레이어가 영향을
	// 줄 수 있는 영역이다. 최대 9개의 영역에 영향을 줄 수 있는데 그 영역을 구한다.
	int oldArea = pPlayer->GetArea();
	if (0 > oldArea || MAX_AREA <= oldArea)
	{
		return;
	}

	AllPopNewArea();

	int arrNewActiveAreas[MAX_ACTIVE_AREAS];
	fill_n(arrNewActiveAreas, MAX_ACTIVE_AREAS, -1);

	
	//왼쪽 위
	int newArea = oldArea - AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTUP] = newArea;
	//위
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_UP] = newArea;
	//오른쪽 위
	newArea = oldArea - AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTUP] = newArea;
	//왼쪽
	newArea = oldArea - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFT] = newArea;
	//중간
	arrNewActiveAreas[EDirection::DIR_CENTER] = oldArea;
	//오른쪽
	newArea = oldArea + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHT] = newArea;
	//왼쪽 아래
	newArea = oldArea + AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTDOWN] = newArea;
	//아래
	newArea = oldArea + AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_DOWN] = newArea;
	//오른쪽 아래
	newArea = oldArea + AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTDOWN] = newArea;
	

	/*
	// 왼쪽 위
	int newArea = oldArea + AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTUP] = newArea;
	//위
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_UP] = newArea;
	//오른쪽 위
	newArea = oldArea + AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTUP] = newArea;
	//왼쪽
	newArea = oldArea - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFT] = newArea;
	//중간
	arrNewActiveAreas[EDirection::DIR_CENTER] = oldArea;
	//오른쪽
	newArea = oldArea + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHT] = newArea;
	//왼쪽 아래
	newArea = oldArea - AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTDOWN] = newArea;
	//아래
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_DOWN] = newArea;
	//오른쪽 아래
	newArea = oldArea - AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTDOWN] = newArea;
	*/

	///////////////////////////////////////////////////////
	// 현재 활동영역과 이전 활동영역을 비교해서 이전 활동 영역중
	// 현재 활동하지 않는 영역 구함
	int* pPlayerActiveAreas = pPlayer->GetActiveAreas();
	int* pPlayerInActiveAreas = pPlayer->GetInActiveAreas();
	for (int i = 0; i < MAX_INACTIVE_AREAS; ++i)
		pPlayerInActiveAreas[i] = -1;
	int inActiveAreaCnt = 0;

	// InActiveArea를 구한다
	// 맨처음 함수가 호출되면 이 반복문에서 continue만 한다 (값이 설정되는 부분이 없다)
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		bool flag = false;

		//설정된 영역이 없다면
		if (0 > pPlayerActiveAreas[i])
			continue;

		for (int j = 0; j < MAX_ACTIVE_AREAS; j++)
		{
			if (pPlayerActiveAreas[i] == arrNewActiveAreas[j])    // 겹치면
			{
				flag = true;
				break;
			}
		}
		if (flag == true)   // 아무연산없이 컨티뉴
			continue;

		// 겹치는 영역이 아니라면 즉 InActiveArea라면
		pPlayerInActiveAreas[inActiveAreaCnt++] = pPlayerActiveAreas[i];
	}

	// 새로 활성화된 지역만 스택에 넣는다
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		bool flag = false;

		if (0 > arrNewActiveAreas[i])
			continue;

		for (int j = 0; j < MAX_ACTIVE_AREAS; j++)
		{
			if (arrNewActiveAreas[i] == pPlayerActiveAreas[j])
			{
				flag = true;
				break;
			}
		}
		if (flag == true)
			continue;

		//printf("push %d\n", byNewActiveAreas[i]);
		m_vecNewActiveArea.push_back(arrNewActiveAreas[i]);
	}

	// 플레이어의 활동영역을 새로운 영역으로 갱신
	memcpy(pPlayerActiveAreas, arrNewActiveAreas, sizeof(int) * MAX_ACTIVE_AREAS);

	/*
	LOG( LOG_INFO_LOW ,
	"SYSTEM | cAreaManager::UpdateActiveAreas() | ActiveArea[%d][%d][%d][%d][%d][%d][%d][%d][%d]"
	, pPlayerActiveAreas[0], pPlayerActiveAreas[1], pPlayerActiveAreas[2], pPlayerActiveAreas[3],
	pPlayerActiveAreas[4], pPlayerActiveAreas[5], pPlayerActiveAreas[6], pPlayerActiveAreas[7],
	pPlayerActiveAreas[8] );

	LOG(LOG_INFO_LOW,
	"SYSTEM | cAreaManager::UpdateActiveAreas() | InActiveArea[%d][%d][%d][%d][%d][%d][%d][%d][%d]"
	, pPlayerInActiveAreas[0], pPlayerInActiveAreas[1], pPlayerInActiveAreas[2], pPlayerInActiveAreas[3],
	pPlayerInActiveAreas[4], pPlayerInActiveAreas[5], pPlayerInActiveAreas[6], pPlayerInActiveAreas[7],
	pPlayerInActiveAreas[8]);
	*/
	return;
}

int CAreaManager::GetPosToArea(const Vector3& _pos)
{
	// 왼쪽위부터 x = 0, y = 0

	// x와 y를 다시 정의한다
	int nPosX = static_cast<int>(_pos.x + AREA_HALF_POS);
	int nPosY = static_cast<int>(abs(_pos.y - AREA_HALF_POS));
	// 실질적인 지역을 구한다
	int nArea = ((nPosY / AREA_SECTOR_LINE) * AREA_SECTOR_CNT) + nPosX / AREA_SECTOR_LINE;

	//printf("%d ", nArea);
	if (nArea < 0 || nArea >= MAX_AREA)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::GetPosToArea() | Wrong Area : %d", nArea);
		return -1;
	}

	return nArea;
}

void CAreaManager::Send_UpdateNPC_VSn()
{
	for (int i = 0; i < MAX_AREA; i++)
	{
		if (false == NPCManager()->GatherVBuffer_NpcInfo(i))
			continue;

		// i영역의 9방향을 구한다
		int area = i;
		int	arrActiveAreas[MAX_ACTIVE_AREAS];
		for (size_t i = 0; i < MAX_ACTIVE_AREAS; i++)
		{
			arrActiveAreas[i] = -1;
		}

		//왼쪽 위
		int fixedArea = area - AREA_SECTOR_CNT - 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_LEFTUP] = fixedArea;
		//위
		fixedArea = area - AREA_SECTOR_CNT;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_UP] = fixedArea;
		//오른쪽 위
		fixedArea = area - AREA_SECTOR_CNT + 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_RIGHTUP] = fixedArea;
		//왼쪽
		fixedArea = area - 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_LEFT] = fixedArea;
		//중간
		arrActiveAreas[EDirection::DIR_CENTER] = area;
		//오른쪽
		fixedArea = area + 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_RIGHT] = fixedArea;
		//왼쪽 아래
		fixedArea = area + AREA_SECTOR_CNT - 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_LEFTDOWN] = fixedArea;
		//아래
		fixedArea = area + AREA_SECTOR_CNT;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_DOWN] = fixedArea;
		//오른쪽 아래
		fixedArea = area + AREA_SECTOR_CNT + 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_RIGHTDOWN] = fixedArea;

		// 9방향 플레이어들에게 전송
		for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
		{
			int area = arrActiveAreas[i];
			if (0 > area || MAX_AREA <= area)
				continue;

			for (auto it = m_mapArea[area].begin(); it != m_mapArea[area].end(); ++it)
			{
				CPlayer* pPlayer = (CPlayer*)*it;
				//if (nullptr == pPlayer || pPlayer->m_isInitNPCInfo == false)
				if (nullptr == pPlayer)
					continue;
				char* pBuf = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
				if (nullptr == pBuf)
					continue;
				tls_pSer->CopyBuffer(pBuf);
				pPlayer->SendPost(tls_pSer->GetCurBufSize());
			}
		}
	} // for
}

void CAreaManager::Send_UpdateAreaForCreateObject(CPlayer* pPlayer)
{
	unsigned int stackSize = (unsigned int)m_vecNewActiveArea.size();
	unsigned int uiAreaPlayerCnt = 0;
	int newArea[MAX_ACTIVE_AREAS];
	for (int i = 0; i < MAX_ACTIVE_AREAS; ++i)
		newArea[i] = -1;

	// 새로활성화된 지역이 없다면 아무것도 안하고 나간다
	if (0 == stackSize || MAX_ACTIVE_AREAS < stackSize)
	{
		//AllPopNewArea();
		return;
	}

	int while_i = 0;
	while (!m_vecNewActiveArea.empty())
	{
		newArea[while_i] = m_vecNewActiveArea.back();
		m_vecNewActiveArea.pop_back();
		while_i++;
	}

	{
		//CMonitorSRW::OwnerSRW lock(m_srwArea, LockShared); // 없어도 된다, 왜냐하면 삭제를 이스레드가 하기 때문
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 액티브지역중에서도 새로생긴 액티브지역으로만 패킷을 보내야한다.
		// 브로드캐스트
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::UpdateAreaForCreateObject_Not));
		tls_pSer->Serialize(pPlayer->GetKey());
		tls_pSer->Serialize(pPlayer->m_pos.x);
		tls_pSer->Serialize(pPlayer->m_pos.y);
		tls_pSer->Serialize(pPlayer->m_pos.z);
		tls_pSer->Serialize(pPlayer->GetGender());
		tls_pSer->Serialize(pPlayer->GetHeight());
		tls_pSer->Serialize(pPlayer->GetWidth());
		tls_pSer->Serialize(pPlayer->m_rot.x);
		tls_pSer->Serialize(pPlayer->m_rot.y);
		tls_pSer->Serialize(pPlayer->m_rot.z);
		tls_pSer->Serialize(pPlayer->m_color.r);
		tls_pSer->Serialize(pPlayer->m_color.g);
		tls_pSer->Serialize(pPlayer->m_color.b);
		tls_pSer->Serialize(pPlayer->m_color.a);


		for (unsigned int i = 0; i < stackSize; i++)
		{
			if (0 > newArea[i] || newArea[i] >= MAX_AREA)
				continue;

			for (auto it = m_mapArea[newArea[i]].begin(); it != m_mapArea[newArea[i]].end(); it++)
			{
				CPlayer* pAreaPlayer = (CPlayer*)*it;
				if (pAreaPlayer == pPlayer)
					continue;

				char* pSendBuffer = pAreaPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
				if (nullptr == pSendBuffer)
					return;
				tls_pSer->CopyBuffer(pSendBuffer);
				pAreaPlayer->SendPost(tls_pSer->GetCurBufSize());

				uiAreaPlayerCnt++; // 밑에서 가변버퍼 보낼때 인원수가 필요하다.
			}
		}
	}

	//printf("Send_UpdateAreaForCreateObject %d\n", uiAreaPlayerCnt);
	// 유니캐스트
	if (0 < uiAreaPlayerCnt)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::UpdateAreaForCreateObjectV_Not));
		tls_pSer->Serialize(uiAreaPlayerCnt);
		for (unsigned int i = 0; i < stackSize; i++)
		{
			if (0 > newArea[i] || newArea[i] >= MAX_AREA)
				continue;

			for (auto it = m_mapArea[newArea[i]].begin(); it != m_mapArea[newArea[i]].end(); it++)
			{
				CPlayer* pAreaPlayer = (CPlayer*)*it;
				if (nullptr == pAreaPlayer || pPlayer == pAreaPlayer)
					continue;
				tls_pSer->Serialize(pAreaPlayer->GetKey());
				tls_pSer->Serialize(pAreaPlayer->m_pos.x);
				tls_pSer->Serialize(pAreaPlayer->m_pos.y);
				tls_pSer->Serialize(pAreaPlayer->m_pos.z);
				tls_pSer->Serialize(pAreaPlayer->GetGender());
				tls_pSer->Serialize(pAreaPlayer->GetHeight());
				tls_pSer->Serialize(pAreaPlayer->GetWidth());
				tls_pSer->Serialize(pAreaPlayer->m_rot.x);
				tls_pSer->Serialize(pAreaPlayer->m_rot.y);
				tls_pSer->Serialize(pAreaPlayer->m_rot.z);
				tls_pSer->Serialize(pAreaPlayer->m_color.r);
				tls_pSer->Serialize(pAreaPlayer->m_color.g);
				tls_pSer->Serialize(pAreaPlayer->m_color.b);
				tls_pSer->Serialize(pAreaPlayer->m_color.a);
			}
		}
		char* pSendBuffer = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
		if (nullptr == pSendBuffer)
			return;
		tls_pSer->CopyBuffer(pSendBuffer);
		pPlayer->SendPost(tls_pSer->GetCurBufSize());
	}

	//AllPopNewArea();
}

void CAreaManager::Send_UpdateAreaForDeleteObject(CPlayer* pPlayer, bool isNormal)
{
	int* pInActiveAreas = pPlayer->GetInActiveAreas();
	m_vecDeleteArea.clear();

	{
		//CMonitorSRW::OwnerSRW lock(m_srwArea, LockShared); // 없어도 된다, 왜냐하면 삭제를 이스레드가 하기 때문
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::UpdateAreaForDeleteObject_Not));
		tls_pSer->Serialize(pPlayer->GetKey());

		for (int i = 0; i < MAX_INACTIVE_AREAS; i++)
		{
			int inActiveArea = pInActiveAreas[i];

			if (0 > inActiveArea || inActiveArea >= MAX_AREA)
				continue;

			for (auto it = m_mapArea[inActiveArea].begin(); it != m_mapArea[inActiveArea].end(); it++)
			{
				// 플레이어 끄라고 브로드캐스팅
				CPlayer* pAreaPlayer = (CPlayer*)*it;
				if (pAreaPlayer == pPlayer)
					continue;

				char* pBuffer = pAreaPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
				if (nullptr == pBuffer)
					continue;
				tls_pSer->CopyBuffer(pBuffer);
				pAreaPlayer->SendPost(tls_pSer->GetCurBufSize());

				// 트리거밟은 플레이어를 끈 플레이어들의 키를 모음
				if (isNormal)
					m_vecDeleteArea.push_back(pAreaPlayer->GetKey());
			}
		}
	}

	if (m_vecDeleteArea.empty())
		return;

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::UpdateAreaForDeleteObjectV_Not));
	tls_pSer->Serialize((UINT)m_vecDeleteArea.size());
	while (!m_vecDeleteArea.empty())
	{
		tls_pSer->Serialize(m_vecDeleteArea.back());
		m_vecDeleteArea.pop_back();
	}

	char* pSendBuffer = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
	if (nullptr == pSendBuffer)
		return;
	tls_pSer->CopyBuffer(pSendBuffer);
	pPlayer->SendPost(tls_pSer->GetCurBufSize());

	m_vecDeleteArea.clear();
}

void CAreaManager::Send_MovePlayerToActiveAreas(CPlayer* pPlayer, char *pRecvedMsg)
{
	///*
	if (false == pPlayer->GetIsConfirm())
		return;
	
	if (!pPlayer->m_bIsDummy)
	{
		MovePlayer_Cn* pMove = (MovePlayer_Cn*)pRecvedMsg;
		pPlayer->m_pos.x = pMove->posX;
		pPlayer->m_pos.y = pMove->posY;
		pPlayer->m_pos.z = pMove->posZ;

		pPlayer->m_rot.x = pMove->rotX;
		pPlayer->m_rot.y = pMove->rotY;
		pPlayer->m_rot.z = pMove->rotZ;
	}
	else
	{
		if (rand() % 2 == 0)
		{
			if (rand() % 2 == 0)
				pPlayer->m_pos.x -= 10.0f;
			else
				pPlayer->m_pos.x += 10.0f;

			if (rand() % 2 == 0)
				pPlayer->m_pos.y -= 10.0f;
			else
				pPlayer->m_pos.y += 10.0f;
		}
	}
	//*/

	//printf("T");
	CMonitorSRW::OwnerSRW lock(m_srwArea, LockShared);

	int* pActiveAreas = pPlayer->GetActiveAreas();
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		int activeArea = pActiveAreas[i];
		if (0 > activeArea || activeArea >= MAX_AREA)
			continue;
		for (auto it = m_mapArea[activeArea].begin(); it != m_mapArea[activeArea].end(); ++it)
		{
			CPlayer* pAreaPlayer = (CPlayer*)*it;
			//if (pAreaPlayer == pPlayer || pAreaPlayer->m_bIsDummy)
			if (pAreaPlayer == pPlayer)
				continue;

			MovePlayer_Sn* pMove = (MovePlayer_Sn*)pAreaPlayer->PrepareSendPacket(sizeof(MovePlayer_Sn));
			//MovePlayer_Sn* pMove = (MovePlayer_Sn*)IocpGameServer()->UDP_unsafe_PrepareSendPacket(sizeof(MovePlayer_Sn));
			
			if (nullptr == pMove)
				continue;
			//pMove->type = static_cast<packet_type>(PacketType::GS_CL_MovePlayer);
			pMove->type = static_cast<packet_type>(PacketType::MovePlayer_Res);
			pMove->key = pPlayer->GetKey();
			pMove->posX = pPlayer->m_pos.x;
			pMove->posY = pPlayer->m_pos.y;
			pMove->posZ = pPlayer->m_pos.z;
			pMove->rotX = pPlayer->m_rot.x;
			pMove->rotY = pPlayer->m_rot.y;
			pMove->rotZ = pPlayer->m_rot.z;

			pAreaPlayer->SendPost(sizeof(MovePlayer_Sn));
			//IocpGameServer()->UDP_unsafe_SendPost(sizeof(MovePlayer_Sn), pAreaPlayer);
		}
	}
}


void CAreaManager::DetectPlayerFromNPC(CNPC* pDetectNPC)
{
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		int area = pDetectNPC->m_activeAreas[i];
		if (0 > area || MAX_AREA <= area)
			continue;

		for (auto it = m_mapArea[area].begin(); it != m_mapArea[area].end(); ++it)
		{
			CPlayer* pPlayer = (CPlayer*)*it;
			if (nullptr == pPlayer || pPlayer->m_isDead || !pPlayer->m_isInitNPCInfo)
				continue;

			//float distance = Vector3::Distance(pPlayer->m_pos, pDetectNPC->m_pos);
			float sqr = (pPlayer->m_pos - pDetectNPC->m_pos).sqrMagnitude();
			//if (DEFAULT_DETECT_DISTANCE < distance)
			if (pDetectNPC->m_detectDisSqr < sqr)
			{
				//printf("몬스터와 아직 안 가깝다   /    몬스터와의 거리: %f\n", distance);
				continue;
			}

			// 가까우면
			pDetectNPC->SetTagetPlayerPKey(pPlayer->GetKey());
			pDetectNPC->SetEvent(Event::EVENT_PLAYER_APPEAR);
			break;
		}
	}
}



void CAreaManager::ChangeColor_Req(CPlayer* pPlayer)
{
	/*
	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::ChangeColor_Res));
	tls_pSer->Serialize(pPlayer->m_color.r);
	tls_pSer->Serialize(pPlayer->m_color.g);
	tls_pSer->Serialize(pPlayer->m_color.b);
	tls_pSer->Serialize(pPlayer->m_color.a);
	tls_pSer->Serialize(pPlayer->m_color.mode);

	int* pActiveAreas = pPlayer->GetActiveAreas();
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		int area = pActiveAreas[i];
		if (area < 0 || area >= MAX_AREA)
			continue;

		for (auto it = m_mapArea[area].begin(); it != m_mapArea[area].end(); ++it)
		{
			CPlayer* pPlayer = (CPlayer*)*it;
			if (nullptr == pPlayer)
				continue;
			char* pBuf = pPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
			if (nullptr == pBuf)
				continue;
			tls_pSer->CopyBuffer(pBuf);
			pPlayer->SendPost(tls_pSer->GetCurBufSize());
		}
	}
	*/
}