// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "thread.h"
#include "mcfwin.h"
#include "fenv.h"
#include "avl_tree.h"
#include "bail.h"
#include "mingw_hacks.h"
#include "eh_top.h"
#include "clocks.h"
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

typedef struct tagTlsObject {
	_MCFCRT_AvlNodeHeader vHeader;

	intptr_t nValue;

	struct tagThreadMap *pMap;
	struct tagTlsObject *pPrevByThread;
	struct tagTlsObject *pNextByThread;

	struct tagTlsKey *pKey;
	struct tagTlsObject *pPrevByKey;
	struct tagTlsObject *pNextByKey;
} TlsObject;

static int ObjectComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(((const TlsObject *)pObj1)->pKey);
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int ObjectComparatorNodes(const _MCFCRT_AvlNodeHeader *pObj1, const _MCFCRT_AvlNodeHeader *pObj2){
	return ObjectComparatorNodeKey(pObj1, (intptr_t)(void *)(((const TlsObject *)pObj2)->pKey));
}

typedef struct tagThreadMap {
	SRWLOCK srwLock;
	_MCFCRT_AvlRoot avlObjects;
	struct tagTlsObject *pLastByThread;
} ThreadMap;

typedef struct tagTlsKey {
	_MCFCRT_AvlNodeHeader vHeader;

	SRWLOCK srwLock;
	void (*pfnCallback)(intptr_t);
	struct tagTlsObject *pLastByKey;
} TlsKey;

static int KeyComparatorNodeKey(const _MCFCRT_AvlNodeHeader *pObj1, intptr_t nKey2){
	const uintptr_t uKey1 = (uintptr_t)(void *)pObj1;
	const uintptr_t uKey2 = (uintptr_t)(void *)nKey2;
	return (uKey1 < uKey2) ? -1 : ((uKey1 > uKey2) ? 1 : 0);
}
static int KeyComparatorNodes(const _MCFCRT_AvlNodeHeader *pObj1, const _MCFCRT_AvlNodeHeader *pObj2){
	return KeyComparatorNodeKey(pObj1, (intptr_t)(void *)pObj2);
}

static SRWLOCK          g_csKeyMutex  = SRWLOCK_INIT;
static DWORD            g_dwTlsIndex  = TLS_OUT_OF_INDEXES;
static _MCFCRT_AvlRoot  g_avlKeys     = nullptr;

bool __MCFCRT_ThreadEnvInit(){
	g_dwTlsIndex = TlsAlloc();
	if(g_dwTlsIndex == TLS_OUT_OF_INDEXES){
		return false;
	}
	return true;
}
void __MCFCRT_ThreadEnvUninit(){
	if(g_avlKeys){
		_MCFCRT_AvlNodeHeader *const pRoot = g_avlKeys;
		g_avlKeys = nullptr;

		TlsKey *pKey;
		_MCFCRT_AvlNodeHeader *pCur = _MCFCRT_AvlPrev(pRoot);
		while(pCur){
			pKey = (TlsKey *)pCur;
			pCur = _MCFCRT_AvlPrev(pCur);
			free(pKey);
		}
		pCur = _MCFCRT_AvlNext(pRoot);
		while(pCur){
			pKey = (TlsKey *)pCur;
			pCur = _MCFCRT_AvlNext(pCur);
			free(pKey);
		}
		pKey = (TlsKey *)pRoot;
		free(pKey);
	}

	TlsFree(g_dwTlsIndex);
	g_dwTlsIndex = TLS_OUT_OF_INDEXES;
}

void __MCFCRT_TlsThreadCleanup(){
	ThreadMap *const pMap = TlsGetValue(g_dwTlsIndex);
	if(pMap){
		TlsObject *pObject = pMap->pLastByThread;
		while(pObject){
			TlsKey *const pKey = pObject->pKey;

			AcquireSRWLockExclusive(&(pKey->srwLock));
			{
				if(pKey->pLastByKey == pObject){
					pKey->pLastByKey = pObject->pPrevByKey;
				}
			}
			ReleaseSRWLockExclusive(&(pKey->srwLock));

			if(pKey->pfnCallback){
				(*pKey->pfnCallback)(pObject->nValue);
			}

			TlsObject *const pTemp = pObject->pPrevByThread;
			free(pObject);
			pObject = pTemp;
		}
		free(pMap);
		TlsSetValue(g_dwTlsIndex, nullptr);
	}

	__MCFCRT_RunEmutlsDtors();
}

