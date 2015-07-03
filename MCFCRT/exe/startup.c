// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/eh_top.h"
#include "../env/heap.h"
#include "../env/last_error.h"
#include "../ext/unref_param.h"

// -static -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCF_ExeStartup 模块入口点。
_Noreturn __MCF_C_STDCALL __MCF_HAS_EH_TOP
DWORD __MCF_ExeStartup(LPVOID pReserved)
	__asm__("__MCF_ExeStartup");

_Noreturn __MCF_C_STDCALL __MCF_HAS_EH_TOP
DWORD __MCF_ExeStartup(LPVOID pReserved){
	UNREF_PARAM(pReserved);

	DWORD dwExitCode;

	if(!__MCF_CRT_HeapInit()){
		MCF_CRT_BailF(L"MCFCRT 堆初始化失败。\n\n错误代码：%lu", MCF_CRT_GetWin32LastError());
	}
	if(!__MCF_CRT_RegisterFrameInfo()){
		MCF_CRT_BailF(L"MCFCRT 异常处理程序初始化失败。\n\n错误代码：%lu", MCF_CRT_GetWin32LastError());
	}

	__MCF_EH_TOP_BEGIN
	{
		if(!__MCF_CRT_BeginModule()){
			MCF_CRT_BailF(L"MCFCRT 初始化失败。\n\n错误代码：%lu", MCF_CRT_GetWin32LastError());
		}
		dwExitCode = MCFMain();
		__MCF_CRT_EndModule();
	}
	__MCF_EH_TOP_END

	__MCF_CRT_UnregisterFrameInfo();
	__MCF_CRT_HeapUninit();

	ExitProcess(dwExitCode);
	__builtin_trap();
}

// 线程局部存储（TLS）目录，用于执行 TLS 的析构函数。
__attribute__((__section__(".tls$@@@"), __used__))
const unsigned char _tls_start;
__attribute__((__section__(".tls$___"), __used__))
const unsigned char _tls_end;
__attribute__((__section__(".tls"), __used__))
const DWORD _tls_index = 0;

__attribute__((__section__(".CRT$@@@"), __used__))
const PIMAGE_TLS_CALLBACK _callback_start = &__MCF_CRT_TlsCallback;
__attribute__((__section__(".CRT$___"), __used__))
const PIMAGE_TLS_CALLBACK _callback_end = nullptr;

__attribute__((__section__(".tls"), __used__))
const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData	= (UINT_PTR)&_tls_start,
	.EndAddressOfRawData	= (UINT_PTR)&_tls_end,
	.AddressOfIndex			= (UINT_PTR)&_tls_index,
	.AddressOfCallBacks		= (UINT_PTR)&_callback_start,
	.SizeOfZeroFill			= 0,
	.Characteristics		= 0,
};
