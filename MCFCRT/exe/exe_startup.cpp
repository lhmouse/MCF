// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "exe_decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/eh_top.h"
#include "../env/last_error.h"

// -static -Wl,-e__MCF_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

extern "C" {

[[noreturn]] __MCF_C_STDCALL __MCF_HAS_EH_TOP
DWORD __MCF_ExeStartup(LPVOID)
	__asm__("__MCF_ExeStartup");

[[noreturn]] __MCF_C_STDCALL __MCF_HAS_EH_TOP
DWORD __MCF_ExeStartup(LPVOID){
	DWORD dwExitCode;
	__MCF_EH_TOP_BEGIN
	{
		if(!::__MCF_CRT_BeginModule()){
			::MCF_CRT_BailF(L"MCFCRT 初始化失败。\n\n错误代码：%lu", ::MCF_CRT_GetWin32LastError());
		}
		dwExitCode = ::MCFMain();
		::__MCF_CRT_EndModule();
	}
	__MCF_EH_TOP_END
	::ExitProcess(dwExitCode);
	__builtin_trap();
}

}
