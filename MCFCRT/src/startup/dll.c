// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "generic.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"

// -Wl,-e@__MCFCRT_DllStartup

// __MCFCRT_DllStartup 模块入口点。
__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

__MCFCRT_C_STDCALL __attribute__((__noinline__))
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	bool bRet = true;

	void *   const pInstance = (void *)hDll;
	unsigned const uReason   = dwReason;
	bool     const bDynamic  = !pReserved;

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		bRet = __MCFCRT_TlsCallbackGeneric(pInstance, uReason, bDynamic);
		if(!bRet){
			goto jCleanup_01;
		}
		if(_MCFCRT_OnDllProcessAttach){
			bRet = _MCFCRT_OnDllProcessAttach(pInstance, bDynamic);
			if(!bRet){
				goto jCleanup_99;
			}
		}
		break;

	case DLL_THREAD_ATTACH:
		__MCFCRT_TlsCallbackGeneric(pInstance, uReason, bDynamic);
		if(_MCFCRT_OnDllThreadAttach){
			_MCFCRT_OnDllThreadAttach(pInstance);
		}
		break;

	case DLL_THREAD_DETACH:
		if(_MCFCRT_OnDllThreadDetach){
			_MCFCRT_OnDllThreadDetach(pInstance);
		}
		__MCFCRT_TlsCallbackGeneric(pInstance, uReason, bDynamic);
		break;

	case DLL_PROCESS_DETACH:
		if(_MCFCRT_OnDllProcessDetach){
			_MCFCRT_OnDllProcessDetach(pInstance, bDynamic);
		}
	jCleanup_99:
		__MCFCRT_TlsCallbackGeneric(pInstance, uReason, bDynamic);
	jCleanup_01:
		break;
	}

	return bRet;
}
