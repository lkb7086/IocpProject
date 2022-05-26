#include "StdAfx.h"
//#include "tickthread.h"
#include "ProcessPacket.h"

CTickThread::CTickThread() : m_zone(-1)
{
	m_pPaketInfo = new PacketInfo[8192];
	for (size_t i = 0; i < 8192; i++)
	{
		m_stackPacketS.Push(&m_pPaketInfo[i]);
	}
}

CTickThread::~CTickThread()
{
	if (nullptr != m_pPaketInfo)
	{
		delete[] m_pPaketInfo;
		m_pPaketInfo = nullptr;
	}
}

void CTickThread::OnInitThread()
{
	tls_pSer = new(nothrow)CSerializer();
	if (nullptr == tls_pSer) return;
	tls_pEngine = new(nothrow) std::mt19937_64((std::random_device())());
	if (nullptr == tls_pEngine) return;
}

void CTickThread::OnCloseThread()
{
	if (nullptr != tls_pSer)
	{
		delete tls_pSer;
		tls_pSer = nullptr;
	}
	if (nullptr != tls_pEngine)
	{
		delete tls_pEngine;
		tls_pEngine = nullptr;
	}
}

void CTickThread::OnProcess()
{
	switch (m_tickThreadType)
	{
	case TickThreadType::Common:
	{
		while (0 < m_queuePacketQ.GetSize())
		{
			PacketInfo* pInfo = m_queuePacketQ.Pop();
			if (nullptr == pInfo)
			{
				printf("null ");
				YieldProcessor();
				continue;
			}
			else if (nullptr == pInfo->pMsg)
			{
				// 종료처리
				IocpGameServer()->OnClose(pInfo->pPlayer);
				IocpGameServer()->ProcessSystemMsg(pInfo->pPlayer, 0, static_cast<WPARAM>(GameServerSystemMsg::SYSTEM_MSG));

				memset(pInfo, 0, sizeof(PacketInfo));
				m_stackPacketS.Push(pInfo);
				continue;
			}

			// 패킷처리
			IocpGameServer()->OnRecv(pInfo->pPlayer, pInfo->dwSize, pInfo->pMsg);

			memset(pInfo, 0, sizeof(PacketInfo));
			m_stackPacketS.Push(pInfo);
			//printf("%d ", m_queuePacketQ.GetSize());
		} // while


//#ifndef _DEBUG
		//if ((m_dwTickCount % KEEPALIVE_TICK) == 0)
			//IocpGameServer()->CheckKeepAliveTick(m_dwTickCount);
//#endif
		//IocpGameServer()->InitRecvCnt();
		//IocpGameServer()->m_secTick++;

		/*
		i % 2  =>  i & 1
		i % 4  =>  i & 3
		i % 8  =>  i & 7
		i % 16  =>  i & 15
		i % 32  =>  i & 31
		*/

		// 플레이어 영역업데이트
		if (0 == (m_dwTickCount & 31))
		{
			//AreaManager()->DoUpdateArea_ZonePlayers();
		}

		// NPC 프레임처리
		if (0 == (m_dwTickCount & 3))
		{
			//NPCManager()->UpdateNpc();
			//AreaManager()->Send_UpdateNPC_VSn();
		}
	}
	break;
	default:
		break;
	}

	/*
	if ((m_dwTickCount % KEEPALIVE_TICK) == 0)
	{
	//printf("CPU            사용량: %f\n", Monitoring()->ProcessCPU_Usage());
	//printf("프로세스 메모리사용량: %u\n", Monitoring()->ProcessMemoryUsage());
	//printf("메모리사용량: %u\n", Monitoring()->PhysyicMemoryUsage());
	}
	*/
}