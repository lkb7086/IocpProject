#include "Precompile.h"

void handle_invalid_parameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved);
void handle_pure_call();

IMPLEMENT_SINGLETON(CLog)

CLog::CLog()
{
	ZeroMemory( m_nLogInfoTypes , MAX_STORAGE_TYPE * sizeof( int ) );      
	ZeroMemory( m_szLogFileName ,  MAX_FILENAME_LENGTH );
	ZeroMemory( m_szIP ,  MAX_IP_LENGTH );
	ZeroMemory( m_szDSNNAME ,  MAX_DSN_NAME );
	ZeroMemory( m_szDSNID ,  MAX_DSN_ID );
	ZeroMemory( m_szDSNPW ,  MAX_DSN_PW );
    m_eLogFileType = FILETYPE_NONE;
	m_hWnd = NULL;
	m_hLogFile = INVALID_HANDLE_VALUE;
	m_sockUdp = INVALID_SOCKET;
	m_nMsgBufferIdx = 0;
	m_nUDPPort = DEFAULT_UDPPORT;
	m_nTCPPort = DEFAULT_TCPPORT;
	m_nServerType = 0;
	m_dwFileMaxSize = 0;
}

CLog::~CLog()
{	
	CloseAllLog();
}

void CLog::OnInitThread() {}

void CLog::OnCloseThread() {}

void CLog::OnProcess()
{
	/*
	CMonitor::Owner lock(g_csLog);

	size_t nLogCount = m_queueLogMsg.GetQueueSize();
	for (size_t i = 0; i < nLogCount; i++)
	{
		sLogMsg* pLogMsg = m_queueLogMsg.GetFrontQueue();
		if (nullptr == pLogMsg) return;
		m_queueLogMsg.PopQueue();
		// �α׸� ��´�
		LogOutput(pLogMsg->s_eLogInfoType, pLogMsg->s_szOutputString);
	}
	*/

	// FIXED
	//CMonitor::Owner lock(g_csLog);

	size_t nLogCount = m_queueLogMsg.GetQueueSize();
	for (register size_t i = 0; i < nLogCount; i++)
	{
		sLogMsg* pLogMsg = m_queueLogMsg.GetFrontQueue();
		if (nullptr == pLogMsg) return;
		//m_queueLogMsg.PopQueue();
		// �α׸� ��´�
		LogOutput(pLogMsg->s_eLogInfoType, pLogMsg->s_szOutputString);
	}

	CMonitor::Owner lock(g_csLog);
	for (register size_t i = 0; i < nLogCount; i++)
	{
		m_queueLogMsg.PopQueue();
	}
}

bool CLog::Init(const sLogConfig &LogConfig)
{
	// ���� �ʱ�ȭ���� ������ �Ǿ��־��ٸ� ��� ������ CLOSE ��Ų��
	CloseAllLog();
	TCHAR        strtime[ 100 ];
	// �������� ����
	CopyMemory( m_nLogInfoTypes, LogConfig.s_nLogInfoTypes, MAX_STORAGE_TYPE * sizeof( int ) );      
	
    time_t    curtime;
	struct tm loctime;

    curtime = time(NULL);
	localtime_s(&loctime, &curtime);
	_tcsftime(strtime, 100, _T("%m��%d��%H��%M��"), &loctime);
	//asctime_s(strtime, sizeof(strtime), &base_date_local);

	// LOG���丮����
	CreateDirectory( _T("./LOG"), NULL );
	_sntprintf_s(m_szLogFileName, _countof(m_szLogFileName), _TRUNCATE,
		_T("./Log/%s_%s.log"), LogConfig.s_szLogFileName, strtime);
	strncpy_s(m_szIP, _countof(m_szIP), LogConfig.s_szIP, _TRUNCATE);
	strncpy_s(m_szDSNNAME, _countof(m_szDSNNAME), LogConfig.s_szDSNNAME, _TRUNCATE);
	strncpy_s(m_szDSNID, _countof(m_szDSNID), LogConfig.s_szDSNID, _TRUNCATE);
	strncpy_s(m_szDSNPW, _countof(m_szDSNPW), LogConfig.s_szDSNPW, _TRUNCATE);
	m_eLogFileType = LogConfig.s_eLogFileType;
	m_nTCPPort = LogConfig.s_nTCPPort;
	m_nUDPPort = LogConfig.s_nUDPPort;
	m_nServerType = LogConfig.s_nServerType;
	m_dwFileMaxSize = LogConfig.s_dwFileMaxSize;
	m_hWnd = LogConfig.s_hWnd;
	
	// ���Ϸα׸� �����ߴٸ�
	if (LOG_NONE != m_nLogInfoTypes[STORAGE_FILE])
	{
		bool bRet = InitFile();
		if (false == bRet)
		{
			puts("error | CLog::Init | STORAGE_FILE");
			return false;
		}
	}

	// db�α׸� �����ߴٸ�
	if (LOG_NONE != m_nLogInfoTypes[STORAGE_DB])
	{
		bool bRet = InitDB();
		if (false == bRet)
		{
			puts("error | CLog::Init | STORAGE_DB");
			return false;
		}
	}

	// udp�α׸� �����ߴٸ� 
	if (LOG_NONE != m_nLogInfoTypes[STORAGE_UDP])
	{
		bool bRet = InitUDP();
		if (false == bRet)
		{
			puts("error | CLog::Init | STORAGE_UDP");
			return false;
		}
	}

	// tcp�α׸� �����ߴٸ�
	if (LOG_NONE != m_nLogInfoTypes[STORAGE_TCP])
	{
		bool bRet = InitTCP();
		if (false == bRet)
		{
			puts("error | CLog::Init | STORAGE_TCP");
			return false;
		}
	}

	CreateThread( LogConfig.s_dwProcessTick );
	Run();

	return true;
}

