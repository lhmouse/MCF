// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#undef MCF_ASSERT_MSG_

#ifdef NDEBUG
#	define MCF_ASSERT_MSG_(plain_, expr_, msg_)	\
	((void)0)
#else
#	define MCF_ASSERT_MSG_(plain_, expr_, msg_)	\
	(!(expr_) && (__MCF_CRT_OnAssertFail(L ## plain_, __FILE__, __LINE__, (msg_)), 1))
#endif

#ifndef MCF_CRT_ASSERT_H_
#define MCF_CRT_ASSERT_H_

#include "../env/_crtdef.h"
#include "../env/bail.h"

static inline void __MCF_CRT_OnAssertFail(
	const wchar_t *pwszexpr_ression,
	const char *pszFile, unsigned long ulLine,
	const wchar_t *pwszMessage
) MCF_NOEXCEPT {
	MCF_CRT_BailF(
		L"调试断言失败。\n\n表达式：%ls\n文件　：%hs\n行号　：%lu\n描述　：%ls",
		pwszexpr_ression, pszFile, ulLine, pwszMessage
	);
}

#define ASSERT(expr_)				(MCF_ASSERT_MSG_(#expr_, (expr_), L""))
#define ASSERT_MSG(expr_, msg_)		(MCF_ASSERT_MSG_(#expr_, (expr_), (msg_)))

#endif
