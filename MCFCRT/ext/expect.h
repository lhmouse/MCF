// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_EXPECT_H_
#define __MCFCRT_EXT_EXPECT_H_

#include "../env/_crtdef.h"

#define EXPECT(__x_)        (__builtin_expect(!!(__x_), true))
#define EXPECT_NOT(__x_)    (__builtin_expect(!!(__x_), false))

#endif
