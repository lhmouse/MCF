// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "decl.h"
#include "../env/fenv.h"
#include "../env/mcfwin.h"
#include "../env/module.h"
#include "../env/thread_env.h"
#include "../env/eh_top.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"

// -static -Wl,-e__MCFCRT_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

static bool OnDllProcessAttach(HINSTANCE hDll, bool bDynamic){
	__MCFCRT_FenvInit();

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
	if(_MCFCRT_OnDllProcessAttach){
		if(!_MCFCRT_OnDllProcessAttach(hDll, bDynamic)){
			__MCFCRT_EndModule();
			__MCFCRT_UnregisterFrameInfo();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			return false;
		}
	}
	return true;
}
static void OnDllThreadAttach(HINSTANCE hDll){
	__MCFCRT_FenvInit();

	if(_MCFCRT_OnDllThreadAttach){
		_MCFCRT_OnDllThreadAttach(hDll);
	}
}
static void OnDllThreadDetach(HINSTANCE hDll){
	if(_MCFCRT_OnDllThreadDetach){
		_MCFCRT_OnDllThreadDetach(hDll);
	}
	__MCFCRT_TlsCleanup();
}
static void OnDllProcessDetach(HINSTANCE hDll, bool bDynamic){
	if(_MCFCRT_OnDllProcessDetach){
		_MCFCRT_OnDllProcessDetach(hDll, bDynamic);
	}
	__MCFCRT_EndModule();
	__MCFCRT_UnregisterFrameInfo();
	__MCFCRT_HeapDbgUninit();
	__MCFCRT_HeapUninit();
}

// __MCFCRT_DllStartup 模块入口点。
__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCFCRT_DllStartup");

static bool g_bInitialized = false;

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	bool bRet = true;

	__MCFCRT_EH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(g_bInitialized){
				break;
			}
			bRet = OnDllProcessAttach(hDll, !pReserved);
			if(!bRet){
				break;
			}
			g_bInitialized = true;
			break;

		case DLL_THREAD_ATTACH:
			OnDllThreadAttach(hDll);
			break;

		case DLL_THREAD_DETACH:
			OnDllThreadDetach(hDll);
			break;

		case DLL_PROCESS_DETACH:
			if(!g_bInitialized){
				break;
			}
			g_bInitialized = false;
			OnDllProcessDetach(hDll, !pReserved);
			break;
		}
	}
	__MCFCRT_EH_TOP_END

	return bRet;
}
