#include "stdafx.h"
#include "Heap.h"


CHeap::CHeap(int maxHeapSize) : currentItemCount(0)
{
	items = new Node*[maxHeapSize];
}


CHeap::~CHeap()
{
}
