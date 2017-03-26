// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "crt_module.h"
#include "../pre/module.h"
#include "mcfwin.h"
#include "mutex.h"
#include "heap.h"

#undef GetCurrentProcess
#define GetCurrentProcess()  ((HANDLE)-1)

#undef GetCurrentThread
#define GetCurrentThread()   ((HANDLE)-2)

bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtCrtModuleExitCallback pfnProc, intptr_t nContext){
	return _MCFCRT_AtModuleExit(pfnProc, nContext);
}

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtQuickExitCallbackBlock {
	struct tagAtQuickExitCallbackBlock *pPrev;

	size_t uSize;
	struct {
		_MCFCRT_AtModuleExitCallback pfnProc;
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

bool _MCFCRT_AtCrtModuleQuickExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
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

__attribute__((__noreturn__))
void _MCFCRT_ExitProcess(unsigned uExitCode, _MCFCRT_ExitType eExitType){
	static volatile bool s_bExiting = false;

	const bool bBailing = __atomic_exchange_n(&s_bExiting, true, __ATOMIC_RELAXED);
	if(bBailing){
		TerminateThread(GetCurrentThread(), uExitCode);
		__builtin_unreachable();
	}

	if(eExitType == _MCFCRT_kExitTypeQuick){
		PumpAtModuleQuickExit();
	}
	if(eExitType != _MCFCRT_kExitTypeNormal){
		TerminateProcess(GetCurrentProcess(), uExitCode);
	} else {
		ExitProcess(uExitCode);
	}
	__builtin_unreachable();
}
