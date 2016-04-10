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

extern __attribute__((__weak__))
void __register_frame_info(const void *pBase, EhObject *pObject);
extern __attribute__((__weak__))
EhObject *__deregister_frame_info(const void *pBase);

typedef void (*__MCFCRT_RegisterFrameInfoProc)(const void *pBase, EhObject *pObject);
typedef EhObject *(*__MCFCRT_DeregisterFrameInfoProc)(const void *pBase);

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_RegisterFrameInfoProc   __MCFCRT_pfnRegisterFrameInfoProc   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_DeregisterFrameInfoProc __MCFCRT_pfnDeregisterFrameInfoProc = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char g_aEhBegin[0] = { };

static const void *FindEhBase(){
	_MCFCRT_ModuleSectionInfo vSectionInfo;
	if(!_MCFCRT_EnumerateFirstModuleSection(&vSectionInfo)){
		return nullptr;
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
		return nullptr;
	}
	return pEhBase;
}

static const void * g_pEhBase;
static EhObject     g_vEhObject;

bool __MCFCRT_RegisterFrameInfo(){
	if(__MCFCRT_pfnRegisterFrameInfoProc){
		const void *const pEhBase = FindEhBase();
		if(!pEhBase){
			return false;
		}
		g_pEhBase = pEhBase;

		(*__MCFCRT_pfnRegisterFrameInfoProc)(pEhBase, &g_vEhObject);
	}
	return true;
}
void __MCFCRT_UnregisterFrameInfo(){
	if(__MCFCRT_pfnDeregisterFrameInfoProc){
		const void *const pEhBase = g_pEhBase;
		(*__MCFCRT_pfnDeregisterFrameInfoProc)(pEhBase);
	}
}
