// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

__attribute__((__noinline__))
void *__wrap_realloc(void *p, size_t cb){
	if(p){
		return __MCFCRT_HeapRealloc(p, cb, __builtin_return_address(0));
	} else {
		return __MCFCRT_HeapAlloc(cb, __builtin_return_address(0));
	}
}

__attribute__((__alias__("__wrap_realloc")))
void *realloc(void *p, size_t cb);
