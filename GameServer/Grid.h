#pragma once

//__declspec(align(#))
//alignas(#)
struct alignas(64) Node
{
public:
	int gridX;
	int gridY;
	int gCost;
	int hCost;
	Node* parent;
	int HeapIndex;
	Vector3 worldPosition;
	bool walkable;
	bool isObstacle;

	Node() = default;
	Node(bool _walkable, Vector3& _worldPos, int _gridX, int _gridY) : gCost(0), hCost(0), isObstacle(false)
	{
		walkable = _walkable;
		worldPosition = _worldPos;
		gridX = _gridX;
		gridY = _gridY;
		parent = nullptr;
		HeapIndex = -1;
	}

	int fCost() const
	{
		return gCost + hCost;
	}

	bool operator==(const Node& nodeToCompare) const
	{
		return gridX == nodeToCompare.gridX && gridY == nodeToCompare.gridY;
	}

	// a가 더 작은 FCost라면 true
	bool CompareNode(Node* b)
	{
		///*
		if (fCost() == b->fCost())
		{
			if (hCost == b->hCost)
			{
				//printf("CompareNode_gCost");
				return gCost < b->gCost;
			}
			else
				return hCost < b->hCost;
		}
		else//*/
			return fCost() < b->fCost();
	}

	/*
	// 최소힙 비교
	bool operator>(const Node& nodeToCompare) const // 공사중
	{
		if (fCost() == nodeToCompare.fCost())
		{
			return hCost > nodeToCompare.hCost;
		}
		
		return fCost() > nodeToCompare.fCost();
	}
	*/

	// 해시
	//friend std::hash<Node>;
	// set중복 검사
	//friend bool operator==(const Node& x1, const Node& x2);
};

/*
namespace std
{                                                                                    
	template <>                                                
	class hash<Node>
	{
	public:
		size_t operator()(const Node& node) const
		{
			using std::hash;
			return hash<int>()(node.) ^ hash<int>()(node.) << 1;
		}
	};
};

bool operator==(const Node& node1, const Node& node2)
{
	return (node1.worldPosition.x == node2.worldPosition.x) && (node1.worldPosition.y == node2.worldPosition.y);
}
*/

class Grid final
{

public:
	Grid();
	~Grid();

	vector<Node*> neighbours;
	Vector3 gridWorldSize;
	float nodeRadius;
	Node grid[200][200];
	float nodeDiameter;
	int gridSizeX, gridSizeY;


	int MaxSize()
	{
		return gridSizeX * gridSizeY;
	}


	void CreateGrid();
	vector<Node*>& GetNeighbours(Node& node);
	Node& NodeFromWorldPoint(const Vector3& worldPosition);

	float Clamp01(float f)
	{
		if (1.0f < f)
			return 1.0f;
		else if (0.0f > f)
			return 0.0f;
		return f;
	}

	int RoundToInt(float val)
	{
		if (val >= 0.0f)
			return (int)(val + 0.5f);
		return (int)(val - 0.5f);
	}

private:
};

