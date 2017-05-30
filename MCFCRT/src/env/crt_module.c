// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "crt_module.h"
#include "mcfwin.h"
#include "mutex.h"
#include "heap.h"
#include "bail.h"

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtQuickExitCallbackBlock {
	struct tagAtQuickExitCallbackBlock *pPrev;

	size_t uSize;
	struct {
		_MCFCRT_AtCrtModuleExitCallback pfnProc;
		intptr_t nContext;
	} aCallbacks[CALLBACKS_PER_BLOCK];
} AtQuickExitCallbackBlock;

static _MCFCRT_Mutex              g_vAtQuickExitMutex      = { 0 };
static AtQuickExitCallbackBlock * g_pAtQuickExitLast       = _MCFCRT_NULLPTR;

static volatile bool              g_bAtQuickExitSpareInUse = false;
static AtQuickExitCallbackBlock   g_vAtQuickExitSpare;

__attribute__((__noinline__))
static void PumpAtModuleQuickExit(void){
	_MCFCRT_WaitForMutexForever(&g_vAtQuickExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	for(;;){
		AtQuickExitCallbackBlock *const pBlock = g_pAtQuickExitLast;
		if(!pBlock){
			break;
		}

		AtQuickExitCallbackBlock *const pPrev = pBlock->pPrev;
		g_pAtQuickExitLast = pPrev;

		_MCFCRT_SignalMutex(&g_vAtQuickExitMutex);
		{
			for(size_t i = pBlock->uSize; i != 0; --i){
				(*(pBlock->aCallbacks[i - 1].pfnProc))(pBlock->aCallbacks[i - 1].nContext);
			}
			if(pBlock == &g_vAtQuickExitSpare){
				__atomic_store_n(&g_bAtQuickExitSpareInUse, false, __ATOMIC_RELEASE);
			} else {
				_MCFCRT_free(pBlock);
			}
		}
		_MCFCRT_WaitForMutexForever(&g_vAtQuickExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	}
	_MCFCRT_SignalMutex(&g_vAtQuickExitMutex);
}

bool _MCFCRT_AtCrtModuleQuickExit(_MCFCRT_AtCrtModuleExitCallback pfnProc, intptr_t nContext){
	_MCFCRT_WaitForMutexForever(&g_vAtQuickExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		AtQuickExitCallbackBlock *pBlock = g_pAtQuickExitLast;
		if(!pBlock || (pBlock->uSize >= CALLBACKS_PER_BLOCK)){
			if((__atomic_load_n(&g_bAtQuickExitSpareInUse, __ATOMIC_ACQUIRE) == false) && (__atomic_exchange_n(&g_bAtQuickExitSpareInUse, true, __ATOMIC_ACQ_REL) == false)){
				pBlock = &g_vAtQuickExitSpare;
			} else {
				_MCFCRT_SignalMutex(&g_vAtQuickExitMutex);
				{
					pBlock = _MCFCRT_malloc(sizeof(AtQuickExitCallbackBlock));
					if(!pBlock){
						SetLastError(ERROR_NOT_ENOUGH_MEMORY);
						return false;
					}
				}
				_MCFCRT_WaitForMutexForever(&g_vAtQuickExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			}
			pBlock->pPrev = g_pAtQuickExitLast;
			pBlock->uSize = 0;
			g_pAtQuickExitLast = pBlock;
		}
		const size_t uIndex = (pBlock->uSize)++;
		pBlock->aCallbacks[uIndex].pfnProc = pfnProc;
		pBlock->aCallbacks[uIndex].nContext = nContext;
	}
	_MCFCRT_SignalMutex(&g_vAtQuickExitMutex);
	return true;
}

void __MCFCRT_DiscardCrtModuleQuickExitCallbacks(void){
	_MCFCRT_WaitForMutexForever(&g_vAtQuickExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	for(;;){
		AtQuickExitCallbackBlock *const pBlock = g_pAtQuickExitLast;
		if(!pBlock){
			break;
		}

		AtQuickExitCallbackBlock *const pPrev = pBlock->pPrev;
		g_pAtQuickExitLast = pPrev;

		if(pBlock == &g_vAtQuickExitSpare){
			__atomic_store_n(&g_bAtQuickExitSpareInUse, false, __ATOMIC_RELEASE);
		} else {
			_MCFCRT_free(pBlock);
		}
	}
	_MCFCRT_SignalMutex(&g_vAtQuickExitMutex);
}

static volatile DWORD s_dwExitingThreadId = 0;

static void CheckExitingThread(unsigned uExitCode){
	const DWORD dwCurrentThreadId = GetCurrentThreadId();
	DWORD dwOldExitingThreadId = 0;
	__atomic_compare_exchange_n(&s_dwExitingThreadId, &dwOldExitingThreadId, dwCurrentThreadId, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
	if(dwOldExitingThreadId != 0){
		if(dwOldExitingThreadId == dwCurrentThreadId){
			_MCFCRT_Bail(L"_MCFCRT_QuickExit() 或 _MCFCRT_Exit() 被递归调用。\n"
				"这可能是在被 static 或 thread_local 修饰的对象的析构函数中调用了 exit() 或 quick_exit() 导致的。");
		}
		ExitThread(uExitCode);
		__builtin_trap();
	}
}

_Noreturn void _MCFCRT_ImmediateExit(unsigned uExitCode){
	TerminateProcess(GetCurrentProcess(), uExitCode);
	__builtin_trap();
}
_Noreturn void _MCFCRT_QuickExit(unsigned uExitCode){
	CheckExitingThread(uExitCode);
	PumpAtModuleQuickExit();
	_MCFCRT_ImmediateExit(uExitCode);
	__builtin_trap();
}
_Noreturn void _MCFCRT_Exit(unsigned uExitCode){
	CheckExitingThread(uExitCode);
	ExitProcess(uExitCode);
	__builtin_trap();
}
