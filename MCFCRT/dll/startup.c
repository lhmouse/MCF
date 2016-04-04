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

// -static -Wl,-e__MCFCRT_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_DllStartup 模块入口点。
__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCFCRT_DllStartup");

static bool g_bInitialized = false;

__MCFCRT_C_STDCALL __MCFCRT_HAS_EH_TOP
BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = TRUE;

	switch(dwReason){
		bool bSucceeded;

	case DLL_PROCESS_ATTACH:
		_MCFCRT_ASSERT(!g_bInitialized);

		bSucceeded = __MCFCRT_HeapInit();
		if(!bSucceeded){
			bRet = FALSE;
			break;
		}

		bSucceeded = __MCFCRT_HeapDbgInit();
		if(!bSucceeded){
			__MCFCRT_HeapUninit();
			bRet = false;
			break;
		}

		bSucceeded = __MCFCRT_RegisterFrameInfo();
		if(!bSucceeded){
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			bRet = false;
			break;
		}

		__MCFCRT_EH_TOP_BEGIN
		{
			bSucceeded = __MCFCRT_BeginModule();
		}
		__MCFCRT_EH_TOP_END
		if(!bSucceeded){
			__MCFCRT_UnregisterFrameInfo();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			bRet = false;
			break;
		}

		__MCFCRT_EH_TOP_BEGIN
		{
			bSucceeded = _MCFCRT_OnDllProcessAttach(hDll, !pReserved);
		}
		__MCFCRT_EH_TOP_END
		if(!bSucceeded){
			__MCFCRT_EndModule();
			__MCFCRT_UnregisterFrameInfo();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			bRet = false;
			break;
		}

		g_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		__MCFCRT_EH_TOP_BEGIN
		{
			_MCFCRT_OnDllThreadAttach(hDll);
		}
		__MCFCRT_EH_TOP_END
		break;

	case DLL_THREAD_DETACH:
		__MCFCRT_EH_TOP_BEGIN
		{
			_MCFCRT_OnDllThreadDetach(hDll);

			__MCFCRT_TlsThreadCleanup();
		}
		__MCFCRT_EH_TOP_END
		break;

	case DLL_PROCESS_DETACH:
		if(!g_bInitialized){
			break;
		}
		g_bInitialized = false;

		__MCFCRT_EH_TOP_BEGIN
		{
			_MCFCRT_OnDllProcessDetach(hDll, !pReserved);
		}
		__MCFCRT_EH_TOP_END

		__MCFCRT_EH_TOP_BEGIN
		{
			__MCFCRT_EndModule();
		}
		__MCFCRT_EH_TOP_END

		__MCFCRT_UnregisterFrameInfo();

		__MCFCRT_HeapDbgUninit();

		__MCFCRT_HeapUninit();

		g_bInitialized = false;
		break;
	}

	return bRet;
}
