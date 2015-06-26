// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"
#include "../env/module.h"
#include "../ext/unref_param.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
struct object {
	void *impl[6];
};

__attribute__((__weak__))
void __register_frame_info(const void *p, struct object *o){
	UNREF_PARAM(p);
	UNREF_PARAM(o);
}
__attribute__((__weak__))
void *__deregister_frame_info(const void *p){
	UNREF_PARAM(p);

	return nullptr;
}

__extension__ __attribute__((__section__(".eh_fram$@@@"), __used__))
static const char _eh_begin[0] = { };

static struct object _eh_obj;
static void *_eh_frame_base;

static bool TraverseModuleSectionsCallback(intptr_t nContext, const char achName[8], void *pBase, size_t uSize){
	UNREF_PARAM(nContext);
	UNREF_PARAM(achName);

	if(((uintptr_t)pBase <= (uintptr_t)_eh_begin) && ((uintptr_t)_eh_begin < (uintptr_t)pBase + uSize)){
		*(void **)nContext = pBase;
		return false;
	}
	return true;
}

bool __MCF_CRT_RegisterFrameInfo(){
	void *pBase = nullptr;
	MCF_CRT_TraverseModuleSections(&TraverseModuleSectionsCallback, (intptr_t)&pBase);
	if(!pBase){
		return false;
	}

	__register_frame_info(pBase, &_eh_obj);
	_eh_frame_base = pBase;
	return true;
}
void __MCF_CRT_UnregisterFrameInfo(){
	void *const pBase = _eh_frame_base;
	_eh_frame_base = nullptr;

	if(pBase){
		__deregister_frame_info(pBase);
	}
}
