// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/eh_top.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"

// -static -Wl,-e__MCFCRT_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_ExeStartup 模块入口点。
_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pReserved)
	__asm__("__MCFCRT_ExeStartup");

_Noreturn __MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
DWORD __MCFCRT_ExeStartup(LPVOID pReserved){
	(void)pReserved;

	DWORD dwExitCode;

	__MCFCRT_EH_TOP_BEGIN
	{
		dwExitCode = MCFCRT_Main();
	}
	__MCFCRT_EH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}

__MCFCRT_C_STDCALL
static BOOL TopCtrlHandler(DWORD dwCtrlType){
	(void)dwCtrlType;

	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
static void TlsCallback(void *hModule, DWORD dwReason, void *pReserved){
	(void)hModule;
	(void)pReserved;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		if(!SetConsoleCtrlHandler(&TopCtrlHandler, true)){
			MCFCRT_BailF(L"MCFCRT Ctrl 处理程序注册失败。\n\n错误代码：%lu", GetLastError());
		}
		if(!__MCFCRT_HeapInit()){
			MCFCRT_BailF(L"MCFCRT 堆初始化失败。\n\n错误代码：%lu", GetLastError());
		}
		if(!__MCFCRT_HeapDbgInit()){
			MCFCRT_BailF(L"MCFCRT 堆调试器初始化失败。\n\n错误代码：%lu", GetLastError());
		}
		if(!__MCFCRT_RegisterFrameInfo()){
			MCFCRT_BailF(L"MCFCRT 异常处理程序初始化失败。\n\n错误代码：%lu", GetLastError());
		}

		__MCFCRT_EH_TOP_BEGIN
		{
			if(!__MCFCRT_BeginModule()){
				MCFCRT_BailF(L"MCFCRT 初始化失败。\n\n错误代码：%lu", GetLastError());
			}
		}
		__MCFCRT_EH_TOP_END
		break;

	case DLL_THREAD_ATTACH:
		__MCFCRT_EH_TOP_BEGIN
		{
		}
		__MCFCRT_EH_TOP_END
		break;

	case DLL_THREAD_DETACH:
		__MCFCRT_EH_TOP_BEGIN
		{
			__MCFCRT_TlsThreadCleanup();
		}
		__MCFCRT_EH_TOP_END
		break;

	case DLL_PROCESS_DETACH:
		__MCFCRT_EH_TOP_BEGIN
		{
			__MCFCRT_EndModule();
		}
		__MCFCRT_EH_TOP_END

		__MCFCRT_UnregisterFrameInfo();
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		SetConsoleCtrlHandler(&TopCtrlHandler, false);
		break;
	}
}

// 线程局部存储（TLS）目录，用于执行 TLS 的析构函数。
__extension__ __attribute__((__section__(".tls$@@@"), __used__))
static const char tls_start[0] = { };
__extension__ __attribute__((__section__(".tls$___"), __used__))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$@@@"), __used__))
static const PIMAGE_TLS_CALLBACK callback_start = &TlsCallback;
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
