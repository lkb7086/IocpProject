#pragma once

typedef unsigned short packet_length;
typedef unsigned short packet_type;
const int PACKET_SIZE_LENGTH = sizeof(packet_length);
const int PACKET_TYPE_LENGTH = sizeof(packet_type);

const int MAX_QUEUESIZE = 10000; // default queue size in CQueue class
const int MAX_STRING_SIZE = 4096;
const int MAX_RINGBUFSIZE = 1024 * 100; // default Ringbuffer size
const int MAX_IP_LENGTH = 20;
const int MAX_PROCESS_THREAD = 1;
const int MAX_WORKER_THREAD = 9;

const int MAX_POOL_USER_COUNT = 100; // MAX_USER_COUNT / 5;

const float PI = 3.141519f;
const float RAD2DEG = static_cast<float>(180 / PI);
const float DEG2RAD = static_cast<float>(PI * 2) / 360;

const __int64 MAX_SIGNED_8BYTE = 0x7FFFFFFFFFFFFFFF;
const __int64 MAX_SIGNED_8BYTE_MINUS = 0x7FFFFFFFFFFFFFFF - 0xFFFFF;

#ifdef NETLIB_EXPORTS
#define NETLIB_API __declspec(dllexport)
#else
#define NETLIB_API __declspec(dllimport)
#endif

enum class eOperationType : int
{
	// Work IOCP operation
	OP_SEND,
	OP_RECV,
	OP_ACCEPT,

	// Process IOCP operation
	OP_CLOSE,
	OP_RECVPACKET,
	OP_SYSTEM,

	// TODO
	OP_RELEASE_TCP_RECVBUF
};

/////////////////////////////////////////////////
//Connection������ ���� config����ü
typedef struct _INITCONFIG
{
	int				nIndex;				// Connection index
	SOCKET			sockListener;		// Listen Socket
	//recv Ringbuffer size  = nRecvBufCnt * nRecvBufSize
	int				nRecvBufCnt;		// ���� ����
	//send Ringbuffer size  = nSendBufCnt * nSendBufSize
	int				nSendBufCnt;		// ���� ����
	int				nRecvBufSize;		// �� ���� �ۼ����� ������ ũ��
	int				nSendBufSize;		// �� ���� �ۼ����� ������ ũ��
	int				nProcessPacketCnt;	// �ִ� ó�� ��Ŷ�� ����
	unsigned short				nServerPort;
	int				nWorkerThreadCnt;	// io ó���� ���� thread ����
	int				nProcessThreadCnt;	// ��Ŷó���� ���� thread ����

	int				connPoolSize;

	_INITCONFIG()
	{
		ZeroMemory(this, sizeof(INITCONFIG));
	}
}INITCONFIG;

typedef struct _OVERLAPPED_EX
{
	WSAOVERLAPPED			s_Overlapped;
	WSABUF					s_WsaBuf;
	int						s_nTotalBytes;   // �� ��Ŷ�� ������
	DWORD					s_dwRemain;      // �� ��Ŷ�� ��� ���� ������ �� ������� ���� ��Ŷ�� ����Ʈ ��
	char*					s_lpSocketMsg; 	 // �� ��Ŷ�� ���� ó���� ����Ű�� ������, ���� ��Ŷ�� ���̰� ��� �߷��� ���� ��� �� ������ ó���κ��� ���
	eOperationType			s_eOperation;    // �۾��� ����
	void*					s_lpConnection;  // Overlapped �۾��� ��û�� Connection��ü�� �����͸� ����, ��)m_pPlayer[i]
	_OVERLAPPED_EX(void* lpConnection)
	{
		ZeroMemory(this, sizeof(OVERLAPPED_EX));
		s_lpConnection = lpConnection;
	}
} OVERLAPPED_EX, *LPOVERLAPPED_EX;

// pNext�� PROCESSPACKET������ �ϸ� �����Ͽ���
typedef struct _PROCESSPACKET
{
	_PROCESSPACKET* pNext;
	eOperationType	s_eOperationType;
	WPARAM			s_wParam;   // �������
	LPARAM			s_lParam;   // �ش���Ŷ ������������
	DWORD           s_dwCurrentSize;
	void*			s_lpConnection;

	_PROCESSPACKET()
	{
		Init();
	}

	void Init()
	{
		ZeroMemory(this, sizeof(PROCESSPACKET));
	}
} PROCESSPACKET, *LPPROCESSPACKET;











#define	GETSET(varType, funcName, memberVar)\
inline varType Get##funcName() const\
{\
    return memberVar;\
}\
inline void Set##funcName(varType parameter)\
{\
    memberVar = parameter;\
}