// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

__attribute__((__noinline__)) void *realloc(void *p, size_t cb){
	if(p){
		return __MCF_CRT_HeapReAlloc(p, cb, __MCF_DEBUG_RET_ADDR);
	} else {
		return __MCF_CRT_HeapAlloc(cb, __MCF_DEBUG_RET_ADDR);
	}
}
