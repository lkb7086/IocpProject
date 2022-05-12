#include "../Precompile.h"
#include "LFH.h"


CLFH::CLFH()
{
	static bool init = false;
	if (init) {
		return;
	}
	init = true;

	ULONG HeapFragValue = 2;
	HANDLE hHeaps[1024];
	DWORD dwHeapCount = GetProcessHeaps(1024, hHeaps);

	// HEAP_NO_SERIALIZE로 생성된 heap에는 적용되지 않는다. 따라서 이힙은 실패한다.
	for (DWORD i = 0; i < dwHeapCount; i++)
	{
		if (HeapSetInformation(hHeaps[i], HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue)))
		{
			//printf("* Low-fragmentation Heap setting | count: %u\n", dwHeapCount);
		}
		else
		{
			//printf("error | * Low-fragmentation Heap | %u\n", GetLastError());
		}
	}
}

CLFH::~CLFH()
{
}


/*
1. 프로세스 힙에 LFH 적용
ULONG ulEnableLFH = 2;
HeapSetInformation (
GetProcessHeap(),
HeapCompatibilityInformation,
&ulEnableLFH,
sizeof(ulEnableLFH));

2. CRT 힙에 LFH 적용
intptr_t hCrtHeap = _get_heap_handle();
ULONG ulEnableLFH = 2;
if (HeapSetInformation((PVOID)hCrtHeap,
HeapCompatibilityInformation,
&ulEnableLFH, sizeof(ulEnableLFH)))
puts("Enabling Low Fragmentation Heap succeeded");
else
puts("Enabling Low Fragmentation Heap failed");
return 0;



3. 프로세스의 모든 힙에 적용

HANDLE Heaps[1025];
DWORD Count = GetProcessHeaps( 1024, Heaps );
for( DWORD i = 0; i < Count; ++i )
{
ULONG  HeapFragValue = 2;

if( HeapSetInformation( Heaps[i], HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue) ) )
{
.............
}
else
{
...................
}
}



제가 적용한 프로젝트에서는 프로세스에서 생성한 힙이 총 9개이며 세번째는 무조건 실패를 합니다.

이유는 이 힙에는 HEAP_NO_SERIALIZE flag가 설정 되어 있기 때문입니다.

HEAP_NO_SERIALIZE flag는 멀티 스레드에서 그 힙에 동시에 접근하여 할당, 해제를 가능하게 하기 위한 설정입니다.
( 참고   http://support.microsoft.com/kb/929136/ja  )


모든 힙에 적용하므로 따로 CRT 힙에는 LFH를 적용하지 않아도 된다.

모든 프로세스의 힙에 LFH를 적용 후 따로 CRT 힙에 LFH를 적용하면 실패합니다.

이유는 이미 적용을 했기 때문이다. 저는 9번째가 CRT 힙이었습니다.
*/





/*
LFH는 다음과 같은 제약 사항들을 가진다.
LFH enabled heap이라도, 할당 크기가 16kb보다 작을 것. 16kb보다 더 크면 LFH 적용이 되지 않는다.
HEAP_NO_SERIALIZE로 생성된 heap엔 적용되지 않는다. 자세한 내용은 MSDN 참고
고정 크기(Fixed size)로 생성된 heap엔 적용되지 않는다.
디버깅 툴에서는 LFH가 동작하지 않는다.
예를 들어 비쥬얼 스튜디오에서의 run 상태에선 활성화 되지 않는다.
release 모드라도 F5로 실행시엔 되지 않으니, 그냥 실행파일을 직접 실행해 보라.
*/




// 싱글스레드에서는 효율이 떨어짐