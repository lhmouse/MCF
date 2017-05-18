// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "mcfcrt.h"
#include "env/standard_streams.h"
#include "env/heap.h"
#include "env/heap_dbg.h"
#include "env/tls.h"
#include "pre/module.h"

static ptrdiff_t g_nCounter = 0;

bool __MCFCRT_InitRecursive(void){
	ptrdiff_t nCounter = g_nCounter;
	if(nCounter == 0){
		if(!__MCFCRT_StandardStreamsInit()){
			return false;
		}
		if(!__MCFCRT_HeapInit()){
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
		if(!__MCFCRT_HeapDbgInit()){
			__MCFCRT_HeapUninit();
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
		if(!__MCFCRT_TlsInit()){
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
		if(!__MCFCRT_ModuleInit()){
			__MCFCRT_TlsUninit();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
	}
	++nCounter;
	g_nCounter = nCounter;
	return true;
}
void __MCFCRT_UninitRecursive(void){
	ptrdiff_t nCounter = g_nCounter;
	_MCFCRT_ASSERT(nCounter > 0);
	--nCounter;
	g_nCounter = nCounter;
	if(nCounter == 0){
		__MCFCRT_TlsUninit(); // Objects with thread local storage duration are destroyed before those with static storage duration.
		__MCFCRT_ModuleUninit();
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		__MCFCRT_StandardStreamsUninit();
	}
}
