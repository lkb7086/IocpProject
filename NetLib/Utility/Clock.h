#pragma once
#include <chrono>
#include <ctime>
#include <array>

typedef std::time_t				tick_t;
typedef std::wstring			wstr_t;
#define SIZE_128			128
#define SIZE_8				8
#define snwprintf(dst, format, ...)    _snwprintf_s(dst.data(), dst.size(), _TRUNCATE, format, __VA_ARGS__)

#define TICK_MIN                (60)
#define TICK_HOUR               (TICK_MIN * 60)
#define TICK_DAY                (TICK_HOUR * 24)

#define TICK_TO_MIN(x)          (x / TICK_MIN)
#define MIN_TO_TICK(x)          (x * TICK_MIN)

#define TICK_TO_HOUR(x)         (x / TICK_HOUR)        
#define HOUR_TO_TICK(x)         (x * TICK_HOUR)

#define TICK_TO_DAY(x)          (x / TICK_DAY)
#define DAY_TO_TICK(x)          (x * TICK_DAY)

typedef enum {
    DAY_SUNDAY      = 0,
    DAY_MONDAY      = 1,        //Áß±¹¾î·Î ¿ù¿äÀÏÀº àøÑ¢ìé
    DAY_TUESDAY     = 2,        //Áß±¹¾î·Î È­¿äÀÏÀº àøÑ¢ì£
    DAY_WEDNESDAY   = 3,        //...
    DAY_THURSDAY    = 4,
    DAY_FRIDAY      = 5,
    DAY_SATURDAY    = 6,
}DayOfTheWeek;

#define DATETIME_FORMAT         L"D%Y-%m-%dT%H:%M:%S"
#define DATE_FORMAT             L"%Y-%m-%d"
#define TIME_FORMAT             L"%H:%M:%S"
#define DB_TIME_FORMAT          L"%4d-%2d-%2d %2d:%2d:%2d"

using namespace std::chrono;
using namespace std;
typedef system_clock::time_point timePoint;

class NETLIB_API CClock final : public CSingleton
{
	DECLEAR_SINGLETON(CClock);

    tick_t	serverStartTick_;
    
    wstr_t	tickToStr(tick_t tick, WCHAR *fmt = DATETIME_FORMAT);

public:
	CClock();
	~CClock();

    tick_t	serverStartTick();
    tick_t	systemTick();
    tick_t	strToTick(wstr_t str, WCHAR *fmt = DB_TIME_FORMAT);
    
    wstr_t	nowTime(WCHAR *fmt = DATETIME_FORMAT);
    wstr_t	nowTimeWithMilliSec(WCHAR *fmt = DATETIME_FORMAT);

    wstr_t today();
    wstr_t tomorrow();
    wstr_t yesterday();
    
    DayOfTheWeek todayOfTheWeek();
};
CREATE_FUNCTION(CClock, Clock)

/*
printf("yesterday %ws\n", Clock()->yesterday().c_str());
printf("today %ws\n", Clock()->today().c_str());
printf("tomorrow %ws\n", Clock()->tomorrow().c_str());
printf("today is %d week of the day\n", Clock()->todayOfTheWeek());
printf("system = %llu\n", Clock()->systemTick());
printf("nowTime %ws\n", Clock()->nowTime().c_str());
printf("MilliSec %ws\n", Clock()->nowTimeWithMilliSec().c_str());
*/