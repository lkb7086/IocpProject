#pragma once

class CAreaManager final : public CSingleton
{
	DECLEAR_SINGLETON(CAreaManager);
public:
	CAreaManager();
	~CAreaManager();

	// ��ƿ��Ƽ
	void Send_TCP_RecvBufferFromServer(DWORD dwSize, char* pRecvBuffer); // �÷��̾�� �������� ���� ���۸� �״�� ������

	void RelayWithoutPlayer(CPlayer* pPlayer, DWORD size, char* pMsg)
	{
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
				char* pBuf = pAreaPlayer->PrepareSendPacket(size);
				if (nullptr == pBuf)
					continue;
				memcpy_s(pBuf, size, pMsg, size);
				pAreaPlayer->SendPost(size);
			}
		}
	}

	inline void AllPopNewArea()
	{
		while (!m_vecNewActiveArea.empty()) { m_vecNewActiveArea.pop_back(); }
	}

	bool AddPlayerToArea(CPlayer* pPlayer, int byArea);
	bool RemovePlayerFromArea(CPlayer* pPlayer, int byArea);

	bool PrepareUpdateArea(CPlayer* pPlayer, int _byAreaIndex); // �÷��̾ �̵��� �Ͽ� ������ ��ȭ
	void UpdateActiveAreas(CPlayer* pPlayer); // �÷��̾ ������ �ִ� ������ ���Ѵ�
	int GetPosToArea(const Vector3& _pos); // ���� ��ǥ�� �ش��ϴ� ������ ��ȯ


	void ExecuteUpdateArea(CPlayer* pPlayer)
	{
		int nArea = GetPosToArea(pPlayer->m_pos);
		bool bRet = PrepareUpdateArea(pPlayer, nArea);
		if (bRet) // ���� �÷��̾ �ٸ� �������� �̵��Ͽ��ٸ�
		{
			// ĳ��Ȱ��ȭ�϶�� �ش��ڿ��� ����ĳ��Ʈ��Ŷ, ��ε�ĳ��Ʈ��Ŷ
			Send_UpdateAreaForCreateObject(pPlayer);
			// ĳ������� ��ε�ĳ��Ʈ��Ŷ, �ش��ڿ��� ����ĳ��Ʈ��Ŷ
			Send_UpdateAreaForDeleteObject(pPlayer);

			// NPC���� �þ�ó��, �̹� �ξ�Ƽ�� �����Ϳ� ��Ƽ�굥���� �ִ�
			// ����������� ��Ŷ������ Ű�� ����(�̰��� ����ĳ��Ʈ��)
			// ��Ƽ�������� �ִ� NPC���� key�� ��Ƽ� Create�϶�� �����ش�
			// �ξ�Ƽ�������� �ִ� NPC���� key�� ��Ƽ� Delete�϶�� �����ش�
		}
	}

	void DoUpdateArea_ZonePlayers()
	{
		CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive);

		for (size_t i = 0; i < MAX_AREA; i++)
		{
			for (auto it = m_mapArea[i].begin(); m_mapArea[i].end() != it;)
			{
				CPlayer* pPlayer = (CPlayer*)*it;
				++it;
				ExecuteUpdateArea(pPlayer);
			}
		}
	}

	void DetectPlayerFromNPC(CNPC* pDetectNPC);


	void Send_UpdateNPC_VSn();

	// ���� ������Ʈ
	void Send_UpdateAreaForCreateObject(CPlayer* pPlayer);
	void Send_UpdateAreaForDeleteObject(CPlayer* pPlayer);
	// �̵�
	void Send_MovePlayerToActiveAreas(CPlayer *pPlayer, char *pRecvedMsg);


	void ChangeColor_Req(CPlayer* pPlayer);

private:
	CMonitorSRW		m_srwArea;
	
	vector<int>     m_vecNewActiveArea;
	vector<unsigned int>     m_vecDeleteArea;

	set<CPlayer*>		m_mapArea[MAX_AREA];
};
CREATE_FUNCTION(CAreaManager, AreaManager);
