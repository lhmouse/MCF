// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "DllRoot.h"
#include <windows.h>

// -nostartfiles -lshlwapi -lshell32 -ladvapi32 -luser32 -lkernel32 -Wl,-e__MCFExeStartup

extern "C" BOOL WINAPI __MCFDllStartup(HINSTANCE hInstance, DWORD dwReason, LPVOID pReserved){
	switch(dwReason){
	case DLL_PROCESS_ATTACH:
		::__MCF_CRTBegin();
		/*if(pReserved != nullptr){
			__PUTS("DLL_PROCESS_ATTACH - static\n");
		} else {
			__PUTS("DLL_PROCESS_ATTACH - dynamic\n");
		}*/
		break;
	case DLL_THREAD_ATTACH:
		/*__PUTS("DLL_THREAD_ATTACH\n");*/
		break;
	case DLL_THREAD_DETACH:
		/*__PUTS("DLL_THREAD_DETACH\n");*/
		break;
	case DLL_PROCESS_DETACH:
		/*if(pReserved != nullptr){
			__PUTS("DLL_PROCESS_DETACH - terminating\n");
		} else {
			__PUTS("DLL_PROCESS_DETACH - freed or failed\n");
		}*/
		::__MCF_CRTEnd();
		break;
	}
	return FALSE;
}

#ifndef __amd64__
extern "C" __attribute__((alias("__MCFDllStartup@12"))) void __cdecl _MCFDllStartup();
#endif
