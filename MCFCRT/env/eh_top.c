// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"
#include "../env/module.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
// 6 个指针大小应该足够，我们在这里保留到 8 个。
struct object {
	void *impl[8];
};

extern __attribute__((__weak__))
void __register_frame_info(const void *, struct object *);
extern __attribute__((__weak__))
void *__deregister_frame_info(const void *);

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

bool __MCFCRT_RegisterFrameInfo(){
	if(__register_frame_info){
		const void *const pBase = FindEhFrameBase();
		if(!pBase){
			return false;
		}
		g_pBase = pBase;

		__register_frame_info(pBase, &g_vObject);
	}
	return true;
}
void __MCFCRT_UnregisterFrameInfo(){
	if(__deregister_frame_info){
		const void *const pBase = g_pBase;

		__deregister_frame_info(pBase);
	}
}
