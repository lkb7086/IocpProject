#pragma once

// 190710부터 수정됨
class NETLIB_API CRingBuffer final
{
public:
	CRingBuffer();
	~CRingBuffer();
	CRingBuffer(const CRingBuffer& rhs) = delete;
	CRingBuffer& operator=(const CRingBuffer& rhs) = delete;

	// 링버퍼 메모리 할당
	bool			Create( int nBufferSize = MAX_RINGBUFSIZE );
	// 초기화
	bool			Initialize();
	// 할당된 버퍼 크기를 반환한다.
	inline int		GetBufferSize() const { return m_nBufferSize; }
	
	// 해당하는 버퍼의 포인터를 반환
	inline char*	GetBeginMark() const { return m_pBeginMark; }
	inline char*	GetCurrentMark() const { return m_pCurrentMark; }
	inline char*	GetEndMark() const { return m_pEndMark; }
	
	// 데이터를 보낼 때
	char*			ForwardMark( int nForwardLength );
	// 데이터를 받을 때
	char*			ForwardMark(__int64 nForwardLength , int nNextLength , DWORD dwRemainLength );
	void			BackwardMark( int nBackwardLength );

	// 사용된 버퍼 해제
	void			ReleaseBuffer( int nReleaseSize );
	void			ReleaseRecvBuffer(int nReleaseSize);
	// 사용된 버퍼 크기 반환
	inline int		GetUsedBufferSize() const { return m_nUsedBufferSize; }
	//사용된 버퍼양 설정(이것을 하는 이유는 SendPost()함수가 멀티 쓰레드에서 돌아가기때문에
	//PrepareSendPacket()에서(ForwardMark()에서) 사용된 양을 늘려버리면 PrepareSendPacket한다음에 데이터를
	//채워 넣기전에 바로 다른 쓰레드에서 SendPost()가 불린다면 엉뚱한 쓰레기 데이터가 갈 수 있다
	//그걸 방지하기 위해 데이터를 다 채운 상태에서만 사용된 버퍼 사이즈를 설정할 수 있어야한다
	//이 함수는 SendPost()함수에서 불리게 된다
	bool			SetUsedBufferSize( int nUsedBufferSize );
	void			SetUsedRecvBufferSize(int nUsedBufferSize);

	// 누적 버퍼 사용양 반환
	inline int		GetAllUsedBufferSize() const { return m_uiAllUserBufSize; }
	
	// 버퍼 데이터 읽어서 반환
	char*			GetBuffer( int nReadSize , int* pReadSize );
private:
	
	char*			m_pRingBuffer;		   // 실제 데이터를 저장하는 버퍼 포인터

	char*			m_pBeginMark;			// 버퍼의 처음부분을 가리키고 있는 포인터
	char*			m_pEndMark;				// 버퍼의 마지막부분을 가리키고 있는 포인터
	char*			m_pCurrentMark;			// 버퍼의 현재 부분을 가리키고 있는 포인터
	char*			m_pGettedBufferMark;	// 현재까지 데이터를 읽은 버퍼의 포인터
	char*			m_pLastMoveMark;		// ForwardMark되기 전의 마지막 포인터

	int				m_nBufferSize;		// 버퍼의 총 크기
	int 			m_nUsedBufferSize;	// 현재 사용된 버퍼의 크기
	unsigned int	m_uiAllUserBufSize; // 총 처리된 데이터량          더큰 자료형으로 해야함
	CMonitor		m_csRingBuffer;
};