void CLog::CloseAllLog()
{
	ZeroMemory( m_nLogInfoTypes , MAX_STORAGE_TYPE * sizeof( int ) );      
	ZeroMemory( m_szLogFileName ,  MAX_FILENAME_LENGTH );
	ZeroMemory( m_szIP ,  MAX_IP_LENGTH );
	ZeroMemory( m_szDSNNAME ,  MAX_DSN_NAME );
	ZeroMemory( m_szDSNID ,  MAX_DSN_ID );
	ZeroMemory( m_szDSNPW ,  MAX_DSN_PW );
	m_nUDPPort = DEFAULT_UDPPORT;
	m_nTCPPort = DEFAULT_TCPPORT;
	m_eLogFileType = FILETYPE_NONE;
	m_hWnd = NULL;
	m_nMsgBufferIdx = 0;
	
	// ȭ�� �α׸� ������
	if (INVALID_HANDLE_VALUE != m_hLogFile)
	{
		CloseHandle( m_hLogFile );
		m_hLogFile = INVALID_HANDLE_VALUE;
	}
	// udp������ �ʱ�ȭ ��Ų��
	if( INVALID_SOCKET != m_sockUdp )
	{
		closesocket( m_sockUdp );
		m_sockUdp = INVALID_SOCKET;
	}
	// TCP������ �ʱ�ȭ ��Ų��
	if( INVALID_SOCKET != m_sockTcp )
	{
		shutdown( m_sockTcp, SD_BOTH );
		closesocket( m_sockTcp );
		m_sockTcp = INVALID_SOCKET;
	}
	// ������ ����
	Stop();

	//puts("�α׸� �ٽ� �������ּ���.");
}

bool CLog::InitDB()
{
	/*
	DB ����
	*/
	return true;
}

bool CLog::InitFile()
{
	m_hLogFile = CreateFile(	m_szLogFileName ,
								GENERIC_WRITE,                
								FILE_SHARE_READ ,            
								NULL,                         
								OPEN_ALWAYS,               
								FILE_ATTRIBUTE_NORMAL,     
								NULL);     

	if (INVALID_HANDLE_VALUE == m_hLogFile)
		return false;
	return true;
}

bool CLog::InitUDP()
{
	WSADATA	WsaData;
	int nRet = WSAStartup(MAKEWORD(2,2), &WsaData);
	if(0 != nRet)
		return false;

	if (INVALID_SOCKET != m_sockUdp)
		return false;
	
	m_sockUdp = socket(PF_INET, SOCK_DGRAM, 0);

	return true;
}

bool CLog::InitTCP()
{
	WSADATA	WsaData;
	int nRet = WSAStartup(MAKEWORD(2,2), &WsaData);
	if (0 != nRet)
		return false;

	if( INVALID_SOCKET != m_sockTcp )
		return false;

	m_sockTcp = socket(PF_INET, SOCK_STREAM, 0);

	sockaddr_in addr;
	memset((char *)&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_szIP);
	addr.sin_port = htons(m_nTCPPort);
	
	nRet = connect(m_sockTcp, (sockaddr*)&addr, sizeof(sockaddr));
	if( SOCKET_ERROR == nRet )
		return false;
	return true;
}

