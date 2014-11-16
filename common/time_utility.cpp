#include "time_utility.h"
#include <ctime>

TimeCounter::TimeCounter()
{
	mStart = clock();
	mStop = 0;
}

int TimeCounter::Reset()
{
	mStart = clock();
	return 0;
}

int TimeCounter::Stop()
{
	mStop = clock();
	return 0;
}

time_t TimeCounter::GetLength()
{
	if (mStop == 0)
		mStop = clock();
	return mStop - mStart;
}

