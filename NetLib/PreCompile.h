#pragma once

#pragma warning(disable : 4251) // DLL 인터페이스를 사용하도록 지정해야 합니다.  // dll에서는 내부에서만 템플릿객체를 사용해야 한다.
//#pragma warning(disable : 4244) // '=' : 'int'에서 'char'(으)로 변환하면서 데이터가 손실될 수 있습니다.  // Serializer.cpp에서만 사용중
//#pragma warning(disable : 4293) // '<<' : 시프트 횟수가 음수이거나 너무 큽니다. 정의되지 않은 동작입니다.  // Serializer.cpp에서만 사용중
#pragma warning(disable : 4505) // 참조되지 않은 지역 함수를 제거했습니다.
#pragma warning(disable : 4100) // 참조되지 않은 정식 매개 변수입니다.
#pragma warning(disable : 4189) // 지역 변수가 초기화되었으나 참조되지 않았습니다.

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