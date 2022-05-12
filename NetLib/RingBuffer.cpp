#include "precompile.h"

CRingBuffer::CRingBuffer()
{
	m_pRingBuffer = nullptr;
	m_pBeginMark = nullptr;
	m_pEndMark = nullptr;
	m_pCurrentMark = nullptr;
	m_pGettedBufferMark = nullptr;
	m_pLastMoveMark = nullptr;
	m_nUsedBufferSize = 0;
	m_uiAllUserBufSize = 0;
}

CRingBuffer::~CRingBuffer()
{
	if (nullptr != m_pBeginMark)
	{
		delete[] m_pBeginMark;
		m_pBeginMark = nullptr;
	}
}

bool CRingBuffer::Initialize()
{
	m_nUsedBufferSize = 0;
	m_pCurrentMark = m_pBeginMark;
	m_pGettedBufferMark = m_pBeginMark;
	m_pLastMoveMark = m_pEndMark;
	m_uiAllUserBufSize = 0;
	return true;
}

bool CRingBuffer::Create(int nBufferSize)
{
	if (nullptr != m_pBeginMark)
		delete[] m_pBeginMark;

	m_pBeginMark = new(nothrow) char[nBufferSize];
	if (nullptr == m_pBeginMark)
	{ printf("CRingBuffer::Create / error / Memory allocation failed\n"); return false; }

	m_pEndMark = m_pBeginMark + nBufferSize - 1;
	m_nBufferSize = nBufferSize;

	Initialize();
	return true;
}

char* CRingBuffer::ForwardMark(int nForwardLength) // danger: thread safe x // 보낼 데이터의 길이
{
	char* pPreCurrentMark = nullptr;

	{
		CMonitor::Owner lock(m_csRingBuffer);

		// 링버퍼 오버플로 체크
		if (m_nUsedBufferSize + nForwardLength > m_nBufferSize)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ForwardMark(1) | Overflow");
			return nullptr;
		}

		// 버퍼가 충분하면
		if ((m_pEndMark - m_pCurrentMark) >= nForwardLength)
		{
			pPreCurrentMark = m_pCurrentMark;
			m_pCurrentMark += nForwardLength;
		}
		else  // 버퍼가 모자르면
		{
			m_pLastMoveMark = m_pCurrentMark;  // GetBuffer()를 부르기전에 미리 끝위치를 설정해서 막아놓는다
			m_pCurrentMark = m_pBeginMark + nForwardLength;
			pPreCurrentMark = m_pBeginMark;
		}
	}

	return pPreCurrentMark;
}

char* CRingBuffer::ForwardMark(__int64 nForwardLength // 현재까지 받고 큐에 보내준 데이터의 길이 (이 길이만큼 앞으로 움직여 줘야함)
	, int nNextLength   // 다음에 받을 최대 데이터의 길이 4096
	, DWORD dwRemainLength) // 한 패킷이 되지 못하고 남은 패킷의 길이
{
	// 링버퍼 오버플로 체크
	{
		CMonitor::Owner lock(m_csRingBuffer);
		if (m_nUsedBufferSize + nForwardLength + nNextLength > m_nBufferSize)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ForwardMark(3) | Overflow");
			return nullptr;
		}
	}

	// 앞으로 받을버퍼가 충분하면
	if ((m_pEndMark - m_pCurrentMark) > (nNextLength + nForwardLength))
	{
		m_pCurrentMark += nForwardLength;
	}
	else  // 앞으로 받을버퍼가 모자르면
	{
		// 짤린 패킷만큼만 m_pBeginMark에 복사하고, m_pCurrentMark는 m_pBeginMark + dwRemainLength으로 순환
		m_pLastMoveMark = m_pCurrentMark;
		CopyMemory(m_pBeginMark, m_pCurrentMark - (dwRemainLength - nForwardLength), dwRemainLength);
		m_pCurrentMark = m_pBeginMark + dwRemainLength;
	}

	return m_pCurrentMark;
}

void CRingBuffer::BackwardMark(int nBackwardLength)
{
	/* 아직 사용하지 않는다.
	CMonitor::Owner lock( m_csRingBuffer );
	{
	// nBackwardLength양만큼 현재 버퍼포인터를 뒤로 보낸다
	m_nUsedBufferSize -= nBackwardLength;
	m_pCurrentMark -= nBackwardLength;
	}
	*/
}

