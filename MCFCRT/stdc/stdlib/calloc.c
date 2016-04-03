// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

__attribute__((__noinline__))
void *__wrap_calloc(size_t nmemb, size_t cnt){
	size_t cb = 0;
	if((nmemb > 0) && (cnt > 0)){
		cb = nmemb * cnt;
		if(((nmemb | cnt) & (size_t)-0x10000) && (cb / cnt != nmemb)){
			return nullptr;
		}
	}
	void *const ret = __MCFCRT_HeapAlloc(cb, __builtin_return_address(0));
	if(ret){
		__builtin_memset(ret, 0, cb);
	}
	return ret;
}

__attribute__((__alias__("__wrap_calloc")))
void *calloc(size_t nmemb, size_t cnt);
