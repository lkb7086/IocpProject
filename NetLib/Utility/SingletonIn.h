#pragma once

template <typename T>
class CSingletonIn
{
protected:
	CSingletonIn() {}
	virtual ~CSingletonIn() {}

public:
	static T* Instance()
	{
		if (nullptr == m_pInstance)
		{
			m_pInstance = new T;
			atexit(DestroyInstance);
		}
		return m_pInstance;
	}

private:
	static void DestroyInstance()
	{
		if (nullptr != m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

	static T* m_pInstance;
};

template <typename T>
T* CSingletonIn<T>::m_pInstance = nullptr;