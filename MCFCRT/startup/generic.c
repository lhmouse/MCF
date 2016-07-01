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
#include "../env/fenv.h"
#include "../env/thread_env.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"
#include "../env/cpp_runtime.h"

static bool OnDllProcessAttach(HINSTANCE hInstance, bool bDynamic){
	__MCFCRT_FEnvInit();

	if(!__MCFCRT_StandardStreamsInit()){
		return false;
	}
	if(!__MCFCRT_HeapInit()){
		__MCFCRT_StandardStreamsUninit();
		return false;
	}
	if(!__MCFCRT_HeapDbgInit()){
		__MCFCRT_HeapUninit();
		__MCFCRT_StandardStreamsUninit();
		return false;
	}
	if(!__MCFCRT_CppRuntimeInit()){
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		__MCFCRT_StandardStreamsUninit();
		return false;
	}
	if(!__MCFCRT_ModuleInit()){
		__MCFCRT_CppRuntimeUninit();
		__MCFCRT_HeapDbgUninit();
		__MCFCRT_HeapUninit();
		__MCFCRT_StandardStreamsUninit();
		return false;
	}
	if(_MCFCRT_OnDllProcessAttach){
		if(!_MCFCRT_OnDllProcessAttach(hInstance, bDynamic)){
			__MCFCRT_ModuleUninit();
			__MCFCRT_CppRuntimeUninit();
			__MCFCRT_HeapDbgUninit();
			__MCFCRT_HeapUninit();
			__MCFCRT_StandardStreamsUninit();
			return false;
		}
	}
	return true;
}
static void OnDllThreadAttach(HINSTANCE hInstance){
	__MCFCRT_FEnvInit();

	if(_MCFCRT_OnDllThreadAttach){
		_MCFCRT_OnDllThreadAttach(hInstance);
	}
}
static void OnDllThreadDetach(HINSTANCE hInstance){
	if(_MCFCRT_OnDllThreadDetach){
		_MCFCRT_OnDllThreadDetach(hInstance);
	}
	__MCFCRT_TlsCleanup();
}
static void OnDllProcessDetach(HINSTANCE hInstance, bool bDynamic){
	__MCFCRT_TlsCleanup();

	if(_MCFCRT_OnDllProcessDetach){
		_MCFCRT_OnDllProcessDetach(hInstance, bDynamic);
	}

	__MCFCRT_ModuleUninit();
	__MCFCRT_CppRuntimeUninit();
	__MCFCRT_HeapDbgUninit();
	__MCFCRT_HeapUninit();
	__MCFCRT_StandardStreamsUninit();
}

static bool g_bInitialized = false;

bool __MCFCRT_TlsCallbackGeneric(void *pInstance, unsigned uReason, bool bDynamic){
	bool bRet = false;

	__MCFCRT_SEH_TOP_BEGIN
	{
		switch(uReason){
		case DLL_PROCESS_ATTACH:
			if(g_bInitialized){
				break;
			}
			if(!OnDllProcessAttach((HINSTANCE)pInstance, bDynamic)){
				break;
			}
			g_bInitialized = true;
			bRet = true;
			break;

		case DLL_THREAD_ATTACH:
			OnDllThreadAttach((HINSTANCE)pInstance);
			bRet = true;
			break;

		case DLL_THREAD_DETACH:
			OnDllThreadDetach((HINSTANCE)pInstance);
			bRet = true;
			break;

		case DLL_PROCESS_DETACH:
			if(!g_bInitialized){
				break;
			}
			g_bInitialized = false;
			OnDllProcessDetach((HINSTANCE)pInstance, bDynamic);
			bRet = true;
			break;
		}
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
