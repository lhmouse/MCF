// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#define __MCFCRT_MODULE_INLINE_OR_EXTERN     extern inline
#include "module.h"
#include "tls.h"
#include "../env/_atexit_queue.h"

extern void _pei386_runtime_relocator(void);
extern void __MCFCRT_libsupcxx_cleanup(void);

extern const intptr_t __CTOR_LIST__[], __CTOR_END__[];
extern const intptr_t __DTOR_LIST__[], __DTOR_END__[];

static void Run_global_ctors(void){
	for(const intptr_t *ptr = __CTOR_LIST__ + 1; ptr != __CTOR_END__; ++ptr){
		const intptr_t value = *ptr;
		if(value <= 0){
			continue;
		}
		(*(void (*)(void))value)();
	}
}
static void Run_global_dtors(void){
	for(const intptr_t *ptr = __DTOR_END__ - 1; ptr != __DTOR_LIST__; --ptr){
		const intptr_t value = *ptr;
		if(value <= 0){
			continue;
		}
		(*(void (*)(void))value)();
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
	Run_global_ctors();
	return true;
}
void __MCFCRT_ModuleUninit(void){
	Dispose_atexit_queue();
	Run_global_dtors();
	__MCFCRT_TlsUninit();
	__MCFCRT_libsupcxx_cleanup();
}

bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	const __MCFCRT_AtExitElement vElement = { pfnProc, nContext };
	return __MCFCRT_AtExitQueuePush(&g_vAtExitQueue, &vElement);
}
