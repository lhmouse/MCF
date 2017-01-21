// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ALLOCA_H_
#define __MCFCRT_EXT_ALLOCA_H_

#include "../env/_crtdef.h"

#define _MCFCRT_ALLOCA(__size_)	\
	(__extension__({	\
		const _MCFCRT_STD size_t __size = __size_;	\
		void *const __ptr = __builtin_alloca(__size);	\
		for(_MCFCRT_STD size_t __offset = 0xFF0; __offset < __size; __offset += 0x1000){	\
			((volatile int *)__ptr)[(__size - __offset) / sizeof(int)] = 0;	\
		}	\
		__ptr;	\
	}))

#endif
