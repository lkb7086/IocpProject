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

	bool	ServerStart();

	//��Ŷó�� �Լ� ����
	void			InitProcessFunc();
private:
	//ƽ ������
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