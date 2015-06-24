// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"

// 参见 gcc/libgcc/unwind-dw2-fde.h 里面的 old_object 的注释。
struct object {
	void *impl[6]; // 实际上在这里这个尺寸并不重要。
};

extern const uintptr_t __MCF_CRT_EhFrameBegin[];

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

void __MCF_CRT_RegisterFrameInfo(){
	static struct object s_obj;

	__register_frame_info(__MCF_CRT_EhFrameBegin + 1, &s_obj);
}
void __MCF_CRT_UnregisterFrameInfo(){
	__deregister_frame_info(__MCF_CRT_EhFrameBegin + 1);
}
