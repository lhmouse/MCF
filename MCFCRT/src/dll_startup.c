// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "env/mcfwin.h"
#include "env/_seh_top.h"
#include "env/_fpu.h"
#include "env/standard_streams.h"
#include "env/heap.h"
#include "env/heap_dbg.h"
#include "pre/module.h"
#include "env/thread.h"

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
		__MCFCRT_FpuInitialize();
		bRet = __MCFCRT_StandardStreamsInit();
		if(!bRet){
			goto jCleanup00;
		}
		bRet = __MCFCRT_HeapInit();
		if(!bRet){
			goto jCleanup01;
		}
		bRet = __MCFCRT_HeapDbgInit();
		if(!bRet){
			goto jCleanup02;
		}
		bRet = __MCFCRT_ModuleInit();
		if(!bRet){
			goto jCleanup03;
		}
		bRet = __MCFCRT_ThreadEnvInit();
		if(!bRet){
			goto jCleanup04;
		}
		s_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		__MCFCRT_FpuInitialize();
		break;

	case DLL_THREAD_DETACH:
		__MCFCRT_TlsCleanup();
		break;

	case DLL_PROCESS_DETACH:
		if(!s_bInitialized){
			break;
		}
		s_bInitialized = false;
		__MCFCRT_TlsCleanup();
		__MCFCRT_ThreadEnvUninit();
	jCleanup04:
		__MCFCRT_ModuleUninit();
	jCleanup03:
		__MCFCRT_HeapDbgUninit();
	jCleanup02:
		__MCFCRT_HeapUninit();
	jCleanup01:
		__MCFCRT_StandardStreamsUninit();
	jCleanup00:
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
