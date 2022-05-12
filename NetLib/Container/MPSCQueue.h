#pragma once

/****************************************************************************
https://gist.github.com/ujentus/4030262
 * Wait-Free MPSC Queue
 * Minimum requirement: Windows XP or Windows Server 2003
 * Author: @sm9kr
 * License type: GPL v3
 * References
 ** [1] http://groups.google.ru/group/comp.programming.threads/browse_frm/thread/33f79c75146582f3
 ** [2] http://www.boost.org/doc/libs/1_35_0/doc/html/intrusive/intrusive_vs_nontrusive.html
 ****************************************************************************/

/*
// �������
// ����Ͻô� �����Ϳ� NodeEntry mNodeEntry�� �����ϼž� �մϴ�.
// Push = mulitple thread safe
// Pop = one thread safe
class DataNode
{
public:
DataNode() {}

__int64		mData;
NodeEntry	mNodeEntry;
};
*/

/*
class NodeEntry
{
public:
	NodeEntry() : mNext(nullptr) {}
	NodeEntry* volatile mNext;
};

template <typename T>
class CMPSCQueue final
{
public:
	CMPSCQueue() : mHead(&mStub), mTail(&mStub)
	{
		mOffset = reinterpret_cast<__int64>(&((reinterpret_cast<T*>(0))->mNodeEntry));
	}
	~CMPSCQueue() = default;

	void Push(T* newData)
	{
		NodeEntry* prevNode = (NodeEntry*)InterlockedExchangePointer((void*)&mHead, (void*)&(newData->mNodeEntry));
		prevNode->mNext = &(newData->mNodeEntry);
	}

	T* Pop()
	{
		NodeEntry* tail = mTail;
		NodeEntry* next = tail->mNext;

		if (tail == &mStub)
		{
			/// �����Ͱ� ���� ��
			if (nullptr == next)
				return nullptr;

			/// ó�� ���� ��
			mTail = next;
			tail = next;
			next = next->mNext;
		}

		/// ��κ��� ��쿡 �����͸� ���� ��
		if (next)
		{
			mTail = next;

			return reinterpret_cast<T*>(reinterpret_cast<__int64>(tail)-mOffset);
		}

		NodeEntry* head = mHead;
		if (tail != head)
			return nullptr;

		/// ������ ������ ���� ��
		mStub.mNext = nullptr;
		NodeEntry* prev = (NodeEntry*)InterlockedExchangePointer((void*)&mHead, (void*)&mStub);
		prev->mNext = &mStub;

		next = tail->mNext;
		if (next)
		{
			mTail = next;

			return reinterpret_cast<T*>(reinterpret_cast<__int64>(tail)-mOffset);
		}

		return nullptr;
	}

private:
	NodeEntry* volatile	mHead;
	NodeEntry*			mTail;
	NodeEntry			mStub;
	__int64				mOffset;
};
*/