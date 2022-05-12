#pragma once
#define 	MAX_STORAGE_TYPE		6
#define		MAX_FILENAME_LENGTH		128
#define		MAX_DSN_NAME			128
#define		MAX_DSN_ID				20
#define		MAX_DSN_PW				20
//#define		MAX_STRING_LENGTH		1024	
#define		MAX_OUTPUT_LENGTH		1024 * 4
#define		MAX_LOG_QUEUE_CNT		MAX_QUEUESIZE
//#define		WM_DEBUGMSG				WM_USER + 1
#define		DEFAULT_UDPPORT			1555
#define		DEFAULT_TCPPORT			1556
#define		DEFAULT_TICK			1000
//#define		MAX_LOGFILE_SIZE		1024 * 200000   // 200MB	

static char szLogInfoType_StringTable[][ 100 ] = 
{
		"LOG_NONE",
		"LOG_INFO_LOW",													//0x00000001
		"LOG_INFO_NORMAL",												//0x00000002
		"LOG_INFO_LOW , LOG_INFO_NORMAL",								//0x00000003	
		"LOG_INFO_HIGH",												//0x00000004	
		"LOG_INFO_LOW , LOG_INFO_HIGH",									//0x00000005
		"LOG_INFO_NORMAL , LOG_INFO_HIGH",								//0x00000006
		"LOG_INFO_LOW , LOG_INFO_NORMAL , LOG_INFO_HIGH",				//0x00000007
		"LOG_INFO_CRITICAL",											//0x00000008
		"LOG_INFO_LOW , LOG_INFO_CRITICAL",								//0x00000009
		"LOG_INFO_NORMAL , LOG_INFO_CRITICAL",							//0x0000000A
		"LOG_INFO_LOW , LOG_INFO_NORMAL , LOG_INFO_CRITICAL",			//0x0000000B
		"LOG_INFO_HIGH , LOG_INFO_CRITICAL",							//0x0000000C
		"LOG_INFO_LOW , LOG_INFO_HIGH , LOG_INFO_CRITICAL",				//0x0000000D
		"LOG_INFO_NORMAL , LOG_INFO_HIGH , LOG_INFO_CRITICAL",			//0x0000000E
		"LOG_INFO_ALL",													//0x0000000F
	
		"LOG_ERROR_LOW",												//0x00000010
		"LOG_ERROR_NORMAL",												//0x00000020
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL",								//0x00000030	
		"LOG_ERROR_HIGH",												//0x00000040	
		"LOG_ERROR_LOW , LOG_ERROR_HIGH",								//0x00000050
		"LOG_ERROR_NORMAL , LOG_ERROR_HIGH",							//0x00000060
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL , LOG_ERROR_HIGH",			//0x00000070
		"LOG_ERROR_CRITICAL",											//0x00000080
		"LOG_ERROR_LOW , LOG_ERROR_CRITICAL",							//0x00000090
		"LOG_ERROR_NORMAL , LOG_ERROR_CRITICAL",						//0x000000A0
		"LOG_ERROR_LOW , LOG_ERROR_NORMAL , LOG_ERROR_CRITICAL",		//0x000000B0
		"LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",							//0x000000C0
		"LOG_ERROR_LOW , LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",			//0x000000D0
		"LOG_ERROR_NORMAL , LOG_ERROR_HIGH , LOG_ERROR_CRITICAL",		//0x000000F0
		"LOG_ERROR_ALL",												//0x00000100
		"LOG_ALL"														//0x00000200	
};

enum enumLogInfoType
{
	  LOG_NONE				= 0x00000000
	, LOG_INFO_LOW 			= 0x00000001 
	, LOG_INFO_NORMAL 		= 0x00000002
	, LOG_INFO_HIGH 		= 0x00000004
	, LOG_INFO_CRITICAL 	= 0x00000008
	, LOG_INFO_ALL			= 0x0000000F // 00001111
	, LOG_ERROR_LOW 		= 0x00000010
	, LOG_ERROR_NORMAL		= 0x00000020
	, LOG_ERROR_HIGH 		= 0x00000040
	, LOG_ERROR_CRITICAL	= 0x00000080
	, LOG_ERROR_ALL = 0x000000F0     // 11110000
	, LOG_ALL = 0x000000FF     // 11111111
};
     
