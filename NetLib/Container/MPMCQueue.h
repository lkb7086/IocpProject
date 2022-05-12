#pragma once
#include <atomic>
#include <mutex>

#define _ASSERTC(expr)\
{\
	if (!(expr))\
		{\
		int* dummy = 0;\
		*dummy = 0xDEADBEEF;\
		}\
}

/*
https://gist.github.com/ujentus/4044153
Author: @sm9kr
License type : GPL v3
������: �̱���
��������: 2^63�� �Ѿ ��밡���մϴ�, pop()�� nullptr�� ��ȯ���� �ʽ��ϴ�.
*/

/*
���� �������
 - QUEUE_MAX_SIZE�� 2�� �ŵ������� �����մϴ�.
 - mHeadPos�� mTailPos�� 2^63�� ���� �ʾƾ� �մϴ�.
 - Pop()�� null�� ��ȯ�� �� �ֽ��ϴ�. null�� ��ȯ�ߴٸ� Pop()�� �ٽ� �õ��ϼž� �մϴ�.
*/

/*
������ �������
 - QUEUE_MAX_SIZE�� 2�� �ŵ������� �����մϴ�.
*/

template <typename T>
class MPMCQueue final
{
public:
	template<int E>
	struct PowerOfTwo
	{
		enum { value = 2 * PowerOfTwo<E - 1>::value };
	};

	template<>
	struct PowerOfTwo<0>
	{
		enum { value = 1 };
	};

	/*
	enum
	{
		QUEUE_MAX_SIZE = PowerOfTwo<16>::value,
		QUEUE_SIZE_MASK = QUEUE_MAX_SIZE - 1
	};
	*/

	MPMCQueue() : mHeadPos(0), mTailPos(0), mRealTailPos(0)
	{
		memset(mElem, 0, sizeof(mElem));
	}
	~MPMCQueue() = default;

	void Push(T* newElem)
	{
		__int64 insertPos = InterlockedIncrement64(&mTailPos) - 1;
		_ASSERTC(insertPos - mHeadPos < QUEUE_MAX_SIZE);

		if (MAX_SIGNED_8BYTE_MINUS < insertPos) // if (1000 < insertPos)
		{
			{
				std::lock_guard<std::mutex> lock(mMutex);
				if (MAX_SIGNED_8BYTE_MINUS < mTailPos) // if (1000 < mTailPos)
				{
					while (MAX_SIGNED_8BYTE_MINUS >= mHeadPos) // while (1000 >= mHeadPos)
					{
						LOG(LOG_ERROR_LOW, "SYSTEM | MPMCQueue::Push() | %d | %d", mTailPos, mHeadPos);
						Sleep(1);
						continue;
					}
					mHeadPos = 0;
					mRealTailPos = 0;
					InterlockedExchange64(&mTailPos, 0);
					LOG(LOG_ERROR_LOW, "SYSTEM | MPMCQueue::Push() | �ѹ�");
				}
			}

			insertPos = InterlockedIncrement64(&mTailPos) - 1;
			_ASSERTC(insertPos - mHeadPos < QUEUE_MAX_SIZE);
		}

		mElem[insertPos & QUEUE_SIZE_MASK] = newElem;
		InterlockedIncrement64(&mRealTailPos);
		//*/
		


		/*
		__int64 insertPos = InterlockedIncrement64(&mTailPos) - 1;
		_ASSERTC(insertPos - mHeadPos < QUEUE_MAX_SIZE);

		mElem[insertPos & QUEUE_SIZE_MASK] = newElem;
		*/
	}

	T* Pop()
	{
		T* popVal = (T*)InterlockedExchangePointer((void**)&mElem[mHeadPos & QUEUE_SIZE_MASK], nullptr);

		if (popVal != nullptr)
			InterlockedIncrement64(&mHeadPos);

		return popVal;
	}

	/*
	// legacy
	inline __int64 GetSize() const
	{
		return mTailPos - mHeadPos;
	}
	*/

	inline __int64 GetSize() const
	{
		return mRealTailPos - mHeadPos;
	}

private:
	enum
	{
		QUEUE_MAX_SIZE = PowerOfTwo<16>::value,
		QUEUE_SIZE_MASK = QUEUE_MAX_SIZE - 1
	};

	T*			mElem[QUEUE_MAX_SIZE];
	volatile __int64	mHeadPos;
	volatile __int64	mTailPos;
	volatile __int64	mRealTailPos; // TODO

	std::mutex mMutex; // TODO

	static_assert((QUEUE_MAX_SIZE & QUEUE_SIZE_MASK) == 0x0, "pool's size must be power of 2");
};