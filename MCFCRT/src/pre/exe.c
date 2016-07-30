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

static void CrtModuleUninitCascading(intptr_t nContext);

static void RealTlsCallback(void *pInstance, unsigned uReason, bool bDynamic){
	(void)pInstance;
	(void)bDynamic;

	static bool s_bInitialized = false;

	bool bRet = true;

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		if(s_bInitialized){
			break;
		}
		bRet = __MCFCRT_ModuleInit();
		if(!bRet){
			goto jCleanup03;
		}
		bRet = _MCFCRT_AtCrtModuleExit(&CrtModuleUninitCascading, bDynamic);
		if(!bRet){
			goto jCleanup99;
		}
		SetConsoleCtrlHandler(&CrtCtrlHandler, true); // 忽略错误。
		s_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if(!s_bInitialized){
			break;
		}
		s_bInitialized = false;
		SetConsoleCtrlHandler(&CrtCtrlHandler, false); // 忽略错误。
	jCleanup99:
		__MCFCRT_ModuleUninit();
	jCleanup03:
		break;
	}

	if(!bRet){
		_MCFCRT_Bail(L"MCFCRT 初始化失败。");
	}
}

__MCFCRT_C_STDCALL
static void CrtExeTlsCallback(LPVOID pInstance, DWORD dwReason, LPVOID pReserved){
	__MCFCRT_SEH_TOP_BEGIN
	{
		RealTlsCallback(pInstance, (unsigned)dwReason, !pReserved);
	}
	__MCFCRT_SEH_TOP_END
}

extern const IMAGE_TLS_DIRECTORY _tls_used;

__extension__ __attribute__((__section__(".tls$AAA")))
static const char tls_begin[0] = { };
__extension__ __attribute__((__section__(".tls$ZZZ")))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$AAA")))
static const PIMAGE_TLS_CALLBACK callback_begin = &CrtExeTlsCallback;
__attribute__((__section__(".CRT$ZZZ"), __used__))
static const PIMAGE_TLS_CALLBACK callback_end   = nullptr;

__attribute__((__section__(".data")))
static DWORD tls_index = 0xDEADBEEF;

__attribute__((__section__(".rdata"), __used__))
const IMAGE_TLS_DIRECTORY _tls_used = { (UINT_PTR)&tls_begin, (UINT_PTR)&tls_end, (UINT_PTR)&tls_index, (UINT_PTR)&callback_begin, 0, 0 };

static void CrtModuleUninitCascading(intptr_t nContext){
	const bool bDynamic = nContext;

	__MCFCRT_SEH_TOP_BEGIN
	{
		RealTlsCallback(_MCFCRT_GetModuleBase(), DLL_PROCESS_DETACH, bDynamic);
	}
	__MCFCRT_SEH_TOP_END
}

__MCFCRT_C_STDCALL __attribute__((__noreturn__))
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	DWORD dwExitCode;

	__MCFCRT_SEH_TOP_BEGIN
	{
		// 如果 EXE 只链接了 KERNEL32.DLL 和 NTDLL.DLL 那么 TLS 回调就收不到 DLL_PROCESS_ATTACH 通知。这里需要处理这种情况。
		RealTlsCallback(_MCFCRT_GetModuleBase(), DLL_PROCESS_ATTACH, false);

		dwExitCode = _MCFCRT_Main();
	}
	__MCFCRT_SEH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}
