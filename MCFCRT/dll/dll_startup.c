// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../env/mcfwin.h"
#include "../env/bail.h"
#include "../env/module.h"
#include "../env/thread.h"
#include "../env/_eh_top.h"

// -static -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

extern bool MCFDll_OnProcessAttach(bool bDynamic);
extern void MCFDll_OnProcessDetach(bool bDynamic);
extern void MCFDll_OnThreadAttach();
extern void MCFDll_OnThreadDetach();

#pragma GCC optimize "-fno-function-sections"

static __attribute__((__cdecl__, __used__)) BOOL AlignedStartup(HINSTANCE hDll, DWORD dwReason, LPVOID pReserved){
	BOOL bRet = FALSE;
	__MCF_EH_TOP_BEGIN
	{
		switch(dwReason){
		case DLL_PROCESS_ATTACH:
			if(!__MCF_CRT_BeginModule()){
				break;
			}
			if(!MCFDll_OnProcessAttach(!pReserved)){
				break;
			}
			bRet = TRUE;
			break;

		case DLL_PROCESS_DETACH:
			MCFDll_OnProcessDetach(!pReserved);
			__MCF_CRT_EndModule();
			break;

		case DLL_THREAD_ATTACH:
			MCFDll_OnThreadAttach();
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			break;

		case DLL_THREAD_DETACH:
			__MCF_CRT_TlsCallback(hDll, dwReason, pReserved);
			MCFDll_OnThreadDetach();
			break;
		}
	}
	__MCF_EH_TOP_END
	return bRet;
}

__asm__(
	"	.text \n"
	"	.align 16 \n"
	".global __MCF_DllStartup \n"
	"__MCF_DllStartup: \n"
#ifdef _WIN64
	"	jmp AlignedStartup \n"
#else
	"	push ebp \n"
	"	mov ebp, esp \n"
	"	and esp, -0x10 \n"
	"	sub esp, 0x10 \n"
	"	mov eax, dword ptr[ebp + 0x08] \n"
	"	mov dword ptr[esp], eax \n"
	"	mov eax, dword ptr[ebp + 0x0C] \n"
	"	mov dword ptr[esp + 0x04], eax \n"
	"	mov eax, dword ptr[ebp + 0x10] \n"
	"	mov dword ptr[esp + 0x08], eax \n"
	"	call _AlignedStartup \n"
	"	leave \n"
	"	ret 0x0C \n"
#endif
);
