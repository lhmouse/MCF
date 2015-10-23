// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "clocks.h"
#include "mcfwin.h"
#include "bail.h"
#include <stdlib.h>
#include <winternl.h>
#include <ntdef.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtQueryPerformanceCounter(LARGE_INTEGER *pCounter, LARGE_INTEGER *pFrequency);

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
	LARGE_INTEGER liCounter, liFrequency;
	const NTSTATUS lStatus = NtQueryPerformanceCounter(&liCounter, &liFrequency);
	if(!NT_SUCCESS(lStatus)){
		MCF_CRT_Bail(L"NtQueryPerformanceCounter() 失败。");
	}
	return liCounter.QuadPart * 1000.0 / liFrequency.QuadPart;
}
