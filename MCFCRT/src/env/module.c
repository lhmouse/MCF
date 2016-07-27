// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MODULE_INLINE_OR_EXTERN     extern inline
#include "module.h"
#include "mcfwin.h"
#include "mutex.h"
#include "heap.h"

typedef struct tagAtExitCallback {
	_MCFCRT_AtModuleExitCallback pfnProc;
	intptr_t nContext;
} AtExitCallback;

#define CALLBACKS_PER_BLOCK   64u

typedef struct tagAtExitCallbackBlock {
	struct tagAtExitCallbackBlock *pPrev;
	size_t uSize;
	AtExitCallback aCallbacks[CALLBACKS_PER_BLOCK];
} AtExitCallbackBlock;

static _MCFCRT_Mutex         g_vAtExitMutex = { 0 };
static AtExitCallbackBlock * g_pAtExitLast  = nullptr;

static void CrtAtModuleExitDestructor(void *pStorage){
	AtExitCallbackBlock *const pBlock = pStorage;
	for(size_t i = pBlock->uSize; i != 0; --i){
		const AtExitCallback *const pCallback = pBlock->aCallbacks + i - 1;
		const _MCFCRT_AtModuleExitCallback pfnProc = pCallback->pfnProc;
		const intptr_t nContext = pCallback->nContext;
		(*pfnProc)(nContext);
	}
}

__attribute__((__noinline__))
static void PumpAtModuleExit(void){
	for(;;){
		AtExitCallbackBlock *pBlock;
		{
			_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
			{
				pBlock = g_pAtExitLast;
				if(pBlock){
					AtExitCallbackBlock *const pPrev = pBlock->pPrev;
					g_pAtExitLast = pPrev;
				}
			}
			_MCFCRT_SignalMutex(&g_vAtExitMutex);
		}
		if(!pBlock){
			break;
		}

		CrtAtModuleExitDestructor(pBlock);
		_MCFCRT_free(pBlock);
	}
}

typedef void (*StaticConstructorDestructorProc)(void);

extern const StaticConstructorDestructorProc __CTOR_LIST__[];
extern const StaticConstructorDestructorProc __DTOR_LIST__[];

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
}

bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);
	{
		AtExitCallbackBlock *pBlock = g_pAtExitLast;
		if(!pBlock || (pBlock->uSize >= CALLBACKS_PER_BLOCK)){
			_MCFCRT_SignalMutex(&g_vAtExitMutex);
			{
				pBlock = _MCFCRT_malloc(sizeof(AtExitCallbackBlock));
				if(!pBlock){
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return false;
				}
				pBlock->uSize = 0;
			}
			_MCFCRT_WaitForMutexForever(&g_vAtExitMutex, _MCFCRT_MUTEX_SUGGESTED_SPIN_COUNT);

			pBlock->pPrev = g_pAtExitLast;
			g_pAtExitLast = pBlock;
		}
		AtExitCallback *const pCallback = pBlock->aCallbacks + ((pBlock->uSize)++);
		pCallback->pfnProc = pfnProc;
		pCallback->nContext = nContext;
	}
	_MCFCRT_SignalMutex(&g_vAtExitMutex);
	return true;
}
