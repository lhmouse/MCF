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

static void Run_global_ctors(void){
	const Pvfv *begin, *end;
	end = begin = __CTOR_LIST__ + 1;
	while(*end){
		++end;
	}
	while(begin != end){
		--end;
		(*end)();
	}
}
static void Run_global_dtors(void){
	const Pvfv *begin, *end;
	end = begin = __DTOR_LIST__ + 1;
	while(*end){
		++end;
	}
	while(begin != end){
		(*begin)();
		++begin;
	}
}

static __MCFCRT_AtExitQueue g_atexit_queue = { 0 };

static void Dispose_atexit_queue(void){
	__MCFCRT_AtExitElement elem;
	while(__MCFCRT_AtExitQueuePop(&elem, &g_atexit_queue)){
		(*(elem.__proc))(elem.__context);
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
	__MCFCRT_TlsUninit();
	Run_global_dtors();
	__MCFCRT_libsupcxx_cleanup();
}

bool _MCFCRT_AtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	__MCFCRT_AtExitElement elem = { pfnProc, nContext };
	return __MCFCRT_AtExitQueuePush(&g_atexit_queue, &elem);
}
