// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "env/mcfwin.h"
#include "env/_seh_top.h"
#include "env/_fpu.h"
#include "env/standard_streams.h"
#include "env/heap.h"
#include "env/heap_dbg.h"
#include "pre/module.h"
#include "env/tls.h"
#include "env/crt_module.h"

__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

static bool RealStartup(void *pInstance, unsigned uReason, bool bDynamic){
	(void)pInstance;
	(void)bDynamic;

	static bool s_bInitialized = false;

	bool bRet = true;

	__MCFCRT_FpuInitialize();

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		if(!s_bInitialized){
			bRet = __MCFCRT_StandardStreamsInit();
			if(!bRet){
				goto jCleanup_00;
			}
			bRet = __MCFCRT_HeapInit();
			if(!bRet){
				goto jCleanup_01;
			}
			bRet = __MCFCRT_HeapDbgInit();
			if(!bRet){
				goto jCleanup_02;
			}
			bRet = __MCFCRT_ModuleInit();
			if(!bRet){
				goto jCleanup_03;
			}
			bRet = __MCFCRT_TlsInit();
			if(!bRet){
				goto jCleanup_04;
			}
			s_bInitialized = true;
		}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if(s_bInitialized){
			s_bInitialized = false;
			__MCFCRT_TlsUninit();
	jCleanup_04:
			__MCFCRT_ModuleUninit();
			__MCFCRT_DiscardCrtModuleQuickExitCallbacks();
	jCleanup_03:
			__MCFCRT_HeapDbgUninit();
	jCleanup_02:
			__MCFCRT_HeapUninit();
	jCleanup_01:
			__MCFCRT_StandardStreamsUninit();
	jCleanup_00:
			;
		}
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

// This file is part of MCFCRT.
// See MCFLicense.txt for licensing information.
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "mcfcrt.h"
#include "env/mcfwin.h"
#include "pre/module.h"
#include "env/tls.h"
#include "env/crt_module.h"

bool __MCFCRT_Init(void){
	if(!__MCFCRT_ModuleInit()){
		return false;
	}
	if(!__MCFCRT_TlsInit()){
		__MCFCRT_ModuleUninit();
		return false;
	}
	return true;
}
void __MCFCRT_Uninit(void){
	__MCFCRT_TlsUninit();
	__MCFCRT_ModuleUninit();
	__MCFCRT_DiscardCrtModuleQuickExitCallbacks();
}
