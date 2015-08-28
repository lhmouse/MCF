// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "decl.h"
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

enum {
	FL_HEAP,
	FL_FRAME_INFO,
	FL_CRT_MODULE,

	FL_COUNT
};

static bool g_abFlags[FL_COUNT] = { 0 };

#define DO_INIT(ret_, fl_, fn_)	\
	if(ret_){	\
		if(!g_abFlags[fl_]){	\
			const bool succ_ = (fn_)();	\
			if(succ_){	\
				g_abFlags[fl_] = true;	\
			} else {	\
				(ret_) = FALSE;	\
			}	\
		}	\
	}

#define DO_UNINIT(fl_, fn_)	\
	if(g_abFlags[fl_]){	\
		(fn_)();	\
		g_abFlags[fl_] = false;	\
	}

__MCF_C_STDCALL __MCF_HAS_EH_TOP
BOOL __MCF_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = TRUE;

	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		DO_INIT(bRet, FL_HEAP, __MCF_CRT_HeapInit);
		DO_INIT(bRet, FL_FRAME_INFO, __MCF_CRT_RegisterFrameInfo);

		__MCF_EH_TOP_BEGIN
		{
			DO_INIT(bRet, FL_CRT_MODULE, __MCF_CRT_BeginModule);

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

			MCF_CRT_TlsClearAll();
		}
		__MCF_EH_TOP_END
		break;

	case DLL_PROCESS_DETACH:
		__MCF_EH_TOP_BEGIN
		{
			MCFDll_OnProcessDetach(hDll, !pReserved);

			DO_UNINIT(FL_CRT_MODULE, __MCF_CRT_EndModule);
		}
		__MCF_EH_TOP_END

		DO_UNINIT(FL_FRAME_INFO, __MCF_CRT_UnregisterFrameInfo);
		DO_UNINIT(FL_HEAP, __MCF_CRT_HeapUninit);
		break;
	}

	return bRet;
}
