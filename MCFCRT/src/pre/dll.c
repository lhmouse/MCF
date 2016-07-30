// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"
#include "module.h"

// -Wl,-e@__MCFCRT_DllStartup
__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

static bool RealStartup(void *pInstance, unsigned uReason, bool bDynamic){
	(void)pInstance;
	(void)bDynamic;

	static bool s_bInitialized = false;

	bool bRet = true;

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		if(s_bInitialized){
			break;
		}
		bRet = __MCFCRT_ModuleInit();
		if(!bRet){
			goto jCleanup03;
		}
		if(_MCFCRT_OnDllProcessAttach){
			bRet = _MCFCRT_OnDllProcessAttach(pInstance, bDynamic);
			if(!bRet){
				goto jCleanup99;
			}
		}
		s_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		if(_MCFCRT_OnDllThreadAttach){
			_MCFCRT_OnDllThreadAttach(pInstance);
		}
		break;

	case DLL_THREAD_DETACH:
		if(_MCFCRT_OnDllThreadDetach){
			_MCFCRT_OnDllThreadDetach(pInstance);
		}
		break;

	case DLL_PROCESS_DETACH:
		if(!s_bInitialized){
			break;
		}
		s_bInitialized = false;
		if(_MCFCRT_OnDllProcessDetach){
			_MCFCRT_OnDllProcessDetach(pInstance, bDynamic);
		}
	jCleanup99:
		__MCFCRT_ModuleUninit();
	jCleanup03:
		break;
	}

	return bRet;
}

__MCFCRT_C_STDCALL
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	bool bRet;

	__MCFCRT_SEH_TOP_BEGIN
	{
		bRet = RealStartup((void *)hDll, (unsigned)dwReason, !pReserved);
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
