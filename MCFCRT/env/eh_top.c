// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/eh_top.h"
#include "../env/mcfwin.h"

// GCC 中无扩展时候数组长度是 6，带扩展时候是 7。
struct __MCF_CRT_DwarfObject {
	void *opaque[8];
};

struct __MCF_CRT_DwarfObject __eh_dwarf_obj;

__attribute__((__weak__))
void __register_frame_info(const void *p, struct __MCF_CRT_DwarfObject *o){
	(void)p;
	(void)o;
}
__attribute__((__weak__))
void *__deregister_frame_info(const void *p){
	(void)p;
	return nullptr;
}
