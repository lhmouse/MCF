// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "exe.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"
#include "module.h"
#include "../env/crt_module.h"
#include "../env/standard_streams.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"

// -Wl,-e@__MCFCRT_ExeStartup
__MCFCRT_C_STDCALL
extern DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("@__MCFCRT_ExeStartup");

static void AtCrtModuleExitProc(intptr_t nContext);

__MCFCRT_C_STDCALL
static BOOL CrtCtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	static const wchar_t kKilledMessage[] = L"进程被 Ctrl-C 终止，因为没有找到自定义的 _MCFCRT_OnCtrlEvent() 响应函数。";
	_MCFCRT_WriteStandardErrorText(kKilledMessage, sizeof(kKilledMessage) / sizeof(wchar_t) - 1, true);

	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}

static bool RealStartup(unsigned uReason){
	static bool s_bInitialized = false;

	bool bRet = true;

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		if(!s_bInitialized){
			bRet = __MCFCRT_ModuleInit();
			if(!bRet){
				goto jCleanup03;
			}
			bRet = SetConsoleCtrlHandler(&CrtCtrlHandler, true);
			if(!bRet){
				goto jCleanup98;
			}
			bRet = _MCFCRT_AtCrtModuleExit(&AtCrtModuleExitProc, 0);
			if(!bRet){
				goto jCleanup99;
			}
			s_bInitialized = true;
		}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if(s_bInitialized){
			s_bInitialized = false;
	jCleanup99:
			SetConsoleCtrlHandler(&CrtCtrlHandler, false);
	jCleanup98:
			__MCFCRT_ModuleUninit();
	jCleanup03:
			;
		}
		break;
	}

	return bRet;
}

static void AtCrtModuleExitProc(intptr_t nContext){
	(void)nContext;

	RealStartup(DLL_PROCESS_DETACH);
}

__MCFCRT_C_STDCALL __attribute__((__noreturn__))
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	DWORD dwExitCode;

	__MCFCRT_SEH_TOP_BEGIN
	{
		if(!RealStartup(DLL_PROCESS_ATTACH)){
			_MCFCRT_Bail(L"MCFCRT 卸载函数注册失败。");
		}

		dwExitCode = _MCFCRT_Main();
	}
	__MCFCRT_SEH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}
