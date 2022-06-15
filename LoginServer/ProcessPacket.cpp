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
	DatabaseManager()->ConfirmID_Req(pConn, dwSize, pRecvedMsg);
}

void CProcessPacket::fnConfirmIDGameServer_Res(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->ConfirmIDGameServer_Res(pRecvedMsg);
}

void CProcessPacket::fnJoinID_Req(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->JoinID_Req(pConn, pRecvedMsg);
}

void CProcessPacket::fnLogoutPlayerID_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->LogoutPlayerID_Not(pRecvedMsg);
}

void CProcessPacket::fnMoveServer_Not1(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->MoveServer_Not1(pRecvedMsg, dwSize);
}

void CProcessPacket::fnMoveServer_Not2(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	DatabaseManager()->MoveServer_Not2(pRecvedMsg, dwSize);
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
	((CPlayer*)pConn)->m_serverID = ((stUtil_Char*)pRecvedMsg)->nChar;
	ConnectionManager()->AddServerCon(((stUtil_Char*)pRecvedMsg)->nChar, pConn);
}



void CProcessPacket::fnNosql_Not(CConnection* pConn, DWORD dwSize, char* pRecvedMsg)
{
	CConnection* pCon = IocpLoginServer()->GetNoSQLServerConn();
	if (pCon == nullptr)
		return;
	char* pBuf = pCon->PrepareSendPacket(dwSize);
	if (pBuf == nullptr)
		return;
	memcpy_s(pBuf, dwSize, pRecvedMsg, dwSize);
	pCon->SendPost(dwSize);
}