// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "dll.h"
#include "../mcfcrt.h"
#include "../env/_seh_top.h"
#include "../env/_fpu.h"
#include "module.h"

// -Wl,-e@__MCFCRT_DllStartup
__MCFCRT_C_STDCALL
extern BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved)
	__asm__("@__MCFCRT_DllStartup");

__MCFCRT_C_STDCALL
BOOL __MCFCRT_DllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved){
	__MCFCRT_FpuInitialize();

	bool bRet = true;

	__MCFCRT_SEH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(!__MCFCRT_InitRecursive()){
				bRet = false;
				break;
			}
			if(!__MCFCRT_ModuleInit()){
				__MCFCRT_UninitRecursive();
				bRet = false;
				break;
			}
			if(_MCFCRT_OnDllProcessAttach){
				if(!_MCFCRT_OnDllProcessAttach(hInstance, !pReserved)){
					__MCFCRT_ModuleUninit();
					__MCFCRT_UninitRecursive();
					bRet = false;
					break;
				}
			}
			break;
		case DLL_THREAD_ATTACH:
			if(_MCFCRT_OnDllThreadAttach){
				_MCFCRT_OnDllThreadAttach(hInstance);
			}
			break;
		case DLL_THREAD_DETACH:
			if(_MCFCRT_OnDllThreadDetach){
				_MCFCRT_OnDllThreadDetach(hInstance);
			}
			break;
		case DLL_PROCESS_DETACH:
			if(_MCFCRT_OnDllProcessDetach){
				_MCFCRT_OnDllProcessDetach(hInstance, !pReserved);
			}
			__MCFCRT_ModuleUninit();
			__MCFCRT_UninitRecursive();
			break;
		}
	}
	__MCFCRT_SEH_TOP_END

	return bRet;
}
