#include "stdafx.h"
#include "Pathfinding.h"


Pathfinding::Pathfinding()
{
	m_vecPath.reserve(3000);
	m_pOpenSet = new CHeap(grid.MaxSize());
}


Pathfinding::~Pathfinding()
{
}
