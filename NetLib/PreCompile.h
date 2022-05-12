#pragma once

#pragma warning(disable : 4251) // DLL �������̽��� ����ϵ��� �����ؾ� �մϴ�.  // dll������ ���ο����� ���ø���ü�� ����ؾ� �Ѵ�.
//#pragma warning(disable : 4244) // '=' : 'int'���� 'char'(��)�� ��ȯ�ϸ鼭 �����Ͱ� �սǵ� �� �ֽ��ϴ�.  // Serializer.cpp������ �����
//#pragma warning(disable : 4293) // '<<' : ����Ʈ Ƚ���� �����̰ų� �ʹ� Ů�ϴ�. ���ǵ��� ���� �����Դϴ�.  // Serializer.cpp������ �����
#pragma warning(disable : 4505) // �������� ���� ���� �Լ��� �����߽��ϴ�.
#pragma warning(disable : 4100) // �������� ���� ���� �Ű� �����Դϴ�.
#pragma warning(disable : 4189) // ���� ������ �ʱ�ȭ�Ǿ����� �������� �ʾҽ��ϴ�.

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <tchar.h>
#include <iostream>
#include <winsock2.h>
#include <Mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <ws2spi.h>
#include <winbase.h>
#include <process.h>
#include <fstream>
#include <math.h>
#include <mutex>
#include <atomic>

#include <String>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

//#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>

using namespace std;
using namespace concurrency;

#include "CommonDef.h"
#include "Vector.h"

#include "./Container/Queue.h"
#include "./Container/List.h"
#include "./Container/MPMCQueue.h"
#include "./Container/MPMCStack.h"
#include "./Container/MPSCQueue.h"
#include "./Container/ObjectPool.h"

#include "Monitor.h"
#include "Singleton.h"
#include "Thread.h"
#include "Log.h"
#include "Connection.h"
#include "IocpServer.h"
#include "Serializer.h"

#include "./Utility/MinidumpHelp.h"
#include "./Utility/MemoryLeak.h"
#include "./Utility/Monitoring.h"
#include "./Utility/LFH.h"
#include "./Utility/Clock.h"
#include "./Utility/SHA512.h"
//#include "./Utility/DllThreadLocal.h"