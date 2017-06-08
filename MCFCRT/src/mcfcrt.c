// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#define __MCFCRT_NO_GENERAL_INCLUDES    1
#include "mcfcrt.h"
#include "env/xassert.h"
#include "env/standard_streams.h"
#include "env/heap_debug.h"
#include "env/_mopthread.h"
#include "env/crt_module.h"

static ptrdiff_t g_nCounter = 0;

bool __MCFCRT_InitRecursive(void){
	ptrdiff_t nCounter = g_nCounter;
	if(nCounter == 0){
		if(!__MCFCRT_StandardStreamsInit()){
			return false;
		}
		if(!__MCFCRT_HeapDebugInit()){
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
		if(!__MCFCRT_MopthreadInit()){
			__MCFCRT_HeapDebugUninit();
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
		// Add more initialization...
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
		// Add more uninitialization...
		__MCFCRT_MopthreadUninit();
		__MCFCRT_DiscardCrtModuleQuickExitCallbacks();
		__MCFCRT_HeapDebugUninit();
		__MCFCRT_StandardStreamsUninit();
	}
}
