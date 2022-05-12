#pragma once

class CHeap final
{
public:

	Node** items;
	int currentItemCount;

	CHeap(int maxHeapSize);
	~CHeap();

	void Add(Node* item)
	{
		item->HeapIndex = currentItemCount;
		items[currentItemCount] = item;
		SortUp(item);
		currentItemCount++;
	}

	Node* RemoveFirst()
	{
		if (0 == currentItemCount)
			return nullptr;

		Node* firstItem = items[0];
		currentItemCount--;
		items[0] = items[currentItemCount];
		items[0]->HeapIndex = 0;
		SortDown(items[0]);
		return firstItem;
	}

	void SortDown(Node* item)
	{
		while (true)
		{
			int childIndexLeft = item->HeapIndex * 2 + 1;
			int childIndexRight = item->HeapIndex * 2 + 2;
			int swapIndex = 0;

			if (childIndexLeft < currentItemCount) // 인덱스경계 검사
			{
				swapIndex = childIndexLeft;

				if (childIndexRight < currentItemCount) // 인덱스경계 검사
				{
					if (!(items[childIndexLeft]->CompareNode(items[childIndexRight]))) // 왼쪽 오른쪽중에 누가더 작은지
					{
						swapIndex = childIndexRight;
					}
				}

				if (!(item->CompareNode(items[swapIndex]))) // 자식이 부모보다 더 작으면 스왑
				{
					Swap(item, items[swapIndex]);
				}
				else
				{
					return;
				}

			}
			else
			{
				return;
			}

		}
	}

	void SortUp(Node* item)
	{
		if (2 > currentItemCount)
			return;

		int parentIndex = (item->HeapIndex - 1) / 2;

		while (true)
		{
			Node* parentItem = items[parentIndex];
			if (item->CompareNode(parentItem)) // item이 더 작은면 true
			{
				Swap(item, parentItem);
			}
			else
			{
				break;
			}

			parentIndex = (item->HeapIndex - 1) / 2;
		}
	}

	void Swap(Node* itemA, Node* itemB)
	{
		items[itemA->HeapIndex] = itemB;
		items[itemB->HeapIndex] = itemA;
		int itemAIndex = itemA->HeapIndex;
		itemA->HeapIndex = itemB->HeapIndex;
		itemB->HeapIndex = itemAIndex;
	}

	bool IsEmpty()
	{
		return 0 == currentItemCount;
	}

	void Clear()
	{
		currentItemCount = 0;
	}
};

