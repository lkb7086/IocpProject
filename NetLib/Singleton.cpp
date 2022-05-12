#include "Precompile.h"

CSingleton::SINGLETON_LIST CSingleton::m_listSingleton;

CSingleton::CSingleton()
{
	m_listSingleton.AddFront(this);
}

CSingleton::~CSingleton() {}

void CSingleton::ReleaseAll()
{
	/*
	SINGLETON_LIST::iterator singleton_it = m_listSingleton.begin();
	while (m_listSingleton.end() != singleton_it)
	{
		if (nullptr != (*singleton_it))
			(*singleton_it)->releaseInstance();
		singleton_it = m_listSingleton.erase(singleton_it);
	}

	m_listSingleton.clear();
	*/

	while (!m_listSingleton.IsEmpty())
	{
		CSingleton* pSingOBJ = m_listSingleton.GetFront();
		if (nullptr == pSingOBJ)
			continue;
		pSingOBJ->ReleaseInstance();
		m_listSingleton.RemoveFront();
	}
}