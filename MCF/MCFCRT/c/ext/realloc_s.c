// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#include "realloc_s.h"

__MCF_CRT_NOINLINE void *realloc(void *ptr, size_t cb);

__MCF_CRT_EXTERN int _realloc_s(void **old, size_t cb){
	void *const new_ptr = realloc(*old, cb);
	if(new_ptr == NULL){
		return 0;
	}
	*old = new_ptr;
	return -1;
}
