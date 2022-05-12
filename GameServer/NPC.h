#pragma once
#include "FSM.h"
#include "tinyxml/tinyxml.h"
#include "Pathfinding.h"

//const float DEFAULT_DETECT_DISTANCE = 20.0f; // 플레이어 감지거리

struct NPCInitData
{
	NPC_Type m_npcType; NPC_Code m_npcCode; UINT m_npcCnt; UCHAR m_teamNum; USHORT m_gruopPos;
	NPCInitData(NPC_Type npcType, NPC_Code npcCode, UINT npcCnt, USHORT gruopPos, UCHAR teamNum)
	{
		m_npcType = npcType; m_npcCode = npcCode; m_npcCnt = npcCnt; m_teamNum = teamNum; m_gruopPos = gruopPos;
	};
};

class CNPC : public CLifeObject
{
public:
	explicit CNPC();
	virtual ~CNPC();

	virtual void OnProcess();  // NPC 상태 처리
	virtual void Init();       // 초기화
	void DoWander();   // 랜덤으로 움직임
	void DoAttack();   // 플레이어 공격
	void DoDetect();     // 탐색 함수
	void DoDisComport(); // 불쾌 상태일때 불리는 함수

	void SetNpcInfo(NPCInitData data, const TiXmlDocument& _xml);
	void SetInfo(NPCInitData data, const TiXmlDocument& xml);
	void InitDetect() { m_targetPlayerKey = 0; m_disComportIndex = 0; };
	bool CheckDead();

	inline void InitPrevMoveNode()
	{
		if (nullptr != m_pPrevMoveNode)
		{
			m_pPrevMoveNode->walkable = true;
			m_pPrevMoveNode = nullptr;
		}
	}

	void RaiseNPC()
	{
		m_pFsmClass->SetCurFsmState(State::NPC_NORMAL);
		InitDetect();
		m_hp = m_maxHP;
		m_autoTargeted_By_PlayerKey = 0;
		m_regenCount = 0;
		m_pos = m_nativePos;
		SetIsDead(false);
	}

	inline unsigned __int32	GetKey() { return m_npcKey; }
	inline NPC_Type GetNPC_Type() { return m_eNPC_Type; }
	inline void		SetNPC_Type(NPC_Type NpcType) { m_eNPC_Type = NpcType; }
	inline NPC_Code GetNPC_Code() { return m_eNPC_Code; }
	inline void		SetNPC_Code(NPC_Code Type) { m_eNPC_Code = Type; }
	inline State	GetState() { return m_pFsmClass->GetCurState(); }
	inline void		SetEvent(Event Event) { m_pFsmClass->TranslateState(Event); }
	inline void		SetTagetPlayerPKey(UINT dwTagetPlayerPKey) { m_targetPlayerKey = dwTagetPlayerPKey; m_updatePacketRefCount++; }
	inline UINT	    GetTagetPlayerPKey() { return m_targetPlayerKey; }
	inline void		SetHp(int _iHp) { m_hp = _iHp; }
	inline int		GetHp() { return m_hp; }
	inline void		SetPos(Vector3 &_v3Pos) { m_pos = _v3Pos; m_nativePos = _v3Pos; }
	inline bool     GetIsDead() { return m_bIsDead; }
	inline void     SetIsDead(bool isDead) { m_bIsDead = isDead; m_updatePacketRefCount++; }

	inline void     SetAuto_PlayerKey(UINT _nPKey) { InterlockedExchange((LPLONG)&m_autoTargeted_By_PlayerKey, _nPKey); };
	inline UINT     GetAuto_PlayerKey() { return m_autoTargeted_By_PlayerKey; }



	//GETSET(bool, IsDead, m_bIsDead);
	GETSET(int, MaxHP, m_maxHP);
	GETSET(float, Scale, m_scale);

	float m_detectDisSqr;
	Vector3 m_tempPosFromHost; // 방장한테 받는 (정확한 위치의)동적인 좌표
	Vector3 m_pos;           // 움직이는 동적인 좌표
	Vector3 m_nativePos;     // NPC의 최초위치
	bool m_bIsDead;
	BYTE m_nMovingCount;
	unsigned int m_damage;
	float m_fMoveSpeed;
	unsigned short m_rank;
	bool m_isStanding;
	bool m_isSpecialAttack;
	bool m_isParty;
	Node* m_pPrevMoveNode;
	unsigned char m_teamNum;
	int	m_activeAreas[MAX_ACTIVE_AREAS];
	int m_area;
	bool m_hiveFlag;
	unsigned int m_regenTime;
	long long m_updatePacketRefCount;
	bool m_isUpdateRefCount;

protected:
	bool m_haveSkill;
	unsigned int m_attackFrameRate;
	unsigned int m_moveFrameRate;
	float m_scale;
	unsigned short m_coolTime;
	NPC_Type m_eNPC_Type;	 // NPC종류, 비선공인지 선공인지
	NPC_Code m_eNPC_Code; // 실질적인 몬스터종류, 좀비 오크등

	UINT m_autoTargeted_By_PlayerKey;    // 자신을 오토타겟한 유저의 키
	unsigned int m_regenCount; // 죽고나서 리젠시간
	int          m_hp;
	int          m_maxHP;
	BYTE	     m_disComportIndex;	// 불쾌 지수, n이상이면 화냄으로 바뀜

	unsigned int		m_npcKey;				// NPC키
	unsigned int		m_targetPlayerKey;		// 탐지된 플레이어 키	

	FSM::FSMClass*  m_pFsmClass;			// 유한상태 기계 클래스
};