// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "mcfcrt.h"
#include "env/_seh_top.h"
#include "env/_fpu.h"

__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

__MCFCRT_C_STDCALL
BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved){
	(void)hInstance, (void)pReserved;

	__MCFCRT_FpuInitialize();

	bool bRet = true;

	__MCFCRT_SEH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			bRet = __MCFCRT_InitRecursive();
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			__MCFCRT_UninitRecursive();
			break;
		}
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
