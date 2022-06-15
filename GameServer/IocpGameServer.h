#pragma once
#include "TickThread.h"
#include "Player.h"

enum class GameServerSystemMsg : int
{
	SYSTEM_MSG,
	SYSTEM_UPDATE_NPCPOS,
	SYSTEM_UPDATE_AUTOPLAYER,
	SYSTEM_UPDATE_AREA
};

class CIocpGameServer final : public CIocpServer, CSingleton
{
private:
	struct stSearchData
	{
		unsigned int nAccox_UID;
		unsigned short userClass;
		char szName[MAX_NICKNAME_LENGTH];

		stSearchData() : nAccox_UID(0xffffffff)
		{
			memset(szName, 0x00, sizeof(szName));
		}
	};

	DECLEAR_SINGLETON(CIocpGameServer);
public:
	CIocpGameServer();
	~CIocpGameServer();
	CIocpGameServer(const CIocpGameServer&) = delete;
	CIocpGameServer& operator=(const CIocpGameServer&) = delete;
	CIocpGameServer(CIocpGameServer&&) = delete;
	CIocpGameServer& operator=(CIocpGameServer&&) = delete;

	void			InitProcessFunc();
	bool			GameServerStart();

	virtual	void	OnInitIocpServer() override;
	virtual void	OnClose_IocpServer() override;
	virtual bool	OnAccept(CConnection *lpConnection) override;
	virtual bool	OnRecv(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	virtual	void	UDP_OnRecv(DWORD dwSize, char* pRecvedMsg) override;
	virtual bool	OnRecvImmediately(CConnection* lpConnection, DWORD dwSize, char* pRecvedMsg) override;
	virtual	void	OnPrepareClose(CConnection* lpConnection) override;
	virtual void	OnClose(CConnection* lpConnection) override;
	virtual bool	OnSystemMsg(CConnection* lpConnection, LPARAM lParam, WPARAM wParam) override;
	
	void ServerOffTest();

	void			ProcessSystemMsg(CPlayer* pPlayer, LPARAM lParam, WPARAM wParam);

	int				GetINIString(TCHAR* szOutStr, TCHAR* szAppName, TCHAR* szKey, int nSize, TCHAR* szFileName);
	int				GetINIInt(TCHAR* szAppName, TCHAR* szKey, TCHAR* szFileName);

	bool			ConnectToNpcServer();
	bool            ConnectToDBAgent();
	bool            ConnectToLoginServer();
	bool			ConnectToNoSQLServer();

	inline CTickThread* GetTickThread() { return m_pTickThread; }
	inline DWORD		GetServerTick() { return m_pTickThread->GetTickCount(); }
	inline CConnection* GetNpcServerConn() { return m_pNpcServerConn; }
	inline CConnection* GetDbAgentConn() { return m_pDbAgentConn; }
	inline CConnection* GetLoginServerConn() { return m_pLoginServerConn; }
	inline CConnection* GetNoSQLServerConn() { return m_pNoSQLServerConn; }


	void ConfirmIDGameServer_Req(CPlayer* pPlayer, char* pRecvedMsg);
	void StartLobby_Req(CPlayer* pPlayer, char* pRecvedMsg);	

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



	CRingBuffer m_ringBuffer;
	char m_dayAndNightTime;
	unsigned int m_hostCount;
	unsigned long long m_secTick;
	static HANDLE		m_hMainThreadEvent;
	CTickThread* m_pTickThread;
	char		m_serverID;
private:
	TCHAR				m_szLogFileName[MAX_LOGFILENAME_LENGTH];
	

	CConnection*		m_pNpcServerConn;
	CConnection*		m_pDbAgentConn;
	CConnection*		m_pLoginServerConn;
	CConnection*		m_pNoSQLServerConn;


	CMonitor m_csAccept;

	//////////////////////////////////////////////////////////
	typedef void(*funcProcessPacket)(CPlayer* pPlayer, DWORD dwSize, char* pRecvedMsg);
	////////////////////////////////////////////////////////////////
	using PACKET_PAIR = pair< PacketType, funcProcessPacket >;
	using PACKET_UNMAP = unordered_map< PacketType, funcProcessPacket >;
	using PACKET_IT = PACKET_UNMAP::iterator;
	PACKET_UNMAP mapPakect;
	////////////////////////////////////////////////////////////////
	concurrent_unordered_map<int, CConnection*> m_setConn;
	////////////////////////////////////////////////////////////////
	map<unsigned long long, string> m_mapSERVER;
};
CREATE_FUNCTION(CIocpGameServer, IocpGameServer);




/*
class CXORCrypt
{
public:
	CXORCrypt() {};
	~CXORCrypt() {};

	static void Encrypt(BYTE* source, BYTE* destination, DWORD length)
	{
		DWORD i;
		INT key = KEY;

		if (!source || !destination || length <= 0)
		{
			return;
		}

		for (i = 0; i < length; i++)
		{
			destination[i] = source[i] ^ key >> 8;
			key = (destination[i] + key) * C1 + C2;
		}

		return;
	}

	static void Decrypt(BYTE* source, BYTE* destination, DWORD length)
	{
		DWORD i;
		BYTE previousBlock;
		INT key = KEY;

		if (!source || !destination || length <= 0)
		{
			return;
		}

		for (i = 0; i < length; i++)
		{
			previousBlock = source[i];
			destination[i] = source[i] ^ key >> 8;
			key = (previousBlock + key) * C1 + C2;
		}

		return;
	}
};


	BYTE origin[] = "abcdefg";
	BYTE encrypt[10];
	BYTE decrypt[10];
	ZeroMemory(encrypt, sizeof(encrypt));
	ZeroMemory(decrypt, sizeof(decrypt));

	CXORCrypt::Encrypt(origin, encrypt, sizeof(origin));

	std::cout << "원  본 : " << origin << std::endl;
	std::cout << "암호화 : " << encrypt << std::endl;

	CXORCrypt::Decrypt(encrypt, decrypt, sizeof(encrypt));

	std::cout << "복호화 : " << decrypt << std::endl;
*/