// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "generic.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"

// -Wl,-e@__MCFCRT_DllStartup

// __MCFCRT_DllStartup 模块入口点。
__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

__MCFCRT_C_STDCALL __attribute__((__noinline__))
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	return __MCFCRT_TlsCallbackGeneric((void *)hDll, (unsigned)dwReason, !pReserved);
}
