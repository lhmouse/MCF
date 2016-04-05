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
void __register_frame_info(const void *base, struct object *obj){
	(void)base;
	(void)obj;
}
__attribute__((__weak__))
void *__deregister_frame_info(const void *base){
	(void)base;

	return nullptr;
}

__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __auto_type __MCFCRT_register_frame_info   = &__register_frame_info;
__attribute__((__section__(".MCFCRT"), __shared__))
const volatile __auto_type __MCFCRT_deregister_frame_info = &__deregister_frame_info;

__extension__ __attribute__((__section__(".eh_frame"), __used__))
static const char eh_begin[0] = { };

static struct object eh_obj;
static void *eh_frame_base;

static bool TraverseModuleSectionsCallback(intptr_t context, const char *name, void *base, size_t size){
	(void)name;

	if(((uintptr_t)base <= (uintptr_t)eh_begin) && ((uintptr_t)eh_begin < (uintptr_t)base + size)){
		*(void **)context = base;
		return false;
	}
	return true;
}

bool __MCFCRT_RegisterFrameInfo(){
	void *base = nullptr;
	_MCFCRT_TraverseModuleSections(&TraverseModuleSectionsCallback, (intptr_t)&base);
	if(!base){
		return false;
	}

	(*__MCFCRT_register_frame_info)(base, &eh_obj);
	eh_frame_base = base;
	return true;
}
void __MCFCRT_UnregisterFrameInfo(){
	void *const base = eh_frame_base;
	eh_frame_base = nullptr;

	if(base){
		(*__MCFCRT_deregister_frame_info)(base);
	}
}
