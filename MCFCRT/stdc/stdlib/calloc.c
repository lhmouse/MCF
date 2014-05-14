// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

extern void *memset(void *dst, int ch, size_t cb);

void *__attribute__((__noinline__)) calloc(size_t nmemb, size_t cnt){
	size_t cb = 0;
	if((nmemb > 0) && (cnt > 0)){
		cb = nmemb * cnt;
		if(((nmemb | cnt) & (size_t)-0x10000) && (cb / cnt != nmemb)){
			return NULL;
		}
	}
	void *const ret = __MCF_CRT_HeapAlloc(cb, __MCF_DEBUG_RET_ADDR);
	if(ret){
		memset(ret, 0, cb);
	}
	return ret;
}
