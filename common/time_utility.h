#ifndef TIME_UTILITY
#define TIME_UTILITY

#include "common_basic.h"

class TimeCounter
{
private:
	time_t mStart;
	time_t mStop;
public:
	TimeCounter();
	int Reset();
	int Stop();
	time_t GetLength();
};

#endif