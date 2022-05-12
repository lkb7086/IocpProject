#include "precompile.h"
#pragma warning(disable : 4244) // '=' : 'int'���� 'char'(��)�� ��ȯ�ϸ鼭 �����Ͱ� �սǵ� �� �ֽ��ϴ�.
#pragma warning(disable : 4293) // '<<' : ����Ʈ Ƚ���� �����̰ų� �ʹ� Ů�ϴ�. ���ǵ��� ���� �����Դϴ�.

//IMPLEMENT_SINGLETON(CSerializer)

// ���ʽ���Ʈ ���x, �����ʽ���Ʈ�� ���o
CSerializer::CSerializer(int nMaxBufSize)
{
	m_pszVBuffer = new(nothrow) char[nMaxBufSize];
	if (nullptr == m_pszVBuffer) { printf("CSerializer::CSerializer / error / Memory allocation failed\n"); return; }
	m_nMaxBufSize = nMaxBufSize;
	StartSerialize();
}

CSerializer::~CSerializer()
{
	if (nullptr != m_pszVBuffer)
	{
		delete[] m_pszVBuffer;
		m_pszVBuffer = nullptr;
	}
}

void CSerializer::StartSerialize() // ���ڵ��� ���ۺκ�
{
	// ��Ŷ���̴� ������ ���� �����
	m_pCurMark = m_pszVBuffer + PACKET_SIZE_LENGTH;	
	m_nCurBufSize = PACKET_SIZE_LENGTH;				
}

/*
unsinged�� ��쿡�� ����Ʈ����� ���� 2�辿 �þ, 2 4 8 16 32
������ unsinged�� ���: << >> ��� ����Ʈ�̵�
������ singed�� ���: <<����Ʈ�̵�, >>�����Ʈ�̵�
*/

// Deserialize
void CSerializer::Deserialize(char& cChar)
{
	cChar = (unsigned char)*m_pCurMark;
	m_pCurMark += 1;
	m_nCurBufSize += 1;
}

void CSerializer::Deserialize(unsigned char& ucChar)
{
	ucChar = (unsigned char)*m_pCurMark;
	m_pCurMark += 1;
	m_nCurBufSize += 1;
}

void CSerializer::Deserialize(short& sShort)
{
	sShort = (unsigned char)*m_pCurMark +
		(((unsigned char)*(m_pCurMark + 1)) << 8);
	m_pCurMark += 2;
	m_nCurBufSize += 2;
}

void CSerializer::Deserialize(unsigned short& usShort)
{
	usShort = (unsigned char)*m_pCurMark +
		(((unsigned char)*(m_pCurMark + 1)) << 8);
	m_pCurMark += 2;
	m_nCurBufSize += 2;
}

void CSerializer::Deserialize(int& iInt)
{
	iInt = (unsigned char)m_pCurMark[0] +
		((unsigned char)m_pCurMark[1] << 8) +
		((unsigned char)m_pCurMark[2] << 16) +
		((unsigned char)m_pCurMark[3] << 24);
	m_pCurMark += 4;
	m_nCurBufSize += 4;
}

void CSerializer::Deserialize(unsigned int& uiInt)
{
	uiInt = (unsigned char)m_pCurMark[0] +
		((unsigned char)m_pCurMark[1] << 8) +
		((unsigned char)m_pCurMark[2] << 16) +
		((unsigned char)m_pCurMark[3] << 24);
	m_pCurMark += 4;
	m_nCurBufSize += 4;
}

void CSerializer::Deserialize(long long& llLongLong)
{
	llLongLong = (unsigned char)m_pCurMark[0] +
		((unsigned char)m_pCurMark[1] << 8) +
		((unsigned char)m_pCurMark[2] << 16) +
		((unsigned char)m_pCurMark[3] << 24) +
		((unsigned char)m_pCurMark[4] << 32) +
		((unsigned char)m_pCurMark[5] << 40) +
		((unsigned char)m_pCurMark[6] << 48) +
		((unsigned char)m_pCurMark[7] << 56);
	m_pCurMark += 8;
	m_nCurBufSize += 8;
}

void CSerializer::Deserialize(unsigned long long& ullLongLong)
{
	ullLongLong = (unsigned char)m_pCurMark[0] +
		((unsigned char)m_pCurMark[1] << 8) +
		((unsigned char)m_pCurMark[2] << 16) +
		((unsigned char)m_pCurMark[3] << 24) +
		((unsigned char)m_pCurMark[4] << 32) +
		((unsigned char)m_pCurMark[5] << 40) +
		((unsigned char)m_pCurMark[6] << 48) +
		((unsigned char)m_pCurMark[7] << 56);
	m_pCurMark += 8;
	m_nCurBufSize += 8;
}

void CSerializer::Deserialize(float& _fFloat)
{
	memcpy_s(&_fFloat, sizeof(float), m_pCurMark, sizeof(float));
	m_pCurMark += sizeof(float);
	m_nCurBufSize += sizeof(float);
}

