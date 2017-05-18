// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "exe.h"
#include "../mcfcrt.h"
#include "../env/_seh_top.h"
#include "../env/_fpu.h"
#include "module.h"
#include "../env/mcfwin.h"
#include "../env/crt_module.h"
#include "../env/standard_streams.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"

// -Wl,-e@__MCFCRT_ExeStartup
_Noreturn __MCFCRT_C_STDCALL
extern DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("@__MCFCRT_ExeStartup");

__MCFCRT_C_STDCALL
static BOOL CtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	static const wchar_t kKilledMessage[] = L"进程被 Ctrl-C 终止，因为没有找到自定义的 _MCFCRT_OnCtrlEvent() 响应函数。";
	_MCFCRT_WriteStandardErrorText(kKilledMessage, sizeof(kKilledMessage) / sizeof(wchar_t) - 1, true);
	_MCFCRT_ExitProcess(1, _MCFCRT_kExitTypeQuick);
}

static void ExeCleanup(intptr_t nUnused){
	(void)nUnused;

	SetConsoleCtrlHandler(&CtrlHandler, false);
	__MCFCRT_ModuleUninit();
	__MCFCRT_UninitRecursive();
}

_Noreturn __MCFCRT_C_STDCALL
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	__MCFCRT_FpuInitialize();

	unsigned uExitCode = 3;

	__MCFCRT_SEH_TOP_BEGIN
	{
		if(!__MCFCRT_InitRecursive()){
			_MCFCRT_Bail(L"MCFCRT 初始化失败。");
		}
		if(!__MCFCRT_ModuleInit()){
			_MCFCRT_Bail(L"MCFCRT 可执行模块初始化失败。");
		}
		if(!SetConsoleCtrlHandler(&CtrlHandler, true)){
			_MCFCRT_Bail(L"MCFCRT 可执行模块 Ctrl-C 响应回调函数注册失败。");
		}
		if(!_MCFCRT_AtCrtModuleExit(&ExeCleanup, 0)){
			_MCFCRT_Bail(L"MCFCRT 清理回调函数注册失败。");
		}
		uExitCode = _MCFCRT_Main();
	}
	__MCFCRT_SEH_TOP_END

	_MCFCRT_ExitProcess(uExitCode, _MCFCRT_kExitTypeNormal);
}
