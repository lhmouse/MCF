// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "exe_decl.h"
#include "../env/mcfwin.h"
#include "../env/standard_streams.h"
#include "../env/module.h"
#include "../env/fenv.h"
#include "../env/thread_env.h"
#include "../env/eh_top.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../env/cpp_runtime.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"

// -static -Wl,-e__MCFCRT_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_ExeStartup 模块入口点。
_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("__MCFCRT_ExeStartup");

__MCFCRT_C_STDCALL
static BOOL CrtTerminalCtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	const HANDLE hStandardError = GetStdHandle(STD_ERROR_HANDLE);
	_MCFCRT_ASSERT(hStandardError != INVALID_HANDLE_VALUE);
	if(hStandardError){
		static const char kKilled[] = "Killed: _MCFCRT_OnCtrlEvent() is undefined.\n";
		DWORD dwBytesWritten;
		WriteFile(hStandardError, kKilled, sizeof(kKilled) - 1, &dwBytesWritten, nullptr);
	}
	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}

static inline bool __MCFCRT_InstallCtrlHandler(void){
	const bool bRet = SetConsoleCtrlHandler(&CrtTerminalCtrlHandler, true);
	return bRet;
}
static inline void __MCFCRT_UninstallCtrlHandler(void){
	const bool bRet = SetConsoleCtrlHandler(&CrtTerminalCtrlHandler, false);
	_MCFCRT_ASSERT(bRet);
}

_Noreturn __attribute__((__noinline__))
static void BailWithErrorCode(const wchar_t *pwszMessage, DWORD dwErrorCode){
	wchar_t awcBuffer[512];
	wchar_t *pwcWrite;
	pwcWrite = _MCFCRT_wcppcpy(awcBuffer, awcBuffer + 448, pwszMessage);
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n错误代码：");
	pwcWrite = _MCFCRT_itow_u(pwcWrite, dwErrorCode);
	*pwcWrite = 0;
	_MCFCRT_Bail(awcBuffer);
}

static void OnExeProcessAttach(void){
	__MCFCRT_FEnvInit();

	if(!__MCFCRT_StandardStreamsInit()){
		BailWithErrorCode(L"MCFCRT 标准输入输出流初始化失败。", GetLastError());
	}
	if(!__MCFCRT_HeapInit()){
		BailWithErrorCode(L"MCFCRT 堆内存初始化失败。", GetLastError());
	}
	if(!__MCFCRT_HeapDbgInit()){
		BailWithErrorCode(L"MCFCRT 堆内存调试器初始化失败。", GetLastError());
	}
	if(!__MCFCRT_CppRuntimeInit()){
		BailWithErrorCode(L"MCFCRT C++ 运行时初始化失败。", GetLastError());
	}
	if(!__MCFCRT_ModuleInit()){
		BailWithErrorCode(L"MCFCRT 模块初始化失败。", GetLastError());
	}
	if(!__MCFCRT_InstallCtrlHandler()){
		BailWithErrorCode(L"MCFCRT Ctrl 响应函数注册失败。", GetLastError());
	}
}
static void OnExeThreadAttach(void){
	__MCFCRT_FEnvInit();
}
static void OnExeThreadDetach(void){
	__MCFCRT_TlsCleanup();
}
static void OnExeProcessDetach(void){
	__MCFCRT_TlsCleanup();

	__MCFCRT_UninstallCtrlHandler();
	__MCFCRT_ModuleUninit();
	__MCFCRT_CppRuntimeUninit();
	__MCFCRT_HeapDbgUninit();
	__MCFCRT_HeapUninit();
	__MCFCRT_StandardStreamsUninit();
}

static bool g_bInitialized = false;

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
void __MCFCRT_ExeTlsCallback(LPVOID hInstance, DWORD dwReason, LPVOID pReserved){
	(void)hInstance;
	(void)pReserved;

	__MCFCRT_EH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(g_bInitialized){
				break;
			}
			OnExeProcessAttach();
			g_bInitialized = true;
			break;

		case DLL_THREAD_ATTACH:
			OnExeThreadAttach();
			break;

		case DLL_THREAD_DETACH:
			OnExeThreadDetach();
			break;

		case DLL_PROCESS_DETACH:
			if(!g_bInitialized){
				break;
			}
			g_bInitialized = false;
			OnExeProcessDetach();
			break;
		}
	}
	__MCFCRT_EH_TOP_END
}

extern const IMAGE_TLS_DIRECTORY _tls_used;

__extension__ __attribute__((__section__(".tls$@@@")))
static const char tls_begin[0] = { };
__extension__ __attribute__((__section__(".tls$___")))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$@@@")))
static const PIMAGE_TLS_CALLBACK callback_begin = &__MCFCRT_ExeTlsCallback;
__attribute__((__section__(".CRT$___"), __used__))
static const PIMAGE_TLS_CALLBACK callback_end   = nullptr;

__attribute__((__section__(".data")))
static DWORD tls_index = 0xDEADBEEF;

__attribute__((__section__(".rdata"), __dllexport__))
const IMAGE_TLS_DIRECTORY _tls_used = { (UINT_PTR)&tls_begin, (UINT_PTR)&tls_end, (UINT_PTR)&tls_index, (UINT_PTR)&callback_begin, 0, 0 };

_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	__MCFCRT_ExeTlsCallback(_MCFCRT_GetModuleBase(), DLL_PROCESS_ATTACH, nullptr);

	DWORD dwExitCode;

	__MCFCRT_EH_TOP_BEGIN
	{
		dwExitCode = _MCFCRT_Main();
	}
	__MCFCRT_EH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}

__attribute__((__used__))
int __MCFCRT_do_not_link_exe_startup_code_and_dll_startup_code_together = 1;