enum enumLogStorageType 
{
	  STORAGE_FILE		= 0x000000000
	, STORAGE_DB 		= 0x000000001
	, STORAGE_WINDOW 	= 0x000000002
 	, STORAGE_OUTPUTWND	= 0x000000003
	, STORAGE_UDP		= 0x000000004
	, STORAGE_TCP		= 0x000000005
	
};
 
enum enumLogFileType 	
{
	  FILETYPE_NONE		= 0x00
	, FILETYPE_XML		= 0x01
	, FILETYPE_TEXT		= 0x02
	, FILETYPE_ALL		= 0x03
};

// �޽��� ����ü
struct sLogMsg
{
	enumLogInfoType		s_eLogInfoType;
	char				s_szOutputString[ MAX_OUTPUT_LENGTH ];
};

//- �ʱ�ȭ ����ü ����
struct sLogConfig
{ 
	////////////////////////////////////////////////////////////////////////////
	//�迭����(����[0],���[1],������[2],�����â[3],udp[4])
	//���迭�� ����ϰ���� LogInfo������ or�����Ͽ� �ִ´�.
	//��)���Ͽ� LOG_INFO_NORMAL, �����쿡 LOG_ALL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_INFO_NORMAL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_ALL
	int 				s_nLogInfoTypes[ MAX_STORAGE_TYPE ];      
	TCHAR				s_szLogFileName[ MAX_FILENAME_LENGTH ];
	// �α� ������ ������ �����Ѵ�. XML or TEXT �Ѵٵ� �����ϴ�
	enumLogFileType		s_eLogFileType;
	// TCP/UDP�� �α׸� ���� IP , PORT , UDP�α׸� ������� �ʴ´ٸ� ���� ��� �ȴ�
	char				s_szIP[	MAX_IP_LENGTH ];
	unsigned short					s_nUDPPort;
	unsigned short					s_nTCPPort;
	// ���� Ÿ�� , �α׼����� ��ϵ� ����Ÿ���� ����
	int					s_nServerType;	

	// DB�� �α׸� ���� DSN����
	char				s_szDSNNAME[ MAX_DSN_NAME ];
	char				s_szDSNID[ MAX_DSN_ID ];
	char				s_szDSNPW[ MAX_DSN_PW ];
	// Window�� �α׸� ���� ������ �ڵ�
	HWND				s_hWnd;
	// Log ó�� �ð� �⺻���� 1�ʸ��� ó��
	DWORD				s_dwProcessTick;
	// Log���� ����� m_dwFileMaxSize���� ũ�� ���ο� ������ �����. 
	DWORD				s_dwFileMaxSize;
	
	sLogConfig() { 
		ZeroMemory( this , sizeof( sLogConfig ) ); 
		s_dwProcessTick = DEFAULT_TICK;
		s_nUDPPort = DEFAULT_UDPPORT;
		s_nTCPPort = DEFAULT_TCPPORT;
		s_dwFileMaxSize = 1024 * 50000;		// 50MB �⺻���� ���� �ִ� 100MB�� ���� ����
	}
};

class NETLIB_API CLog final : public CThread, public CSingleton
{
	DECLEAR_SINGLETON(CLog);
public:

	CLog();
	~CLog();

	//////////////////////////////////////////////////////////////////////////////////////
	// �������̽� �Լ�
	bool Init(const sLogConfig &LogConfig);
	void LogOutput( enumLogInfoType eLogInfo , char *OutputString );
	void LogOutputLastErrorToMsgBox( TCHAR *szOutputString );
	// ��� �α׸� ������
	void CloseAllLog();

	// ������ ó�� �Լ�
	virtual void OnInitThread() override;
	virtual void OnCloseThread() override;
	virtual void OnProcess() override;
	void SetHWND( HWND hWnd = NULL ) { m_hWnd = hWnd; }

	/////////////////////////////////////////////////////////////////////////
	// ť�� ���õ� �Լ�
	// ���� queue ũ��
	size_t	GetQueueSize() { return m_queueLogMsg.GetQueueSize(); }
	// ť�� �޼����� �߰��Ѵ�
	inline void InsertMsgToQueue(sLogMsg* pLogMsg)
	{
		m_queueLogMsg.PushQueue(pLogMsg);
	}

