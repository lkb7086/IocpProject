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

	//CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive); // ��� �ȴ�, �ֳ��ϸ� ������ �̽����尡 �ϱ� ����, �ٵ� ����� ���� �����

	auto it = m_mapArea[area].find(pPlayer);
	if (it != m_mapArea[area].end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::AddPlayerToArea() | PKey[%d]�� �̹� m_mapArea[%d]�� �ֽ��ϴ�.",
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
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CAreaManager::RemovePlayerFromArea() | PKey[%d]�� m_mapArea[%d]�� �������� �ʽ��ϴ�.",
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
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CAreaManager::PrepareUpdateArea() | PKey[%d]�� m_mapArea[%d]", pPlayer->GetKey(), newArea);
		return false;
	}

	int oldArea = pPlayer->GetArea();
	if (oldArea == newArea) // ������ �ٲ��� �ʾҴٸ�
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
	// ���� �÷��̾ ���� ������ ������ 8���� ������ �÷��̾ ������
	// �� �� �ִ� �����̴�. �ִ� 9���� ������ ������ �� �� �ִµ� �� ������ ���Ѵ�.
	int oldArea = pPlayer->GetArea();
	if (0 > oldArea || MAX_AREA <= oldArea)
	{
		return;
	}

	AllPopNewArea();

	int arrNewActiveAreas[MAX_ACTIVE_AREAS];
	fill_n(arrNewActiveAreas, MAX_ACTIVE_AREAS, -1);

	
	//���� ��
	int newArea = oldArea - AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTUP] = newArea;
	//��
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_UP] = newArea;
	//������ ��
	newArea = oldArea - AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTUP] = newArea;
	//����
	newArea = oldArea - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFT] = newArea;
	//�߰�
	arrNewActiveAreas[EDirection::DIR_CENTER] = oldArea;
	//������
	newArea = oldArea + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHT] = newArea;
	//���� �Ʒ�
	newArea = oldArea + AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTDOWN] = newArea;
	//�Ʒ�
	newArea = oldArea + AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_DOWN] = newArea;
	//������ �Ʒ�
	newArea = oldArea + AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTDOWN] = newArea;
	

	/*
	// ���� ��
	int newArea = oldArea + AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTUP] = newArea;
	//��
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_UP] = newArea;
	//������ ��
	newArea = oldArea + AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTUP] = newArea;
	//����
	newArea = oldArea - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFT] = newArea;
	//�߰�
	arrNewActiveAreas[EDirection::DIR_CENTER] = oldArea;
	//������
	newArea = oldArea + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHT] = newArea;
	//���� �Ʒ�
	newArea = oldArea - AREA_SECTOR_CNT - 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_LEFTDOWN] = newArea;
	//�Ʒ�
	newArea = oldArea - AREA_SECTOR_CNT;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_DOWN] = newArea;
	//������ �Ʒ�
	newArea = oldArea - AREA_SECTOR_CNT + 1;
	if (newArea >= 0 && newArea < MAX_AREA)
		arrNewActiveAreas[EDirection::DIR_RIGHTDOWN] = newArea;
	*/

	///////////////////////////////////////////////////////
	// ���� Ȱ�������� ���� Ȱ�������� ���ؼ� ���� Ȱ�� ������
	// ���� Ȱ������ �ʴ� ���� ����
	int* pPlayerActiveAreas = pPlayer->GetActiveAreas();
	int* pPlayerInActiveAreas = pPlayer->GetInActiveAreas();
	for (int i = 0; i < MAX_INACTIVE_AREAS; ++i)
		pPlayerInActiveAreas[i] = -1;
	int inActiveAreaCnt = 0;

	// InActiveArea�� ���Ѵ�
	// ��ó�� �Լ��� ȣ��Ǹ� �� �ݺ������� continue�� �Ѵ� (���� �����Ǵ� �κ��� ����)
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		bool flag = false;

		//������ ������ ���ٸ�
		if (0 > pPlayerActiveAreas[i])
			continue;

		for (int j = 0; j < MAX_ACTIVE_AREAS; j++)
		{
			if (pPlayerActiveAreas[i] == arrNewActiveAreas[j])    // ��ġ��
			{
				flag = true;
				break;
			}
		}
		if (flag == true)   // �ƹ�������� ��Ƽ��
			continue;

		// ��ġ�� ������ �ƴ϶�� �� InActiveArea���
		pPlayerInActiveAreas[inActiveAreaCnt++] = pPlayerActiveAreas[i];
	}

	// ���� Ȱ��ȭ�� ������ ���ÿ� �ִ´�
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

	// �÷��̾��� Ȱ�������� ���ο� �������� ����
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
	// ���������� x = 0, y = 0

	// x�� y�� �ٽ� �����Ѵ�
	int nPosX = static_cast<int>(_pos.x + AREA_HALF_POS);
	int nPosY = static_cast<int>(abs(_pos.y - AREA_HALF_POS));
	// �������� ������ ���Ѵ�
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

		// i������ 9������ ���Ѵ�
		int area = i;
		int	arrActiveAreas[MAX_ACTIVE_AREAS];
		for (size_t i = 0; i < MAX_ACTIVE_AREAS; i++)
		{
			arrActiveAreas[i] = -1;
		}

		//���� ��
		int fixedArea = area - AREA_SECTOR_CNT - 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_LEFTUP] = fixedArea;
		//��
		fixedArea = area - AREA_SECTOR_CNT;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_UP] = fixedArea;
		//������ ��
		fixedArea = area - AREA_SECTOR_CNT + 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_RIGHTUP] = fixedArea;
		//����
		fixedArea = area - 1;
		if (fixedArea >= 0)
			arrActiveAreas[EDirection::DIR_LEFT] = fixedArea;
		//�߰�
		arrActiveAreas[EDirection::DIR_CENTER] = area;
		//������
		fixedArea = area + 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_RIGHT] = fixedArea;
		//���� �Ʒ�
		fixedArea = area + AREA_SECTOR_CNT - 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_LEFTDOWN] = fixedArea;
		//�Ʒ�
		fixedArea = area + AREA_SECTOR_CNT;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_DOWN] = fixedArea;
		//������ �Ʒ�
		fixedArea = area + AREA_SECTOR_CNT + 1;
		if (fixedArea < MAX_AREA)
			arrActiveAreas[EDirection::DIR_RIGHTDOWN] = fixedArea;

		// 9���� �÷��̾�鿡�� ����
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

	// ����Ȱ��ȭ�� ������ ���ٸ� �ƹ��͵� ���ϰ� ������
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
		//CMonitorSRW::OwnerSRW lock(m_srwArea, LockShared); // ��� �ȴ�, �ֳ��ϸ� ������ �̽����尡 �ϱ� ����
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ��Ƽ�������߿����� ���λ��� ��Ƽ���������θ� ��Ŷ�� �������Ѵ�.
		// ��ε�ĳ��Ʈ
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

				uiAreaPlayerCnt++; // �ؿ��� �������� ������ �ο����� �ʿ��ϴ�.
			}
		}
	}

	//printf("Send_UpdateAreaForCreateObject %d\n", uiAreaPlayerCnt);
	// ����ĳ��Ʈ
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
		//CMonitorSRW::OwnerSRW lock(m_srwArea, LockShared); // ��� �ȴ�, �ֳ��ϸ� ������ �̽����尡 �ϱ� ����
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
				// �÷��̾� ����� ��ε�ĳ����
				CPlayer* pAreaPlayer = (CPlayer*)*it;
				if (pAreaPlayer == pPlayer)
					continue;

				char* pBuffer = pAreaPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
				if (nullptr == pBuffer)
					continue;
				tls_pSer->CopyBuffer(pBuffer);
				pAreaPlayer->SendPost(tls_pSer->GetCurBufSize());

				// Ʈ���Ź��� �÷��̾ �� �÷��̾���� Ű�� ����
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
				//printf("���Ϳ� ���� �� ������   /    ���Ϳ��� �Ÿ�: %f\n", distance);
				continue;
			}

			// ������
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