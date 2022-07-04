#include "stdafx.h"


IMPLEMENT_SINGLETON(CObjectManager);


CObjectManager::CObjectManager()
{
	m_pCube = new(nothrow) Cube[10];
	for (unsigned int i = 0; i < 10; i++)
	{
		if (m_mapCube.find(i) == m_mapCube.end())
		{
			m_pCube[i].uid = i;
			m_mapCube.insert(pair<unsigned int, Cube*>(i, &m_pCube[i]));
		}
	}


}