void CSerializer::GetStream(void* pszBuffer, short sLen) // ���ڿ����� byte stream�� ������
{
	CopyMemory( pszBuffer , m_pCurMark , sLen );
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

void CSerializer::Deserialize(char *pszBuffer, unsigned short usLen) // ���ڿ�
{
	unsigned short sLength = 0;
	Deserialize(sLength);

	if (sLength < 0 || sLength > MAX_STRING_SIZE)
	{
		LOG(LOG_ERROR_LOW, "CSerializer::Deserialize() | sLength < 0 || sLength > MAX_STRING_SIZE %d", sLength);
		return;
	}

	if (sLength >= usLen)
	{
		LOG(LOG_ERROR_LOW, "CSerializer::Deserialize() | sLength >= usLen %d", sLength);
		return;
	}

	//strncpy_s(pszBuffer, usLen, m_pCurMark, sLength);
	memcpy_s(pszBuffer, usLen, m_pCurMark, sLength);

	m_pCurMark += sLength;
	m_nCurBufSize += sLength;
}

// Serialize 
void CSerializer::Serialize(float fFloat)
{
	int nNum = *((int*)(&fFloat));
	*m_pCurMark++ = nNum;
	*m_pCurMark++ = nNum >> 8;
	*m_pCurMark++ = nNum >> 16;
	*m_pCurMark++ = nNum >> 24;
	m_nCurBufSize += sizeof(float);
}

void CSerializer::Serialize(const long long llLongLong)
{
	*m_pCurMark++ = llLongLong;
	*m_pCurMark++ = llLongLong >> 8;
	*m_pCurMark++ = llLongLong >> 16;
	*m_pCurMark++ = llLongLong >> 24;
	*m_pCurMark++ = llLongLong >> 32;
	*m_pCurMark++ = llLongLong >> 40;
	*m_pCurMark++ = llLongLong >> 48;
	*m_pCurMark++ = llLongLong >> 56;

	m_nCurBufSize += 8;
}

void CSerializer::Serialize(const unsigned long long ullLongLong)
{
	*m_pCurMark++ = ullLongLong;
	*m_pCurMark++ = ullLongLong >> 8;
	*m_pCurMark++ = ullLongLong >> 16;
	*m_pCurMark++ = ullLongLong >> 24;
	*m_pCurMark++ = ullLongLong >> 32;
	*m_pCurMark++ = ullLongLong >> 40;
	*m_pCurMark++ = ullLongLong >> 48;
	*m_pCurMark++ = ullLongLong >> 56;

	m_nCurBufSize += 8;
}

void CSerializer::Serialize(const int iInt)
{
	*m_pCurMark++ = iInt;
	*m_pCurMark++ = iInt >> 8;
	*m_pCurMark++ = iInt >> 16;
	*m_pCurMark++ = iInt >> 24;

	m_nCurBufSize += 4;
}

void CSerializer::Serialize(const unsigned int uiInt)
{
	*m_pCurMark++ = uiInt;
	*m_pCurMark++ = uiInt >> 8;
	*m_pCurMark++ = uiInt >> 16;
	*m_pCurMark++ = uiInt >> 24;

	m_nCurBufSize += 4;
}
	
void CSerializer::Serialize(const short sShort)
{
	*m_pCurMark++ = sShort;
	*m_pCurMark++ = sShort >> 8;
	m_nCurBufSize += 2;
}

void CSerializer::Serialize(const unsigned short usShort)
{
	*m_pCurMark++ = usShort;
	*m_pCurMark++ = usShort >> 8;
	m_nCurBufSize += 2;
}

void CSerializer::Serialize(const char cChar)
{
	*m_pCurMark++ = cChar;
	m_nCurBufSize += 1;
}

void CSerializer::Serialize(const unsigned char ucChar)
{
	*m_pCurMark++ = ucChar;
	m_nCurBufSize += 1;
}

void CSerializer::SetStream(const void* const pszBuffer, const short sLen) // ���ڿ����� byte stream�� ������
{
	CopyMemory( m_pCurMark , pszBuffer , sLen );
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

void CSerializer::Serialize(const char* const pszBuffer) // ���ڿ�
{
	unsigned short sLen = (unsigned short)strlen(pszBuffer);
	// 2018 0322�� ��ħ
	if (sLen < 0 || sLen > MAX_STRING_SIZE)
	{
		LOG(LOG_ERROR_LOW, "CSerializer::Serialize() | sLen < 0 || sLen > MAX_STRING_SIZE %d", sLen);
		return;
	}

	if (sLen > 512)
		sLen = 512;
	Serialize(sLen);
	
	memcpy(m_pCurMark, pszBuffer, sLen);
	//memcpy_s(m_pCurMark, , pszBuffer, sLen);
	m_pCurMark += sLen;
	m_nCurBufSize += sLen;
}

bool CSerializer::CopyBuffer(char* pDestBuffer) // Serialize�� �������� ���۸� ����
{
	if (m_nCurBufSize > m_nMaxBufSize)
	{
		LOG(LOG_ERROR_LOW, "SYSTEM | cVBuffer::CopyBuffer | m_nCurBufSize Overflow %d", m_nCurBufSize);
		m_nCurBufSize = m_nMaxBufSize;
	}

	CopyMemory( m_pszVBuffer, &m_nCurBufSize, PACKET_SIZE_LENGTH );
	CopyMemory( pDestBuffer, m_pszVBuffer, m_nCurBufSize );
	return true;
}

void CSerializer::StartDeserialize(char* pVBuffer) // ���ڵ��� ���ۺκ�
{
	m_pCurMark = pVBuffer + (PACKET_SIZE_LENGTH + PACKET_TYPE_LENGTH);
	m_nCurBufSize = 0;
}