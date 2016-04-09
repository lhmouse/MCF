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

static bool CrtFindEhFrameCallback(intptr_t nContext, const char *pchName, size_t uRawSize, void *pBase, size_t uSize){
	(void)pchName;
	(void)uRawSize;

	if(((char *)pBase <= g_aEhBegin) && (g_aEhBegin < (char *)pBase + uSize)){
		*(void **)nContext = pBase;
		return false; // 终止遍历。
	}
	return true;
}

bool __MCFCRT_RegisterFrameInfo(){
	void *pEhBase = nullptr;
	const bool bResult = _MCFCRT_TraverseModuleSections(&CrtFindEhFrameCallback, (intptr_t)&pEhBase);
	if(!bResult){
		return false;
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