void CRingBuffer::ReleaseBuffer(int nReleaseSize) // danger: thread safe x
{
	CMonitor::Owner lock(m_csRingBuffer);
	//InterlockedExchangeAdd((LPLONG)&m_nUsedBufferSize, -nReleaseSize);
	m_nUsedBufferSize -= nReleaseSize;
	//printf("ReleaseBuffer: %d / Who: %p\n", m_nUsedBufferSize, this);
	if (0 > m_nUsedBufferSize || 450000 < m_nUsedBufferSize)
		LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ReleaseBuffer() | m_nUsedBufferSize %d / Who: %p", m_nUsedBufferSize, this);
}
void CRingBuffer::ReleaseRecvBuffer(int nReleaseSize)
{
	CMonitor::Owner lock(m_csRingBuffer);
	//InterlockedExchangeAdd((LPLONG)&m_nUsedBufferSize, -nReleaseSize);
	m_nUsedBufferSize -= nReleaseSize;
	if (0 > m_nUsedBufferSize || 500 < m_nUsedBufferSize)
		LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ReleaseRecvBuffer() | m_nUsedBufferSize %d / Who: %p", m_nUsedBufferSize, this);
}

//사용된 버퍼량 설정, 이것을 하는 이유는 SendPost()함수가 멀티 쓰레드에서 돌아가기때문에
//PrepareSendPacket()에서(ForwardMark()에서) 사용된 양을 늘려버리면 PrepareSendPacket한다음에 데이터를
//채워 넣기전에 바로 다른 쓰레드에서 SendPost()가 불린다면 엉뚱한 쓰레기 데이터가 갈 수 있다
//그걸 방지하기 위해 데이터를 다 채운 상태에서만 사용된 버퍼 사이즈를 설정할 수 있어야한다
//이함수는 sendpost함수에서 불리게 된다
bool CRingBuffer::SetUsedBufferSize(int nUsedBufferSize) // danger: thread safe x
{
	CMonitor::Owner lock(m_csRingBuffer);
	//InterlockedExchangeAdd((LPLONG)&m_nUsedBufferSize, nUsedBufferSize);
	//InterlockedExchangeAdd((LPLONG)&m_uiAllUserBufSize, nUsedBufferSize);
	m_nUsedBufferSize += nUsedBufferSize;
	//m_uiAllUserBufSize += nUsedBufferSize; // 현재 필요없다

	// 링버퍼 오버플로 체크
	if (m_nUsedBufferSize > m_nBufferSize)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::SetUsedBufferSize() | overflow %d, %p", m_nUsedBufferSize, this);
		return false;
	}
	
	return true;
}
void CRingBuffer::SetUsedRecvBufferSize(int nUsedBufferSize)
{
	CMonitor::Owner lock(m_csRingBuffer);
	//InterlockedExchangeAdd((LPLONG)&m_nUsedBufferSize, nUsedBufferSize);
	m_nUsedBufferSize += nUsedBufferSize;
}

// ForwardMark(1)에서 먼저 copy해놓은 메모리를 실질적으로 불러서 보내는 함수, ForwardMark(1)은 copy만 한다
char* CRingBuffer::GetBuffer(int nReadSize, int* pReadSize)  // danger: thread safe x
{
	char* pRet = nullptr;

	{
		// 버퍼의 끝까지 다 보냈다면 맨 앞으로 순환해야 한다
		CMonitor::Owner lock(m_csRingBuffer);
		if (m_pLastMoveMark == m_pGettedBufferMark)
		{
			m_pGettedBufferMark = m_pBeginMark;
			m_pLastMoveMark = m_pEndMark;
		}

		// 이렇게 if문 두개로 나누는 이유: 정의한 최대 전송크기(페이지크기, 페이지락, 너무큰 패킷)때문, *pReadSize = nReadSize;
		if (m_nUsedBufferSize > nReadSize) // 쌓인 패킷들이 4096보다 크면 (정의한 페이지크기) 
		{
			if ((m_pLastMoveMark - m_pGettedBufferMark) >= nReadSize) // 4096보다 남은버퍼가 크다면(충분하면)
			{
				*pReadSize = nReadSize;
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += nReadSize;
			}
			else // 작다면 마지막 남은버퍼만 일단 보내고 순환, 완료되면 다시 (진입)에서 순환하고 남은거 보낸다
			{
				*pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark);
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += *pReadSize;
			}
		}
		else if (m_nUsedBufferSize > 0) // 쌓인 패킷들이 4096보다 작으면 (통상)
		{
			// 통상, 남은버퍼가 보내려는 크기보다 같거나 크다면(충분하면)
			if ((m_pLastMoveMark - m_pGettedBufferMark) >= m_nUsedBufferSize) // m_pLastMoveMark는 유동적이다. 버퍼의 끝이 될 수도 있고, 끝이 아닐 수도 있다. m_pLastMoveMark = m_pCurrentMark;
			{
				*pReadSize = m_nUsedBufferSize;
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += m_nUsedBufferSize;
			}
			else  // 버퍼가 모자르다면 일단 남은 걸로 보낸다, 나중에 (진입)에서 순환
			{
				*pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark); // 일단 남은거 마져 끝까지 보내고, 나중에 순환한다
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += *pReadSize;     // 이런 경우 m_pLastMoveMark == m_pGettedBufferMark, 때문에 본 함수 진입전 m_pGettedBufferMark = m_pBeginMark;
			}
		}
	}

	return pRet;
}