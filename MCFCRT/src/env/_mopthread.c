// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_mopthread.h"
#include "mutex.h"
#include "avl_tree.h"
#include "mcfwin.h"
#include "heap.h"
#include "condition_variable.h"
#include "inline_mem.h"
#include "bail.h"
#include "expect.h"
#include <ntdef.h>

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

__attribute__((__dllimport__, __stdcall__))
extern NTSTATUS NtDuplicateObject(HANDLE hSourceProcess, HANDLE hSource, HANDLE hTargetProcess, HANDLE *pTarget, ACCESS_MASK dwDesiredAccess, ULONG dwAttributes, DWORD dwOptions);

static const _MCFCRT_ThreadHandle g_hPseudoSelfHandle = (_MCFCRT_ThreadHandle)GetCurrentThread();

static _MCFCRT_Mutex   g_mtxControl    = { 0 };
static _MCFCRT_AvlRoot g_avlControlMap = _MCFCRT_NULLPTR;

static intptr_t TerminationUnlockCallback(intptr_t nContext){
	_MCFCRT_Mutex *const pMutex = (void *)nContext;

	_MCFCRT_SignalMutex(pMutex);
	return 1;
}
static void TerminationRelockCallback(intptr_t nContext, intptr_t nUnlocked){
	_MCFCRT_Mutex *const pMutex = (void *)nContext;

	_MCFCRT_ASSERT((size_t)nUnlocked == 1);
	_MCFCRT_WaitForMutexForever(pMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
}

typedef enum tagMopthreadState {
	kStateJoinable,
	kStateZombie,
	kStateJoining,
	kStateJoined,
	kStateDetached,
} MopthreadState;

typedef struct tagMopthreadControl {
	_MCFCRT_AvlNodeHeader avlhTidIndex;

	MopthreadState eState;
	size_t uRefCount;
	_MCFCRT_ConditionVariable condTermination;

	uintptr_t uTid;
	_MCFCRT_ThreadHandle hThread;

	void (*pfnProc)(void *);
	size_t uSizeOfParams;
	unsigned char abyParams[];
} MopthreadControl;

static inline int MopthreadControlComparatorNodeOther(const _MCFCRT_AvlNodeHeader *pNodeSelf, intptr_t nTidOther){
	const uintptr_t uTidSelf = (uintptr_t)(((const MopthreadControl *)pNodeSelf)->uTid);
	const uintptr_t uTidOther = (uintptr_t)nTidOther;
	if(uTidSelf != uTidOther){
		return (uTidSelf < uTidOther) ? -1 : 1;
	}
	return 0;
}
static inline int MopthreadControlComparatorNodes(const _MCFCRT_AvlNodeHeader *pNodeSelf, const _MCFCRT_AvlNodeHeader *pNodeOther){
	return MopthreadControlComparatorNodeOther(pNodeSelf, (intptr_t)(((const MopthreadControl *)pNodeOther)->uTid));
}

static unsigned char g_abyInitialControlStorage[sizeof(MopthreadControl) + sizeof(void *) * 3]; // XXX: This should suffice for both gthread and c11thread.
static_assert(sizeof(g_abyInitialControlStorage) == sizeof(void *) * 17, "??");

// The caller must have the global mutex locked!
static inline void DropControlRefUnsafe(MopthreadControl *restrict pControl){
	_MCFCRT_ASSERT(pControl->uRefCount > 0);
	if(--(pControl->uRefCount) == 0){
		_MCFCRT_ASSERT(pControl->eState == kStateJoined);
		_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
		_MCFCRT_CloseThread(pControl->hThread);
#ifndef NDEBUG
		pControl->hThread = (HANDLE)0xDEADBEEF;
#endif
		if(pControl != (void *)g_abyInitialControlStorage){
			_MCFCRT_free(pControl);
		}
	}
}

// *** WARNING: No other threads shall be running when calling these functions. ***
static void AttachInitialThread(void){
	MopthreadControl *const restrict pControl = (void *)g_abyInitialControlStorage;

	pControl->pfnProc       = _MCFCRT_NULLPTR;
	pControl->uSizeOfParams = sizeof(g_abyInitialControlStorage) - sizeof(MopthreadControl);
	_MCFCRT_inline_mempset_fwd(pControl->abyParams, 0, pControl->uSizeOfParams);
	pControl->eState        = kStateJoinable;
	pControl->uRefCount     = 2;
	_MCFCRT_InitializeConditionVariable(&(pControl->condTermination));

	HANDLE hThread;
	NTSTATUS lStatus = NtDuplicateObject(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hThread, 0, 0, DUPLICATE_SAME_ACCESS);
	if(!NT_SUCCESS(lStatus)){
		_MCFCRT_Bail(L"NtDuplicateObject() 失败：无法获取主线程句柄。");
	}
	pControl->uTid    = (uintptr_t)GetCurrentThreadId();
	pControl->hThread = (HANDLE)hThread;

	_MCFCRT_AvlAttach(&g_avlControlMap, (_MCFCRT_AvlNodeHeader *)pControl, &MopthreadControlComparatorNodes);
}
static void DetachInitialThread(void){
	MopthreadControl *const restrict pControl = (void *)g_abyInitialControlStorage;

	switch(pControl->eState){
	case kStateJoinable:
		pControl->eState = kStateDetached;
		goto jJoinSuccess;
	case kStateZombie:
		pControl->eState = kStateJoined;
		goto jJoinSuccess;
	case kStateJoining:
	case kStateJoined:
	case kStateDetached:
		break;
	default:
		_MCFCRT_ASSERT(false);
	jJoinSuccess:
		DropControlRefUnsafe(pControl);
		// bSuccess = true;
		break;
	}
}

__attribute__((__noreturn__))
static inline void SignalMutexAndExitThread(_MCFCRT_Mutex *restrict pMutex, MopthreadControl *restrict pControl, void (*pfnModifier)(void *, size_t, intptr_t), intptr_t nContext){
	switch(pControl->eState){
	case kStateJoinable:
		if(pfnModifier){
			(*pfnModifier)(pControl->abyParams, pControl->uSizeOfParams, nContext);
		}
		pControl->eState = kStateZombie;
		break;
	case kStateZombie:
		_MCFCRT_ASSERT(false);
	case kStateJoining:
		if(pfnModifier){
			(*pfnModifier)(pControl->abyParams, pControl->uSizeOfParams, nContext);
		}
		pControl->eState = kStateJoined;
		_MCFCRT_BroadcastConditionVariable(&(pControl->condTermination));
		break;
	case kStateJoined:
		_MCFCRT_ASSERT(false);
	case kStateDetached:
		pControl->eState = kStateJoined;
		break;
	default:
		_MCFCRT_ASSERT(false);
	}
	DropControlRefUnsafe(pControl);
	_MCFCRT_SignalMutex(pMutex);

	ExitThread(0);
	__builtin_unreachable();
}

bool __MCFCRT_MopthreadInit(void){
	AttachInitialThread();
	return true;
}
void __MCFCRT_MopthreadUninit(void){
	DetachInitialThread();
}

static unsigned long MopthreadProc(void *pParam){
	MopthreadControl *const restrict pControl = pParam;
	_MCFCRT_DEBUG_CHECK(pControl);
	(*(pControl->pfnProc))(pControl->abyParams);
	return 0;
}
__MCFCRT_C_STDCALL __attribute__((__noreturn__))
static unsigned long NativeMopthreadProc(void *pParam){
	MopthreadControl *const restrict pControl = pParam;
	_MCFCRT_DEBUG_CHECK(pControl);
	_MCFCRT_WrapThreadProcWithSehTop(&MopthreadProc, pControl);
	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	SignalMutexAndExitThread(&g_mtxControl, pControl, _MCFCRT_NULLPTR, 0);
}

static inline uintptr_t ReallyCreateMopthread(void (*pfnProc)(void *), const void *pParams, size_t uSizeOfParams, bool bJoinable){
	const size_t uSizeToAlloc = sizeof(MopthreadControl) + uSizeOfParams;
	if(uSizeToAlloc < sizeof(MopthreadControl)){
		return 0;
	}
	MopthreadControl *const restrict pControl = _MCFCRT_malloc(uSizeToAlloc);
	if(!pControl){
		return 0;
	}
	pControl->pfnProc       = pfnProc;
	pControl->uSizeOfParams = uSizeOfParams;
	if(pParams){
		_MCFCRT_inline_mempcpy_fwd(pControl->abyParams, pParams, uSizeOfParams);
	} else {
		_MCFCRT_inline_mempset_fwd(pControl->abyParams, 0, uSizeOfParams);
	}
	if(bJoinable){
		pControl->eState    = kStateJoinable;
		pControl->uRefCount = 2;
	} else {
		pControl->eState    = kStateDetached;
		pControl->uRefCount = 1;
	}
	_MCFCRT_InitializeConditionVariable(&(pControl->condTermination));

	uintptr_t uTid;
	const _MCFCRT_ThreadHandle hThread = _MCFCRT_CreateNativeThread(&NativeMopthreadProc, pControl, true, &uTid);
	if(!hThread){
		_MCFCRT_free(pControl);
		return 0;
	}
	pControl->uTid    = uTid;
	pControl->hThread = hThread;
	// XXX: Note that at the moment you must attach the control block unconditionally, because the thread could call
	//      `__MCFCRT_MopthreadExit()` which must be able to get a valid pointer to it by thread ID.
	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		_MCFCRT_AvlAttach(&g_avlControlMap, (_MCFCRT_AvlNodeHeader *)pControl, &MopthreadControlComparatorNodes);
	}
	_MCFCRT_SignalMutex(&g_mtxControl);

	_MCFCRT_ResumeThread(hThread);
	return uTid;
}

