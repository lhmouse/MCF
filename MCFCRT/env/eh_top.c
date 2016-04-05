// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"
#include "../env/module.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
struct object {
	void *impl[8];
};

__attribute__((__weak__))
void __register_frame_info(const void *p, struct object *o){
	(void)p;
	(void)o;
}
__attribute__((__weak__))
void *__deregister_frame_info(const void *p){
	(void)p;

	return nullptr;
}

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __auto_type __MCFCRT_register_frame_info   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __auto_type __MCFCRT_deregister_frame_info = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_fram$@@@"), __used__))
static const char eh_begin[0] = { };
__extension__ __attribute__((__section__(".eh_fram$___"), __used__))
static const char eh_end[16]  = { };

static struct object eh_obj;
static void *eh_frame_base;

static bool TraverseModuleSectionsCallback(intptr_t nContext, const char *pchName, void *pBase, size_t uSize){
	(void)pchName;

	if(((uintptr_t)pBase <= (uintptr_t)eh_begin) && ((uintptr_t)eh_begin < (uintptr_t)pBase + uSize)){
		*(void **)nContext = pBase;
		return false;
	}
	return true;
}

bool __MCFCRT_RegisterFrameInfo(){
	void *pBase = nullptr;
	_MCFCRT_TraverseModuleSections(&TraverseModuleSectionsCallback, (intptr_t)&pBase);
	if(!pBase){
		return false;
	}

	(*__MCFCRT_register_frame_info)(pBase, &eh_obj);
	eh_frame_base = pBase;
	return true;
}
void __MCFCRT_UnregisterFrameInfo(){
	void *const pBase = eh_frame_base;
	eh_frame_base = nullptr;

	if(pBase){
		(*__MCFCRT_deregister_frame_info)(pBase);
	}
}
