// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "mutex.h"
#include "eh_top.h"
#include "_nt_timeout.h"
#include "../ext/assert.h"
#include <stdlib.h>
#include <winternl.h>
#include <ntdef.h>

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS RtlCreateUserThread(HANDLE hProcess, const SECURITY_DESCRIPTOR *pSecurityDescriptor, BOOLEAN bSuspended,
	ULONG ulStackZeroBits, ULONG *pulStackReserved, ULONG *pulStackCommitted, PTHREAD_START_ROUTINE pfnThreadProc, VOID *pParam, HANDLE *pHandle, CLIENT_ID *pClientId);

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtDelayExecution(BOOLEAN bAlertable, const LARGE_INTEGER *pInterval);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtYieldExecution(void);

extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtSuspendThread(HANDLE hThread, LONG *plPrevCount);
extern __attribute__((__dllimport__, __stdcall__))
NTSTATUS NtResumeThread(HANDLE hThread, LONG *plPrevCount);

void *_MCFCRT_CreateNativeThread(_MCFCRT_NativeThreadProc pfnThreadProc, void *pParam, bool bSuspended, uintptr_t *restrict puThreadId){
	HANDLE hThread;
	CLIENT_ID vClientId;
	ULONG ulStackReserved = 0, ulStackCommitted = 0;
	const NTSTATUS lStatus = RtlCreateUserThread(GetCurrentProcess(), nullptr, bSuspended, 0, &ulStackReserved, &ulStackCommitted, pfnThreadProc, pParam, &hThread, &vClientId);
	if(!NT_SUCCESS(lStatus)){
		SetLastError(RtlNtStatusToDosError(lStatus));
		return nullptr;
	}
	if(puThreadId){
		*puThreadId = (uintptr_t)vClientId.UniqueThread;
	}
	return (void *)hThread;
}

typedef struct tagThreadInitParams {
	_MCFCRT_ThreadProc pfnProc;
	intptr_t nParam;
} ThreadInitParams;

static __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD CrtThreadProc(LPVOID pParam){
	const _MCFCRT_ThreadProc pfnProc = ((ThreadInitParams *)pParam)->pfnProc;
	const intptr_t           nParam  = ((ThreadInitParams *)pParam)->nParam;
	free(pParam);

	DWORD dwExitCode;

	__MCFCRT_EH_TOP_BEGIN
	{
		__MCFCRT_FEnvInit();

		dwExitCode = (*pfnProc)(nParam);
	}
	__MCFCRT_EH_TOP_END

	return dwExitCode;
}

void *_MCFCRT_CreateThread(_MCFCRT_ThreadProc pfnThreadProc, intptr_t nParam, bool bSuspended, uintptr_t *restrict puThreadId){
	ThreadInitParams *const pInitParams = malloc(sizeof(ThreadInitParams));
	if(!pInitParams){
		return nullptr;
	}
	pInitParams->pfnProc = pfnThreadProc;
	pInitParams->nParam  = nParam;

	void *const hThread = _MCFCRT_CreateNativeThread(&CrtThreadProc, pInitParams, bSuspended, puThreadId);
	if(!hThread){
		const DWORD dwLastError = GetLastError();
		free(pInitParams);
		SetLastError(dwLastError);
		return nullptr;
	}
	return (void *)hThread;
}
void _MCFCRT_CloseThread(void *hThread){
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
void _MCFCRT_AlertableSleepForever(){
	LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = INT64_MAX;
	const NTSTATUS lStatus = NtDelayExecution(true, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtDelayExecution() 失败。");
}
void _MCFCRT_YieldThread(){
	const NTSTATUS lStatus = NtYieldExecution();
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtYieldExecution() 失败。");
}

long _MCFCRT_SuspendThread(void *hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtSuspendThread((HANDLE)hThread, &lPrevCount);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtSuspendThread() 失败。");
	return lPrevCount;
}
long _MCFCRT_ResumeThread(void *hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtResumeThread((HANDLE)hThread, &lPrevCount);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtResumeThread() 失败。");
	return lPrevCount;
}

bool _MCFCRT_WaitForThread(void *hThread, uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	__MCF_CRT_InitializeNtTimeout(&liTimeout, u64UntilFastMonoClock);
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, &liTimeout);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void _MCFCRT_WaitForThreadForever(void *hThread){
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, nullptr);
	_MCFCRT_ASSERT_MSG(NT_SUCCESS(lStatus), L"NtWaitForSingleObject() 失败。");
}

// 移动到 thread_env.c 中。
/*uintptr_t _MCFCRT_GetCurrentThreadId(){
	return GetCurrentThreadId();
}*/
