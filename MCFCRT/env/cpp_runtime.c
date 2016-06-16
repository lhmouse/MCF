// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "cpp_runtime.h"
#include "mcfwin.h"
#include "module.h"

static bool SupportLibraryInit(void){
	return true;
}
static void SupportLibraryUninit(void){
}

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
// 6 个指针大小应该足够，我们在这里保留到 8 个。
struct object {
	void *impl[8];
};

__attribute__((__weak__))
extern void __register_frame_info(const void *, struct object *);
__attribute__((__weak__))
extern void *__deregister_frame_info(const void *);

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char    g_probe[0] = { };

static const void *  g_eh_base;
static struct object g_eh_object;

bool RegisterFrameInfo(void){
	if(__register_frame_info){
		const void *base = nullptr;
		_MCFCRT_ModuleSectionInfo section;
		for(bool valid = _MCFCRT_EnumerateFirstModuleSection(&section); valid; valid = _MCFCRT_EnumerateNextModuleSection(&section)){
			const char *const begin = section.__pBase;
			const char *const end   = begin + section.__uSize;
			if((begin <= g_probe) && (g_probe < end)){
				base = begin;
				break;
			}
		}
		if(!base){
			SetLastError(ERROR_BAD_FORMAT);
			return false;
		}
		__register_frame_info(base, &g_eh_object);
		g_eh_base = base;
	}
	return true;
}
void DeregisterFrameInfo(void){
	if(__deregister_frame_info){
		const void *const base = g_eh_base;
		if(!base){
			return;
		}
		g_eh_base = nullptr;
		__deregister_frame_info(base);
	}
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
