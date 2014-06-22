// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/_eh_top.h"
#include "../ext/unref_param.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

#pragma GCC optimize "-fno-function-sections"
#pragma GCC optimize "-fno-data-sections"

#define MCF_SECTION(x)	__attribute__((__section__(x), __used__))

MCF_SECTION(".CRT$XL@") PIMAGE_TLS_CALLBACK vCallbackAt	= &__MCF_CRT_TlsCallback;
MCF_SECTION(".CRT$XL_") PIMAGE_TLS_CALLBACK vCallback_	= NULL;

DWORD _tls_index = 0;

MCF_SECTION(".tls$@@@") unsigned char _tls_start	= 0;
MCF_SECTION(".tls$___") unsigned char _tls_end		= 0;

MCF_SECTION(".tls") const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData	= (UINT_PTR)&_tls_start,
	.EndAddressOfRawData	= (UINT_PTR)&_tls_end,
	.AddressOfIndex			= (UINT_PTR)&_tls_index,
	.AddressOfCallBacks		= (UINT_PTR)&vCallbackAt,
	.SizeOfZeroFill			= 0,
	.Characteristics		= 0
};

extern unsigned int MCFMain();

static __attribute__((__stdcall__, __used__, __noreturn__)) int AlignedStartup(LPVOID pReserved){
	DWORD dwExitCode;
	__MCF_EH_TOP_BEGIN
	{
		UNREF_PARAM(pReserved);

		if(!__MCF_CRT_BeginModule()){
			MCF_CRT_BailF(L"MCFCRT 初始化失败。\n\n错误代码：%lu", (unsigned long)GetLastError());
		}
		dwExitCode = MCFMain();
		__MCF_CRT_EndModule();
	}
	__MCF_EH_TOP_END
	ExitProcess(dwExitCode);
	__builtin_trap();
}

#ifdef _WIN64
extern __attribute__((__stdcall__, __noreturn__, __alias__("AlignedStartup")))
	int __MCF_ExeStartup(LPVOID) __asm__("__MCF_ExeStartup");
#else
__asm__(
	"	.text \n"
	"	.align 16 \n"
	".global __MCF_ExeStartup \n"
	"__MCF_ExeStartup: \n"
	"	mov eax, dword ptr[esp + 4] \n"
	"	and esp, -0x10 \n"
	"	sub esp, 0x10 \n"
	"	mov dword ptr[esp], eax \n"
	"	call _AlignedStartup@4 \n"
	"	int3 \n"
);
#endif
