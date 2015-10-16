// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "system.h"
#include <stdlib.h>
#include "mcfwin.h"

static SYSTEM_INFO g_vSystemInfo;

__attribute__((__constructor__))
static void SystemInfoCtor(){
	GetSystemInfo(&g_vSystemInfo);
}

size_t MCF_GetLogicalProcessvrCount(){
	return g_vSystemInfo.dwNumberOfProcessors;
}
size_t MCF_GetPageSize(){
	return g_vSystemInfo.dwPageSize;
}

static OSVERSIONINFOW g_vOsVersionInfo;

__attribute__((__constructor__))
static void OsVersionInfoCtor(){
	g_vOsVersionInfo.dwOSVersionInfoSize = sizeof(g_vOsVersionInfo);
	GetVersionExW(&g_vOsVersionInfo);
}

void MCF_GetWindowsVersion(MCF_WindowsVersion *pVersion){
	pVersion->uMajor       = g_vOsVersionInfo.dwMajorVersion;
	pVersion->uMinor       = g_vOsVersionInfo.dwMinorVersion;
	pVersion->uBuild       = g_vOsVersionInfo.dwBuildNumber;
	pVersion->pwszServPack = g_vOsVersionInfo.szCSDVersion;
}

uint64_t MCF_GetUtcTime(){
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} unUtc;
	GetSystemTimeAsFileTime(&unUtc.ft);
	// 0x019DB1DED53E8000 = 从 1601-01-01 到 1970-01-01 经历的时间纳秒数。
	return (unUtc.uli.QuadPart - 0x019DB1DED53E8000ull) / 10000;
}
uint64_t MCF_GetLocalTime(){
	union {
		FILETIME ft;
		ULARGE_INTEGER uli;
	} unUtc, unLocal;
	GetSystemTimeAsFileTime(&unUtc.ft);
	FileTimeToLocalFileTime(&unUtc.ft, &unLocal.ft);
	return (unLocal.uli.QuadPart - 0x019DB1DED53E8000ull) / 10000;
}

static double g_lfFrequencyReciprocal;

__attribute__((__constructor__))
static void FrequencyReciprocalCtor(){
	LARGE_INTEGER liFrequency;
	if(!QueryPerformanceFrequency(&liFrequency)){
		abort();
	}
	g_lfFrequencyReciprocal = 1000.0 / liFrequency.QuadPart;
}

uint64_t MCF_GetFastMonoClock(){
	return GetTickCount64();
}
double MCF_GetHiResMonoClock(){
	LARGE_INTEGER liCounter;
	if(!QueryPerformanceCounter(&liCounter)){
		abort();
	}
	return liCounter.QuadPart * g_lfFrequencyReciprocal;
}
