// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#define WIN32_LEAN_AND_MEAN

#include "../MCFCRT.h"
#include <windows.h>

// -static -nostartfiles -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,-lmcfcrt,-lstdc++,-lgcc,-lgcc_eh,-lmingwex,-lmcfcrt

extern unsigned long __MCF_CRT_ExeInitializeArgV();
extern void __MCF_CRT_ExeUninitializeArgV();

extern unsigned int MCFMain();

#pragma GCC optimize "-fno-function-sections"

static __attribute__((__cdecl__, __used__, __noreturn__)) int AlignedStartup(){
#ifdef __SEH__
	__asm__ __volatile__(
		"seh_try: \n"
	);
#endif

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

#ifdef __SEH__
	__asm__ __volatile__(
		"seh_except: \n"
		"	.seh_handler __C_specific_handler, @except \n"
		"	.seh_handlerdata \n"
		"	.long 1 \n"
		"	.rva seh_try, seh_except, _gnu_exception_handler, seh_except \n"
		"	.text \n"
	);
#endif

	ExitProcess(dwExitCode);
	__builtin_trap();
}

__asm__(
	"	.text \n"
	"	.align 16 \n"
	".global __MCF_ExeStartup \n"
	"__MCF_ExeStartup: \n"
#ifdef _WIN64
	"	and rsp, -0x10 \n"
	"	sub rsp, 0x10 \n"
	"	call AlignedStartup \n"
#else
	"	and esp, -0x10 \n"
	"	call _AlignedStartup \n"
#endif
);
