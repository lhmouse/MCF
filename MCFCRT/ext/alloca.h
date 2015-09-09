// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_EXT_ALLOCA_H_
#define __MCF_CRT_EXT_ALLOCA_H_

#include "../env/_crtdef.h"

static inline void __MCF_CRT_ProbeStack(void *__pAllocated, MCF_STD size_t __uBytes) MCF_NOEXCEPT {
	const MCF_STD size_t __kPageSize = 0x1000, __kMinToProbe = 0x200;
	const MCF_STD size_t __uPages = (__uBytes + __kPageSize - __kMinToProbe) / __kPageSize;
	for(MCF_STD size_t __i = __uPages; __i != 0; --__i){
		*(volatile MCF_STD uintptr_t *)((char *)__pAllocated + (__i - 1) * __kPageSize);
	}
}

#define ALLOCA(__cb_)	\
	__extension__ ({	\
		const MCF_STD size_t __uBytes_ = (__cb_);	\
		void *const __pAllocated_ = __builtin_alloca(__uBytes_);	\
		__MCF_CRT_ProbeStack(__pAllocated_, __uBytes_);	\
		__pAllocated_;	\
	})

#endif