void CLog::LogOutput(enumLogInfoType eLogInfo, char *szOutputString )
{
	///////////////////////////////////////////////////////////////////////
	//��� �������ݷ� �ٸ� ���α׷����� �α׸� ��������
	//���� Ÿ�԰� �޼����� �������Ѵ�
	/*
	if( m_nLogInfoTypes[ STORAGE_UDP ] & eLogInfo )
	{
		OutputUDP( eLogInfo , szOutputString );
	}
	if( m_nLogInfoTypes[ STORAGE_TCP ] & eLogInfo )
	{
		OutputTCP( eLogInfo , szOutputString );
	}
	*/

	// �α�, �ð� : �������� : �������� : Ŭ���� : �Լ� : ��������
	char        szTime[ 25 ];
	time_t      curTime;
	struct tm locTime;

	// LOG ENUM�� StringTable���� ������ ��ġ ��Ų��
	int nIdx = ( int )eLogInfo;
	if( 0 != ( eLogInfo >> 8 ) )
		nIdx = ( eLogInfo >> 8 ) + 0x20 - 3;
	else if( 0 != ( eLogInfo >> 4 ) )
		nIdx = ( eLogInfo >> 4 ) + 0x10 - 1;
	if( nIdx < 0 || nIdx > 31 )
		return;

	curTime = time( NULL );
	localtime_s(&locTime, &curTime);
	strftime(szTime, 25, "%Y/%m/%d(%H:%M:%S)", &locTime);
	//asctime_s(szTime, sizeof(szTime), &base_date_local);

	_snprintf_s(m_szOutStr, _countof(m_szOutStr), _TRUNCATE, "%s | %s | %s | %s%c%c"
		, szTime
		, (eLogInfo >> 4) ? "Error" : "Info"
		, szLogInfoType_StringTable[nIdx]
		, szOutputString, 0x0d, 0x0a);

    if( m_nLogInfoTypes[ STORAGE_FILE ] & eLogInfo )
	{
		OutputFile( m_szOutStr );
	}
	if( m_nLogInfoTypes[ STORAGE_DB ] & eLogInfo )
	{
		OutputDB( m_szOutStr ); // ������
	}
	if( m_nLogInfoTypes[ STORAGE_WINDOW ] & eLogInfo )
	{
		OutputWindow( eLogInfo , m_szOutStr );
	}
	if( m_nLogInfoTypes[ STORAGE_OUTPUTWND ] & eLogInfo )
	{
		OutputDebugWnd( m_szOutStr ); // ������
	}
}

void CLog::OutputFile(char* szOutputString)
{
	if (INVALID_HANDLE_VALUE == m_hLogFile)
		return;

	DWORD dwWrittenBytes = 0;
	DWORD dwSize = 0;
	dwSize = GetFileSize( m_hLogFile , NULL );
	// ȭ�� �뷮�� ���ѿ� �ɷȴٸ�
	//if( dwSize > m_dwFileMaxSize || dwSize > MAX_LOGFILE_SIZE )
	if (dwSize > m_dwFileMaxSize)
	{
		TCHAR        strtime[ 100 ];
		time_t      curtime = 0;
		struct tm locTime;

		curtime = time( NULL);
		localtime_s(&locTime, &curtime);
		_tcsftime(strtime, 100, _T("%m��%d��%H��%M��"), &locTime);
		//asctime_s(strtime, sizeof(strtime), &base_date_local);

		m_szLogFileName[_tcsclen(m_szLogFileName) - 21] = NULL;
		_sntprintf_s(m_szLogFileName, _countof(m_szLogFileName), _TRUNCATE, _T("%s_%s.log"), m_szLogFileName, strtime);
		CloseHandle( m_hLogFile );
		m_hLogFile = INVALID_HANDLE_VALUE;
		InitFile();
	}
	// ȭ���� ������ ���� �����͸� �ű��
	SetFilePointer( m_hLogFile , 0 , 0 , FILE_END );
	BOOL bRet = WriteFile( m_hLogFile , szOutputString ,
		(DWORD)strlen( szOutputString ) , &dwWrittenBytes , NULL );
}

void CLog::OutputDB(char* szOutputString)
{
	// DB�� �α׸� ����� �ڵ�
}

void CLog::OutputWindow(enumLogInfoType eLogInfo, char* szOutputString )
{
	printf("%s", szOutputString);
	/*
	if( NULL == m_hWnd )
		return;
	SendMessage( m_hWnd , WM_DEBUGMSG , ( WPARAM )szOutputString 
		, ( LPARAM )eLogInfo );
	*/
}

