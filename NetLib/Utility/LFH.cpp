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

	// HEAP_NO_SERIALIZE�� ������ heap���� ������� �ʴ´�. ���� ������ �����Ѵ�.
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
1. ���μ��� ���� LFH ����
ULONG ulEnableLFH = 2;
HeapSetInformation (
GetProcessHeap(),
HeapCompatibilityInformation,
&ulEnableLFH,
sizeof(ulEnableLFH));

2. CRT ���� LFH ����
intptr_t hCrtHeap = _get_heap_handle();
ULONG ulEnableLFH = 2;
if (HeapSetInformation((PVOID)hCrtHeap,
HeapCompatibilityInformation,
&ulEnableLFH, sizeof(ulEnableLFH)))
puts("Enabling Low Fragmentation Heap succeeded");
else
puts("Enabling Low Fragmentation Heap failed");
return 0;



3. ���μ����� ��� ���� ����

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



���� ������ ������Ʈ������ ���μ������� ������ ���� �� 9���̸� ����°�� ������ ���и� �մϴ�.

������ �� ������ HEAP_NO_SERIALIZE flag�� ���� �Ǿ� �ֱ� �����Դϴ�.

HEAP_NO_SERIALIZE flag�� ��Ƽ �����忡�� �� ���� ���ÿ� �����Ͽ� �Ҵ�, ������ �����ϰ� �ϱ� ���� �����Դϴ�.
( ����   http://support.microsoft.com/kb/929136/ja  )


��� ���� �����ϹǷ� ���� CRT ������ LFH�� �������� �ʾƵ� �ȴ�.

��� ���μ����� ���� LFH�� ���� �� ���� CRT ���� LFH�� �����ϸ� �����մϴ�.

������ �̹� ������ �߱� �����̴�. ���� 9��°�� CRT ���̾����ϴ�.
*/





/*
LFH�� ������ ���� ���� ���׵��� ������.
LFH enabled heap�̶�, �Ҵ� ũ�Ⱑ 16kb���� ���� ��. 16kb���� �� ũ�� LFH ������ ���� �ʴ´�.
HEAP_NO_SERIALIZE�� ������ heap�� ������� �ʴ´�. �ڼ��� ������ MSDN ����
���� ũ��(Fixed size)�� ������ heap�� ������� �ʴ´�.
����� �������� LFH�� �������� �ʴ´�.
���� ��� ����� ��Ʃ��������� run ���¿��� Ȱ��ȭ ���� �ʴ´�.
release ���� F5�� ����ÿ� ���� ������, �׳� ���������� ���� ������ ����.
*/




// �̱۽����忡���� ȿ���� ������