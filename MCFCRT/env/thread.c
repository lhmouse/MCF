// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "heap.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include <winternl.h>
#include <ntdef.h>

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtDelayExecution(BOOLEAN bAlertable, const LARGE_INTEGER *pInterval);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtYieldExecution(void);

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtSuspendThread(HANDLE hThread, LONG *plPrevCount);
__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtResumeThread(HANDLE hThread, LONG *plPrevCount);

_MCFCRT_ThreadHandle _MCFCRT_CreateNativeThread(_MCFCRT_NativeThreadProc pfnThreadProc, void *pParam, bool bSuspended, uintptr_t *restrict puThreadId){
	DWORD dwThreadId;
	const HANDLE hThread = CreateRemoteThread(GetCurrentProcess(), nullptr, 0, pfnThreadProc, pParam, bSuspended ? CREATE_SUSPENDED : 0, &dwThreadId);
	if(!hThread){
		return nullptr;
	}
	if(puThreadId){
		*puThreadId = dwThreadId;
	}
	return (_MCFCRT_ThreadHandle)hThread;
}
void _MCFCRT_CloseThread(_MCFCRT_ThreadHandle hThread){
	const NTSTATUS lStatus = NtClose((HANDLE)hThread);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtClose() 失败。");
}

void _MCFCRT_Sleep(uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const NTSTATUS lStatus = NtDelayExecution(false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtDelayExecution() 失败。");
}
bool _MCFCRT_AlertableSleep(uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const NTSTATUS lStatus = NtDelayExecution(true, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtDelayExecution() 失败。");
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void _MCFCRT_AlertableSleepForever(void){
	LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = INT64_MAX;
	const NTSTATUS lStatus = NtDelayExecution(true, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtDelayExecution() 失败。");
}
void _MCFCRT_YieldThread(void){
	const NTSTATUS lStatus = NtYieldExecution();
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtYieldExecution() 失败。");
}

long _MCFCRT_SuspendThread(_MCFCRT_ThreadHandle hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtSuspendThread((HANDLE)hThread, &lPrevCount);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtSuspendThread() 失败。");
	return lPrevCount;
}
long _MCFCRT_ResumeThread(_MCFCRT_ThreadHandle hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtResumeThread((HANDLE)hThread, &lPrevCount);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtResumeThread() 失败。");
	return lPrevCount;
}

bool _MCFCRT_WaitForThread(_MCFCRT_ThreadHandle hThread, uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void _MCFCRT_WaitForThreadForever(_MCFCRT_ThreadHandle hThread){
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, nullptr);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
}

uintptr_t _MCFCRT_GetCurrentThreadId(void){
	return GetCurrentThreadId();
}
