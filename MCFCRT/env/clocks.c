// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "clocks.h"
#include "mcfwin.h"
#include "bail.h"

static uint64_t GetTimeZoneOffsetInMillisecondsOnce(){
	static uint64_t *volatile s_pu64Inited;
	static uint64_t           s_u64Value;

	uint64_t *pInited = __atomic_load_n(&s_pu64Inited, __ATOMIC_CONSUME);
	if(!pInited){
		pInited = &s_u64Value;

		DYNAMIC_TIME_ZONE_INFORMATION vInfo;
		if(GetDynamicTimeZoneInformation(&vInfo) == TIME_ZONE_ID_INVALID){
			_MCFCRT_Bail(L"GetDynamicTimeZoneInformation() 失败。");
		}
		*pInited = (uint64_t)(vInfo.Bias * -60000ll);

		__atomic_store_n(&s_pu64Inited, pInited, __ATOMIC_RELEASE);
	}
	return *pInited;
}
static double QueryPerformanceFrequencyReciprocalOnce(){
	static double *volatile s_plfInited;
	static double           s_ulfValue;

	double *pInited = __atomic_load_n(&s_plfInited, __ATOMIC_CONSUME);
	if(!pInited){
		pInited = &s_ulfValue;

		LARGE_INTEGER liFreq;
		if(!QueryPerformanceFrequency(&liFreq)){
			_MCFCRT_Bail(L"QueryPerformanceFrequency() 失败。");
		}
		*pInited = 1000.0 / (double)liFreq.QuadPart;

		__atomic_store_n(&s_plfInited, pInited, __ATOMIC_RELEASE);
	}
	return *pInited;
}

uint64_t _MCFCRT_GetUtcClock(){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc;
	GetSystemTimeAsFileTime(&unUtc.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (uint64_t)(unUtc.li.QuadPart - 0x019DB1DED53E8000ll) / 10000;;
}
uint64_t _MCFCRT_GetLocalClock(){
	return _MCFCRT_GetLocalClockFromUtc(_MCFCRT_GetUtcClock());
}

uint64_t _MCFCRT_GetUtcClockFromLocal(uint64_t u64LocalClock){
	return u64LocalClock - GetTimeZoneOffsetInMillisecondsOnce();
}
uint64_t _MCFCRT_GetLocalClockFromUtc(uint64_t u64UtcClock){
	return u64UtcClock + GetTimeZoneOffsetInMillisecondsOnce();
}

uint64_t _MCFCRT_GetFastMonoClock(){
	return GetTickCount64();
}
double _MCFCRT_GetHiResMonoClock(){
	LARGE_INTEGER liCounter;
	if(!QueryPerformanceCounter(&liCounter)){
		_MCFCRT_Bail(L"QueryPerformanceCounter() 失败。");
	}
	return (double)liCounter.QuadPart * QueryPerformanceFrequencyReciprocalOnce();
}
