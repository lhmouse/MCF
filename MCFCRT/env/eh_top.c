// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "eh_top.h"
#include "mcfwin.h"

// 参见 [gcc trunk]/libgcc/unwind-dw2-fde.h。
// GCC 中无扩展时候数组长度是 6，带扩展时候是 7。
struct object {
	void *opaque[8];
};

__extension__ __attribute__((__section__(".eh_frame$@@@")))
static _Alignas(void *) const char
	g_vEhFrameBegin[0];
static struct object
	g_vObj;

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

void __MCF_CRT_RegisterFrame(){
	__register_frame_info(g_vEhFrameBegin, &g_vObj);
}
void __MCF_CRT_UnregisterFrame(){
	__deregister_frame_info(g_vEhFrameBegin);
}
