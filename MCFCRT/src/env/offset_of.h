// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_OFFSET_OF_H_
#define __MCFCRT_ENV_OFFSET_OF_H_

#include "_crtdef.h"

#define _MCFCRT_OFFSET_OF(__s_, __m_)          (__builtin_offsetof(__s_, __m_))
// 成员指针转换成聚合指针。
#define _MCFCRT_DOWN_CAST(__s_, __m_, __p_)    ((__s_ *)((char *)(__p_) - _MCFCRT_OFFSET_OF(__s_, __m_)))

#endif
