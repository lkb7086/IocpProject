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

char* CRingBuffer::ForwardMark(int nForwardLength) // danger: thread safe x // ���� �������� ����
{
	char* pPreCurrentMark = nullptr;

	{
		CMonitor::Owner lock(m_csRingBuffer);

		// ������ �����÷� üũ
		if (m_nUsedBufferSize + nForwardLength > m_nBufferSize)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ForwardMark(1) | Overflow");
			return nullptr;
		}

		// ���۰� ����ϸ�
		if ((m_pEndMark - m_pCurrentMark) >= nForwardLength)
		{
			pPreCurrentMark = m_pCurrentMark;
			m_pCurrentMark += nForwardLength;
		}
		else  // ���۰� ���ڸ���
		{
			m_pLastMoveMark = m_pCurrentMark;  // GetBuffer()�� �θ������� �̸� ����ġ�� �����ؼ� ���Ƴ��´�
			m_pCurrentMark = m_pBeginMark + nForwardLength;
			pPreCurrentMark = m_pBeginMark;
		}
	}

	return pPreCurrentMark;
}

char* CRingBuffer::ForwardMark(__int64 nForwardLength // ������� �ް� ť�� ������ �������� ���� (�� ���̸�ŭ ������ ������ �����)
	, int nNextLength   // ������ ���� �ִ� �������� ���� 4096
	, DWORD dwRemainLength) // �� ��Ŷ�� ���� ���ϰ� ���� ��Ŷ�� ����
{
	// ������ �����÷� üũ
	{
		CMonitor::Owner lock(m_csRingBuffer);
		if (m_nUsedBufferSize + nForwardLength + nNextLength > m_nBufferSize)
		{
			LOG(LOG_ERROR_LOW, "SYSTEM | CRingBuffer::ForwardMark(3) | Overflow");
			return nullptr;
		}
	}

	// ������ �������۰� ����ϸ�
	if ((m_pEndMark - m_pCurrentMark) > (nNextLength + nForwardLength))
	{
		m_pCurrentMark += nForwardLength;
	}
	else  // ������ �������۰� ���ڸ���
	{
		// ©�� ��Ŷ��ŭ�� m_pBeginMark�� �����ϰ�, m_pCurrentMark�� m_pBeginMark + dwRemainLength���� ��ȯ
		m_pLastMoveMark = m_pCurrentMark;
		CopyMemory(m_pBeginMark, m_pCurrentMark - (dwRemainLength - nForwardLength), dwRemainLength);
		m_pCurrentMark = m_pBeginMark + dwRemainLength;
	}

	return m_pCurrentMark;
}

void CRingBuffer::BackwardMark(int nBackwardLength)
{
	/* ���� ������� �ʴ´�.
	CMonitor::Owner lock( m_csRingBuffer );
	{
	// nBackwardLength�縸ŭ ���� ���������͸� �ڷ� ������
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

//���� ���۷� ����, �̰��� �ϴ� ������ SendPost()�Լ��� ��Ƽ �����忡�� ���ư��⶧����
//PrepareSendPacket()����(ForwardMark()����) ���� ���� �÷������� PrepareSendPacket�Ѵ����� �����͸�
//ä�� �ֱ����� �ٷ� �ٸ� �����忡�� SendPost()�� �Ҹ��ٸ� ������ ������ �����Ͱ� �� �� �ִ�
//�װ� �����ϱ� ���� �����͸� �� ä�� ���¿����� ���� ���� ����� ������ �� �־���Ѵ�
//���Լ��� sendpost�Լ����� �Ҹ��� �ȴ�
bool CRingBuffer::SetUsedBufferSize(int nUsedBufferSize) // danger: thread safe x
{
	CMonitor::Owner lock(m_csRingBuffer);
	//InterlockedExchangeAdd((LPLONG)&m_nUsedBufferSize, nUsedBufferSize);
	//InterlockedExchangeAdd((LPLONG)&m_uiAllUserBufSize, nUsedBufferSize);
	m_nUsedBufferSize += nUsedBufferSize;
	//m_uiAllUserBufSize += nUsedBufferSize; // ���� �ʿ����

	// ������ �����÷� üũ
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

// ForwardMark(1)���� ���� copy�س��� �޸𸮸� ���������� �ҷ��� ������ �Լ�, ForwardMark(1)�� copy�� �Ѵ�
char* CRingBuffer::GetBuffer(int nReadSize, int* pReadSize)  // danger: thread safe x
{
	char* pRet = nullptr;

	{
		// ������ ������ �� ���´ٸ� �� ������ ��ȯ�ؾ� �Ѵ�
		CMonitor::Owner lock(m_csRingBuffer);
		if (m_pLastMoveMark == m_pGettedBufferMark)
		{
			m_pGettedBufferMark = m_pBeginMark;
			m_pLastMoveMark = m_pEndMark;
		}

		// �̷��� if�� �ΰ��� ������ ����: ������ �ִ� ����ũ��(������ũ��, ��������, �ʹ�ū ��Ŷ)����, *pReadSize = nReadSize;
		if (m_nUsedBufferSize > nReadSize) // ���� ��Ŷ���� 4096���� ũ�� (������ ������ũ��) 
		{
			if ((m_pLastMoveMark - m_pGettedBufferMark) >= nReadSize) // 4096���� �������۰� ũ�ٸ�(����ϸ�)
			{
				*pReadSize = nReadSize;
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += nReadSize;
			}
			else // �۴ٸ� ������ �������۸� �ϴ� ������ ��ȯ, �Ϸ�Ǹ� �ٽ� (����)���� ��ȯ�ϰ� ������ ������
			{
				*pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark);
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += *pReadSize;
			}
		}
		else if (m_nUsedBufferSize > 0) // ���� ��Ŷ���� 4096���� ������ (���)
		{
			// ���, �������۰� �������� ũ�⺸�� ���ų� ũ�ٸ�(����ϸ�)
			if ((m_pLastMoveMark - m_pGettedBufferMark) >= m_nUsedBufferSize) // m_pLastMoveMark�� �������̴�. ������ ���� �� ���� �ְ�, ���� �ƴ� ���� �ִ�. m_pLastMoveMark = m_pCurrentMark;
			{
				*pReadSize = m_nUsedBufferSize;
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += m_nUsedBufferSize;
			}
			else  // ���۰� ���ڸ��ٸ� �ϴ� ���� �ɷ� ������, ���߿� (����)���� ��ȯ
			{
				*pReadSize = (int)(m_pLastMoveMark - m_pGettedBufferMark); // �ϴ� ������ ���� ������ ������, ���߿� ��ȯ�Ѵ�
				pRet = m_pGettedBufferMark;
				m_pGettedBufferMark += *pReadSize;     // �̷� ��� m_pLastMoveMark == m_pGettedBufferMark, ������ �� �Լ� ������ m_pGettedBufferMark = m_pBeginMark;
			}
		}
	}

	return pRet;
}