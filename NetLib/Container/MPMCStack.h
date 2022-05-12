#pragma once

/*
������ ����

�������
 - ����Ͻô� �����Ϳ� �ڷ���* pNext�� �����ϼž� �մϴ�.
struct Node
{
	Node* pNext; <--����
};

// �� ������ CAS��뿡 �־ delete�� ABA������ �ֽ��ϴ�.
// Ǯ���뵵�� �ƴ϶�� ���� �ڷ� ������ ����ϼž� �մϴ�.
// Win32
union Node{ __int64 total; struct { Node* next; __int32 count; } Segment; };
InterlockedCompareExchange64
*/

/*
std::memory_order_relaxed
std::memory_order_release
std::memory_order_acqire
std::memory_order_acq_rel
std::memory_order_seq_cast

atomic_thread_fence(std::memory_order_seq_cast);
*/

template<typename T>
class MPMCStack final
{
public:
	MPMCStack() : m_pTop(nullptr) {};
	~MPMCStack() = default;

	void Push(T* _pNewElem)
	{
		do {
			_pNewElem->pNext = m_pTop;
		} while (InterlockedCompareExchangePointer((PVOID volatile*)&m_pTop, _pNewElem, _pNewElem->pNext) != _pNewElem->pNext);
	}

	T* Pop()
	{
		T* pOldTop = nullptr;

		do {
			pOldTop = m_pTop;
			if (nullptr == pOldTop)
				return nullptr;
		} while (InterlockedCompareExchangePointer((PVOID volatile*)&m_pTop, pOldTop->pNext, pOldTop) != pOldTop);

		return pOldTop;
	}

	inline bool IsEmpty() const
	{
		return (nullptr == m_pTop);
	}

private:
	T* volatile m_pTop;
};