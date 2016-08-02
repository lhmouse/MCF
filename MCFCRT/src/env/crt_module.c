// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "crt_module.h"
#include "mcfwin.h"

bool _MCFCRT_AtCrtModuleExit(_MCFCRT_AtModuleExitCallback pfnProc, intptr_t nContext){
	return _MCFCRT_AtModuleExit(pfnProc, nContext);
}

static volatile _MCFCRT_ExitType g_eExitType = 0;

_MCFCRT_ExitType __MCFCRT_GetExitType(void){
	return __atomic_load_n(&g_eExitType, __ATOMIC_RELAXED);
}

__attribute__((__noreturn__))
void _MCFCRT_ExitProcess(unsigned uExitCode, _MCFCRT_ExitType eExitType){
	if((eExitType != _MCFCRT_kExitTypeImmediate) && (eExitType != _MCFCRT_kExitTypeQuick)){
		eExitType = _MCFCRT_kExitTypeNormal;
	}

	if(__atomic_exchange_n(&g_eExitType, eExitType, __ATOMIC_RELAXED) != 0){
		TerminateThread(GetCurrentThread(), uExitCode);
		__builtin_unreachable();
	}

	if(eExitType == _MCFCRT_kExitTypeImmediate){
		TerminateProcess(GetCurrentProcess(), uExitCode);
	} else {
		ExitProcess(uExitCode);
	}
	__builtin_unreachable();
}
