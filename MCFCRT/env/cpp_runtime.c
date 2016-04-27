// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "cpp_runtime.h"
#include "mcfwin.h"
#include "module.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
// 6 个指针大小应该足够，我们在这里保留到 8 个。
struct object {
	void *impl[8];
};

extern __attribute__((__weak__))
void __register_frame_info(const void *, struct object *);
extern __attribute__((__weak__))
void *__deregister_frame_info(const void *);

typedef void (*__MCFCRT_EhTopInitProc)(const void *, struct object *);
typedef void *(*__MCFCRT_DeregisterFrameInfoProc)(const void *);

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_EhTopInitProc   __MCFCRT_pfnRegisterFrameInfoProc   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __MCFCRT_DeregisterFrameInfoProc __MCFCRT_pfnDeregisterFrameInfoProc = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char g_aEhBegin[0] = { };

static const void *FindEhFrameBase(){
	_MCFCRT_ModuleSectionInfo vSectionInfo;
	if(!_MCFCRT_EnumerateFirstModuleSection(&vSectionInfo)){
		return nullptr;
	}

	void *pBase = nullptr;
	{
		do {
			char *const pchBegin = vSectionInfo.__pBase;
			char *const pchEnd   = pchBegin + vSectionInfo.__uSize;
			if((pchBegin <= g_aEhBegin) && (g_aEhBegin < pchEnd)){
				pBase = pchBegin;
				break;
			}
		} while(_MCFCRT_EnumerateNextModuleSection(&vSectionInfo));
	}
	if(!pBase){
		SetLastError(ERROR_BAD_FORMAT);
		return nullptr;
	}
	return pBase;
}

static const void *  g_pBase;
static struct object g_vObject;

bool __MCFCRT_CppRuntimeInit(){
	const __MCFCRT_EhTopInitProc pfnRegisterFrameInfoProc = __MCFCRT_pfnRegisterFrameInfoProc;
	if(pfnRegisterFrameInfoProc){
		const void *const pBase = FindEhFrameBase();
		if(!pBase){
			return false;
		}
		(*pfnRegisterFrameInfoProc)(pBase, &g_vObject);
		g_pBase = pBase;
	}
	return true;
}
void __MCFCRT_CppRuntimeUninit(){
	const __MCFCRT_DeregisterFrameInfoProc pfnDeregisterFrameInfoProc = __MCFCRT_pfnDeregisterFrameInfoProc;
	if(pfnDeregisterFrameInfoProc){
		const void *const pBase = g_pBase;
		g_pBase = nullptr;
		(*pfnDeregisterFrameInfoProc)(pBase);
	}
}
