// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#undef MCF_ASSERT_MSG_

#ifdef NDEBUG
#	define MCF_ASSERT_MSG_(plain, exp, msg)	((void)0)
#else
#	define MCF_ASSERT_MSG_(plain, exp, msg)	(!(exp) && (__MCF_CRT_OnAssertFail(L##plain, __FILE__, __LINE__, (msg)), 1))
#endif

#ifndef MCF_CRT_ASSERT_H_
#define MCF_CRT_ASSERT_H_

#include "../env/_crtdef.h"
#include "../env/bail.h"

static inline void __MCF_CRT_OnAssertFail(
	const wchar_t *pwszExpression,
	const char *pszFile, unsigned long ulLine,
	const wchar_t *pwszMessage
) MCF_NOEXCEPT {
	MCF_CRT_BailF(
		L"调试断言失败。\n\n表达式：%ls\n文件　：%hs\n行号　：%lu\n描述　：%ls",
		pwszExpression, pszFile, ulLine, pwszMessage
	);
}

#define ASSERT(exp)				(MCF_ASSERT_MSG_(#exp, (exp), L""))
#define ASSERT_MSG(exp, msg)	(MCF_ASSERT_MSG_(#exp, (exp), (msg)))

#endif
