#pragma once
class CDBThread final
{
public:
	CDBThread();
	~CDBThread();

	bool CreateThread();
	void OnDBThread();
	inline void SetEventDBThread()
	{
		if (false == m_bDBThreadUP)
			SetEvent(m_hQuitEvent);
	}
	void DestroyThread();

private:
	static bool m_bQuitFlag;
	HANDLE m_hThread;
	HANDLE m_hQuitEvent;
	bool m_bDBThreadUP;
};