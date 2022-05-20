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

bool CAreaManager::AddPlayerToArea(CPlayer* pPlayer, int byArea)
{
	if (0 > byArea || MAX_AREA <= byArea)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::AddPlayerToArea() | Wrong Area : %d", byArea);
		return false;
	}

	CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive);

	auto area_it = m_mapArea[byArea].find(pPlayer);
	if (area_it != m_mapArea[byArea].end())
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CAreaManager::AddPlayerToArea() | PKey[%d]�� �̹� m_mapArea[%d]�� �ֽ��ϴ�.",
			pPlayer->GetKey(), byArea);
		return false;
	}
	pPlayer->SetArea(byArea);
	m_mapArea[byArea].insert(pPlayer);
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

	m_mapArea[byArea].erase(pPlayer);
	pPlayer->SetArea(-1);

	return true;
}

bool CAreaManager::PrepareUpdateArea(CPlayer* pPlayer, int newAreaIndex)
{
	if (0 > newAreaIndex || MAX_AREA <= newAreaIndex)
	{
		LOG(LOG_ERROR_NORMAL, "SYSTEM | CAreaManager::PrepareUpdateArea() | PKey[%d]�� m_mapArea[%d]",
			pPlayer->GetKey(), newAreaIndex);
		return false;
	}

	int nOldArea = pPlayer->GetArea();
	int nNewArea = newAreaIndex;
	if (nOldArea == nNewArea) // ������ �ٲ��� �ʾҴٸ�
	{
		//printf("PrepareUpdateArea / nOldArea == nNewArea   %d   %d  \n", nOldArea, nNewArea);
		return false;
	}

	bool bRet = RemovePlayerFromArea(pPlayer, nOldArea);
	if (false == bRet)
	{
		//printf("CAreaManager::PrepareUpdateArea() | if (false == bRet) 1\n");
		//return false;
	}
	
	bRet = AddPlayerToArea(pPlayer, nNewArea);
	if (false == bRet)
	{
		printf("CAreaManager::PrepareUpdateArea() | if (false == bRet) 2\n");
		//IocpGameServer()->CloseConnection(pPlayer);
		return false;
	}

	UpdateActiveAreas(pPlayer);

	return true;
}

