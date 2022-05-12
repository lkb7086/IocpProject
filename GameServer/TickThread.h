#pragma once
//#include "../NetLib/Container/MPMCQueue.h"
#include "Player.h"

enum class TickThreadType : int
{
	Update,
	Common
};

struct PacketInfo
{
	PacketInfo* pNext;
	CPlayer* pPlayer;
	DWORD dwSize;
	char *pMsg;

	PacketInfo() { memset(this, 0, sizeof(PacketInfo)); }
	PacketInfo(CPlayer *_pPlayer, DWORD _dwSize, char *_pMsg, __int32 _packetType) : pPlayer(_pPlayer), dwSize(_dwSize), pMsg(_pMsg){}
};

class CTickThread final : public CThread
{
public:
	CTickThread();
	~CTickThread();

	virtual void OnInitThread() override;
	virtual void OnCloseThread() override;
	virtual void OnProcess() override;

	void Enqueue_PacketQ(CPlayer* pPlayer, DWORD size, char* pMsg)
	{
		PacketInfo* pPacketInfo = m_stackPacketS.Pop();
		pPacketInfo->pPlayer = pPlayer; pPacketInfo->dwSize = size; pPacketInfo->pMsg = pMsg;
		m_queuePacketQ.Push(pPacketInfo);
	}

	GETSET(TickThreadType, TickThreadType, m_tickThreadType);
	GETSET(int, Zone, m_zone);
private:
	TickThreadType m_tickThreadType;
	PacketInfo* m_pPaketInfo;
	MPMCQueue<PacketInfo> m_queuePacketQ;
	MPMCStack<PacketInfo> m_stackPacketS;
	int m_zone;
};