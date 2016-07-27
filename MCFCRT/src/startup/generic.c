// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "generic.h"
#include "_frame_info.h"
#include "../env/_seh_top.h"
#include "../env/mcfwin.h"
#include "../env/standard_streams.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/fenv.h"
#include "../env/heap.h"
#include "../env/heap_dbg.h"

static bool RealTlsCallback(void *pInstance, unsigned uReason, bool bDynamic){
	(void)pInstance;
	(void)bDynamic;

	static bool s_bInitialized = false;

	bool bRet = true;

	switch(uReason){
	case DLL_PROCESS_ATTACH:
		if(s_bInitialized){
			break;
		}

		__MCFCRT_FEnvInit();
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
		bRet = __MCFCRT_FrameInfoInit();
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

		s_bInitialized = true;
		break;

	case DLL_THREAD_ATTACH:
		__MCFCRT_FEnvInit();
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
	jCleanup_06:
		__MCFCRT_ModuleUninit();
	jCleanup_05:
		__MCFCRT_FrameInfoUninit();
	jCleanup_04:
		__MCFCRT_HeapDbgUninit();
	jCleanup_03:
		__MCFCRT_HeapUninit();
	jCleanup_02:
		__MCFCRT_StandardStreamsUninit();
	jCleanup_01:
		break;
	}

	return bRet;
}

bool __MCFCRT_TlsCallbackGeneric(void *pInstance, unsigned uReason, bool bDynamic){
	bool bRet;

	__MCFCRT_SEH_TOP_BEGIN
	{
		bRet = RealTlsCallback(pInstance, uReason, bDynamic);
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