void *_MCFCRT_TlsAllocKey(void (*pfnCallback)(intptr_t)){
	TlsKey *const pKey = malloc(sizeof(TlsKey));
	if(!pKey){
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return nullptr;
	}
	InitializeSRWLock(&(pKey->srwLock));
	pKey->pfnCallback = pfnCallback;
	pKey->pLastByKey  = nullptr;

	AcquireSRWLockExclusive(&g_csKeyMutex);
	{
		_MCFCRT_AvlAttach(&g_avlKeys, (_MCFCRT_AvlNodeHeader *)pKey, &KeyComparatorNodes);
	}
	ReleaseSRWLockExclusive(&g_csKeyMutex);

	return pKey;
}
bool _MCFCRT_TlsFreeKey(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	AcquireSRWLockExclusive(&g_csKeyMutex);
	{
		_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pKey);
	}
	ReleaseSRWLockExclusive(&g_csKeyMutex);

	TlsObject *pObject = pKey->pLastByKey;
	while(pObject){
		ThreadMap *const pMap = pObject->pMap;

		AcquireSRWLockExclusive(&(pMap->srwLock));
		{
			TlsObject *const pPrev = pObject->pPrevByThread;
			TlsObject *const pNext = pObject->pNextByThread;
			if(pPrev){
				pPrev->pNextByThread = pNext;
			}
			if(pNext){
				pNext->pPrevByThread = pPrev;
			}

			if(pMap->pLastByThread == pObject){
				pMap->pLastByThread = pObject->pPrevByThread;
			}
		}
		ReleaseSRWLockExclusive(&(pMap->srwLock));

		if(pKey->pfnCallback){
			(*pKey->pfnCallback)(pObject->nValue);
		}

		TlsObject *const pTemp = pObject->pPrevByKey;
		free(pObject);
		pObject = pTemp;
	}
	free(pKey);

	return true;
}

