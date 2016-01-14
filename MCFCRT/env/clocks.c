// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "clocks.h"
#include "mcfwin.h"
#include "bail.h"

uint64_t MCFCRT_GetUtcClock(){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc;
	GetSystemTimeAsFileTime(&unUtc.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (uint64_t)(unUtc.li.QuadPart - 0x019DB1DED53E8000ll) / 10000;
}
uint64_t MCFCRT_GetLocalClock(){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc, unLocal;
	GetSystemTimeAsFileTime(&unUtc.ft);
	FileTimeToLocalFileTime(&unUtc.ft, &unLocal.ft);
	return (uint64_t)(unLocal.li.QuadPart - 0x019DB1DED53E8000ll) / 10000;
}

uint64_t MCFCRT_GetFastMonoClock(){
	return GetTickCount64();
}
double MCFCRT_GetHiResMonoClock(){
	static volatile LARGE_INTEGER s_liFrequency;

	LARGE_INTEGER liFrequency;
	liFrequency.QuadPart = __atomic_load_n(&s_liFrequency.QuadPart, __ATOMIC_CONSUME);
	if(liFrequency.QuadPart == 0){
		if(!QueryPerformanceFrequency(&liFrequency)){
			MCFCRT_Bail(L"QueryPerformanceFrequency() 失败。");
		}
		__atomic_store_n(&s_liFrequency.QuadPart, liFrequency.QuadPart, __ATOMIC_RELEASE);
	}
	LARGE_INTEGER liCounter;
	if(!QueryPerformanceCounter(&liCounter)){
		MCFCRT_Bail(L"QueryPerformanceCounter() 失败。");
	}
	return (double)liCounter.QuadPart * 1000.0 / (double)liFrequency.QuadPart;
}
