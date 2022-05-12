#include "stdafx.h"
#include "NPC.h"

CNPC::CNPC()
{
	m_pFsmClass = new(nothrow)FSM::FSMClass();
	Init();
}

CNPC::~CNPC()
{
	if (nullptr != m_pFsmClass)
	{
		delete m_pFsmClass;
		m_pFsmClass = nullptr;
	}
}

void CNPC::Init()
{
	m_detectDisSqr = 0.0f;
	m_teamNum = 0;
	m_haveSkill = false;
	m_coolTime = 0;
	m_moveFrameRate = 0;
	m_attackFrameRate = 0;
	m_pPrevMoveNode = nullptr;
	m_isStanding = false;
	m_isSpecialAttack = false;
	m_isParty = false;
	m_damage = 0;
	m_fMoveSpeed = 0.0f;
	m_bIsDead = false;
	m_regenCount = 0;
	m_hp = 0;
	m_maxHP = 0;
	m_autoTargeted_By_PlayerKey = 0;
	m_eNPC_Type = NPC_Type::NONE_NPC;
	m_npcKey = 0;
	m_targetPlayerKey = 0;
	m_nMovingCount = 0;
	m_rank = 0;
	m_hiveFlag = false;
	m_area = -1;
	m_regenTime = 0xFFFFFFFF;
	m_updatePacketRefCount = 0;
	m_isUpdateRefCount = false;
}

void CNPC::OnProcess()
{
	switch (m_pFsmClass->GetCurState())
	{
	case State::NPC_ANGRY:
	{
		DoAttack();
	}
	break;
	case State::NPC_NONE:
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CNpc::OnProcess() | ����(%d)�� �������� ���� ��Ȳ�̴�.",
			m_pFsmClass->GetCurState());
	}
	break;
	}
}

void CNPC::SetNpcInfo(NPCInitData data, const TiXmlDocument& xml)
{
	CNPC::SetInfo(data, xml);
}

