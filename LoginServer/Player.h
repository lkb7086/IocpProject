#pragma once

class CPlayer final : public CConnection
{
public:
	CPlayer() { Init(); };
	~CPlayer() {};

	void Init()
	{
		memset(m_szID, 0x00, sizeof(m_szID));
		m_isConfirm = false;
		m_nAccox_UID = 0xffffffff;
		m_nLSKey = 0xffffffff;
		m_bIsNotSelectedUnit = true;
		m_nTime = 2;
	}

	inline char* GetID() { return m_szID; }
	inline void	 SetID(char szId[]) { strncpy_s(m_szID, _countof(m_szID), szId, _TRUNCATE); }
	inline bool GetIsConfirm() { return m_isConfirm; }
	inline void SetIsConfirm(bool bIsConfirm) { m_isConfirm = bIsConfirm; }

	inline unsigned int GetAccox_UID() { return m_nAccox_UID; }
	inline void SetAccox_UID(unsigned int _nAccox_UID) { m_nAccox_UID = _nAccox_UID; }

	inline unsigned int GetLSKey() { return m_nLSKey; }
	inline void SetLSKey(unsigned int _nLSKey) { m_nLSKey = _nLSKey; }

	bool m_bIsNotSelectedUnit; // 캐릭터를 선택하지 않고 그냥 나갔으면 true
	char m_nTime;

private:
	char	m_szID[MAX_ID_LENGTH];
	bool    m_isConfirm;
	unsigned int m_nAccox_UID;
	unsigned int m_nLSKey;
};