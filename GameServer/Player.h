#pragma once
#include "LifeObject.h"
#include "NPC.h"

struct Rank {
public:
	Rank() : nWin(0), nKill(0), nPKey(0) {}
	void Init(){ nWin = 0; nKill = 0; nPKey = 0; }

	/* greater operator for min_heap */
	bool operator>(const Rank& _rank) const{
		return ((this->nWin * 5) + this->nKill) > ((_rank.nWin * 5) + _rank.nKill);
	}
	/* less operator for max_heap */
	bool operator<(const Rank& _rank) const{
		return ((this->nWin * 5) + this->nKill) < ((_rank.nWin * 5) + _rank.nKill);
	}

public:
	unsigned int nPKey;
	int nWin;
	int nKill;
};

struct stItem
{
	// 타입: 장비, 소모
	unsigned long long nUid;
	unsigned int nCode;
	int nAmount;
	unsigned short nSlot;
	bool isMerge;
	unsigned char rank;
	stItem() {
		memset(this, 0, sizeof(stItem));
	}
	void Init() { memset(this, 0, sizeof(stItem));
	}
};

class CPlayer final : public CConnection, CLifeObject
{
public:
	explicit CPlayer();
	~CPlayer();
	CPlayer(const CPlayer& rhs) = delete;
	CPlayer& operator=(const CPlayer &rhs) = delete;

	/*
	// greater operator for min_heap
	bool operator>(const CPlayer& _player) const{
	return ((m_nWinCnt * 5) + m_nKillCnt) > ((_player.m_nWinCnt * 5) + _player.m_nKillCnt);
	}
	// less operator for max_heap
	bool operator<(const CPlayer& _player) const{
	return ((m_nWinCnt * 5) + m_nKillCnt) < ((_player.m_nWinCnt * 5) + _player.m_nKillCnt);
	}
	*/

	void InitPlayer();
	void SetPlayInfo(char* pRecvedMsg);

	inline int GetArea() { return m_nArea; } inline void SetArea(int byArea) { m_nArea = byArea; }
	inline int* GetInActiveAreas() { return m_inActiveAreas; }
	inline int* GetActiveAreas() { return m_activeAreas; }
	void InitAreas()
	{
		for (int i = 0; i < MAX_INACTIVE_AREAS; ++i)
			m_inActiveAreas[i] = -1;
		for (int i = 0; i < MAX_ACTIVE_AREAS; ++i)
			m_activeAreas[i] = -1;
	}

	void InitInventory()
	{
		m_arrItem.fill(stItem());
		m_nItemSlotsCnt = 0;
	}

	void FuncDummy(BYTE _nClass, int _nArea, float _x, float _y, int ii)
	{
		m_nClass = _nClass; m_nArea = _nArea; m_pos.x = _x; m_pos.y = _y;
		//LOG(LOG_ERROR_LOW, "SYSTEM | 1111    | %f %f  %d", m_vPos.x, m_vPos.y, m_nArea);
		char temp[16]; memset(temp, 0, sizeof(temp));
		_snprintf_s(temp, _countof(temp), _TRUNCATE, "Dummy#%d", ii);
		SetNickName(temp);
		m_bIsDummy = true;
	}

	inline void InitPrevMoveNode()
	{
		if (nullptr != m_pPrevMoveNode)
		{
			m_pPrevMoveNode->walkable = true;
			m_pPrevMoveNode = nullptr;
		}
	}



	// Get Set
	inline unsigned __int32 GetKey() { return m_key; } inline void SetPKey(unsigned __int32 dwPKey) { m_key = dwPKey; }
	inline char* GetID() { return m_szID; }
	inline void	 SetID(char* szId) { strncpy_s(m_szID, _countof(m_szID), szId, _TRUNCATE); } // _TRUNCATE == MAX_ID_LENGTH - 1
	inline char* GetNickName() { return m_szNickName; }
	inline void	 SetNickName(char* szNickName) { strncpy_s(m_szNickName, _countof(m_szNickName), szNickName, _TRUNCATE); }
	inline int GetLevel() { return m_nLevel; } inline void SetLevel(int byLevel) { m_nLevel = byLevel; }
	inline BYTE GetClass() { return m_nClass; } inline void SetClass(BYTE byClass) { m_nClass = byClass; }
	inline int GetHP() { return m_nHP; }
	inline void SetHP(int hp) { m_nHP = (hp >= 100) ? 100 : hp; }
	inline int GetExp() { return m_nExp; } inline void SetExp(int dwExp) { m_nExp = dwExp; }
	inline int GetMoney() { return m_nMoney; } inline void SetMoney(int _nMoney) { m_nMoney = _nMoney; }
	inline bool GetIsConfirm() { return m_isConfirm; } inline void SetIsConfirm(bool bIsConfirm) { m_isConfirm = bIsConfirm; }
	inline UINT GetAuto_TargetNPCKey() { return m_autoTargetedNPCKey; }
	inline void SetAuto_TargetNPCKey(UINT _nNPCKey) { m_autoTargetedNPCKey= _nNPCKey; }

	//GETSET(bool, IsDead, m_isDead);

	Node* m_pPrevMoveNode;
	CNPC* m_pTargetNPC;
	Vector3 m_targetPos;
	bool    m_bIsAutoAttack;
	Vector3 m_pos;
	bool    m_isDead;
	bool    m_isAccept;
	bool    m_isInitNPCInfo;
	bool    m_bIsDummy;
	short   m_nItemSlotsCnt;
	int  	m_damage;
	unsigned int m_nAccox_UID;
	unsigned int m_nUnit_UID;
	unsigned char m_equipGun;
	//stItem m_arrItem[MAX_INVEN_SLOT];
	std::array<stItem, MAX_INVEN_SLOT> m_arrItem;
	bool m_isHost;
	bool m_isInfection;

private:
	UINT    m_autoTargetedNPCKey; // 오토사냥시 타겟된 npc키
	unsigned __int32	m_key;
	char	m_szID[MAX_ID_LENGTH];
	char	m_szNickName[MAX_NICKNAME_LENGTH];

	BYTE	m_nClass;
	int 	m_nLevel;
	int	    m_nPower;
	int     m_nDefen;
	int	    m_nHP;
	int     m_nImmunity;
	int  	m_nExp;
	int     m_nMoney;

	bool    m_isConfirm;
	int  	m_nArea;

	int	m_inActiveAreas[MAX_INACTIVE_AREAS];
	int	m_activeAreas[MAX_ACTIVE_AREAS];
};