void CNPC::SetInfo(NPCInitData data, const TiXmlDocument& xml)
{
	m_npcKey = NPCManager()->GenerateNPCKey();
	SetNPC_Type(data.m_npcType);
	SetNPC_Code(data.m_npcCode);
	m_teamNum = data.m_teamNum;

	// �⺻ NPCƯ��
	///*
	switch (data.m_npcCode)
	{
	case NPC_Code::ZOMBIE1:
		m_maxHP = 10;
		m_damage = 5;
		m_detectDisSqr = DEFAULT_DETECT_DISTANCE_SQR;
		break;
	case NPC_Code::ZOMBIE2:
		m_maxHP = 20;
		m_damage = 8;
		m_detectDisSqr = DEFAULT_DETECT_DISTANCE_SQR;
		break;
	case NPC_Code::GHOUL:
		m_maxHP = 40;
		m_damage = 15;
		m_detectDisSqr = DEFAULT_DETECT_DISTANCE_SQR;
		break;
	case NPC_Code::SLIDER:
		m_maxHP = 20;
		m_damage = 5;
		m_detectDisSqr = 2500.0f;
		break;
	case NPC_Code::FLY:
		m_maxHP = 30;
		m_damage = 15;
		m_detectDisSqr = DEFAULT_DETECT_DISTANCE_SQR;
		break;
	case NPC_Code::FAT:
		m_maxHP = 80;
		m_damage = 20;
		m_detectDisSqr = DEFAULT_DETECT_DISTANCE_SQR;
		break;
	case NPC_Code::SPIDER:
		m_maxHP = 140;
		m_damage = 30;
		m_detectDisSqr = 2500.0f;
		break;
	case NPC_Code::TROLL:
		m_maxHP = 400;
		m_damage = 50;
		m_detectDisSqr = 2500.0f;
		break;
	default:
		break;
	}
	//*/

	m_hp = m_maxHP;

	if (data.m_npcType == NPC_Type::NORMAL_NPC)
	{
		int time = 1200; // 1�п� 600
		m_regenTime = time;
		switch (data.m_npcCode)
		{
		default:
			break;
		}
	}

	if (data.m_npcType == NPC_Type::MOVING_NPC)
	{
		SetIsDead(true);
	}


	/*
	// xml
	string str("monster");
	char s1[64];
	sprintf_s(s1, _countof(s1), "%d", static_cast<int>(npcCode));
	str += s1;

	const TiXmlElement* readRoot = xml.FirstChildElement("NPC");
	const TiXmlElement* sub = readRoot->FirstChildElement(str.c_str())->FirstChildElement("info");
	const TiXmlAttribute* pAttrib = sub->FirstAttribute();

	m_rank = atoi(pAttrib->Value()); pAttrib = pAttrib->Next();
	m_damage = atoi(pAttrib->Value()); pAttrib = pAttrib->Next();
	m_hp = atoi(pAttrib->Value()); pAttrib = pAttrib->Next();
	m_maxHP = m_hp;
	m_scale = static_cast<float>(atof(pAttrib->Value()));
	*/



	// NPC ���� ��ġ����
	Vector3 pos;

	// 1���� ��Ʈ����
	if (data.m_npcType == NPC_Type::DETECT_NPC && data.m_gruopPos == 1)
	{
		int scopeX = 80;
		int scopeZ = 80;

		int num = CMTRand::GetRand_uint64(0, 1);
		pos.x = 450.0f; pos.y = 0.0f; pos.z = -310.0f;
		if (0 == num)
			pos.x += rand() % scopeX;
		else
			pos.x += -(rand() % scopeX);
		num = rand() % 2;
		if (0 == num)
			pos.z += rand() % scopeZ;
		else
			pos.z += -(rand() % scopeZ);
	}

	// MCP ��Ʈ����
	if (data.m_npcType == NPC_Type::DETECT_NPC && data.m_gruopPos == 3)
	{
		int scopeX = 250;
		int scopeZ = 200;

		int num = CMTRand::GetRand_uint64(0, 1);
		pos.x = -620.0f; pos.y = 0.0f; pos.z = -330.0f;
		if (0 == num)
			pos.x += rand() % scopeX;
		else
			pos.x += -(rand() % scopeX);
		num = rand() % 2;
		if (0 == num)
			pos.z += rand() % scopeZ;
		else
			pos.z += -(rand() % scopeZ);
	}

	// ��Ʈ�� ��Ʈ����
	if (data.m_gruopPos == 4)
	{
		//pos.x = 550.0f, pos.y = 0.0f, pos.z = 650.0f;

		int now = CMTRand::GetRand_uint64(0, 1);
		if (now == 0)
		{
			unsigned int num = CMTRand::GetRand_uint64(0, 9);
			num *= 100;
			if (CMTRand::GetBernoulliDist(0.5f)) // ����
			{
				pos.x = num;
				pos.x -= 10;
				pos.x += CMTRand::GetRand_uint64(0, 20);

				pos.z = CMTRand::GetRand_uint64(0, 900);
			}
			else
			{
				pos.z = num;
				pos.z -= 10;
				pos.z += CMTRand::GetRand_uint64(0, 20);

				pos.x = CMTRand::GetRand_uint64(0, 900);
			}
		}
		else
		{
			unsigned int num = CMTRand::GetRand_uint64(0, 9);
			num *= 100;
			if (CMTRand::GetBernoulliDist(0.5f)) // ����
			{
				pos.x = -((float)num);
				pos.x -= 10;
				pos.x += CMTRand::GetRand_uint64(0, 20);

				pos.z = CMTRand::GetRand_uint64(0, 900);
			}
			else
			{
				pos.z = num;
				pos.z -= 10;
				pos.z += CMTRand::GetRand_uint64(0, 20);

				pos.x = -((float)CMTRand::GetRand_uint64(0, 900));
			}
		}
	}

	SetPos(pos);



	// ��������
	int area = AreaManager()->GetPosToArea(m_pos);
	m_area = area;

	//���� ��
	int fixedArea = area - AREA_SECTOR_CNT - 1;
	if (fixedArea >= 0)
		m_activeAreas[EDirection::DIR_LEFTUP] = fixedArea;
	//��
	fixedArea = area - AREA_SECTOR_CNT;
	if (fixedArea >= 0)
		m_activeAreas[EDirection::DIR_UP] = fixedArea;
	//������ ��
	fixedArea = area - AREA_SECTOR_CNT + 1;
	if (fixedArea >= 0)
		m_activeAreas[EDirection::DIR_RIGHTUP] = fixedArea;
	//����
	fixedArea = area - 1;
	if (fixedArea >= 0)
		m_activeAreas[EDirection::DIR_LEFT] = fixedArea;
	//�߰�
	m_activeAreas[EDirection::DIR_CENTER] = area;
	//������
	fixedArea = area + 1;
	if (fixedArea < MAX_AREA)
		m_activeAreas[EDirection::DIR_RIGHT] = fixedArea;
	//���� �Ʒ�
	fixedArea = area + AREA_SECTOR_CNT - 1;
	if (fixedArea < MAX_AREA)
		m_activeAreas[EDirection::DIR_LEFTDOWN] = fixedArea;
	//�Ʒ�
	fixedArea = area + AREA_SECTOR_CNT;
	if (fixedArea < MAX_AREA)
		m_activeAreas[EDirection::DIR_DOWN] = fixedArea;
	//������ �Ʒ�
	fixedArea = area + AREA_SECTOR_CNT + 1;
	if (fixedArea < MAX_AREA)
		m_activeAreas[EDirection::DIR_RIGHTDOWN] = fixedArea;
}

