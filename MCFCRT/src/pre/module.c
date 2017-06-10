// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MODULE_INLINE_OR_EXTERN     extern inline
#include "module.h"
#include "tls.h"
#include "../env/_atexit_queue.h"

extern void _pei386_runtime_relocator(void);
extern void __MCFCRT_libsupcxx_cleanup(void);

typedef void (*Pvfv)(void);

extern const Pvfv __CTOR_LIST__[], __DTOR_LIST__[];

static void RunGlobalConstructors(void){
	const Pvfv *ppfnBegin, *ppfnEnd;
	ppfnEnd = ppfnBegin = __CTOR_LIST__ + 1;
	while(*ppfnEnd){
		++ppfnEnd;
	}
	while(ppfnBegin != ppfnEnd){
		--ppfnEnd;
		(**ppfnEnd)();
	}
}
static void RunGlobalDestructors(void){
	const Pvfv *ppfnBegin, *ppfnEnd;
	ppfnEnd = ppfnBegin = __DTOR_LIST__ + 1;
	while(*ppfnEnd){
		++ppfnEnd;
	}
	while(ppfnBegin != ppfnEnd){
		(**ppfnBegin)();
		++ppfnBegin;
	}
}

static __MCFCRT_AtExitQueue g_vAtExitQueue = __MCFCRT_ATEXIT_QUEUE_INIT;

static void Dispose_atexit_queue(void){
	__MCFCRT_AtExitElement vElement;
	while(__MCFCRT_AtExitQueuePop(&vElement, &g_vAtExitQueue)){
		__MCFCRT_AtExitQueueInvoke(&vElement);
	}
}

bool __MCFCRT_ModuleInit(void){
	_pei386_runtime_relocator();
	__MCFCRT_TlsInit();
	RunGlobalConstructors();
	return true;
}
void __MCFCRT_ModuleUninit(void){
	Dispose_atexit_queue();
	__MCFCRT_TlsUninit();
	RunGlobalDestructors();
	__MCFCRT_libsupcxx_cleanup();
}

bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	__MCFCRT_AtExitElement vElement = { pfnProc, nContext };
	return __MCFCRT_AtExitQueuePush(&g_vAtExitQueue, &vElement);
}
