// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "../MCFCRT.h"
#include <windows.h>

// -static -nostartfiles -Wl,-e__MCFExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,-lmcfcrt,-lstdc++,-lgcc,-lgcc_eh,-lmingwex,-lmcfcrt

__MCF_CRT_EXTERN void __MCF_CRTExeInitializeArgV();
__MCF_CRT_EXTERN void __MCF_CRTExeUninitializeArgV();

extern unsigned int __cdecl MCFMain();

extern __attribute__((noreturn)) void __stdcall __MCFExeStartup(){
	DWORD dwExitCode;

	if((dwExitCode = __MCF_CRTBegin()) == ERROR_SUCCESS){
		if((dwExitCode = __MCF_CRTThreadInitialize()) == ERROR_SUCCESS){
			__MCF_CRTExeInitializeArgV();
			MCFMain();
			__MCF_CRTExeUninitializeArgV();

			__MCF_CRTThreadUninitialize();
		}
		__MCF_CRTEnd();
	}

	ExitProcess(dwExitCode);
	for(;;);
}

#ifndef __amd64__
extern __attribute__((noreturn, alias("__MCFExeStartup@0"))) void __cdecl _MCFExeStartup();
#endif
