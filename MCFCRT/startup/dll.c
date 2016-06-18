// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "generic.h"
#include "../env/mcfwin.h"
#include "../env/_eh_top.h"

// -static -Wl,-e__MCFCRT_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_DllStartup 模块入口点。
__MCFCRT_C_STDCALL
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCFCRT_DllStartup");

__attribute__((__used__))
int __MCFCRT_do_not_link_exe_startup_code_and_dll_startup_code_together = 0;

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP __attribute__((__noinline__))
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet;

	__MCFCRT_EH_TOP_BEGIN
	{
		bRet = __MCFCRT_TlsCallbackGeneric((void *)hDll, (unsigned)dwReason, !pReserved);
	}
	__MCFCRT_EH_TOP_END

	return bRet;
}
