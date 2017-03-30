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

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

static const _MCFCRT_ThreadHandle g_hPseudoSelfHandle = (_MCFCRT_ThreadHandle)GetCurrentThread();

static _MCFCRT_Mutex   g_vMopthreadMutex      = { 0 };
static _MCFCRT_AvlRoot g_avlMopthreadControls = _MCFCRT_NULLPTR;

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
	volatile size_t uRefCount;
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

static intptr_t UnlockCallbackNative(intptr_t nContext){
	_MCFCRT_Mutex *const pMutex = (_MCFCRT_Mutex *)nContext;

	_MCFCRT_SignalMutex(pMutex);
	return 1;
}
static void RelockCallbackNative(intptr_t nContext, intptr_t nUnlocked){
	_MCFCRT_Mutex *const pMutex = (_MCFCRT_Mutex *)nContext;

	_MCFCRT_ASSERT((size_t)nUnlocked == 1);
	_MCFCRT_WaitForMutexForever(pMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
}

__attribute__((__noreturn__))
static void SignalMutexAndExitThread(_MCFCRT_Mutex *restrict pMutex, MopthreadControl *restrict pControl, void (*pfnModifier)(void *, intptr_t), intptr_t nContext){
	if(pControl){
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
			_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
			if(__atomic_sub_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED) == 0){
				_MCFCRT_CloseThread(pControl->hThread);
				_MCFCRT_free(pControl);
			}
			break;
		default:
			_MCFCRT_ASSERT(false);
		}
	}
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

	_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	SignalMutexAndExitThread(&g_vMopthreadMutex, pControl, _MCFCRT_NULLPTR, 0);
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
		__builtin_memcpy(pControl->abyParams, pParams, uSizeOfParams);
	} else {
		__builtin_memset(pControl->abyParams, 0, uSizeOfParams);
	}
	if(bJoinable){
		pControl->eState = kStateJoinable;
	} else {
		pControl->eState = kStateDetached;
	}
	__atomic_store_n(&(pControl->uRefCount), 1, __ATOMIC_RELAXED);
	_MCFCRT_InitializeConditionVariable(&(pControl->condTermination));

	uintptr_t uTid;
	const _MCFCRT_ThreadHandle hThread = _MCFCRT_CreateNativeThread(&MopthreadProcNative, pControl, bJoinable, &uTid);
	if(!hThread){
		_MCFCRT_free(pControl);
		return 0;
	}
	pControl->uTid    = uTid;
	pControl->hThread = hThread;

	if(bJoinable){
		_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
		{
			_MCFCRT_AvlAttach(&g_avlMopthreadControls, (_MCFCRT_AvlNodeHeader *)pControl, &MopthreadControlComparatorNodes);
		}
		_MCFCRT_SignalMutex(&g_vMopthreadMutex);

		_MCFCRT_ResumeThread(hThread);
	}
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

	_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlMopthreadControls, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
	SignalMutexAndExitThread(&g_vMopthreadMutex, pControl, pfnModifier, nContext);
}
bool __MCFCRT_MopthreadJoin(uintptr_t uTid, void *restrict pParams){
	bool bSuccess = false;

	_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlMopthreadControls, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateJoining;
				do {
					_MCFCRT_WaitForConditionVariableForever(&(pControl->condTermination), &UnlockCallbackNative, &RelockCallbackNative, (intptr_t)&g_vMopthreadMutex, 0);
				} while(pControl->eState != kStateJoined);
				bSuccess = true;
				_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
				_MCFCRT_WaitForThreadForever(pControl->hThread);
				if(pParams){
					__builtin_memcpy(pParams, pControl->abyParams, pControl->uSizeOfParams);
				}
				if(__atomic_sub_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED) == 0){
					_MCFCRT_CloseThread(pControl->hThread);
					_MCFCRT_free(pControl);
				}
				break;
			case kStateZombie:
				pControl->eState = kStateJoined;
				bSuccess = true;
				_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
				_MCFCRT_WaitForThreadForever(pControl->hThread);
				if(pParams){
					__builtin_memcpy(pParams, pControl->abyParams, pControl->uSizeOfParams);
				}
				if(__atomic_sub_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED) == 0){
					_MCFCRT_CloseThread(pControl->hThread);
					_MCFCRT_free(pControl);
				}
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
	_MCFCRT_SignalMutex(&g_vMopthreadMutex);

	return bSuccess;
}
bool __MCFCRT_MopthreadDetach(uintptr_t uTid){
	bool bSuccess = false;

	_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlMopthreadControls, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
		if(pControl){
			switch(pControl->eState){
			case kStateJoinable:
				pControl->eState = kStateDetached;
				bSuccess = true;
				break;
			case kStateZombie:
				pControl->eState = kStateJoined;
				bSuccess = true;
				_MCFCRT_AvlDetach((_MCFCRT_AvlNodeHeader *)pControl);
				if(__atomic_sub_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED) == 0){
					_MCFCRT_CloseThread(pControl->hThread);
					_MCFCRT_free(pControl);
				}
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
	_MCFCRT_SignalMutex(&g_vMopthreadMutex);

	return bSuccess;
}

static const _MCFCRT_ThreadHandle *GetAttachedThreadAndLockRealHandle(uintptr_t uTid){
	const _MCFCRT_ThreadHandle *phThread = _MCFCRT_NULLPTR;

	_MCFCRT_WaitForMutexForever(&g_vMopthreadMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		MopthreadControl *const pControl = (MopthreadControl *)_MCFCRT_AvlFind(&g_avlMopthreadControls, (intptr_t)uTid, &MopthreadControlComparatorNodeKey);
		if(pControl){
			__atomic_add_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED);
			phThread = &(pControl->hThread);
		}
	}
	_MCFCRT_SignalMutex(&g_vMopthreadMutex);

	return phThread;
}
static void UnlockRealHandle(const _MCFCRT_ThreadHandle *phThread){
	MopthreadControl *const pControl = (void *)((char *)phThread - __builtin_offsetof(MopthreadControl, hThread));
	if(__atomic_sub_fetch(&(pControl->uRefCount), 1, __ATOMIC_RELAXED) == 0){
		_MCFCRT_CloseThread(pControl->hThread);
		_MCFCRT_free(pControl);
	}
}

const _MCFCRT_ThreadHandle *__MCFCRT_MopthreadLockHandle(uintptr_t uTid){
	const _MCFCRT_ThreadHandle *phThread;
	if(uTid == _MCFCRT_GetCurrentThreadId()){
		// This is a pseudo handle.
		phThread = &g_hPseudoSelfHandle;
	} else {
		phThread = GetAttachedThreadAndLockRealHandle(uTid);
	}
	return phThread;
}
void __MCFCRT_MopthreadUnlockHandle(const _MCFCRT_ThreadHandle *phThread){
	if(phThread == &g_hPseudoSelfHandle){
		// Pseudo handles need not be closed.
	} else {
		UnlockRealHandle(phThread);
	}
}