void CNPC::DoWander()
{
	if (m_isStanding)
		return;

	switch (m_pFsmClass->GetCurState())
	{
	case State::NPC_NORMAL:
	{
		if (30 < ++m_nMovingCount) // �����ð� �� �ٽ� ���ڸ��� ���ư���
		{
			m_nMovingCount = 0;
			m_pos = m_nativePos;
			return;
		}

		int num = CMTRand::GetRand_uint64(0, 19);
		if (18 > num)
			return;

		num = CMTRand::GetRand_uint64(0, 3);
		float n = 1.0f;

		// �������� ��ǥ ����
		if (0 == num)
		{
			m_pos.x = (m_pos.x + n); // �ι�° ����Ƽ���
			m_pos.y = (m_pos.y + n);
		}
		else if (1 == num)
		{
			m_pos.x = (m_pos.x - n);
			m_pos.y = (m_pos.y - n);
		}
		else if (2 == num)
		{
			m_pos.x = (m_pos.x + n);
			m_pos.y = (m_pos.y - n);
		}
		else if (3 == num)
		{
			m_pos.x = (m_pos.x - n);
			m_pos.y = (m_pos.y + n);
		}
		break;
	}
	default:
		break;
	}

	m_updatePacketRefCount++;
}

void CNPC::DoAttack()
{
	CPlayer* pPlayer = PlayerManager()->FindPlayer(m_targetPlayerKey);
	if (nullptr == pPlayer || pPlayer->m_isDead || false == pPlayer->GetIsConfirm())
	{
jump: // goto
		InitPrevMoveNode();
		//m_pos.x = m_nativePos.x;
		//m_pos.y = m_nativePos.y;
		//m_pos.z = m_nativePos.z;
		m_attackFrameRate = 0;
		m_moveFrameRate = 0;
		SetTagetPlayerPKey(0);
		m_autoTargeted_By_PlayerKey = 0;
		m_hiveFlag = false;
		m_pFsmClass->TranslateState(Event::EVENT_PLAYER_RUNAWAY);
		return;
	}

	/*
	float distance = Vector3::Distance(m_nativePos, pPlayer->m_pos);
	if ((DEFAULT_DETECT_DISTANCE) < distance)
	{
		goto jump; // goto
	}
	*/

	m_pos.x = pPlayer->m_pos.x;
	m_pos.y = pPlayer->m_pos.y;
	m_pos.z = pPlayer->m_pos.z;





	/*
	// �ӷ����� �̵�
	float x = (pPlayer->m_vPos.x - m_vPos.x);
	float y = (pPlayer->m_vPos.y - m_vPos.y);
	float z = (pPlayer->m_vPos.z - m_vPos.z);

	float scalar = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

	float nomal_x = x / scalar; float nomal_y = y / scalar; float nomal_z = z / scalar;

	float move_x = nomal_x * (m_fMoveSpeed * 0.5f);
	float move_y = nomal_y * (m_fMoveSpeed * 0.5f);
	float move_z = nomal_z * (m_fMoveSpeed * 0.5f);
	m_vPos.x += move_x; m_vPos.y += move_y; m_vPos.z += move_z;

	float d = sqrt(pow(m_vPos.x - pPlayer->m_vPos.x, 2) + pow(m_vPos.y - pPlayer->m_vPos.y, 2));
	//printf("�Ÿ�: %f\n", d);
	*/

	// ����
	//float width = fabs(pPlayer->m_v3Pos.fPosX - m_v3Pos.fPosX);
	//float Hight = fabs(pPlayer->m_v3Pos.fPosZ - m_v3Pos.fPosZ);
	//float Radian = atan2(Hight, width);          // Radian�� ȣ�������� ��ȯ�Ǿ� �����
	//float Angle = Radian * 180 / 3.141519;       // Radian�� �ٽ� 60�й����� ��ȯ
	//printf("%f\n", Angle);

	//printf("���Ͱ� ����       Ž���� ǥ��key: %d\n", m_dwTagetPlayerPKey);
	//printf("�������� �Ÿ�:   %f \n", distance);
}

