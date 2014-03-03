// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#undef __MCF_ASSERT_MSG

#ifdef NDEBUG
#	define __MCF_ASSERT_MSG(plain, exp, msg)	((void)0)
#else
#	define __MCF_ASSERT_MSG(plain, exp, msg)	(!(exp) && (__MCF_CRT_OnAssertFail(L##plain, __FILE__, __LINE__, (msg)), 1))
#endif

#ifndef __MCF_CRT_ASSERT_H__
#define __MCF_CRT_ASSERT_H__

#include "../../env/_crtdef.h"
#include "../../env/bail.h"

static inline void __MCF_CRT_OnAssertFail(
	const wchar_t *pwszExpression,
	const char *pszFile,
	unsigned long ulLine,
	const wchar_t *pwszMessage
) __MCF_NOEXCEPT {
	__MCF_CRT_BailF(L"调试断言失败。\n\n表达式：%ls\n文件　：%hs\n行号　：%lu\n描述　：%ls", pwszExpression, pszFile, ulLine, pwszMessage);
}

#define ASSERT(exp)				__MCF_ASSERT_MSG(#exp, (exp), L"")
#define ASSERT_MSG(exp, msg)	__MCF_ASSERT_MSG(#exp, (exp), (msg))

#endif
