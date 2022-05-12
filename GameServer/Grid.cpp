#include "stdafx.h"
#include "Grid.h"


Grid::Grid()
{
	neighbours.reserve(400);
	nodeRadius = 0.5f;
	nodeDiameter = nodeRadius * 2;
	gridWorldSize.x = 200;
	gridWorldSize.y = 200;
	gridSizeX = RoundToInt(gridWorldSize.x / nodeDiameter);
	gridSizeY = RoundToInt(gridWorldSize.y / nodeDiameter);
	CreateGrid();
}


Grid::~Grid()
{
}

void Grid::CreateGrid()
{
	FILE* fp = NULL;
	if ((0 != fopen_s(&fp, "WorldMap.txt", "rt")))
	{
		fputs("���� ���� ����", stderr);
		system("pause");
		exit(1);
	}

	Vector3 worldBottomLeft = Vector3(-100.0f, -100.0f, 0.0f);

	// ������� �ʴ´�. ���Ϸ� �ʵ����͸� �޾ƾ� �Ѵ�, grid[x, y]�� �ڷḦ ������Ʈ �ؾ��Ѵ�.
	// �ӽ�
	for (int x = 0; x < gridSizeX; x++)
	{
		for (int y = 0; y < gridSizeY; y++)
		{
			bool isWalkable = true;
			bool isObstacle = false;

			// ������ �ҷ�����
			int ch = EOF;
			/*
			while ((ch = fgetc(fp)) != EOF)
			{
				printf("%d", ch);
			}*/

			if ((ch = fgetc(fp)) != EOF)
			{
				if (49 == ch) // ��ֹ��̸�
				{
					isWalkable = false;
					isObstacle = true;
					//printf("%d", ch);
				}
			}

			Vector3 worldPoint = worldBottomLeft + Vector3((x*nodeDiameter) + nodeRadius, (y*nodeDiameter) + nodeRadius, 0.0f);
			grid[x][y] = Node(isWalkable, worldPoint, x, y);
			grid[x][y].isObstacle = isObstacle;

			if (false == grid[x][y].walkable) // debug
			{
				//printf("%d %d / ", (int)grid[x][y].worldPosition.x, (int)grid[x][y].worldPosition.y);
			}
		}
	}

	fclose(fp);

	/*
	grid = new Node[gridSizeX, gridSizeY];
	Vector3 worldBottomLeft = transform.position - Vector3.right * gridWorldSize.x / 2 - Vector3.forward * gridWorldSize.y / 2;

	for (int x = 0; x < gridSizeX; x++)
	{
		for (int y = 0; y < gridSizeY; y++)
		{
			Vector3 worldPoint = worldBottomLeft + Vector3.right * (x * nodeDiameter + nodeRadius) + Vector3.forward * (y * nodeDiameter + nodeRadius);

			// Fixed
			Vector3 temp = new Vector3(worldPoint.x, worldPoint.z, 0.0f);

			// Fixed
			//bool walkable = !(Physics.CheckSphere(temp, nodeRadius, unwalkableMask));
			bool walkable = !(Physics2D.OverlapCircle(temp, nodeRadius - 0.1f, unwalkableMask));

			// TODO
			//if (!walkable)
			//Instantiate(prefab_noway).transform.position = temp;

			grid[x, y] = new Node(walkable, worldPoint, x, y);
		}
	}
	*/
}

vector<Node*>& Grid::GetNeighbours(Node& node)
{
	neighbours.clear();
	//vector<Node*> neighbours;

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x == 0 && y == 0)
				continue;

			int checkX = node.gridX + x;
			int checkY = node.gridY + y;

			if (checkX >= 0 && checkX < gridSizeX && checkY >= 0 && checkY < gridSizeY)
			{
				neighbours.push_back(&grid[checkX][checkY]);
			}
		}
	}

	return neighbours;
}

Node& Grid::NodeFromWorldPoint(const Vector3& worldPosition)
{
	// ��ã�� ���������� ������Ʈ�� �ڸ���� �ִ� ��ġ�� ������ �󸶳� �ǳ� ���Ѵ�
	// �� ���ʿ� ������ 0.06���� �ȴ�. ������ 0.0���� 1.0����
	float percentX = (worldPosition.x + gridWorldSize.x / 2) / gridWorldSize.x;
	float percentY = (worldPosition.y + gridWorldSize.y / 2) / gridWorldSize.y;

	// Clamp01�� ���ǹ� ������ ���� ���׷� ��ã�� ������ ����� ���� ���� ������ġ
	percentX = Clamp01(percentX);
	percentY = Clamp01(percentY);

	// 99*0.06, 5.94, 6, RoundToInt�� �ݿø��Ϸ���
	int x = RoundToInt((gridSizeX - 1) * percentX);
	int y = RoundToInt((gridSizeY - 1) * percentY);
	return grid[x][y];
}