uintptr_t __MCFCRT_MopthreadCreate(void (*pfnProc)(void *), const void *pParams, size_t uSizeOfParams){
	return ReallyCreateMopthread(pfnProc, pParams, uSizeOfParams, true);
}
uintptr_t __MCFCRT_MopthreadCreateDetached(void (*pfnProc)(void *), const void *pParams, size_t uSizeOfParams){
	return ReallyCreateMopthread(pfnProc, pParams, uSizeOfParams, false);
}
__attribute__((__noreturn__))
void __MCFCRT_MopthreadExit(void (*pfnModifier)(void *, size_t, intptr_t), intptr_t nContext){
	const uintptr_t uTid = _MCFCRT_GetCurrentThreadId();

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	MopthreadControl *const restrict pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeOther);
	if(!pControl){
		_MCFCRT_Bail(L"Calling thread of __MCFCRT_MopthreadExit() was not created using __MCFCRT_MopthreadCreate().");
	}
	SignalMutexAndExitThread(&g_mtxControl, pControl, pfnModifier, nContext);
}
bool __MCFCRT_MopthreadJoin(uintptr_t uTid, void *restrict pParams, size_t *restrict puSizeOfParams){
	bool bSuccess = false;

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const restrict pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeOther);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateJoining;
				do {
					_MCFCRT_WaitForConditionVariableForever(&(pControl->condTermination), &TerminationUnlockCallback, &TerminationRelockCallback, (intptr_t)&g_mtxControl, 0);
				} while(pControl->eState != kStateJoined);
				goto jJoinSuccess;
			case kStateZombie:
				pControl->eState = kStateJoined;
				goto jJoinSuccess;
			case kStateJoining:
			case kStateJoined:
			case kStateDetached:
				break;
			default:
				_MCFCRT_ASSERT(false);
			jJoinSuccess:
				_MCFCRT_WaitForThreadForever(pControl->hThread);
				if(pParams){
					const size_t uSizeCopied = (pControl->uSizeOfParams < *puSizeOfParams) ? pControl->uSizeOfParams : *puSizeOfParams;
					_MCFCRT_inline_mempcpy_fwd(pParams, pControl->abyParams, uSizeCopied);
					*puSizeOfParams = uSizeCopied;
				}
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			}
		}
	}
	_MCFCRT_SignalMutex(&g_mtxControl);

	return bSuccess;
}
bool __MCFCRT_MopthreadDetach(uintptr_t uTid){
	bool bSuccess = false;

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const restrict pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeOther);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateDetached;
				goto jJoinSuccess;
			case kStateZombie:
				pControl->eState = kStateJoined;
				goto jJoinSuccess;
			case kStateJoining:
			case kStateJoined:
			case kStateDetached:
				break;
			default:
				_MCFCRT_ASSERT(false);
			jJoinSuccess:
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			}
		}
	}
	_MCFCRT_SignalMutex(&g_mtxControl);

	return bSuccess;
}

