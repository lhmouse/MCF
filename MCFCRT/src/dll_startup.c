// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "mcfcrt.h"
#include "env/cpu.h"
#include "env/thread.h"
#include "env/mcfwin.h"

__attribute__((__stdcall__)) extern BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

typedef struct tagDllStartupParams {
	HINSTANCE hInstance;
	DWORD dwReason;
	LPVOID pReserved;
} DllStartupParams;

static unsigned long WrappedDllStartup(void *pOpaque){
	DllStartupParams *const pParams = pOpaque;

	switch(pParams->dwReason){
	case DLL_PROCESS_ATTACH:
		if(!__MCFCRT_InitRecursive()){
			return false;
		}
		return true;

	case DLL_PROCESS_DETACH:
		__MCFCRT_UninitRecursive();
		return true;

	case DLL_THREAD_ATTACH:
		return true;

	case DLL_THREAD_DETACH:
		return true;

	default:
		return false;
	}
}

__attribute__((__stdcall__)) BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved){
	__MCFCRT_CpuResetFloatingPointEnvironment();
	DllStartupParams vParams = { hInstance, dwReason, pReserved };
	const unsigned long dwResult = _MCFCRT_WrapThreadProcWithSehTop(&WrappedDllStartup, &vParams);
	return dwResult != 0;
}
