// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "exe_decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/_eh_top.h"
#include "../ext/unref_param.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

#define MCF_SECTION(x)	__attribute__((__section__(x), __used__))

MCF_SECTION(".CRT$XL@") const PIMAGE_TLS_CALLBACK vCallbackAt	= &__MCF_CRT_TlsCallback;
MCF_SECTION(".CRT$XL_") const PIMAGE_TLS_CALLBACK vCallback_	= NULL;

DWORD _tls_index = 0;

MCF_SECTION(".tls$@@@") const unsigned char _tls_start	= 0;
MCF_SECTION(".tls$___") const unsigned char _tls_end	= 0;

MCF_SECTION(".tls") const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData	= (UINT_PTR)&_tls_start,
	.EndAddressOfRawData	= (UINT_PTR)&_tls_end,
	.AddressOfIndex			= (UINT_PTR)&_tls_index,
	.AddressOfCallBacks		= (UINT_PTR)&vCallbackAt,
	.SizeOfZeroFill			= 0,
	.Characteristics		= 0
};

_Noreturn DWORD __stdcall __MCF_ExeStartup(LPVOID pReserved)
	__asm__("__MCF_ExeStartup");

_Noreturn
__attribute__((__force_align_arg_pointer__)) __MCF_HAS_EH_TOP
DWORD __stdcall __MCF_ExeStartup(LPVOID pReserved){
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
