// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "mcfcrt.h"

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
		__MCFCRT_DiscardCrtModuleQuickExitCallbacks();
		__MCFCRT_HeapDebugUninit();
		__MCFCRT_StandardStreamsUninit();
	}
}
