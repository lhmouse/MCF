// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "generic.h"
#include "exe.h"
#include "dll.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"
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
			bRet = __MCFCRT_StandardStreamsInit();
			if(!bRet){
				break;
			}
			bRet = __MCFCRT_HeapInit();
			if(!bRet){
				__MCFCRT_StandardStreamsUninit();
				break;
			}
			bRet = __MCFCRT_HeapDbgInit();
			if(!bRet){
				__MCFCRT_HeapUninit();
				__MCFCRT_StandardStreamsUninit();
				break;
			}
			bRet = __MCFCRT_CppRuntimeInit();
			if(!bRet){
				__MCFCRT_HeapDbgUninit();
				__MCFCRT_HeapUninit();
				__MCFCRT_StandardStreamsUninit();
				break;
			}
			bRet = __MCFCRT_ModuleInit();
			if(!bRet){
				__MCFCRT_CppRuntimeUninit();
				__MCFCRT_HeapDbgUninit();
				__MCFCRT_HeapUninit();
				__MCFCRT_StandardStreamsUninit();
				break;
			}
			bRet = __MCFCRT_ThreadEnvInit();
			if(!bRet){
				__MCFCRT_ModuleUninit();
				__MCFCRT_CppRuntimeUninit();
				__MCFCRT_HeapDbgUninit();
				__MCFCRT_HeapUninit();
				__MCFCRT_StandardStreamsUninit();
				break;
			}
			if(_MCFCRT_OnDllProcessAttach){
				bRet = _MCFCRT_OnDllProcessAttach(pInstance, bDynamic);
				if(!bRet){
					__MCFCRT_TlsCleanup();
					__MCFCRT_ThreadEnvUninit();
					__MCFCRT_ModuleUninit();
					__MCFCRT_CppRuntimeUninit();
					__MCFCRT_HeapDbgUninit();
					__MCFCRT_HeapUninit();
					__MCFCRT_StandardStreamsUninit();
					break;
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
			__MCFCRT_TlsCleanup();
			__MCFCRT_ThreadEnvUninit();
			__MCFCRT_ModuleUninit();
			__MCFCRT_CppRuntimeUninit();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			__MCFCRT_StandardStreamsUninit();
			break;
		}
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
