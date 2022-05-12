#pragma once

// 제약사항
// T는 포인터형으로만 사용가능합니다.
template <typename T>
class CQueue final
{
public:
	CQueue(int nMaxSize = MAX_QUEUESIZE);
	~CQueue();

	bool	PushQueue(const T& typeQueueItem);
	void	PopQueue();

	bool	IsEmptyQueue();
	T		GetFrontQueue();
	int		GetQueueSize();
	int		GetQueueMaxSize() { return m_nQueueMaxSize; }
	void	SetQueueMaxSize(int nMaxSize) { m_nQueueMaxSize = nMaxSize; }
	void	ClearQueue();

private:
	T*		    	m_pQueue;
	int				m_nQueueMaxSize;

	int				m_nCurSize;
	int				m_nEndMark;
	int				m_nBeginMark;
};

template <typename T>
CQueue<T>::CQueue(int nMaxSize)
{
	m_pQueue = new(nothrow)T[nMaxSize];
	if (nullptr == m_pQueue) return;
	m_nQueueMaxSize = nMaxSize;
	ClearQueue();
}

template <typename T>
CQueue<T>::~CQueue()
{
	if (nullptr != m_pQueue)
	{
		delete[] m_pQueue;
		m_pQueue = nullptr;
	}
}

template <typename T>
bool CQueue<T>::PushQueue(const T& typeQueueItem)
{
	if (m_nCurSize >= m_nQueueMaxSize)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CQueue::PushQueue() | queue is full, m_nCurSize: %d", m_nCurSize);
		return false;
	}

	if (m_nEndMark == m_nQueueMaxSize)
		m_nEndMark = 0;
	m_pQueue[m_nEndMark++] = typeQueueItem;
	m_nCurSize++;

	return true;
}

template <typename T>
T CQueue<T>::GetFrontQueue()
{
	if (m_nCurSize <= 0)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | CQueue::GetFrontQueue() | queue is empty, m_nCurSize: %d", m_nCurSize);
		return nullptr;
	}

	if (m_nBeginMark == m_nQueueMaxSize)
		m_nBeginMark = 0;

	return m_pQueue[m_nBeginMark++];
}

template <typename T>
void CQueue<T>::PopQueue()
{
	m_nCurSize--;
}

template <typename T>
bool CQueue<T>::IsEmptyQueue()
{
	return (m_nCurSize > 0) ? false : true;
}

template <typename T>
int CQueue<T>::GetQueueSize()
{
	return m_nCurSize;
}

template <typename T>
void CQueue<T>::ClearQueue()
{
	m_nCurSize = 0;
	m_nEndMark = 0;
	m_nBeginMark = 0;
}