_MCFCRT_TlsCallback _MCFCRT_TlsGetCallback(void *pTlsKey){
	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}
	SetLastError(ERROR_SUCCESS);
	return pKey->pfnCallback;
}
bool _MCFCRT_TlsGet(void *pTlsKey, intptr_t **restrict ppnValue){
	*ppnValue = nullptr;

	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	ThreadMap *pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		return true;
	}

	AcquireSRWLockExclusive(&(pMap->srwLock));
	{
		TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pMap->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
		if(!pObject){
			return true;
		}
		*ppnValue = &(pObject->nValue);
	}
	ReleaseSRWLockExclusive(&(pMap->srwLock));

	return true;
}
bool _MCFCRT_TlsRequire(void *pTlsKey, intptr_t **restrict ppnValue, _MCFCRT_STD intptr_t nInitValue){
	*ppnValue = nullptr;

	TlsKey *const pKey = pTlsKey;
	if(!pKey){
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	ThreadMap *pMap = TlsGetValue(g_dwTlsIndex);
	if(!pMap){
		pMap = malloc(sizeof(ThreadMap));
		if(!pMap){
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return false;
		}
		InitializeSRWLock(&(pMap->srwLock));
		pMap->avlObjects    = nullptr;
		pMap->pLastByThread = nullptr;

		TlsSetValue(g_dwTlsIndex, pMap);
	}

	AcquireSRWLockExclusive(&(pMap->srwLock));
	{
		TlsObject *pObject = (TlsObject *)_MCFCRT_AvlFind(&(pMap->avlObjects), (intptr_t)pKey, &ObjectComparatorNodeKey);
		if(!pObject){
			ReleaseSRWLockExclusive(&(pMap->srwLock));
			{
				pObject = malloc(sizeof(TlsObject));
				if(!pObject){
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return false;
				}
				pObject->nValue = nInitValue;
				pObject->pMap   = pMap;
				pObject->pKey   = pKey;

				AcquireSRWLockExclusive(&(pKey->srwLock));
				{
					TlsObject *const pPrev = pKey->pLastByKey;
					pKey->pLastByKey = pObject;

					pObject->pPrevByKey = pPrev;
					pObject->pNextByKey = nullptr;
					if(pPrev){
						pPrev->pNextByKey = pObject;
					}
				}
				ReleaseSRWLockExclusive(&(pKey->srwLock));
			}
			AcquireSRWLockExclusive(&(pMap->srwLock));

			TlsObject *const pPrev = pMap->pLastByThread;
			pMap->pLastByThread = pObject;

			pObject->pPrevByThread = pPrev;
			pObject->pNextByThread = nullptr;
			if(pPrev){
				pPrev->pNextByThread = pObject;
			}
			_MCFCRT_AvlAttach(&(pMap->avlObjects), (_MCFCRT_AvlNodeHeader *)pObject, &ObjectComparatorNodes);
		}
		*ppnValue = &(pObject->nValue);
	}
	ReleaseSRWLockExclusive(&(pMap->srwLock));

	return true;
}

int _MCFCRT_AtThreadExit(_MCFCRT_TlsCallback pfnProc, _MCFCRT_STD intptr_t nContext){
	void *const pKey = _MCFCRT_TlsAllocKey(pfnProc);
	if(!pKey){
		return -1;
	}
	intptr_t *pnValue;
	if(!_MCFCRT_TlsRequire(pKey, &pnValue, nContext)){
		const DWORD dwLastError = GetLastError();
		_MCFCRT_TlsFreeKey(pKey);
		SetLastError(dwLastError);
		return -1;
	}
	ASSERT(*pnValue == nContext);
	return 0;
}

static NTSTATUS ReallyCreateNativeThread(HANDLE *phThread, DWORD (*__attribute__((__stdcall__)) pfnThreadProc)(LPVOID), LPVOID pParam, bool bSuspended, uintptr_t *restrict puThreadId){
	ULONG ulStackReserved = 0, ulStackCommitted = 0;
	CLIENT_ID vClientId;
	const NTSTATUS lStatus = RtlCreateUserThread(GetCurrentProcess(), nullptr, bSuspended, 0, &ulStackReserved, &ulStackCommitted, pfnThreadProc, pParam, phThread, &vClientId);
	if(!NT_SUCCESS(lStatus)){
		return lStatus;
	}
	if(puThreadId){
		*puThreadId = (uintptr_t)(vClientId.UniqueThread);
	}
	return lStatus;
}

void *_MCFCRT_CreateNativeThread(unsigned long (*__attribute__((__stdcall__)) pfnThreadProc)(void *), void *pParam, bool bSuspended, uintptr_t *restrict puThreadId){
	HANDLE hThread;
	const NTSTATUS lStatus = ReallyCreateNativeThread(&hThread, pfnThreadProc, pParam, bSuspended, puThreadId);
	if(!NT_SUCCESS(lStatus)){
		SetLastError(RtlNtStatusToDosError(lStatus));
		return nullptr;
	}
	return (void *)hThread;
}

typedef struct tagThreadInitParams {
	unsigned (*pfnProc)(intptr_t);
	intptr_t nParam;
} ThreadInitParams;

static __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD CRTThreadProc(LPVOID pParam){
	DWORD dwExitCode;
	__MCFCRT_EH_TOP_BEGIN
	{
		const ThreadInitParams vInitParams = *(ThreadInitParams *)pParam;
		free(pParam);

		__MCFCRT_FEnvInit();

		dwExitCode = (*vInitParams.pfnProc)(vInitParams.nParam);
	}
	__MCFCRT_EH_TOP_END
	return dwExitCode;
}

void *_MCFCRT_CreateThread(unsigned (*pfnThreadProc)(intptr_t), intptr_t nParam, bool bSuspended, uintptr_t *restrict puThreadId){
	ThreadInitParams *const pInitParams = malloc(sizeof(ThreadInitParams));
	if(!pInitParams){
		return nullptr;
	}
	pInitParams->pfnProc = pfnThreadProc;
	pInitParams->nParam  = nParam;

	HANDLE hThread;
	const NTSTATUS lStatus = ReallyCreateNativeThread(&hThread, &CRTThreadProc, pInitParams, bSuspended, puThreadId);
	if(!NT_SUCCESS(lStatus)){
		free(pInitParams);
		SetLastError(RtlNtStatusToDosError(lStatus));
		return nullptr;
	}
	return (void *)hThread;
}
void _MCFCRT_CloseThread(void *hThread){
	const NTSTATUS lStatus = NtClose((HANDLE)hThread);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtClose() 失败。");
	}
}

