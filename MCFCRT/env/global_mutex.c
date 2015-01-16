// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "global_mutex.h"
#include "mcfwin.h"

static CRITICAL_SECTION g_csGlobalMutex;

bool __MCF_CRT_GlobalMutexInit() MCF_NOEXCEPT {
	if(!InitializeCriticalSectionEx(&g_csGlobalMutex, 0x400u,
#ifdef NDEBUG
		CRITICAL_SECTION_NO_DEBUG_INFO
#else
		0
#endif
		))
	{
		return false;
	}
	return true;
}
void __MCF_CRT_GlobalMutexUninit(){
	DeleteCriticalSection(&g_csGlobalMutex);
}

bool MCF_CRT_GlobalMutexTryLock(){
	return TryEnterCriticalSection(&g_csGlobalMutex);
}
void MCF_CRT_GlobalMutexLock(){
	EnterCriticalSection(&g_csGlobalMutex);
}
void MCF_CRT_GlobalMutexUnlock(){
	LeaveCriticalSection(&g_csGlobalMutex);
}
