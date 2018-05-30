// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "exe.h"
#include "module.h"
#include "tls.h"
#include "../mcfcrt.h"
#include "../env/cpu.h"
#include "../env/xassert.h"
#include "../env/standard_streams.h"
#include "../env/crt_module.h"
#include "../env/bail.h"
#include "../env/thread.h"
#include "../env/mcfwin.h"

extern unsigned _MCFCRT_Main(void);
__attribute__((__weak__)) extern void _MCFCRT_OnCtrlEvent(bool bIsSigInt);

// -Wl,-e@__MCFCRT_ExeStartup
__attribute__((__noreturn__, __stdcall__)) extern DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("@__MCFCRT_ExeStartup");

__attribute__((__force_align_arg_pointer__, __stdcall__)) static BOOL CtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	static const wchar_t kKilledMessage[] = L"进程被 Ctrl-C 终止，因为没有找到自定义的 _MCFCRT_OnCtrlEvent() 响应函数。";
	_MCFCRT_WriteStandardErrorText(kKilledMessage, sizeof(kKilledMessage) / sizeof(wchar_t) - 1, true);
	_MCFCRT_QuickExit(3);
}

static bool s_bProcessAttachNotified = false;

typedef struct tagTlsCallbackParams {
	HINSTANCE hInstance;
	DWORD dwReason;
	LPVOID pReserved;
} TlsCallbackParams;

static unsigned long WrappedTlsCallback(void *pOpaque){
	TlsCallbackParams *const pParams = pOpaque;

	switch(pParams->dwReason){
	case DLL_PROCESS_ATTACH:
		if(!__MCFCRT_InitRecursive()){
			_MCFCRT_Bail(L"MCFCRT 初始化失败。");
		}
		if(!__MCFCRT_ModuleInit()){
			_MCFCRT_Bail(L"MCFCRT 可执行模块初始化失败。");
		}
		if(!SetConsoleCtrlHandler(&CtrlHandler, true)){
			_MCFCRT_Bail(L"MCFCRT 可执行模块 Ctrl-C 响应回调函数注册失败。");
		}
		s_bProcessAttachNotified = true;
		return true;

	case DLL_PROCESS_DETACH:
		SetConsoleCtrlHandler(&CtrlHandler, false);
		__MCFCRT_TlsCleanup();
		__MCFCRT_ModuleUninit();
		__MCFCRT_UninitRecursive();
		return true;

	case DLL_THREAD_ATTACH:
		return true;

	case DLL_THREAD_DETACH:
		__MCFCRT_TlsCleanup();
		return true;

	default:
		return false;
	}
}

__attribute__((__stdcall__)) void __MCFCRT_ExeTlsCallback(PVOID hInstance, DWORD dwReason, LPVOID pReserved){
	__MCFCRT_CpuResetFloatingPointEnvironment();
	TlsCallbackParams vParams = { hInstance, dwReason, pReserved };
	_MCFCRT_WrapThreadProcWithSehTop(&WrappedTlsCallback, &vParams);
}

__extension__ __attribute__((__section__(".tls$AAA"))) const char _tls_start = 0;
__extension__ __attribute__((__section__(".tls$ZZZ"))) const char _tls_end   = 0;

DWORD _tls_index = 0;

__attribute__((__section__(".CRT$XLA"))) const PIMAGE_TLS_CALLBACK __xl_a = &__MCFCRT_ExeTlsCallback;
__attribute__((__section__(".CRT$XLZ"))) const PIMAGE_TLS_CALLBACK __xl_z = _MCFCRT_NULLPTR;

__attribute__((__used__)) const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData = (UINT_PTR)&_tls_start,
	.EndAddressOfRawData   = (UINT_PTR)&_tls_end,
	.AddressOfIndex        = (UINT_PTR)&_tls_index,
	.AddressOfCallBacks    = (UINT_PTR)&__xl_a,
	.SizeOfZeroFill        = 0,
	.Characteristics       = 0,
};

static unsigned long WrappedExeStartup(void *pOpaque){
	(void)pOpaque;

	if(!s_bProcessAttachNotified){
		__MCFCRT_ExeTlsCallback(_MCFCRT_GetModuleBase(), DLL_PROCESS_ATTACH, _MCFCRT_NULLPTR);
	}
	_MCFCRT_ASSERT(s_bProcessAttachNotified);

	const unsigned uExitCode = _MCFCRT_Main();
	return uExitCode;
}

__attribute__((__noreturn__, __stdcall__)) DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;
	__MCFCRT_CpuResetFloatingPointEnvironment();
	const unsigned long dwResult = _MCFCRT_WrapThreadProcWithSehTop(&WrappedExeStartup, _MCFCRT_NULLPTR);
	_MCFCRT_Exit((unsigned)dwResult);
}
