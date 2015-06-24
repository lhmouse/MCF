// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "dll_decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/eh_top.h"
#include "../env/heap.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCF_DllStartup 模块入口点。
__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCF_DllStartup");

__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = FALSE;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		if(!__MCF_CRT_HeapInit()){
			break;
		}
		__MCF_EH_TOP_BEGIN
		{
			if(!__MCF_CRT_BeginModule()){
				goto jTryInitDone;
			}
			if(!MCFDll_OnProcessAttach(!pReserved)){
				__MCF_CRT_EndModule();
				goto jTryInitDone;
			}
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			bRet = TRUE;
		jTryInitDone:
			;
		}
		__MCF_EH_TOP_END
		if(!bRet){
			__MCF_CRT_HeapUninit();
		}
		break;

	case DLL_THREAD_ATTACH:
		__MCF_EH_TOP_BEGIN
		{
			MCFDll_OnThreadAttach();
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
		}
		__MCF_EH_TOP_END
		break;

	case DLL_THREAD_DETACH:
		__MCF_EH_TOP_BEGIN
		{
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			MCFDll_OnThreadDetach();
		}
		__MCF_EH_TOP_END
		break;

	case DLL_PROCESS_DETACH:
		__MCF_EH_TOP_BEGIN
		{
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			MCFDll_OnProcessDetach(!pReserved);
			__MCF_CRT_EndModule();
		}
		__MCF_EH_TOP_END
		__MCF_CRT_HeapUninit();
		break;
	}

	return bRet;
}
