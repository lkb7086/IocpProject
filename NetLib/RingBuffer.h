#pragma once

// 190710���� ������
class NETLIB_API CRingBuffer final
{
public:
	CRingBuffer();
	~CRingBuffer();
	CRingBuffer(const CRingBuffer& rhs) = delete;
	CRingBuffer& operator=(const CRingBuffer& rhs) = delete;

	// ������ �޸� �Ҵ�
	bool			Create( int nBufferSize = MAX_RINGBUFSIZE );
	// �ʱ�ȭ
	bool			Initialize();
	// �Ҵ�� ���� ũ�⸦ ��ȯ�Ѵ�.
	inline int		GetBufferSize() const { return m_nBufferSize; }
	
	// �ش��ϴ� ������ �����͸� ��ȯ
	inline char*	GetBeginMark() const { return m_pBeginMark; }
	inline char*	GetCurrentMark() const { return m_pCurrentMark; }
	inline char*	GetEndMark() const { return m_pEndMark; }
	
	// �����͸� ���� ��
	char*			ForwardMark( int nForwardLength );
	// �����͸� ���� ��
	char*			ForwardMark(__int64 nForwardLength , int nNextLength , DWORD dwRemainLength );
	void			BackwardMark( int nBackwardLength );

	// ���� ���� ����
	void			ReleaseBuffer( int nReleaseSize );
	void			ReleaseRecvBuffer(int nReleaseSize);
	// ���� ���� ũ�� ��ȯ
	inline int		GetUsedBufferSize() const { return m_nUsedBufferSize; }
	//���� ���۾� ����(�̰��� �ϴ� ������ SendPost()�Լ��� ��Ƽ �����忡�� ���ư��⶧����
	//PrepareSendPacket()����(ForwardMark()����) ���� ���� �÷������� PrepareSendPacket�Ѵ����� �����͸�
	//ä�� �ֱ����� �ٷ� �ٸ� �����忡�� SendPost()�� �Ҹ��ٸ� ������ ������ �����Ͱ� �� �� �ִ�
	//�װ� �����ϱ� ���� �����͸� �� ä�� ���¿����� ���� ���� ����� ������ �� �־���Ѵ�
	//�� �Լ��� SendPost()�Լ����� �Ҹ��� �ȴ�
	bool			SetUsedBufferSize( int nUsedBufferSize );
	void			SetUsedRecvBufferSize(int nUsedBufferSize);

	// ���� ���� ���� ��ȯ
	inline int		GetAllUsedBufferSize() const { return m_uiAllUserBufSize; }
	
	// ���� ������ �о ��ȯ
	char*			GetBuffer( int nReadSize , int* pReadSize );
private:
	
	char*			m_pRingBuffer;		   // ���� �����͸� �����ϴ� ���� ������

	char*			m_pBeginMark;			// ������ ó���κ��� ����Ű�� �ִ� ������
	char*			m_pEndMark;				// ������ �������κ��� ����Ű�� �ִ� ������
	char*			m_pCurrentMark;			// ������ ���� �κ��� ����Ű�� �ִ� ������
	char*			m_pGettedBufferMark;	// ������� �����͸� ���� ������ ������
	char*			m_pLastMoveMark;		// ForwardMark�Ǳ� ���� ������ ������

	int				m_nBufferSize;		// ������ �� ũ��
	int 			m_nUsedBufferSize;	// ���� ���� ������ ũ��
	unsigned int	m_uiAllUserBufSize; // �� ó���� �����ͷ�          ��ū �ڷ������� �ؾ���
	CMonitor		m_csRingBuffer;
};

