// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_ENV_XASSERT_H_
#define __MCFCRT_ENV_XASSERT_H_

#include "_crtdef.h"
#include "pp.h"

_MCFCRT_EXTERN_C_BEGIN

__attribute__((__noreturn__)) extern void __MCFCRT_OnAssertionFailure(const wchar_t *__pwszExpression, const wchar_t *__pwszFile, unsigned long __ulLine, const wchar_t *__pwszMessage) _MCFCRT_NOEXCEPT;

#define __MCFCRT_DECLARE_UNREACHABLE()                (__builtin_unreachable(), 1)
#define __MCFCRT_FAIL_ASSERTION(__s_, __msg_)         (__MCFCRT_OnAssertionFailure((__s_),  _MCFCRT_PP_LAZY(_MCFCRT_PP_WIDEN, __FILE__), __LINE__, (__msg_)), 1)

_MCFCRT_EXTERN_C_END

#endif

#undef _MCFCRT_ASSERT
#undef _MCFCRT_ASSERT_MSG

#undef _MCFCRT_DEBUG_CHECK
#undef _MCFCRT_DEBUG_CHECK_MSG

#ifdef NDEBUG
#  define _MCFCRT_ASSERT(__expr_)                     ((void)(((__expr_) ? true : false) || __MCFCRT_DECLARE_UNREACHABLE()))
#  define _MCFCRT_ASSERT_MSG(__expr_, __msg_)         ((void)(((__expr_) ? true : false) || __MCFCRT_DECLARE_UNREACHABLE()))
#  define _MCFCRT_DEBUG_CHECK(__expr_)                ((void)0)
#  define _MCFCRT_DEBUG_CHECK_MSG(__expr_, __msg_)    ((void)0)
#else
#  define _MCFCRT_ASSERT(__expr_)                     ((void)(((__expr_) ? true : false) || __MCFCRT_FAIL_ASSERTION(_MCFCRT_PP_WIDEN(#__expr_), (L""   ))))
#  define _MCFCRT_ASSERT_MSG(__expr_, __msg_)         ((void)(((__expr_) ? true : false) || __MCFCRT_FAIL_ASSERTION(_MCFCRT_PP_WIDEN(#__expr_), (__msg_))))
#  define _MCFCRT_DEBUG_CHECK(__expr_)                _MCFCRT_ASSERT(__expr_)
#  define _MCFCRT_DEBUG_CHECK_MSG(__expr_, __msg_)    _MCFCRT_ASSERT_MSG(__expr_, __msg_)
#endif
