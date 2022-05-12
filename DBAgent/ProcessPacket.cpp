#include "stdafx.h"
//#include "processpacket.h"

CProcessPacket::CProcessPacket() {}

CProcessPacket::~CProcessPacket() {}

void CProcessPacket::fnSearchUnit_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DB_SearchUnit_Rq(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnLogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DB_LogoutPlayer_Cn(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnInsertDropItem_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DB_InsertDropItem_Cn(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnMerseDropItem_Cn(CConnection *_pConn, DWORD _dwSize, char *_pRecvMsg)
{
	DatabaseManager()->DB_MerseDropItem_Cn(_pConn, _dwSize, _pRecvMsg);
}

void CProcessPacket::fnDeleteItem_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DB_DeleteItem_Cn(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnConnectToGSRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_UnitCheck(pConn, dwSize, pRecvedMsg);
}
void CProcessPacket::fnCLAllSearchMarketCn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_AllSearchMarket(pConn, dwSize, pRecvedMsg);
}
void CProcessPacket::fnCLSearchTextFromMarketRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_SearchTextFromMarket(pConn, dwSize, pRecvedMsg);
}
void CProcessPacket::fnCL_DA_SearchInven_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_SearchInven(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnDoPurchaseItemInMarket(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_DoPurchaseItemInMarket(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnCL_DA_DoInsertItemToMarket_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_CL_DA_DoInsertItemToMarket_Rq(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnDA_GS_LogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_DA_GS_LogoutPlayer_Cn(pConn, dwSize, pRecvedMsg);
}