void CLog::OutputDebugWnd(char* szOutputString)
{
	// �����â�� ������ش�
	//OutputDebugString( szOutputString );
}

void CLog::OutputUDP(enumLogInfoType eLogInfo, char* szOutputString)
{
	/*
	if (INVALID_SOCKET == m_sockUdp)
		return;

	// udp packet�� ������ ����
	sockaddr_in Addr;
	memset( (char *)&Addr, 0x00, sizeof(Addr) );
	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = inet_addr( m_szIP ); 
	Addr.sin_port = htons( m_nUDPPort );
	// ���� ��Ŷ����
	int nBufLen = (int)strlen( szOutputString );
	
	int retval = sendto(m_sockUdp, (char*)szOutputString, nBufLen,
		0 , (const struct sockaddr *)&Addr, sizeof(Addr));
	if (SOCKET_ERROR == retval)
		return;
	*/
}

void CLog::OutputTCP(enumLogInfoType eLogInfo, char* szOutputString)
{
	/*
	if (INVALID_SOCKET == m_sockTcp)
		return;

	int nLen = (int)strlen( szOutputString );
	int retval = send(m_sockTcp, (char*)szOutputString, nLen, 0);
	if (SOCKET_ERROR == retval)
		return;
	*/
}

void CLog::LogOutputLastErrorToMsgBox(TCHAR *szOutputString)
{
	/*
	int nLastError = GetLastError();
	if( nLastError == 0 ) 
		return;
	LPVOID pDump;
	DWORD  result;
	result = FormatMessage(	
							FORMAT_MESSAGE_ALLOCATE_BUFFER |
							FORMAT_MESSAGE_FROM_SYSTEM,
							NULL,
							nLastError,
							MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
							(LPTSTR)&pDump,
							0,
							NULL
							);

	_sntprintf_s(g_szOutStr, _countof(g_szOutStr), _TRUNCATE, _T("������ġ : %s \n������ȣ : %d\n���� : %s"),
		szOutputString , nLastError , pDump );
	MessageBox(NULL, g_szOutStr, _T("GetLastError"), MB_OK);
	
	if(result) 
		LocalFree(pDump);
	*/
}

// �ʱ�ȭ �Լ�
bool INIT_LOG( sLogConfig &LogConfig )
{
	_wsetlocale(LC_ALL, L"korean");

	//CLFH lfh;

	// �����߰�
	_set_invalid_parameter_handler(handle_invalid_parameter);
	_set_purecall_handler(handle_pure_call);

	// �̴ϴ���
	CMinidumpHelp dump;
	dump.install_self_mini_dump();

	// �޸� ����
#ifdef _DEBUG
	CMemoryLeak leak;
#endif

	return Log()->Init( LogConfig );
}

// �α׸� ����� �Լ�
void LOG( enumLogInfoType eLogInfoType , char *szOutputString , ... )
{
	CMonitor::Owner lock( g_csLog );

	size_t nQueueCnt = Log()->GetQueueSize();
	// ���� ť Size�� �ʰ��Ͽ��ٸ� 
	if (MAX_LOG_QUEUE_CNT <= nQueueCnt)
		return;
	
	va_list	argptr; 
	va_start( argptr, szOutputString );
	vsprintf_s( g_stLogMsg[ nQueueCnt ].s_szOutputString ,
		szOutputString, argptr );
	va_end( argptr );

	g_stLogMsg[ nQueueCnt ].s_eLogInfoType = eLogInfoType;
	
	Log()->InsertMsgToQueue( &g_stLogMsg[ nQueueCnt ] );
}

// ��Ʈ������ �޼��� �ڽ��� ��� �Լ�
void LOG_LASTERROR( TCHAR *szOutputString , ... )
{
	va_list	argptr; 
    va_start( argptr, szOutputString );
	_vstprintf_s(g_szOutStr, szOutputString, argptr);
	va_end( argptr );

	Log()->LogOutputLastErrorToMsgBox( g_szOutStr );
}
// �α׸� ������
void CLOSE_LOG()
{
	Log()->CloseAllLog();
}

void OnCrush()
{
	int* p = nullptr;
	*p = 1;
}



void handle_invalid_parameter(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved)
{
	RaiseException(0, 0, 0, nullptr);
}

void handle_pure_call()
{
	RaiseException(0, 0, 0, nullptr);
}