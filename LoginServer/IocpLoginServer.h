#pragma once
#include "TickThread.h"
#include "ProcessPacket.h"

class CIocpLoginServer final : public CIocpServer, CSingleton
{
	DECLEAR_SINGLETON(CIocpLoginServer);
public:
	CIocpLoginServer();
	~CIocpLoginServer();

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

	//서버 시작 함수
	bool	ServerStart();

	inline unsigned long long	GeneratePrivateKey() { return ++m_nPrivateKey; }

	//패킷처리 함수 설정
	void			InitProcessFunc();

	CConnection*	GetGameServerConn() { return m_pGameServerConn; }

	bool ConnectToGameServer();

	void CalcXor(char *packet, int packetOffset, DWORD packetLen)
	{
		//string key_ = "fl392jd9wjc9ajdjqurobngjfh3";
		//int keyLength_ = (int)key_.length();
		char key_[] = "vdskjsdgwioenv";
		int keyLength_ = sizeof(key_);

		int keyIdx = packetOffset % keyLength_;
		for (DWORD packetIdx = sizeof(unsigned int); packetIdx < packetLen; packetIdx++)
		{
			packet[packetIdx] ^= (char)key_[keyIdx++];

			if (keyIdx == keyLength_)
				keyIdx = 0;
		}
	}



private:
	CConnection*		m_pGameServerConn;
	//틱 쓰레드
	CTickThread*		m_pTickThread;

	unsigned long long				m_nPrivateKey;


	// TODO ////////////////////////////////////////////////////////
private:
	typedef void(*funcProcessPacket)(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg);
	typedef pair< PacketType, funcProcessPacket >			PACKET_PAIR;
	typedef unordered_map< PacketType, funcProcessPacket >  PACKET_UNMAP;
	typedef PACKET_UNMAP::iterator							PACKET_IT;
	PACKET_UNMAP un_mapPakect;
};
CREATE_FUNCTION(CIocpLoginServer, IocpLoginServer);