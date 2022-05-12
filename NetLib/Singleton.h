#pragma once

#define DECLEAR_SINGLETON( className )\
public:\
	static className* Instance();\
	virtual void ReleaseInstance() override;\
private:\
	static className* m_pInstance;

#define CREATE_FUNCTION( className , funcName )\
	inline static className* funcName()\
	{\
	return className::Instance();\
	};

#define IMPLEMENT_SINGLETON( className )\
	className* className::m_pInstance = nullptr;\
	className* className::Instance()\
	{\
		if ( nullptr == m_pInstance )\
		{\
		m_pInstance = new className;\
		}\
	return m_pInstance;\
	}\
	void className::ReleaseInstance()\
	{\
		if ( nullptr != m_pInstance )\
		{\
		delete m_pInstance;\
		m_pInstance = nullptr;\
		}\
	}

class NETLIB_API CSingleton
{
public:
	CSingleton();
	virtual ~CSingleton();

public:
	virtual void ReleaseInstance() = 0;
	static void ReleaseAll();

private:
	using SINGLETON_LIST = NetLib::CList<CSingleton*>;
	//static SINGLETON_LIST m_listSingleton;

	static SINGLETON_LIST m_listSingleton;
};