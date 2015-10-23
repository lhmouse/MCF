// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "clocks.h"
#include "mcfwin.h"
#include "bail.h"

uint64_t MCF_GetUtcClock(){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc;
	GetSystemTimeAsFileTime(&unUtc.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (uint64_t)(unUtc.li.QuadPart - 0x019DB1DED53E8000ll) / 10000;
}
uint64_t MCF_GetLocalClock(){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc, unLocal;
	GetSystemTimeAsFileTime(&unUtc.ft);
	FileTimeToLocalFileTime(&unUtc.ft, &unLocal.ft);
	return (uint64_t)(unLocal.li.QuadPart - 0x019DB1DED53E8000ll) / 10000;
}

uint64_t MCF_GetFastMonoClock(){
	return GetTickCount64();
}
double MCF_GetHiResMonoClock(){
	static int64_t s_n64Frequency = 0;

	int64_t n64Frequency = __atomic_load_n(&s_n64Frequency, __ATOMIC_CONSUME);
	if(n64Frequency == 0){
		LARGE_INTEGER liFrequency;
		if(!QueryPerformanceFrequency(&liFrequency)){
			MCF_CRT_Bail(L"QueryPerformanceFrequency() 失败。");
		}
		n64Frequency = liFrequency.QuadPart;
		__atomic_store_n(&s_n64Frequency, n64Frequency, __ATOMIC_RELEASE);
	}
	LARGE_INTEGER liCounter;
	if(!QueryPerformanceCounter(&liCounter)){
		MCF_CRT_Bail(L"QueryPerformanceCounter() 失败。");
	}
	return liCounter.QuadPart * 1000.0 / n64Frequency;
}