const _MCFCRT_ThreadHandle *__MCFCRT_MopthreadLockHandle(uintptr_t uTid){
	if(_MCFCRT_EXPECT_NOT(uTid == 0)){
		return _MCFCRT_NULLPTR;
	}

	if(_MCFCRT_EXPECT(uTid == _MCFCRT_GetCurrentThreadId())){
		// Return a pseudo handle.
		return &g_hPseudoSelfHandle;
	}

	// Increment the reference count and return a real handle.
	const _MCFCRT_ThreadHandle *phThread = _MCFCRT_NULLPTR;

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const restrict pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeOther);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
			case kStateZombie:
			case kStateJoining:
				_MCFCRT_ASSERT(pControl->uRefCount > 0);
				++(pControl->uRefCount);
				phThread = &(pControl->hThread);
				break;
			case kStateJoined:
			case kStateDetached:
				break;
			default:
				_MCFCRT_ASSERT(false);
			}
		}
	}
	_MCFCRT_SignalMutex(&g_mtxControl);

	return phThread;
}
void __MCFCRT_MopthreadUnlockHandle(const _MCFCRT_ThreadHandle *phThread){
	if(!phThread){
		return;
	}

	if(_MCFCRT_EXPECT(phThread == &g_hPseudoSelfHandle)){
		// Don't close the pseudo handle.
		return;
	}

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const restrict pControl = (void *)((char *)phThread - __builtin_offsetof(MopthreadControl, hThread));
		_MCFCRT_ASSERT(pControl);
		DropControlRefUnsafe(pControl);
	}
	_MCFCRT_SignalMutex(&g_mtxControl);
}
