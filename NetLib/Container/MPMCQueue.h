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
수정자: 이광빈
수정내용: 2^63이 넘어도 사용가능합니다, pop()이 nullptr을 반환하지 않습니다.
*/

/*
원본 제약사항
 - QUEUE_MAX_SIZE는 2의 거듭제곱만 가능합니다.
 - mHeadPos와 mTailPos는 2^63이 넘지 않아야 합니다.
 - Pop()은 null을 반환할 수 있습니다. null을 반환했다면 Pop()을 다시 시도하셔야 합니다.
*/

/*
수정본 제약사항
 - QUEUE_MAX_SIZE는 2의 거듭제곱만 가능합니다.
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
					LOG(LOG_ERROR_LOW, "SYSTEM | MPMCQueue::Push() | 롤백");
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