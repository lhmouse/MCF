// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "_mopthread.h"
#include "mutex.h"
#include "avl_tree.h"
#include "mcfwin.h"
#include "heap.h"
#include "_seh_top.h"
#include "condition_variable.h"
#include "inline_mem.h"
#include "../ext/expect.h"

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

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

static inline int MopthreadControlComparatorNodeKey(const _MCFCRT_AvlNodeHeader *lhs, intptr_t rhs){
	const uintptr_t u1 = (uintptr_t)(((const MopthreadControl *)lhs)->uTid);
	const uintptr_t u2 = (uintptr_t)rhs;
	if(u1 != u2){
		return (u1 < u2) ? -1 : 1;
	}
	return 0;
}
static inline int MopthreadControlComparatorNodes(const _MCFCRT_AvlNodeHeader *lhs, const _MCFCRT_AvlNodeHeader *rhs){
	return MopthreadControlComparatorNodeKey(lhs, (intptr_t)(((const MopthreadControl *)rhs)->uTid));
}

// The caller must have the global mutex locked!
static inline void DropControlRefUnsafe(MopthreadControl *pControl){
	if(--(pControl->uRefCount) == 0){
		_MCFCRT_ASSERT(pControl->eState == kStateJoined);
		_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
		_MCFCRT_CloseThread(pControl->hThread);
		_MCFCRT_free(pControl);
	}
}

__attribute__((__noreturn__))
static inline void SignalMutexAndExitThread(_MCFCRT_Mutex *restrict pMutex, MopthreadControl *restrict pControl, void (*pfnModifier)(void *, intptr_t), intptr_t nContext){
	switch(pControl->eState){
	case kStateJoinable:
		if(pfnModifier){
			(*pfnModifier)(pControl->abyParams, nContext);
		}
		pControl->eState = kStateZombie;
		break;
	case kStateZombie:
		_MCFCRT_ASSERT(false);
	case kStateJoining:
		if(pfnModifier){
			(*pfnModifier)(pControl->abyParams, nContext);
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

__MCFCRT_C_STDCALL __attribute__((__noreturn__))
static unsigned long MopthreadProcNative(void *pParam){
	MopthreadControl *const pControl = pParam;
	_MCFCRT_ASSERT(pControl);

	__MCFCRT_SEH_TOP_BEGIN
	{
		(*(pControl->pfnProc))(pControl->abyParams);
	}
	__MCFCRT_SEH_TOP_END

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	SignalMutexAndExitThread(&g_mtxControl, pControl, _MCFCRT_NULLPTR, 0);
}

static inline uintptr_t ReallyCreateMopthread(void (*pfnProc)(void *), const void *pParams, size_t uSizeOfParams, bool bJoinable){
	const size_t uSizeToAlloc = sizeof(MopthreadControl) + uSizeOfParams;
	if(uSizeToAlloc < sizeof(MopthreadControl)){
		return 0;
	}
	MopthreadControl *const pControl = _MCFCRT_malloc(uSizeToAlloc);
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
	const _MCFCRT_ThreadHandle hThread = _MCFCRT_CreateNativeThread(&MopthreadProcNative, pControl, true, &uTid);
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
void __MCFCRT_MopthreadExit(void (*pfnModifier)(void *, intptr_t), intptr_t nContext){
	const uintptr_t uTid = _MCFCRT_GetCurrentThreadId();

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
	_MCFCRT_ASSERT_MSG(pControl,L"Calling thread of __MCFCRT_MopthreadExit() was not created using __MCFCRT_MopthreadCreate().");
	SignalMutexAndExitThread(&g_mtxControl, pControl, pfnModifier, nContext);
}
bool __MCFCRT_MopthreadJoin(uintptr_t uTid, void *restrict pParams){
	bool bSuccess = false;

	_MCFCRT_WaitForMutexForever(&g_mtxControl, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateJoining;
				do {
					_MCFCRT_WaitForConditionVariableForever(&(pControl->condTermination), &TerminationUnlockCallback, &TerminationRelockCallback, (intptr_t)&g_mtxControl, 0);
				} while(pControl->eState != kStateJoined);
				_MCFCRT_WaitForThreadForever(pControl->hThread);
				if(pParams){
					_MCFCRT_inline_mempcpy_fwd(pParams, pControl->abyParams, pControl->uSizeOfParams);
				}
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			case kStateZombie:
				pControl->eState = kStateJoined;
				_MCFCRT_WaitForThreadForever(pControl->hThread);
				if(pParams){
					_MCFCRT_inline_mempcpy_fwd(pParams, pControl->abyParams, pControl->uSizeOfParams);
				}
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			case kStateJoining:
			case kStateJoined:
			case kStateDetached:
				break;
			default:
				_MCFCRT_ASSERT(false);
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
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateDetached;
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			case kStateZombie:
				pControl->eState = kStateJoined;
				DropControlRefUnsafe(pControl);
				bSuccess = true;
				break;
			case kStateJoining:
			case kStateJoined:
			case kStateDetached:
				break;
			default:
				_MCFCRT_ASSERT(false);
			}
		}
	}
	_MCFCRT_SignalMutex(&g_mtxControl);

	return bSuccess;
}

const _MCFCRT_ThreadHandle *__MCFCRT_MopthreadLockHandle(uintptr_t uTid){
	if(uTid == 0){
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
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlControlMap, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
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
		MopthreadControl *const pControl = (void *)((char *)phThread - __builtin_offsetof(MopthreadControl, hThread));
		DropControlRefUnsafe(pControl);
	}
	_MCFCRT_SignalMutex(&g_mtxControl);
}
