// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MODULE_INLINE_OR_EXTERN     extern inline
#include "module.h"
#include "../env/mcfwin.h"
#include "../env/mutex.h"
#include "../env/heap.h"

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtExitCallbackBlock {
	struct tagAtExitCallbackBlock *pPrev;

	size_t uSize;
	struct {
		_MCFCRT_AtModuleExitCallback pfnProc;
		intptr_t nContext;
	} aCallbacks[CALLBACKS_PER_BLOCK];
} AtExitCallbackBlock;

static _MCFCRT_Mutex         g_vAtExitMutex      = { 0 };
static AtExitCallbackBlock * g_pAtExitLast       = nullptr;

static volatile bool         g_bAtExitSpareInUse = false;
static AtExitCallbackBlock   g_vAtExitSpare;

__attribute__((__noinline__))
static void PumpAtModuleExit(void){
	_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	for(;;){
		AtExitCallbackBlock *const pBlock = g_pAtExitLast;
		if(!pBlock){
			break;
		}

		AtExitCallbackBlock *const pPrev = pBlock->pPrev;
		g_pAtExitLast = pPrev;

		_MCFCRT_SignalMutex(&g_vAtExitMutex);
		{
			for(size_t i = pBlock->uSize; i != 0; --i){
				(*(pBlock->aCallbacks[i - 1].pfnProc))(pBlock->aCallbacks[i - 1].nContext);
			}
			if(pBlock == &g_vAtExitSpare){
				__atomic_store_n(&g_bAtExitSpareInUse, false, __ATOMIC_RELEASE);
			} else {
				_MCFCRT_free(pBlock);
			}
		}
		_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	}
	_MCFCRT_SignalMutex(&g_vAtExitMutex);
}

typedef void (*StaticConstructorDestructorProc)(void);

extern const StaticConstructorDestructorProc __CTOR_LIST__[];
extern const StaticConstructorDestructorProc __DTOR_LIST__[];

extern void __MCFCRT_libsupcxx_freeres(void);

static void CallStaticConstructors(void){
	const StaticConstructorDestructorProc *const pfnBegin = __CTOR_LIST__ + 1;

	const StaticConstructorDestructorProc *pfnCurrent = pfnBegin;
	while(*pfnCurrent){
		++pfnCurrent;
	}
	while(pfnCurrent != pfnBegin){
		--pfnCurrent;
		(*pfnCurrent)();
	}
}
static void CallStaticDestructors(void){
	const StaticConstructorDestructorProc *const pfnBegin = __DTOR_LIST__ + 1;

	const StaticConstructorDestructorProc *pfnCurrent = pfnBegin;
	while(*pfnCurrent){
		(*pfnCurrent)();
		++pfnCurrent;
	}
}

bool __MCFCRT_ModuleInit(void){
	CallStaticConstructors();
	return true;
}
void __MCFCRT_ModuleUninit(void){
	PumpAtModuleExit();
	CallStaticDestructors();
	__MCFCRT_libsupcxx_freeres();
}

bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		AtExitCallbackBlock *pBlock = g_pAtExitLast;
		if(!pBlock || (pBlock->uSize >= CALLBACKS_PER_BLOCK)){
			if((__atomic_load_n(&g_bAtExitSpareInUse, __ATOMIC_ACQUIRE) == false) && (__atomic_exchange_n(&g_bAtExitSpareInUse, true, __ATOMIC_ACQ_REL) == false)){
				pBlock = &g_vAtExitSpare;
			} else {
				_MCFCRT_SignalMutex(&g_vAtExitMutex);
				{
					pBlock = _MCFCRT_malloc(sizeof(AtExitCallbackBlock));
					if(!pBlock){
						SetLastError(ERROR_NOT_ENOUGH_MEMORY);
						return false;
					}
				}
				_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			}
			pBlock->pPrev = g_pAtExitLast;
			pBlock->uSize = 0;
			g_pAtExitLast = pBlock;
		}
		const size_t uIndex = (pBlock->uSize)++;
		pBlock->aCallbacks[uIndex].pfnProc = pfnProc;
		pBlock->aCallbacks[uIndex].nContext = nContext;
	}
	_MCFCRT_SignalMutex(&g_vAtExitMutex);
	return true;
}
