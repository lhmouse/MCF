// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/heap.h"

void *__attribute__((__noinline__)) malloc(size_t cb){
	return __MCF_CRT_HeapAlloc(cb, __MCF_DEBUG_RET_ADDR);
}
