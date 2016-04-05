// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "mutex.h"
#include "bail.h"
#include "mingw_hacks.h"
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

enum {
	kMutexSpinCount = 100,
};

typedef struct tagTlsObject {
	_MCFCRT_AvlNodeHeader vHeader;

	struct tagTlsThread *pThread;
	struct tagTlsObject *pPrevByThread;
	struct tagTlsObject *pNextByThread;

	struct tagTlsKey *pKey;
	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;

	alignas(max_align_t) unsigned char abyStorage[];
} TlsObject;

static int ObjectComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const TlsObject *)pObj1)->pKey);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int ObjectComparatorNodes(const _MCFCRT_AvlNodeHeader *pObj1, const _MCFCRT_AvlNodeHeader *pObj2){
	return ObjectComparatorNodeKey(pObj1, (intptr_t)(void *)(((const TlsObject *)pObj2)->pKey));
}

typedef struct tagTlsThread {
	_MCFCRT_Mutex vMutex;

	_MCFCRT_AvlRoot avlObjects;

	struct tagTlsObject *pFirstByThread;
	struct tagTlsObject *pLastByThread;
} TlsThread;

typedef struct tagTlsKey {
	size_t uSize;
	_MCFCRT_TlsConstructor pfnConstructor;
	_MCFCRT_TlsDestructor pfnDestructor;

	_MCFCRT_Mutex vMutex;

	struct tagTlsObject *pFirstByKey;
	struct tagTlsObject *pLastByKey;
} TlsKey;

static DWORD g_dwTlsIndex = TLS_OUT_OF_INDEXES;

bool __MCFCRT_ThreadEnvInit(){
	const DWORD dwTlsIndex = TlsAlloc();
	if(dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}

	g_dwTlsIndex = dwTlsIndex;
	return true;
}
void __MCFCRT_ThreadEnvUninit(){
	const DWORD dwTlsIndex = g_dwTlsIndex;
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;

	TlsFree(dwTlsIndex);
}

void __MCFCRT_TlsThreadCleanup(){
	TlsThread *const pThread = TlsGetValue(g_dwTlsIndex);
	if(pThread){
		TlsObject *pObject = pThread->pLastByThread;
		while(pObject){
			TlsKey *const pKey = pObject->pKey;

			_MCFCRT_WaitForMutexForever(&(pKey->vMutex), kMutexSpinCount);
			{
				TlsObject *const pPrev = pObject->pPrevByKey;
				TlsObject *const pNext = pObject->pNextByKey;
				if(pPrev){
					pPrev->pNextByKey = pNext;
				} else {
					pKey->pFirstByKey = pNext;
				}
				if(pNext){
					pNext->pPrevByKey = pPrev;
				} else {
					pKey->pLastByKey = pPrev;
				}
			}
			_MCFCRT_SignalMutex(&(pKey->vMutex));

			if(pKey->pfnDestructor){
				(*(pKey->pfnDestructor))(pObject->abyStorage);
			}

			TlsObject *const pPrevByThread = pObject->pPrevByThread;
			free(pObject);
			pObject = pPrevByThread;
		}
		free(pThread);
		TlsSetValue(g_dwTlsIndex, nullptr);
	}

	__MCFCRT_RunEmutlsDtors();
}

