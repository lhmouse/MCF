// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "crt_module.h"
#include "mcfwin.h"

bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	return _MCFCRT_AtModuleExit(pfnProc, nContext);
}

typedef enum tagExitType {
	kExitNone   = 0,
	kExitNormal = 1,
	kExitQuick  = 2,
} ExitType;

static volatile ExitType g_eExitType = kExitNone;

bool __MCFCRT_IsQuickExitInProgress(void){
	return __atomic_load_n(&g_eExitType, __ATOMIC_RELAXED) == kExitQuick;
}
bool __MCFCRT_ExitProcess(int nExitCode, bool bIsQuick){
	if(!((__atomic_load_n(&g_eExitType, __ATOMIC_RELAXED) == kExitNone) && (__atomic_exchange_n(&g_eExitType, !bIsQuick ? kExitNormal : kExitQuick, __ATOMIC_RELAXED) == kExitNone))){
		TerminateThread(GetCurrentThread(), (DWORD)nExitCode);
		__builtin_trap();
	}
	ExitProcess((DWORD)nExitCode);
	__builtin_trap();
}
