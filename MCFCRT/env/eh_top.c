// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"
#include "../env/module.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
typedef struct tagEhObject {
	void *a[8];
} EhObject;

__attribute__((__weak__))
void __register_frame_info(void *pBase, EhObject *pObject){
	(void)pBase;
	(void)pObject;
}
__attribute__((__weak__))
void *__deregister_frame_info(void *pBase){
	(void)pBase;

	return nullptr;
}

typedef void (*__MCFCRT_RegisterFrameInfoProc)(void *pBase, EhObject *pObject);
typedef void *(*__MCFCRT_DeregisterFrameInfoProc)(void *pBase);

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_RegisterFrameInfoProc   __MCFCRT_pfnRegisterFrameInfoProc   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_DeregisterFrameInfoProc __MCFCRT_pfnDeregisterFrameInfoProc = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char g_aEhBegin[0] = { };

static void *   g_pEhBase;
static EhObject g_vEhObject;

bool __MCFCRT_RegisterFrameInfo(){
	_MCFCRT_ModuleSectionInfo vSectionInfo;
	if(!_MCFCRT_EnumerateFirstModuleSection(&vSectionInfo)){
		return false;
	}

	void *pEhBase = nullptr;
	{
		do {
			char *const pchBegin = vSectionInfo.__pBase;
			char *const pchEnd   = pchBegin + vSectionInfo.__uSize;
			if((pchBegin <= g_aEhBegin) && (g_aEhBegin < pchEnd)){
				pEhBase = pchBegin;
				break;
			}
		} while(_MCFCRT_EnumerateNextModuleSection(&vSectionInfo));
	}
	if(!pEhBase){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}

	(*__MCFCRT_pfnRegisterFrameInfoProc)(pEhBase, &g_vEhObject);
	g_pEhBase = pEhBase;
	return true;
}
void __MCFCRT_UnregisterFrameInfo(){
	void *const pEhBase = g_pEhBase;
	g_pEhBase = nullptr;

	if(pEhBase){
		(*__MCFCRT_pfnDeregisterFrameInfoProc)(pEhBase);
	}
}
