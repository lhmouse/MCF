// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "dll_decl.h"
#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/_eh_top.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

extern "C" {

__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCF_DllStartup");

__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = FALSE;
	__MCF_EH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(!::__MCF_CRT_BeginModule()){
				break;
			}
			if(!::MCFDll_OnProcessAttach(!pReserved)){
				::__MCF_CRT_EndModule();
				break;
			}
			bRet = TRUE;
			break;

		case DLL_PROCESS_DETACH:
			::MCFDll_OnProcessDetach(!pReserved);
			::__MCF_CRT_EndModule();
			break;

		case DLL_THREAD_ATTACH:
			::MCFDll_OnThreadAttach();
			::__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			break;

		case DLL_THREAD_DETACH:
			::__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			::MCFDll_OnThreadDetach();
			break;
		}
	}
	__MCF_EH_TOP_END
	return bRet;
}

}
