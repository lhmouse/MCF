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
#include "../ext/unref_param.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCF_DllStartup 模块入口点。
__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("__MCF_DllStartup");

enum {
	kFlagHeap,
	kFlagHeapDbg,
	kFlagFrameInfo,
	kFlagCrtModule,

	kFlagCount
};

static bool g_abFlags[kFlagCount] = { 0 };

#define DO_INIT(ret_, fl_, fn_)	\
	if(ret_){	\
		if(!g_abFlags[fl_]){	\
			const bool succ = (fn_)();	\
			if(succ){	\
				g_abFlags[fl_] = true;	\
			} else {	\
				(ret_) = false;	\
			}	\
		}	\
	}

#define DO_UNINIT(fl_, fn_)	\
	if(g_abFlags[fl_]){	\
		g_abFlags[fl_] = false;	\
		(fn_)();	\
	}

__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = true;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		DO_INIT(bRet, kFlagHeap,      __MCF_CRT_HeapInit);
		DO_INIT(bRet, kFlagHeapDbg,   __MCF_CRT_HeapDbgInit);
		DO_INIT(bRet, kFlagFrameInfo, __MCF_CRT_RegisterFrameInfo);

		__MCF_EH_TOP_BEGIN
		{
			DO_INIT(bRet, kFlagCrtModule, __MCF_CRT_BeginModule);

			if(bRet){
				bRet = MCFDll_OnProcessAttach(hDll, !pReserved);
			}
		}
		__MCF_EH_TOP_END
		break;

	case DLL_THREAD_ATTACH:
		__MCF_EH_TOP_BEGIN
		{
			MCFDll_OnThreadAttach(hDll);
		}
		__MCF_EH_TOP_END
		break;

	case DLL_THREAD_DETACH:
		__MCF_EH_TOP_BEGIN
		{
			MCFDll_OnThreadDetach(hDll);

			__MCF_CRT_TlsThreadCleanup();
		}
		__MCF_EH_TOP_END
		break;

	case DLL_PROCESS_DETACH:
		__MCF_EH_TOP_BEGIN
		{
			MCFDll_OnProcessDetach(hDll, !pReserved);

			DO_UNINIT(kFlagCrtModule, __MCF_CRT_EndModule);
		}
		__MCF_EH_TOP_END

		DO_UNINIT(kFlagFrameInfo, __MCF_CRT_UnregisterFrameInfo);
		DO_UNINIT(kFlagHeapDbg,   __MCF_CRT_HeapDbgUninit);
		DO_UNINIT(kFlagHeap,      __MCF_CRT_HeapUninit);
		break;
	}

	return bRet;
}
