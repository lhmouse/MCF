// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_MAKE_CONSTANT_H_
#define __MCFCRT_EXT_MAKE_CONSTANT_H_

#define __MCFCRT_MAKE_CONSTANT(__expr_)     (__builtin_constant_p(__expr_) ? (__expr_) : (__expr_))

#endif
