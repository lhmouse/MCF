// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "module.h"
#include "tls.h"
#include "../mcfcrt.h"
#include "../env/_fpu.h"
#include "../env/xassert.h"
#include "../env/standard_streams.h"
#include "../env/crt_module.h"
#include "../env/bail.h"
#include "../env/thread.h"
#include "../env/mcfwin.h"

__attribute__((__weak__))
extern bool _MCFCRT_OnDllProcessAttach(void *pInstance, bool bDynamic);
__attribute__((__weak__))
extern void _MCFCRT_OnDllProcessDetach(void *pInstance, bool bDynamic);
__attribute__((__weak__))
extern void _MCFCRT_OnDllThreadAttach(void *pInstance);
__attribute__((__weak__))
extern void _MCFCRT_OnDllThreadDetach(void *pInstance);

// -Wl,-e@__MCFCRT_DllStartup
__attribute__((__stdcall__))
extern BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

typedef struct tagDllStartupParams {
	HINSTANCE hInstance;
	DWORD dwReason;
	LPVOID pReserved;
} DllStartupParams;

static unsigned long WrappedDllStartup(void *pOpaque){
	DllStartupParams *const pParams = pOpaque;

	switch(pParams->dwReason){
	case DLL_PROCESS_ATTACH:
		if(!__MCFCRT_InitRecursive()){
			return false;
		}
		if(!__MCFCRT_ModuleInit()){
			__MCFCRT_UninitRecursive();
			return false;
		}
		if(_MCFCRT_OnDllProcessAttach){
			if(!_MCFCRT_OnDllProcessAttach(pParams->hInstance, pParams->pReserved == _MCFCRT_NULLPTR)){
				__MCFCRT_ModuleUninit();
				__MCFCRT_UninitRecursive();
				return false;
			}
		}
		return true;

	case DLL_PROCESS_DETACH:
		if(_MCFCRT_OnDllProcessDetach){
			_MCFCRT_OnDllProcessDetach(pParams->hInstance, pParams->pReserved == _MCFCRT_NULLPTR);
		}
		__MCFCRT_TlsCleanup();
		__MCFCRT_ModuleUninit();
		__MCFCRT_UninitRecursive();
		return true;

	case DLL_THREAD_ATTACH:
		if(_MCFCRT_OnDllThreadAttach){
			_MCFCRT_OnDllThreadAttach(pParams->hInstance);
		}
		return true;

	case DLL_THREAD_DETACH:
		if(_MCFCRT_OnDllThreadDetach){
			_MCFCRT_OnDllThreadDetach(pParams->hInstance);
		}
		return true;

	default:
		return false;
	}
}

__attribute__((__stdcall__))
BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved){
	__MCFCRT_FpuInitialize();
	DllStartupParams vParams = { hInstance, dwReason, pReserved };
	const unsigned long dwResult = _MCFCRT_WrapThreadProcWithSehTop(&WrappedDllStartup, &vParams);
	return dwResult != 0;
}