void CAreaManager::UpdateActiveAreas(CPlayer* pPlayer)
{
	// ���� �÷��̾ ���� ������ ������ 8���� ������ �÷��̾ ������
	// �� �� �ִ� �����̴�. �ִ� 9���� ������ ������ �� �� �ִµ� �� ������ ���Ѵ�.
	int byArea = pPlayer->GetArea();
	if (0 > byArea || MAX_AREA <= byArea)
	{
		printf("UpdateActiveAreas fail %d\n", byArea);
		return;
	}

	AllPopNewArea();

	int byNewActiveAreas[MAX_ACTIVE_AREAS];
	fill_n(byNewActiveAreas, MAX_ACTIVE_AREAS, -1);

	//���� ��
	int nArea = byArea - AREA_SECTOR_CNT - 1;
	if (nArea >= 0)
		byNewActiveAreas[EDirection::DIR_LEFTUP] = nArea;
	//��
	nArea = byArea - AREA_SECTOR_CNT;
	if (nArea >= 0)
		byNewActiveAreas[EDirection::DIR_UP] = nArea;
	//������ ��
	nArea = byArea - AREA_SECTOR_CNT + 1;
	if (nArea >= 0)
		byNewActiveAreas[EDirection::DIR_RIGHTUP] = nArea;
	//����
	nArea = byArea - 1;
	if (nArea >= 0)
		byNewActiveAreas[EDirection::DIR_LEFT] = nArea;
	//�߰�
	byNewActiveAreas[EDirection::DIR_CENTER] = byArea;
	//������
	nArea = byArea + 1;
	if (nArea < MAX_AREA)
		byNewActiveAreas[EDirection::DIR_RIGHT] = nArea;
	//���� �Ʒ�
	nArea = byArea + AREA_SECTOR_CNT - 1;
	if (nArea < MAX_AREA)
		byNewActiveAreas[EDirection::DIR_LEFTDOWN] = nArea;
	//�Ʒ�
	nArea = byArea + AREA_SECTOR_CNT;
	if (nArea < MAX_AREA)
		byNewActiveAreas[EDirection::DIR_DOWN] = nArea;
	//������ �Ʒ�
	nArea = byArea + AREA_SECTOR_CNT + 1;
	if (nArea < MAX_AREA)
		byNewActiveAreas[EDirection::DIR_RIGHTDOWN] = nArea;

	///////////////////////////////////////////////////////
	// ���� Ȱ�������� ���� Ȱ�������� ���ؼ� ���� Ȱ�� ������
	// ���� Ȱ������ �ʴ� ���� ����
	int* pPlayerActiveAreas = pPlayer->GetActiveAreas();
	int* pPlayerInActiveAreas = pPlayer->GetInActiveAreas();
	for (int i = 0; i < MAX_INACTIVE_AREAS; ++i)
		pPlayerInActiveAreas[i] = -1;
	BYTE byInActiveAreaCnt = 0;

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
			if (pPlayerActiveAreas[i] == byNewActiveAreas[j])    // ��ġ��
			{
				flag = true;
				break;
			}
		}
		if (flag == true)   // �ƹ�������� ��Ƽ��
			continue;

		// ��ġ�� ������ �ƴ϶�� �� InActiveArea���
		pPlayerInActiveAreas[byInActiveAreaCnt++] = pPlayerActiveAreas[i];
	}

	// ���� Ȱ��ȭ�� ������ ���ÿ� �ִ´�
	for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
	{
		bool flag = false;

		if (0 > byNewActiveAreas[i])
			continue;

		for (int j = 0; j < MAX_ACTIVE_AREAS; j++)
		{
			if (byNewActiveAreas[i] == pPlayerActiveAreas[j])
			{
				flag = true;
				break;
			}
		}
		if (flag == true)
			continue;

		//printf("push %d\n", byNewActiveAreas[i]);
		m_vecNewActiveArea.push_back(byNewActiveAreas[i]);
	}

	// �÷��̾��� Ȱ�������� ���ο� �������� ����
	memcpy(pPlayerActiveAreas, byNewActiveAreas, sizeof(int) * MAX_ACTIVE_AREAS);

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
	int nPosY = static_cast<int>(abs(_pos.z - AREA_HALF_POS));
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
		// ���� 9���ⱸ��

		// �÷��̾�� �����̳ʿ� �ִ��� �˻�

		// ���������̳� i������ ����
		if (false == NPCManager()->GatherVBuffer_NpcInfo(i)) // �̰ǻ�� �ǹ̾���(NPC�� ��� �������� �ϸ�)
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
				if (nullptr == pPlayer || pPlayer->m_isInitNPCInfo == false)
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
	int byNewArea[MAX_ACTIVE_AREAS];
	for (int i = 0; i < MAX_ACTIVE_AREAS; ++i)
		byNewArea[i] = -1;

	// ����Ȱ��ȭ�� ������ ���ٸ� �ƹ��͵� ���ϰ� ������
	if (0 == stackSize || MAX_ACTIVE_AREAS < stackSize)
	{
		//AllPopNewArea();
		return;
	}

	int while_i = 0;
	while (!m_vecNewActiveArea.empty())
	{
		byNewArea[while_i] = m_vecNewActiveArea.back();
		m_vecNewActiveArea.pop_back();
		while_i++;
	}

	{
		//CMonitorSRW::OwnerSRW lock(m_srwArea[nZone], LockShared);
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ��Ƽ�������߿����� ���λ��� ��Ƽ���������θ� ��Ŷ�� �������Ѵ�.
		// ��ε�ĳ��Ʈ
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::UpdateAreaForCreateObject_Not));
		tls_pSer->Serialize(pPlayer->GetKey());
		tls_pSer->Serialize(pPlayer->GetHP());
		tls_pSer->Serialize(pPlayer->m_pos.x);
		tls_pSer->Serialize(pPlayer->m_pos.y);
		tls_pSer->Serialize(pPlayer->m_pos.z);

		for (unsigned int i = 0; i < stackSize; i++)
		{
			if (0 > byNewArea[i] || byNewArea[i] >= MAX_AREA)
				continue;

			for (auto area_it = m_mapArea[byNewArea[i]].begin(); area_it != m_mapArea[byNewArea[i]].end(); area_it++)
			{
				CPlayer* pAreaPlayer = (CPlayer*)*area_it;
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
			if (0 > byNewArea[i] || byNewArea[i] >= MAX_AREA)
				continue;

			for (auto area_it = m_mapArea[byNewArea[i]].begin(); area_it != m_mapArea[byNewArea[i]].end(); area_it++)
			{
				CPlayer* pAreaPlayer = (CPlayer*)*area_it;
				if (nullptr == pAreaPlayer || pPlayer == pAreaPlayer)
					continue;
				tls_pSer->Serialize(pAreaPlayer->GetKey());
				tls_pSer->Serialize(pAreaPlayer->GetHP());
				tls_pSer->Serialize(pAreaPlayer->m_pos.x);
				tls_pSer->Serialize(pAreaPlayer->m_pos.y);
				tls_pSer->Serialize(pAreaPlayer->m_pos.z);
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

void CAreaManager::Send_UpdateAreaForDeleteObject(CPlayer* pPlayer)
{
	int* pInActiveAreas = pPlayer->GetInActiveAreas();
	m_vecDeleteArea.clear();

	{
		//CMonitorSRW::OwnerSRW lock(m_srwArea[nZone], LockShared);
		for (int i = 0; i < MAX_INACTIVE_AREAS; i++)
		{
			int byInActiveArea = pInActiveAreas[i];

			if (0 > byInActiveArea || byInActiveArea >= MAX_AREA)
				continue;

			for (auto area_it = m_mapArea[byInActiveArea].begin(); area_it != m_mapArea[byInActiveArea].end(); area_it++)
			{
				// Ʈ���� ���� �÷��̾� ����� ��ε�ĳ����
				CPlayer* pAreaPlayer = (CPlayer*)*area_it;
				if (pAreaPlayer == pPlayer)
					continue;
				stCL_GS_UpdateAreaForDeleteObject* pDelete =
					(stCL_GS_UpdateAreaForDeleteObject*)pAreaPlayer->PrepareSendPacket(sizeof(stCL_GS_UpdateAreaForDeleteObject));
				if (nullptr == pDelete)
					continue;
				pDelete->type = static_cast<packet_type>(PacketType::CL_GS_UpdateAreaForDeleteObject);
				pDelete->uiPKey = pPlayer->GetKey();
				pAreaPlayer->SendPost(sizeof(stCL_GS_UpdateAreaForDeleteObject));

				// Ʈ���Ź��� �÷��̾ �� �÷��̾���� Ű�� ����
				m_vecDeleteArea.push_back(pAreaPlayer->GetKey());
			}
		}
	}

	if (m_vecDeleteArea.empty())
		return;

	tls_pSer->StartSerialize();
	tls_pSer->Serialize(static_cast<packet_type>(PacketType::CL_GS_UpdateAreaForDeleteObjectV));
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
	/*
	if (false == pPlayer->GetIsConfirm())
		return;
	MovePlayer_Cn* pMove = (MovePlayer_Cn*)pRecvedMsg;
	pPlayer->m_pos.x = pMove->x;
	pPlayer->m_pos.y = pMove->y;
	pPlayer->m_pos.z = pMove->z;
	*/

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
			if (pAreaPlayer == pPlayer)
				continue;

			MovePlayer_Sn* pMove = (MovePlayer_Sn*)pAreaPlayer->PrepareSendPacket(sizeof(MovePlayer_Sn));
			//MovePlayer_Sn* pMove = (MovePlayer_Sn*)IocpGameServer()->UDP_unsafe_PrepareSendPacket(sizeof(MovePlayer_Sn));
			
			if (nullptr == pMove)
				continue;
			//pMove->type = static_cast<packet_type>(PacketType::GS_CL_MovePlayer);
			pMove->type = static_cast<packet_type>(PacketType::GS_CL_MovePlayer);
			pMove->key = pPlayer->GetKey();
			pMove->posX = pPlayer->m_pos.x;
			pMove->posY = pPlayer->m_pos.y;
			pMove->posZ = pPlayer->m_pos.z;

			pAreaPlayer->SendPost(sizeof(MovePlayer_Sn));
			//IocpGameServer()->UDP_unsafe_SendPost(sizeof(MovePlayer_Sn), pAreaPlayer);
		}
	}
}

void CAreaManager::Send_SuccessNPCAttackToPlayer_Aq(CPlayer* pPlayer, char* pRecvedMsg)
{
	// ���ݹ��� player
	CPlayer* pInjuredPlayer = pPlayer;
	if (pInjuredPlayer->m_isDead)
		return;
	// �����ϴ� npc
	unsigned int attackingNPCKey = ((stUtil_UInteger*)pRecvedMsg)->nUInteger;
	CNPC* pAttackingNPC = NPCManager()->FindNpc(attackingNPCKey);
	if (nullptr == pAttackingNPC)
		return;
	// �÷��̾� ü�°���
	pInjuredPlayer->SetHP(pInjuredPlayer->GetHP() - pAttackingNPC->m_damage);

	if (0 >= pInjuredPlayer->GetHP()) // �÷��̾ ���ݹް� �׾�����
	{
		pInjuredPlayer->m_isDead = true;

		PlayerManager()->GS_CL_PlayerDead(pPlayer);
	}
	else
	{
		// ��������
		if (pAttackingNPC->GetNPC_Code() == NPC_Code::SLIDER || pAttackingNPC->GetNPC_Code() == NPC_Code::SPIDER ||
			CMTRand::GetBernoulliDist(0.1f))
		{
			CSerializer& ser = *tls_pSer;
			ser.StartSerialize();
			ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_PlayerInfection));
			char* buf = pInjuredPlayer->PrepareSendPacket(ser.GetCurBufSize());
			if (nullptr == buf)
				return;
			ser.CopyBuffer(buf);
			pInjuredPlayer->SendPost(ser.GetCurBufSize());

			pInjuredPlayer->m_isInfection = true;
		}
	}

	CSerializer& ser = *tls_pSer;
	ser.StartSerialize();
	ser.Serialize(static_cast<packet_type>(PacketType::GS_CL_PlayerHP));
	ser.Serialize(pInjuredPlayer->GetHP());
	ser.Serialize((char)false);
	char* buf = pInjuredPlayer->PrepareSendPacket(ser.GetCurBufSize());
	if (nullptr == buf)
		return;
	ser.CopyBuffer(buf);
	pInjuredPlayer->SendPost(ser.GetCurBufSize());
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