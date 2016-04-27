// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "exe_decl.h"
#include "../env/mcfwin.h"
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

	const HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	_MCFCRT_ASSERT(hStdErr != INVALID_HANDLE_VALUE);
	if(hStdErr){
		static const char kKilled[] = "Killed: _MCFCRT_OnCtrlEvent() is undefined.\n";
		DWORD dwBytesWritten;
		WriteFile(hStdErr, kKilled, sizeof(kKilled) - 1, &dwBytesWritten, nullptr);
	}
	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}

static inline bool __MCFCRT_InstallCtrlHandler(){
	const bool bRet = SetConsoleCtrlHandler(&CrtTerminalCtrlHandler, true);
	return bRet;
}
static inline void __MCFCRT_UninstallCtrlHandler(){
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

static void OnExeProcessAttach(){
	__MCFCRT_FEnvInit();

	if(!__MCFCRT_HeapInit()){
		BailWithErrorCode(L"MCFCRT 堆内存初始化失败。", GetLastError());
	}
	if(!__MCFCRT_HeapDbgInit()){
		BailWithErrorCode(L"MCFCRT 堆内存调试器初始化失败。", GetLastError());
	}
	if(!__MCFCRT_CppRuntimeInit()){
		BailWithErrorCode(L"MCFCRT C++ 运行时初始化失败。", GetLastError());
	}
	if(!__MCFCRT_BeginModule()){
		BailWithErrorCode(L"MCFCRT 模块初始化失败。", GetLastError());
	}
	if(!__MCFCRT_InstallCtrlHandler()){
		BailWithErrorCode(L"MCFCRT Ctrl 响应函数注册失败。", GetLastError());
	}
}
static void OnExeThreadAttach(){
	__MCFCRT_FEnvInit();
}
static void OnExeThreadDetach(){
	__MCFCRT_TlsCleanup();
}
static void OnExeProcessDetach(){
	__MCFCRT_TlsCleanup();

	__MCFCRT_UninstallCtrlHandler();
	__MCFCRT_EndModule();
	__MCFCRT_CppRuntimeUninit();
	__MCFCRT_HeapDbgUninit();
	__MCFCRT_HeapUninit();
}

static bool g_bInitialized = false;

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
static void CrtTlsCallback(LPVOID hInstance, DWORD dwReason, LPVOID pReserved){
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
static const char tls_start[0] = { };
__extension__ __attribute__((__section__(".tls$___")))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$@@@")))
static const PIMAGE_TLS_CALLBACK callback_start = &CrtTlsCallback;
__attribute__((__section__(".CRT$___")))
static const PIMAGE_TLS_CALLBACK callback_end   = nullptr;

__attribute__((__section__(".data")))
DWORD tls_index = 0xDEADBEEF;

// 如果没有 dllexport 而在编译链接时开启了 lto 则 TLS 目录就不会被正确链接。
__attribute__((__dllexport__, __section__(".rdata")))
const IMAGE_TLS_DIRECTORY _tls_used = { (UINT_PTR)&tls_start, (UINT_PTR)&tls_end, (UINT_PTR)&tls_index, (UINT_PTR)&callback_start, 0, 0 };

_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

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