void CNPC::DoDetect()
{
	AreaManager()->DetectPlayerFromNPC(this);
}

void CNPC::DoDisComport()
{
	CPlayer* pPlayer = PlayerManager()->FindPlayer(m_targetPlayerKey);
	if (nullptr == pPlayer || pPlayer->m_isDead || false == pPlayer->GetIsConfirm())
	{
		InitDetect();
		m_pFsmClass->TranslateState(Event::EVENT_PLAYER_RUNAWAY);
		return;
	}

	float distance = sqrt(
		(pPlayer->m_pos.x - m_pos.x) * (pPlayer->m_pos.x - m_pos.x)
		+ (pPlayer->m_pos.y - m_pos.y) * (pPlayer->m_pos.y - m_pos.y)
		+ (pPlayer->m_pos.z - m_pos.z) *  (pPlayer->m_pos.z - m_pos.z)
	);
	if (DEFAULT_DETECT_DISTANCE < distance)
	{
		InitDetect();
		m_pFsmClass->TranslateState(Event::EVENT_PLAYER_RUNAWAY);
		//printf("�÷��̾ ��������    �Ÿ�: %f\n", distance);
		return;
	}

	// �÷��̾ �������� �ʰ� ��� ������ NPC�� ȭ�� ���·�
	++m_disComportIndex;
	if (MAX_DISCOMPORT_INDEX == m_disComportIndex)
	{
		m_disComportIndex = 0;
		m_pFsmClass->TranslateState(Event::EVENT_DISCOMPORT_INDEX);
	}
}

bool CNPC::CheckDead()
{
	// Npc�� �׾�����
	if (m_bIsDead)
	{
		// ���� ī��Ʈ�� ������Ų��
		if (m_regenTime <= m_regenCount++) // ī��Ʈ�� ����ϸ�
		{
			RaiseNPC();
			return false;
		}
		return true;
	}
	else
		return false;
}