void *_MCFCRT_TlsAllocKey(size_t uSize, _MCFCRT_TlsConstructor pfnConstructor, _MCFCRT_TlsDestructor pfnDestructor){
	TlsKey *const pKey = malloc(sizeof(TlsKey));
	if(!pKey){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	pKey->uSize          = uSize;
	pKey->pfnConstructor = pfnConstructor;
	pKey->pfnDestructor  = pfnDestructor;
	pKey->vMutex         = _MCFCRT_MUTEX_INITIALIZER;
	pKey->pFirstByKey    = nullptr;
	pKey->pLastByKey     = nullptr;

	return pKey;
}
bool _MCFCRT_TlsFreeKey(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	TlsObject *pObject = pKey->pLastByKey;
	while(pObject){
		TlsThread *const pThread = pObject->pThread;

		_MCFCRT_WaitForMutexForever(&(pThread->vMutex), kMutexSpinCount);
		{
			TlsObject *const pPrev = pObject->pPrevByThread;
			TlsObject *const pNext = pObject->pNextByThread;
			if(pPrev){
				pPrev->pNextByThread = pNext;
			} else {
				pThread->pFirstByThread = pNext;
			}
			if(pNext){
				pNext->pPrevByThread = pPrev;
			} else {
				pThread->pLastByThread = pPrev;
			}
		}
		_MCFCRT_SignalMutex(&(pThread->vMutex));

		if(pKey->pfnDestructor){
			(*(pKey->pfnDestructor))(pObject->abyStorage);
		}

		TlsObject *const pPrevByKey = pObject->pPrevByKey;
		free(pObject);
		pObject = pPrevByKey;
	}
	free(pKey);

	return true;
}

size_t _MCFCRT_TlsGetSize(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->uSize;
}
_MCFCRT_TlsConstructor _MCFCRT_TlsGetConstructor(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnConstructor;
}
_MCFCRT_TlsDestructor _MCFCRT_TlsGetDestructor(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnDestructor;
}

bool _MCFCRT_TlsGet(void *pTlsKey, void **restrict ppStorage){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	TlsThread *pThread = TlsGetValue(g_dwTlsIndex);
	if(!pThread){
		*ppStorage = nullptr;
		return true;
	}

	TlsObject *pObject;

	_MCFCRT_WaitForMutexForever(&(pThread->vMutex), kMutexSpinCount);
	{
		pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThread->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
	}
	_MCFCRT_SignalMutex(&(pThread->vMutex));

	if(!pObject){
		*ppStorage = nullptr;
		return true;
	}

	*ppStorage = pObject->abyStorage;
	return true;
}
bool _MCFCRT_TlsRequire(void *pTlsKey, void **restrict ppStorage){
	*ppStorage = nullptr;

	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	TlsThread *pThread = TlsGetValue(g_dwTlsIndex);
	if(!pThread){
		pThread = malloc(sizeof(TlsThread));
		if(!pThread){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pThread->vMutex        = _MCFCRT_MUTEX_INITIALIZER;
		pThread->avlObjects    = nullptr;
		pThread->pLastByThread = nullptr;

		TlsSetValue(g_dwTlsIndex, pThread);
	}

	TlsObject *pObject;

	_MCFCRT_WaitForMutexForever(&(pThread->vMutex), kMutexSpinCount);
	{
		pObject = (TlsObject *)_MCFCRT_AvlFind(&(pThread->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
	}
	_MCFCRT_SignalMutex(&(pThread->vMutex));

	if(!pObject){
		const size_t uSizeToAlloc = sizeof(TlsObject) + pKey->uSize;
		if(uSizeToAlloc < sizeof(TlsObject)){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pObject = malloc(uSizeToAlloc);
		if(!pObject){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		pObject->pThread = pThread;
		pObject->pKey = pKey;
		if(pKey->pfnConstructor){
			const DWORD dwErrorCode = (*(pKey->pfnConstructor))(pObject->abyStorage);
			if(dwErrorCode != 0){
				free(pObject);
				SetLastError(dwErrorCode);
				return false;
			}
		}

		_MCFCRT_WaitForMutexForever(&(pKey->vMutex), kMutexSpinCount);
		{
			TlsObject *const pPrev = pKey->pLastByKey;
			pKey->pLastByKey = pObject;

			pObject->pPrevByKey = pPrev;
			pObject->pNextByKey = nullptr;
			if(pPrev){
				pPrev->pNextByKey = pObject;
			}
		}
		_MCFCRT_SignalMutex(&(pKey->vMutex));

		_MCFCRT_WaitForMutexForever(&(pThread->vMutex), kMutexSpinCount);
		{
			TlsObject *const pPrev = pThread->pLastByThread;
			pThread->pLastByThread = pObject;

			pObject->pPrevByThread = pPrev;
			pObject->pNextByThread = nullptr;
			if(pPrev){
				pPrev->pNextByThread = pObject;
			}
			_MCFCRT_AvlAttach(&(pThread->avlObjects), (_MCFCRT_AvlNodeHeader *)pObject, &ObjectComparatorNodes);
		}
		_MCFCRT_SignalMutex(&(pThread->vMutex));
	}

	*ppStorage = pObject->abyStorage;
	return true;
}

typedef struct tagAtThreadExitParams {
	_MCFCRT_AtThreadExitCallback pfnProc;
	intptr_t nContext;
} AtThreadExitParams;

static void CRTAtExitThreadProc(void *pStorage){
	const __auto_type pfnProc  = ((AtThreadExitParams *)pStorage)->pfnProc;
	const __auto_type nContext = ((AtThreadExitParams *)pStorage)->nContext;

	(*pfnProc)(nContext);
}

bool _MCFCRT_AtThreadExit(_MCFCRT_AtThreadExitCallback pfnProc, intptr_t nContext){
	void *const pKey = _MCFCRT_TlsAllocKey(sizeof(AtThreadExitParams), nullptr, &CRTAtExitThreadProc);
	if(!pKey){
		return false;
	}
	void *pStorage;
	if(!_MCFCRT_TlsRequire(pKey, &pStorage)){
		const DWORD dwLastError = GetLastError();
		_MCFCRT_TlsFreeKey(pKey);
		SetLastError(dwLastError);
		return false;
	}
	AtThreadExitParams *const pParams = pStorage;
	pParams->pfnProc  = pfnProc;
	pParams->nContext = nContext;
	return true;
}

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
DWORD ThreadProc(LPVOID pParam){
	DWORD dwExitCode;
	__MCFCRT_EH_TOP_BEGIN
	{
		const __auto_type pfnProc = ((ThreadInitParams *)pParam)->pfnProc;
		const __auto_type nParam  = ((ThreadInitParams *)pParam)->nParam;
		free(pParam);

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

	void *const hThread = _MCFCRT_CreateNativeThread(&ThreadProc, pInitParams, bSuspended, puThreadId);
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

uintptr_t _MCFCRT_GetCurrentThreadId(){
	return GetCurrentThreadId();
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
