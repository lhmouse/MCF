// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "generic.h"
#include "exe.h"
#include "dll.h"
#include "../env/_seh_top.h"
#include "../env/_crt_process.h"
#include "../env/mcfwin.h"
#include "../env/standard_streams.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/fenv.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../env/cpp_runtime.h"

static bool g_bInitialized = false;

bool __MCFCRT_TlsCallbackGeneric(void *pInstance, unsigned uReason, bool bDynamic){
	bool bRet = true;

	__MCFCRT_SEH_TOP_BEGIN
	{
		switch(uReason){
		case DLL_PROCESS_ATTACH:
			if(g_bInitialized){
				break;
			}
			__MCFCRT_FEnvInit();
			bRet = __MCFCRT_ProcessInit();
			if(!bRet){
				goto jCleanup_00;
			}
			bRet = __MCFCRT_StandardStreamsInit();
			if(!bRet){
				goto jCleanup_01;
			}
			bRet = __MCFCRT_HeapInit();
			if(!bRet){
				goto jCleanup_02;
			}
			bRet = __MCFCRT_HeapDbgInit();
			if(!bRet){
				goto jCleanup_03;
			}
			bRet = __MCFCRT_CppRuntimeInit();
			if(!bRet){
				goto jCleanup_04;
			}
			bRet = __MCFCRT_ModuleInit();
			if(!bRet){
				goto jCleanup_05;
			}
			bRet = __MCFCRT_ThreadEnvInit();
			if(!bRet){
				goto jCleanup_06;
			}
			if(_MCFCRT_OnDllProcessAttach){
				bRet = _MCFCRT_OnDllProcessAttach(pInstance, bDynamic);
				if(!bRet){
					goto jCleanup_99;
				}
			}
			g_bInitialized = true;
			break;

		case DLL_THREAD_ATTACH:
			__MCFCRT_FEnvInit();
			if(_MCFCRT_OnDllThreadAttach){
				_MCFCRT_OnDllThreadAttach(pInstance);
			}
			break;

		case DLL_THREAD_DETACH:
			if(_MCFCRT_OnDllThreadDetach){
				_MCFCRT_OnDllThreadDetach(pInstance);
			}
			__MCFCRT_TlsCleanup();
			break;

		case DLL_PROCESS_DETACH:
			if(!g_bInitialized){
				break;
			}
			g_bInitialized = false;
			if(_MCFCRT_OnDllProcessDetach){
				_MCFCRT_OnDllProcessDetach(pInstance, bDynamic);
			}
		jCleanup_99:
			__MCFCRT_TlsCleanup();
			__MCFCRT_ThreadEnvUninit();
		jCleanup_06:
			__MCFCRT_ModuleUninit();
		jCleanup_05:
			__MCFCRT_CppRuntimeUninit();
		jCleanup_04:
			__MCFCRT_HeapDbgUninit();
		jCleanup_03:
			__MCFCRT_HeapUninit();
		jCleanup_02:
			__MCFCRT_StandardStreamsUninit();
		jCleanup_01:
			__MCFCRT_ModuleUninit();
		jCleanup_00:
			break;
		}
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
