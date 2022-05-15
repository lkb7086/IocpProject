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
	//client�� ���� ������ �Ǿ��� �� ȣ��Ǵ� �Լ�
	virtual bool	OnAccept(CConnection *lpConnection) override;
	//client���� packet�� �������� �� ���� �� �ְ� ó���Ǿ����� ��Ŷó��
	virtual bool	OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	//client���� packet�� �������� �� ���� �� ���� ��ٷ� ó�� �Ǵ� ��Ŷó��
	virtual bool	OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	//client�� ������ ����Ǿ��� �� ȣ��Ǵ� �Լ�
	virtual	void	OnPrepareClose(CConnection* lpConnection) override;
	virtual void	OnClose(CConnection* lpConnection) override;
	virtual bool	OnSystemMsg(CConnection* lpConnection, LPARAM msgType, WPARAM wParam) override;
	virtual	void	UDP_OnRecv(DWORD dwSize, char* pRecvedMsg) override {}

	//���� ���� �Լ�
	bool	ServerStart();

	inline unsigned long long	GeneratePrivateKey() { return ++m_nPrivateKey; }

	//��Ŷó�� �Լ� ����
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
	//ƽ ������
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