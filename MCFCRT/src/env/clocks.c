// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "clocks.h"
#include "mcfwin.h"
#include "bail.h"

static uint64_t GetTimeZoneOffsetInMillisecondsOnce(void){
	static uint64_t s_u64Value, *volatile s_pu64Inited;

	uint64_t *pInited = __atomic_load_n(&s_pu64Inited, __ATOMIC_CONSUME);
	if(!pInited){
		pInited = &s_u64Value;

		TIME_ZONE_INFORMATION vTzInfo;
		if(GetTimeZoneInformation(&vTzInfo) == TIME_ZONE_ID_INVALID){
			_MCFCRT_Bail(L"GetTimeZoneInformation() 失败。");
		}
		*pInited = (uint64_t)(vTzInfo.Bias * -60000ll);

		__atomic_store_n(&s_pu64Inited, pInited, __ATOMIC_RELEASE);
	}
	return *pInited;
}
static double QueryPerformanceFrequencyReciprocalOnce(void){
	static double s_lfValue, *volatile s_plfInited;

	double *pInited = __atomic_load_n(&s_plfInited, __ATOMIC_CONSUME);
	if(!pInited){
		LARGE_INTEGER liFreq;
		if(!QueryPerformanceFrequency(&liFreq)){
			_MCFCRT_Bail(L"QueryPerformanceFrequency() 失败。");
		}
		const double lfValue = 1000.0 / (double)liFreq.QuadPart;

		pInited = __builtin_memcpy(&s_lfValue, &lfValue, sizeof(lfValue));
		__atomic_store_n(&s_plfInited, pInited, __ATOMIC_RELEASE);
	}
	return *pInited;
}

uint64_t _MCFCRT_GetUtcClock(void){
	union {
		FILETIME ft;
		LARGE_INTEGER li;
	} unUtc;
	GetSystemTimeAsFileTime(&unUtc.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (uint64_t)(((double)unUtc.li.QuadPart - 0x019DB1DED53E8000ll) / 10000.0);
}
uint64_t _MCFCRT_GetLocalClock(void){
	return _MCFCRT_GetLocalClockFromUtc(_MCFCRT_GetUtcClock());
}

uint64_t _MCFCRT_GetUtcClockFromLocal(uint64_t u64LocalClock){
	return u64LocalClock - GetTimeZoneOffsetInMillisecondsOnce();
}
uint64_t _MCFCRT_GetLocalClockFromUtc(uint64_t u64UtcClock){
	return u64UtcClock + GetTimeZoneOffsetInMillisecondsOnce();
}

#ifdef NDEBUG
#	define DEBUG_MONO_CLOCK_OFFSET   0ull
#else
#	define DEBUG_MONO_CLOCK_OFFSET   0x100000000ull
#endif

uint64_t _MCFCRT_GetFastMonoClock(void){
	return GetTickCount64() + DEBUG_MONO_CLOCK_OFFSET;
}
double _MCFCRT_GetHiResMonoClock(void){
	LARGE_INTEGER liCounter;
	if(!QueryPerformanceCounter(&liCounter)){
		_MCFCRT_Bail(L"QueryPerformanceCounter() failed.");
	}
	return ((double)liCounter.QuadPart + (double)(DEBUG_MONO_CLOCK_OFFSET * 2)) * QueryPerformanceFrequencyReciprocalOnce();
}
