#include "stdafx.h"
#include "processpacket.h"

CProcessPacket::CProcessPacket()
{
}

CProcessPacket::~CProcessPacket()
{
}

void CProcessPacket::fnConfirmID_Req(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->ConfirmID_Rq(pConn, pRecvedMsg);
}

void CProcessPacket::fnJoinID_Req(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->JoinID_Req(pConn, pRecvedMsg);
}

void CProcessPacket::LogoutPlayerID_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->LogoutPlayerID_Not(pRecvedMsg);
}







void CProcessPacket::fnSelectUnit_Rq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->DB_SelectUnit_Rq(pConn, pRecvedMsg);
}

void CProcessPacket::fnSelectUnit_Aq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//unsigned int nLSPKey = 0xffffffff;
	//Serializer()->StartDeserialize(pRecvedMsg);
	//Serializer()->Deserialize(nLSPKey);

	stUtil_UInteger *pPacket = (stUtil_UInteger*)pRecvedMsg;

	CConnection *pPlayer = ConnectionManager()->FindConnection(pPacket->nUInteger); // PKey
	if (nullptr == pPlayer) return;
	char* pBuffer = pPlayer->PrepareSendPacket(dwSize);
	if (nullptr == pBuffer)
		return;
	CopyMemory(pBuffer, pRecvedMsg, dwSize);
	pPlayer->SendPost(dwSize);
}

void CProcessPacket::fnLogoutPlayer_Cn(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->Recv_LogoutPlayer_Cn(pConn, pRecvedMsg);
}

void CProcessPacket::fnCreateUnitRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_CreateUnit(pConn, pRecvedMsg);
}

void CProcessPacket::fnDeleteUnitRq(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	//DatabaseManager()->DB_DeleteUnit(pConn, pRecvedMsg);
}

void CProcessPacket::fnImServer_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	pConn->m_bIsCilent = false;
	ConnectionManager()->SetServerConn(pConn);
}

