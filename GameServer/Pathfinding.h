#pragma once
#include "Grid.h"
#include <queue>
#include "Heap.h"

class Pathfinding final
{
public:
	Pathfinding();
	~Pathfinding();

	Grid grid;

	// 공사중
	struct CompareNode
	{
		bool operator()(Node* a, Node* b)
		{
			///*
			if (a->fCost() == b->fCost())
			{
				if (a->hCost == b->hCost)
				{
					printf("CompareNode_gCost");
					return a->gCost > b->gCost;
				}
				else
					return a->hCost > b->hCost;
			}
			else//*/
				return a->fCost() > b->fCost();
		}
	};

	bool FindPath(Vector3& startPos, Vector3& targetPos, vector<Node*>** pVecPath)
	{
		Node& startNode = grid.NodeFromWorldPoint(startPos);
		Node& targetNode = grid.NodeFromWorldPoint(targetPos);

		m_vecPath.clear();

		// 최소힙
		//std::priority_queue<Node*, vector<Node*>, CompareNode> openSet;
		//std::set<Node*> openSetForFind;
		//std::set<Node*> closedSet;
		
		//while (!openSet.empty())
			//openSet.pop();
		m_pOpenSet->Clear();
		openSetForFind.clear();
		closedSet.clear();
		//
		//openSet.push(&startNode);
		m_pOpenSet->Add(&startNode);
		openSetForFind.insert(&startNode);
		//

		//
		// TODO
		Node* pNodeForFail = nullptr;
		//

		int maxFindCnt = 0;
		while (1000 > maxFindCnt++ && !m_pOpenSet->IsEmpty())
		{
			//
			Node& currentNode = *m_pOpenSet->RemoveFirst();
			//openSet.pop();
			openSetForFind.erase(&currentNode);
			//
			closedSet.insert(&currentNode);

			//
			// TODO
			if (2 == maxFindCnt)
				pNodeForFail = &currentNode;
			//

			if (currentNode == targetNode)
			{
				RetracePath(startNode, targetNode);
				*pVecPath = &m_vecPath;
				return true;
			}

			vector<Node*>& vecNeighbours = grid.GetNeighbours(currentNode);

			while(0 < vecNeighbours.size())
			{
				Node& neighbour = *vecNeighbours.back();
				vecNeighbours.pop_back();

				if (!neighbour.walkable || (closedSet.end() != closedSet.find(&neighbour)))
				{
					continue;
				}

				int newMovementCostToNeighbour = currentNode.gCost + GetDistance(currentNode, neighbour);
				if ((newMovementCostToNeighbour < neighbour.gCost) || (openSetForFind.end() == openSetForFind.find(&neighbour)))
				{
					// OpenSet에 있어도 새로 비용을 업데이트 해야한다면, 추가하지는 않고 업데이트만 새롭게 한다
					//if((openSetForFind.end() != openSetForFind.find(&neighbour) && (newMovementCostToNeighbour < neighbour.gCost)))
						//printf("newAstarUpdate ");

					neighbour.gCost = newMovementCostToNeighbour;
					neighbour.hCost = GetDistance(neighbour, targetNode);
					neighbour.parent = &currentNode;

					if ((openSetForFind.end() == openSetForFind.find(&neighbour)))
					{
						//openSet.push(&neighbour);
						m_pOpenSet->Add(&neighbour);
						openSetForFind.insert(&neighbour);
					}
					else
					{
						// G비용이 수정되었으니 OpenSet을 다시 수정해야한다
						m_pOpenSet->SortUp(&neighbour);
					}
				}
			} // while
		} // while

		m_vecPath.push_back(pNodeForFail);
		*pVecPath = &m_vecPath;
		return false;
	}

	void RetracePath(Node& startNode, Node& endNode)
	{
		Node* currentNode = &endNode;
		int maxLoop = 0;
		while (!(*currentNode == startNode))
		{
			if (1000 <= maxLoop++)
			{
				puts("RetracePath 루프벗어남");
				break;
			}
			m_vecPath.push_back(currentNode);

			Node* pTemp = currentNode->parent;
			currentNode->parent = nullptr; // test
			currentNode = pTemp;
		}

		///*
		// 뒤집기
		size_t startIndex = 0;
		size_t lastIndex = m_vecPath.size();
		if (0 < lastIndex)
			lastIndex--;
		while (startIndex < lastIndex)
		{
			Node& startTemp = *m_vecPath[startIndex];
			m_vecPath[startIndex] = m_vecPath[lastIndex];
			m_vecPath[lastIndex] = &startTemp;
			startIndex++;
			lastIndex--;
		}
		//*/
	}

	int GetDistance(Node& nodeA, Node& nodeB)
	{
		//int dstX = abs(nodeA.gridX - nodeB.gridX);
		//int dstY = abs(nodeA.gridY - nodeB.gridY);
		// -(51, 52)

	    //- (50, 50)

		int dstX = MathfAbs(nodeA.gridX - nodeB.gridX);
		int dstY = MathfAbs(nodeA.gridY - nodeB.gridY);

		if (dstX > dstY)
			return 14 * dstY + 10 * (dstX - dstY);
		return 14 * dstX + 10 * (dstY - dstX);
	}

	bool IsWalkable(const Vector3& targetPos)
	{
		return grid.NodeFromWorldPoint(targetPos).walkable;
	}

	int MathfAbs(int num)
	{
		int result = 0;
		if (num < 0)
			result = num * -1;
		else
			result = num;
		return result;
	}


	bool IsBlockedByObjects(const Vector3& worldPosition)
	{
		int cnt = 0;
		vector<Node*>& vecNeighbours = grid.GetNeighbours(grid.NodeFromWorldPoint(worldPosition));
		for (const Node* pNode : vecNeighbours)
		{
			if (false == pNode->walkable)
				cnt++;
		}

		if(8 == cnt)
			return true;
		else
			return false;
	}

	Node& BlockNode(const Vector3& worldPosition)
	{
		Node& node = grid.NodeFromWorldPoint(worldPosition);
		if (node.walkable)
		{
			node.walkable = false;
			return node;
		}
		else
		{
			printf("BlockNode 이미 막힘/ ");
			return node;
		}
	}

	private:
		//std::priority_queue<Node*, vector<Node*>, CompareNode> openSet;
		std::set<Node*> openSetForFind;
		std::set<Node*> closedSet;
		std::vector<Node*> m_vecPath;

		CHeap* m_pOpenSet;
};