// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

#undef calloc

__attribute__((__noinline__))
void *calloc(size_t nmemb, size_t size){
	return _MCFCRT_calloc(nmemb, size);
}
