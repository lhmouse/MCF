// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "../MCFCRT.h"
#include <windows.h>

// -static -nostartfiles -Wl,-e__MCFExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,-lmcfcrt,-lstdc++,-lgcc,-lgcc_eh,-lmingwex,-lmcfcrt

extern unsigned long __MCF_CRT_ExeInitializeArgV();
extern void __MCF_CRT_ExeUninitializeArgV();

extern unsigned int MCFMain();

static int __attribute__((stdcall, unused)) MCFExeStartup(){
	DWORD dwExitCode;

#define INIT(exp)		if((dwExitCode = (exp)) == ERROR_SUCCESS){ ((void)0)
#define CLEANUP(exp)	(exp); } ((void)0)

	INIT(__MCF_CRT_Begin());
	INIT(__MCF_CRT_ThreadInitialize());
	INIT(__MCF_CRT_ExeInitializeArgV());

	dwExitCode = MCFMain();

	CLEANUP(__MCF_CRT_ExeUninitializeArgV());
	CLEANUP(__MCF_CRT_ThreadUninitialize());
	CLEANUP(__MCF_CRT_End());

	ExitProcess(dwExitCode);
	return (int)dwExitCode;		// 这有什么意义吗？
}

#ifdef __amd64__
extern __attribute__((alias("MCFExeStartup"))) void __cdecl __MCFExeStartup();
#else
extern __attribute__((alias("MCFExeStartup@0"))) void __cdecl _MCFExeStartup();
#endif