uintptr_t _MCFCRT_GetCurrentThreadId(){
	return GetCurrentThreadId();
}

void _MCFCRT_Sleep(uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	const uint64_t u64Now = _MCFCRT_GetFastMonoClock();
	if(u64Now >= u64UntilFastMonoClock){
		liTimeout.QuadPart = 0;
	} else {
		const uint64_t u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
		const int64_t n64Delta100Nanosec = (int64_t)(u64DeltaMillisec * 10000);
		if((uint64_t)(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
			liTimeout.QuadPart = INT64_MIN;
		} else {
			liTimeout.QuadPart = -n64Delta100Nanosec;
		}
	}
	const NTSTATUS lStatus = NtDelayExecution(false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtDelayExecution() 失败。");
	}
}
bool _MCFCRT_AlertableSleep(uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	const uint64_t u64Now = _MCFCRT_GetFastMonoClock();
	if(u64Now >= u64UntilFastMonoClock){
		liTimeout.QuadPart = 0;
	} else {
		const uint64_t u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
		const int64_t n64Delta100Nanosec = (int64_t)(u64DeltaMillisec * 10000);
		if((uint64_t)(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
			liTimeout.QuadPart = INT64_MIN;
		} else {
			liTimeout.QuadPart = -n64Delta100Nanosec;
		}
	}
	const NTSTATUS lStatus = NtDelayExecution(true, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtDelayExecution() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void _MCFCRT_AlertableSleepForever(){
	LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = INT64_MAX;
	const NTSTATUS lStatus = NtDelayExecution(true, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtDelayExecution() 失败。");
	}
}
void _MCFCRT_YieldThread(){
	const NTSTATUS lStatus = NtYieldExecution();
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtYieldExecution() 失败。");
	}
}

long _MCFCRT_SuspendThread(void *hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtSuspendThread((HANDLE)hThread, &lPrevCount);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtSuspendThread() 失败。");
	}
	return lPrevCount;
}
long _MCFCRT_ResumeThread(void *hThread){
	LONG lPrevCount;
	const NTSTATUS lStatus = NtResumeThread((HANDLE)hThread, &lPrevCount);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtResumeThread() 失败。");
	}
	return lPrevCount;
}

bool _MCFCRT_WaitForThread(void *hThread, _MCFCRT_STD uint64_t u64UntilFastMonoClock){
	LARGE_INTEGER liTimeout;
	liTimeout.QuadPart = 0;
	if(u64UntilFastMonoClock != 0){
		const uint64_t u64Now = _MCFCRT_GetFastMonoClock();
		if(u64Now < u64UntilFastMonoClock){
			const uint64_t u64DeltaMillisec = u64UntilFastMonoClock - u64Now;
			const int64_t n64Delta100Nanosec = (int64_t)(u64DeltaMillisec * 10000);
			if((uint64_t)(n64Delta100Nanosec / 10000) != u64DeltaMillisec){
				liTimeout.QuadPart = INT64_MIN;
			} else {
				liTimeout.QuadPart = -n64Delta100Nanosec;
			}
		}
	}
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, &liTimeout);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
	if(lStatus == STATUS_TIMEOUT){
		return false;
	}
	return true;
}
void _MCFCRT_WaitForThreadForever(void *hThread){
	const NTSTATUS lStatus = NtWaitForSingleObject((HANDLE)hThread, false, nullptr);
	if(!NT_SUCCESS(lStatus)){
		ASSERT_MSG(false, L"NtWaitForSingleObject() 失败。");
	}
}
