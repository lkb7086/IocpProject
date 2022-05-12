#pragma once
//#include "StdAfx.h"

class CProcessPacket final
{
public:
	CProcessPacket();
	~CProcessPacket();

	static void fnSearchUnit_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnLogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnInsertDropItem_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnMerseDropItem_Cn(CConnection *_pConn, DWORD _dwSize, char *_pRecvMsg);
	static void fnDeleteItem_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnConnectToGSRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnCLAllSearchMarketCn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnCLSearchTextFromMarketRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
	static void fnCL_DA_SearchInven_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnDoPurchaseItemInMarket(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnCL_DA_DoInsertItemToMarket_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);

	static void fnDA_GS_LogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg);
};