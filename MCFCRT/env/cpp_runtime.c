// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "cpp_runtime.h"
#include "mcfwin.h"
#include "module.h"

bool SupportLibraryInit(void){
	return true;
}
void SupportLibraryUninit(void){
}

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
// 6 个指针大小应该足够，我们在这里保留到 8 个。
struct object {
	void *impl[8];
};

extern __attribute__((__weak__))
void __register_frame_info(const void *, struct object *);
extern __attribute__((__weak__))
void *__deregister_frame_info(const void *);

typedef void (*RegisterFrameInfoProc)(const void *, struct object *);
typedef void *(*DeregisterFrameInfoProc)(const void *);

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile RegisterFrameInfoProc   __MCFCRT_pfnRegisterFrameInfoProc   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile DeregisterFrameInfoProc __MCFCRT_pfnDeregisterFrameInfoProc = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char    g_aEhFrameProbe[0] = { };

static const void *  g_pEhFrameBase;
static struct object g_vEhObject;

bool RegisterFrameInfo(void){
	const RegisterFrameInfoProc pfnRegisterFrameInfo = __MCFCRT_pfnRegisterFrameInfoProc;
	if(!pfnRegisterFrameInfo){
		// 不需要进行任何操作。
		return true;
	}

	const void *pEhFrameBase = nullptr;
	_MCFCRT_ModuleSectionInfo vSection;
	for(bool bHasSection = _MCFCRT_EnumerateFirstModuleSection(&vSection); bHasSection; bHasSection = _MCFCRT_EnumerateNextModuleSection(&vSection)){
		const char *const pchBegin = vSection.__pBase;
		const char *const pchEnd   = pchBegin + vSection.__uSize;
		if((pchBegin <= g_aEhFrameProbe) && (g_aEhFrameProbe < pchEnd)){
			pEhFrameBase = pchBegin;
			break;
		}
	}
	if(!pEhFrameBase){
		SetLastError(ERROR_BAD_FORMAT);
		return false;
	}

	(*pfnRegisterFrameInfo)(pEhFrameBase, &g_vEhObject);
	g_pEhFrameBase = pEhFrameBase;

	return true;
}
void DeregisterFrameInfo(void){
	const DeregisterFrameInfoProc pfnDeregisterFrameInfo = __MCFCRT_pfnDeregisterFrameInfoProc;
	if(!pfnDeregisterFrameInfo){
		// 不需要进行任何操作。
		return;
	}

	const void *const pEhFrameBase = g_pEhFrameBase;
	if(!pEhFrameBase){
		return;
	}

	g_pEhFrameBase = nullptr;
	(*pfnDeregisterFrameInfo)(pEhFrameBase);
}

bool __MCFCRT_CppRuntimeInit(void){
	if(!SupportLibraryInit()){
		return false;
	}
	if(!RegisterFrameInfo()){
		SupportLibraryUninit();
		return false;
	}
	return true;
}
void __MCFCRT_CppRuntimeUninit(void){
	DeregisterFrameInfo();
	SupportLibraryUninit();
}
