// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../env/mcfwin.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/eh_top.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"

// -static -Wl,-e__MCFCRT_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_ExeStartup 模块入口点。
_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("__MCFCRT_ExeStartup");

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

static bool OnDllProcessAttach(){
	if(!__MCFCRT_HeapInit()){
		return false;
	}
	if(!__MCFCRT_HeapDbgInit()){
		__MCFCRT_HeapUninit();
		return false;
	}
	if(!__MCFCRT_RegisterFrameInfo()){
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		return false;
	}
	if(!__MCFCRT_BeginModule()){
		__MCFCRT_UnregisterFrameInfo();
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		return false;
	}
	return true;
}
static void OnDllThreadAttach(){
}
static void OnDllThreadDetach(){
	__MCFCRT_TlsThreadCleanup();
}
static void OnDllProcessDetach(){
	__MCFCRT_EndModule();
	__MCFCRT_UnregisterFrameInfo();
	__MCFCRT_HeapDbgUninit();
	__MCFCRT_HeapUninit();
}

static bool g_bInitialized = false;

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

_Noreturn static void BailWithErrorCode(const wchar_t *pwszMessage, DWORD dwErrorCode){
	wchar_t awcBuffer[512];
	wchar_t *pwcWrite;
	pwcWrite = _MCFCRT_wcppcpy(awcBuffer, awcBuffer + 448, pwszMessage);
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n错误代码：");
	pwcWrite = _MCFCRT_itow_u(pwcWrite, dwErrorCode);
	*pwcWrite = 0;
	_MCFCRT_Bail(awcBuffer);
}

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
static void CrtTlsCallback(LPVOID hInstance, DWORD dwReason, LPVOID pReserved){
	(void)hInstance;
	(void)pReserved;

	bool bRet = true;

	__MCFCRT_EH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(g_bInitialized){
				break;
			}
			bRet = SetConsoleCtrlHandler(&CrtTerminalCtrlHandler, true);
			if(!bRet){
				BailWithErrorCode(L"MCFCRT Ctrl 事件处理程序注册失败。", GetLastError());
			}
			bRet = OnDllProcessAttach();
			if(!bRet){
				BailWithErrorCode(L"MCFCRT 初始化失败。", GetLastError());
			}
			g_bInitialized = true;
			break;

		case DLL_THREAD_ATTACH:
			OnDllThreadAttach();
			break;

		case DLL_THREAD_DETACH:
			OnDllThreadDetach();
			break;

		case DLL_PROCESS_DETACH:
			if(!g_bInitialized){
				break;
			}
			g_bInitialized = false;
			OnDllProcessDetach();
			break;
		}
	}
	__MCFCRT_EH_TOP_END
}

// 线程局部存储（TLS）目录，用于执行 TLS 的析构函数。
__extension__ __attribute__((__section__(".tls$@@@"), __used__))
static const char tls_start[0] = { };
__extension__ __attribute__((__section__(".tls$___"), __used__))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$@@@"), __used__))
static const PIMAGE_TLS_CALLBACK callback_start = &CrtTlsCallback;
__attribute__((__section__(".CRT$___"), __used__))
static const PIMAGE_TLS_CALLBACK callback_end   = nullptr;

DWORD _tls_index;

__attribute__((__section__(".rdata"), __used__))
const IMAGE_TLS_DIRECTORY _tls_used = {
	.StartAddressOfRawData = (UINT_PTR)&tls_start,
	.EndAddressOfRawData   = (UINT_PTR)&tls_end,
	.AddressOfIndex        = (UINT_PTR)&_tls_index,
	.AddressOfCallBacks    = (UINT_PTR)&callback_start,
	.SizeOfZeroFill        = 0,
	.Characteristics       = 0,
};
