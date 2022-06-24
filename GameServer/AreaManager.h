#pragma once

class CAreaManager final : public CSingleton
{
	DECLEAR_SINGLETON(CAreaManager);
public:
	CAreaManager();
	~CAreaManager();

	// 유틸리티
	void Send_TCP_RecvBufferFromServer(DWORD dwSize, char* pRecvBuffer); // 플레이어에게 서버에서 받은 버퍼를 그대로 보낸다

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

	bool PrepareUpdateArea(CPlayer* pPlayer, int _byAreaIndex); // 플레이어가 이동을 하여 영역이 변화
	void UpdateActiveAreas(CPlayer* pPlayer); // 플레이어가 영향을 주는 영역을 구한다
	int GetPosToArea(const Vector3& _pos); // 현재 좌표에 해당하는 영역을 반환


	void ExecuteUpdateArea(CPlayer* pPlayer)
	{
		if (PrepareUpdateArea(pPlayer, GetPosToArea(pPlayer->m_pos))) // 현재 플레이어가 다른 지역으로 이동하였다면
		{
			// 캐릭활성화하라고 해당자에게 유니캐스트패킷, 브로드캐스트패킷
			Send_UpdateAreaForCreateObject(pPlayer);
			// 캐릭끄라고 브로드캐스트패킷, 해당자에게 유니캐스트패킷
			Send_UpdateAreaForDeleteObject(pPlayer);

			// NPC스폰 시야처리, 이미 인액티브 데이터와 액티브데이터 있다
			// 같은방법으로 패킷보내서 키고 끈다(이경우는 유니캐스트만)
			// 액티브지역에 있는 NPC들의 key를 모아서 Create하라고 보내준다
			// 인액티브지역에 있는 NPC들의 key를 모아서 Delete하라고 보내준다
		}
	}

	void DoUpdateArea_ZonePlayers()
	{
		//CMonitorSRW::OwnerSRW lock(m_srwArea, LockExclusive); // 없어도 된다, 왜냐하면 삭제를 이스레드가 하기 때문

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

	// 지역 업데이트
	void Send_UpdateAreaForCreateObject(CPlayer* pPlayer);
	void Send_UpdateAreaForDeleteObject(CPlayer* pPlayer, bool isNormal = true);

	void LogoutAreaPlayer_Not(CPlayer* pPlayer)
	{
		tls_pSer->StartSerialize();
		tls_pSer->Serialize(static_cast<packet_type>(PacketType::LogoutAreaPlayer_Not));
		tls_pSer->Serialize(pPlayer->GetKey());

		int* pActiveAreas = pPlayer->GetActiveAreas();
		for (int i = 0; i < MAX_ACTIVE_AREAS; i++)
		{
			int area = pActiveAreas[i];
			if (area < 0 || area >= MAX_AREA)
				continue;

			for (auto it = m_mapArea[area].begin(); it != m_mapArea[area].end(); ++it)
			{
				CPlayer* pAreaPlayer = (CPlayer*)*it;
				if (nullptr == pAreaPlayer || pAreaPlayer == pPlayer)
					continue;
				char* pBuf = pAreaPlayer->PrepareSendPacket(tls_pSer->GetCurBufSize());
				if (nullptr == pBuf)
					continue;
				tls_pSer->CopyBuffer(pBuf);
				pAreaPlayer->SendPost(tls_pSer->GetCurBufSize());
			}
		}
	}

	// 이동
	void Send_MovePlayerToActiveAreas(CPlayer *pPlayer, char *pRecvedMsg);


	void ChangeColor_Req(CPlayer* pPlayer);

private:
	CMonitorSRW		m_srwArea;
	
	vector<int>     m_vecNewActiveArea;
	vector<unsigned int>     m_vecDeleteArea;

	concurrent_unordered_set<CPlayer*>		m_mapArea[MAX_AREA];
};
CREATE_FUNCTION(CAreaManager, AreaManager);
