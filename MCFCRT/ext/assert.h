// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_EXT_ASSERT_H_
#define __MCFCRT_EXT_ASSERT_H_

#include "../env/_crtdef.h"

__MCFCRT_EXTERN_C_BEGIN

extern __attribute__((__noreturn__))
int __MCFCRT_OnAssertionFailure(const wchar_t *__pwszExpression, const char *__pszFile, unsigned long __ulLine, const wchar_t *__pwszMessage) _MCFCRT_NOEXCEPT;

__MCFCRT_EXTERN_C_END

#define _MCFCRT_ASSERT(__expr_)              __MCFCRT_ASSERT_IMPL(#__expr_, __expr_, L"")
#define _MCFCRT_ASSERT_MSG(__expr_, __msg_)  __MCFCRT_ASSERT_IMPL(#__expr_, __expr_, __msg_)

#endif

#ifdef NDEBUG
#	define __MCFCRT_ASSERT_IMPL(__plain_, __expr_, __msg_)  ((void)(sizeof(__expr_) + sizeof(__msg_)))
#else
#	define __MCFCRT_ASSERT_IMPL(__plain_, __expr_, __msg_)  ((void)(!(__expr_) && __MCFCRT_OnAssertionFailure(L ## __plain_, __FILE__, __LINE__, (__msg_))))
#endif
