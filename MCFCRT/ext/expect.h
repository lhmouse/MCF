// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_EXPECT_H_
#define MCF_CRT_EXPECT_H_

#include "../env/_crtdef.h"

#define EXPECT(x)		(__builtin_expect(!!(x), true))
#define EXPECT_NOT(x)	(__builtin_expect(!!(x), false))

#endif
