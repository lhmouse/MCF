// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "exe.h"
#include "generic.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"
#include "../env/standard_streams.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/module.h"

// -Wl,-e@__MCFCRT_ExeStartup

// __MCFCRT_ExeStartup 模块入口点。
__MCFCRT_C_STDCALL
extern DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("@__MCFCRT_ExeStartup");

__MCFCRT_C_STDCALL
static BOOL CrtCtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	static const wchar_t kKilledMessage[] = L"进程被 Ctrl-C 终止，因为没有找到自定义的 _MCFCRT_OnCtrlEvent() 响应函数。";
	_MCFCRT_WriteStandardErrorString(kKilledMessage, sizeof(kKilledMessage) / sizeof(wchar_t) - 1, true);

	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}
static void RemoveConsoleControlHandler(intptr_t nContext){
	(void)nContext;

	SetConsoleCtrlHandler(&CrtCtrlHandler, false);
}

__MCFCRT_C_STDCALL __attribute__((__noinline__, __noreturn__))
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	// 忽略错误。
	if(SetConsoleCtrlHandler(&CrtCtrlHandler, true)){
		_MCFCRT_AtModuleExit(&RemoveConsoleControlHandler, 0);
	}

	DWORD dwExitCode;

	__MCFCRT_SEH_TOP_BEGIN
	{
		dwExitCode = _MCFCRT_Main();
	}
	__MCFCRT_SEH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}
