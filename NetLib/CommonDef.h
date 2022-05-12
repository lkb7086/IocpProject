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
//Connection설정을 위한 config구조체
typedef struct _INITCONFIG
{
	int				nIndex;				// Connection index
	SOCKET			sockListener;		// Listen Socket
	//recv Ringbuffer size  = nRecvBufCnt * nRecvBufSize
	int				nRecvBufCnt;		// 버퍼 개수
	//send Ringbuffer size  = nSendBufCnt * nSendBufSize
	int				nSendBufCnt;		// 버퍼 개수
	int				nRecvBufSize;		// 한 번에 송수신할 버퍼의 크기
	int				nSendBufSize;		// 한 번에 송수신할 버퍼의 크기
	int				nProcessPacketCnt;	// 최대 처리 패킷의 개수
	unsigned short				nServerPort;
	int				nWorkerThreadCnt;	// io 처리를 위한 thread 개수
	int				nProcessThreadCnt;	// 패킷처리를 위한 thread 개수

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
	int						s_nTotalBytes;   // 한 패킷의 사이즈
	DWORD					s_dwRemain;      // 한 패킷을 모두 받지 못했을 때 현재까지 받은 패킷의 바이트 수
	char*					s_lpSocketMsg; 	 // 한 패킷의 가장 처음을 가리키는 포인터, 만약 패킷의 길이가 길어 잘려서 오는 경우 이 변수로 처음부분을 기억
	eOperationType			s_eOperation;    // 작업의 종류
	void*					s_lpConnection;  // Overlapped 작업을 요청한 Connection객체의 포인터를 설정, 예)m_pPlayer[i]
	_OVERLAPPED_EX(void* lpConnection)
	{
		ZeroMemory(this, sizeof(OVERLAPPED_EX));
		s_lpConnection = lpConnection;
	}
} OVERLAPPED_EX, *LPOVERLAPPED_EX;

// pNext를 PROCESSPACKET형으로 하면 컴파일오류
typedef struct _PROCESSPACKET
{
	_PROCESSPACKET* pNext;
	eOperationType	s_eOperationType;
	WPARAM			s_wParam;   // 릴리즈내용
	LPARAM			s_lParam;   // 해당패킷 링버퍼포인터
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