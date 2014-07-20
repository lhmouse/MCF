// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"
#include <string.h>

__attribute__((__noinline__)) void *__wrap_calloc(size_t nmemb, size_t cnt){
	size_t cb = 0;
	if((nmemb > 0) && (cnt > 0)){
		cb = nmemb * cnt;
		if(((nmemb | cnt) & (size_t)-0x10000) && (cb / cnt != nmemb)){
			return NULL;
		}
	}
	void *const ret = __MCF_CRT_HeapAlloc(cb, __builtin_return_address(0));
	if(ret){
		memset(ret, 0, cb);
	}
	return ret;
}

void *calloc(size_t nmemb, size_t cnt)
	__attribute__((__alias__("__wrap_calloc")));
