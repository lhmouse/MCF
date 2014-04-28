// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"
#include <limits.h>

extern void *memset(void *dst, int ch, size_t cb);

void *__attribute__((noinline)) calloc(size_t nmemb, size_t cnt){
	const uint64_t cb = ((uint64_t)nmemb) * cnt;
	if(cb > SIZE_MAX){
		return NULL;
	}
	void *const ret = __MCF_CRT_HeapAlloc(cb, __builtin_return_address(0));
	if(ret){
		memset(ret, 0, cb);
	}
	return ret;
}
