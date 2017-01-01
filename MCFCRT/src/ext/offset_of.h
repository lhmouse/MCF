// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_OFFSET_OF_H_
#define __MCFCRT_EXT_OFFSET_OF_H_

#include "../env/_crtdef.h"
#include "_make_constant.h"

#ifdef __cplusplus
#	define __MCFCRT_ADDRESS_OF(__lval_)     (const_cast<char *>(reinterpret_cast<const volatile char (&)[1]>(__lval_)))
#else
#	define __MCFCRT_ADDRESS_OF(__lval_)     ((char *)&(__lval_))
#endif

#define _MCFCRT_OFFSET_OF(__s_, __m_)       (__MCFCRT_MAKE_CONSTANT((_MCFCRT_STD size_t)(__MCFCRT_ADDRESS_OF(((__s_ *)(char *)1)->__m_) - (char *)1)))
// 成员指针转换成聚合指针。
#define _MCFCRT_DOWN_CAST(__s_, __m_, __p_) ((__s_ *)((char *)(__p_) - _MCFCRT_OFFSET_OF(__s_, __m_)))

#endif
