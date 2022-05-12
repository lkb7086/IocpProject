#pragma once

// shift연산 직렬화/역직렬화 양수 음수 모두테스트 이상없다
class NETLIB_API CSerializer final// : public CSingleton
{
	//DECLEAR_SINGLETON(CSerializer);
public:
	CSerializer(int nMaxBufSize = 1024 * 100);
    ~CSerializer();
	CSerializer(const CSerializer& rhs) = delete;
	CSerializer& operator=(const CSerializer& rhs) = delete;

	// Deserialize
	CSerializer& operator>>(char& _value)
	{
		memcpy_s(&_value, sizeof(char), m_pCurMark, sizeof(char));
		m_pCurMark += sizeof(char); m_nCurBufSize += sizeof(char); return *this;
	}
	CSerializer& operator>>(short& _value)
	{
		memcpy_s(&_value, sizeof(short), m_pCurMark, sizeof(short));
		m_pCurMark += sizeof(short); m_nCurBufSize += sizeof(short); return *this;
	}

	CSerializer& operator>>(int& _value)
	{
		memcpy_s(&_value, sizeof(int), m_pCurMark, sizeof(int));
		m_pCurMark += sizeof(int); m_nCurBufSize += sizeof(int); return *this;
	}
	CSerializer& operator>>(unsigned int& _value)
	{
		memcpy_s(&_value, sizeof(unsigned int), m_pCurMark, sizeof(unsigned int));
		m_pCurMark += sizeof(unsigned int); m_nCurBufSize += sizeof(unsigned int); return *this;
	}

	CSerializer& operator>>(long long& _value)
	{
		memcpy_s(&_value, sizeof(long long), m_pCurMark, sizeof(long long));
		m_pCurMark += sizeof(long long); m_nCurBufSize += sizeof(long long); return *this;
	}

	void Deserialize(char& cCh);
	void Deserialize(unsigned char& ucChar);
	void Deserialize(short& sNum);
	void Deserialize(unsigned short& usShort);
	void Deserialize(int& nNum);
	void Deserialize(unsigned int& uiInt);
	void Deserialize(long long& llLongLong);
	void Deserialize(unsigned long long& ullLongLong);
	void Deserialize(float& _fFloat);
	void Deserialize(char* pszBuffer, unsigned short usLen);
	void GetStream(void* pszBuffer, short sLen);

	// Serialize
	CSerializer& operator<<(const char _value)
	{
		memcpy_s(m_pCurMark, m_nMaxBufSize - m_nCurBufSize, &_value, sizeof(char));
		m_pCurMark += sizeof(char); m_nCurBufSize += sizeof(char); return *this;
	}
	CSerializer& operator<<(const short _value)
	{
		memcpy_s(m_pCurMark, m_nMaxBufSize - m_nCurBufSize, &_value, sizeof(short));
		m_pCurMark += sizeof(short); m_nCurBufSize += sizeof(short); return *this;
	}
	CSerializer& operator<<(const int _value)
	{
		memcpy_s(m_pCurMark, m_nMaxBufSize - m_nCurBufSize, &_value, sizeof(int));
		m_pCurMark += sizeof(int); m_nCurBufSize += sizeof(int); return *this;
	}
	CSerializer& operator<<(const long long _value)
	{
		memcpy_s(m_pCurMark, m_nMaxBufSize - m_nCurBufSize, &_value, sizeof(long long));
		m_pCurMark += sizeof(long long); m_nCurBufSize += sizeof(long long); return *this;
	}

	void Serialize(float fFloat);
	void Serialize(long long llLongLong);
	void Serialize(unsigned long long ullLongLong);
	void Serialize(int iInt);
	void Serialize(unsigned int uiInt);
	void Serialize(short sShort);
	void Serialize(unsigned short usShort);
	void Serialize(char cCh);
	void Serialize(unsigned char ucChar);
	void Serialize(const char* const pszBuffer);
	void SetStream(const void* const pszBuffer, const short sLen);

	void StartSerialize();
	void StartDeserialize(char* pVBuffer);
	bool CopyBuffer(char* pDestBuffer);
	inline int GetMaxBufSize() { return m_nMaxBufSize; }
	inline int GetCurBufSize() { return m_nCurBufSize; }
	inline char* GetCurMark() { return m_pCurMark; }
	inline char* GetBeginMark() { return m_pszVBuffer; }

private:
	char* m_pszVBuffer;		// 실제 버퍼
	char* m_pCurMark;		// 현재 버퍼 위치
	int	m_nMaxBufSize;		// 최대 버퍼 사이즈
	int m_nCurBufSize;		// 현재 사용된 버퍼 사이즈
};
//CREATE_FUNCTION(CSerializer, Serializer)