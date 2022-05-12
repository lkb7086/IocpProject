#pragma once
#include "TickThread.h"
#include "ProcessPacket.h"
//#include "StdAfx.h"

class CIocpDBAgent final : public CIocpServer, CSingleton
{
	DECLEAR_SINGLETON(CIocpDBAgent);
public:
	CIocpDBAgent();
	~CIocpDBAgent();

	virtual	void	OnInitIocpServer() override;
	virtual void	OnClose_IocpServer() override;
	//client가 접속 수락이 되었을 때 호출되는 함수
	virtual bool	OnAccept(CConnection *lpConnection) override;
	//client에서 packet이 도착했을 때 순서 성 있게 처리되어지는 패킷처리
	virtual bool	OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	//client에서 packet이 도착했을 때 순서 성 없이 곧바로 처리 되는 패킷처리
	virtual bool	OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	//client와 연결이 종료되었을 때 호출되는 함수
	virtual	void	OnPrepareClose(CConnection* lpConnection) override;
	virtual void	OnClose(CConnection* lpConnection) override;
	virtual bool	OnSystemMsg(CConnection* lpConnection, LPARAM msgType, WPARAM wParam) override;
	virtual	void	UDP_OnRecv(DWORD dwSize, char* pRecvedMsg) override {}

	bool	ServerStart();

	//패킷처리 함수 설정
	void			InitProcessFunc();
private:
	//틱 쓰레드
	CTickThread*	   m_pTickThread;

	// TODO ////////////////////////////////////////////////////////
private:
	typedef void(*funcProcessPacket)(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg);
	typedef pair< USHORT, funcProcessPacket >			PACKET_PAIR;
	typedef unordered_map< USHORT, funcProcessPacket >  PACKET_UNMAP;
	typedef PACKET_UNMAP::iterator						PACKET_IT;
	PACKET_UNMAP un_mapPakect;
	////////////////////////////////////////////////////////////////
};
CREATE_FUNCTION(CIocpDBAgent, IocpDBAgent);