	void Destroy_LogThread()
	{
		if (NULL != m_hThread)
		{
			DestroyThread();
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}
		
private:
	////////////////////////////////////////////////////////////////////////////
	//�迭����(����[0],���[1],������[2],�����â[3],udp[4])
	//���迭�� ����ϰ���� LogInfo������ or�����Ͽ� �ִ´�
	//��)���Ͽ� LOG_INFO_NORMAL, �����쿡 LOG_ALL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_INFO_NORMAL
	//s_eLogInfoType[ STORAGE_FILE ] = LOG_ALL
	int				 	m_nLogInfoTypes[ MAX_STORAGE_TYPE ];      
	TCHAR				m_szLogFileName[ MAX_FILENAME_LENGTH ];
	// �α� ������ ������ �����Ѵ�. XML or TEXT �Ѵٵ� �����ϴ�
	enumLogFileType		m_eLogFileType;
	// TCP/UDP�� �α׸� ���� IP , PORT , UDP�α׸� ������� �ʴ´ٸ� ���� ��� �ȴ�
	char				m_szIP[	MAX_IP_LENGTH ];
	unsigned short					m_nUDPPort;
	unsigned short					m_nTCPPort;
	// ���� Ÿ�� , �α׼����� ��ϵ� ����Ÿ���� ����
	int					m_nServerType;	
	
	// DB�� �α׸� ���� DSN����
	char				m_szDSNNAME[ MAX_DSN_NAME ];
	char				m_szDSNID[ MAX_DSN_ID ];
	char				m_szDSNPW[ MAX_DSN_PW ];
	// �α� ���� ����
	char				m_szOutStr[ MAX_OUTPUT_LENGTH ];
	
	////////////////////////////////////////////////////////////////////////////
	// output ��ü�� �ڵ� ����
	// Window�� �α׸� ���� ������ �ڵ�
	HWND				m_hWnd;
	// File handle����
	HANDLE				m_hLogFile;
	// UDP ����
	SOCKET				m_sockUdp;
	SOCKET				m_sockTcp;

	// �޼��� ť
	CQueue< sLogMsg* >	m_queueLogMsg;
	// ���� �޼��� ���� ��ġ
	int					m_nMsgBufferIdx;
	DWORD				m_dwFileMaxSize;
	
	/////////////////////////////////////////////////////////////////////////////
	// ���� ȣ�� �Լ�
	// ��°��� �Լ�	
	void OutputFile( char* szOutputString );
	void OutputDB( char* szOutputString );
	void OutputWindow(  enumLogInfoType eLogInfo , char* szOutputString );
	void OutputDebugWnd( char* szOutputString );
	void OutputUDP( enumLogInfoType eLogInfo , char* szOutputString );
	void OutputTCP( enumLogInfoType eLogInfo , char* szOutputString );
	/////////////////////////////////////////////////////////////////////////////
	
	//�ʱ�ȭ �Լ���
	bool InitDB();
	bool InitFile();
	bool InitUDP();
	bool InitTCP();
};
CREATE_FUNCTION(CLog, Log)

// �۷ι� ����
static TCHAR g_szOutStr[ MAX_OUTPUT_LENGTH ];
static sLogMsg g_stLogMsg[MAX_LOG_QUEUE_CNT];
static CMonitor g_csLog;
//////////////////////////////////////////////////////////////////////////////////////////
// ���� ����ڰ� ���� �Լ� 
// �ʱ�ȭ �Լ�
bool NETLIB_API INIT_LOG( sLogConfig &LogConfig );
// �α׸� ����� �Լ� 
// �ΰ� �޼��� ���� ��� : ( ������ ������ �߻��� �Լ� | ������ ���������ѳ��� )
// EX) cIocpServer::BindIocp() | bind() , Failed ..
void NETLIB_API LOG( enumLogInfoType eLogInfoType , char *szOutputString , ... );
// ��Ʈ������ �޼��� �ڽ��� ��� �Լ�
void NETLIB_API LOG_LASTERROR( char *szOutputString , ... );
// �α׸� ������
void NETLIB_API CLOSE_LOG();

void NETLIB_API OnCrush();