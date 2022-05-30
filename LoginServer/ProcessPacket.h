#pragma once

class CProcessPacket final
{
public:
	CProcessPacket();
	~CProcessPacket();

	static void fnConfirmID_Req(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnConfirmIDGameServer_Res(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnJoinID_Req(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnLogoutPlayerID_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnMoveServer_Not1(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnMoveServer_Not2(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	




	static void fnSelectUnit_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnSelectUnit_Aq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnLogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnCreateUnitRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnDeleteUnitRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);


	static void fnImServer_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnNosql_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
};