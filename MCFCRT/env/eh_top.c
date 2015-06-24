// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"
#include "../ext/unref_param.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
struct object {
	void *impl[6];
};

// 异常处理帧目录初始化，用于 DWARF 实现退栈。
__attribute__((__section__(".eh_frame$@@@"), __used__))
	static const uintptr_t _eh_frame_begin[1] = { 0 };
__attribute__((__section__(".data"), __used__))
	static struct object _eh_obj[1];

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

void __MCF_CRT_RegisterFrameInfo(){
	__register_frame_info(_eh_frame_begin + 1, _eh_obj);
}
void __MCF_CRT_UnregisterFrameInfo(){
	__deregister_frame_info(_eh_frame_begin + 1);
}
