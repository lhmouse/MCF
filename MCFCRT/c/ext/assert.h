// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#undef ASSERT_MSG

#ifdef NDEBUG
#	define ASSERT_MSG(exp, msg)		((void)0)
#else
#	define ASSERT_MSG(exp, msg)		(!(exp) && (__MCF_CRT_OnAssertFail(L## #exp, __FILE__, __LINE__, (msg)), 1))
#endif

#ifndef __MCF_CRT_ASSERT_H__
#define __MCF_CRT_ASSERT_H__

#include "../../env/_crtdef.h"
#include "../../env/bail.h"

static inline void __MCF_CRT_OnAssertFail(
	const wchar_t *pwszExpression,
	const char *pszFile,			// 这个使用系统缺省多字节编码。
	__MCF_STD size_t uLine,
	const wchar_t *pwszMessage
){
	__MCF_BailF(
		L"调试断言失败。\n\n表达式：%ls\n文件　：%S\n行号　：%lu\n描述　：%ls",
		pwszExpression,
		pszFile,
		(unsigned long)uLine,
		pwszMessage
	);
}

#define ASSERT(exp)					ASSERT_MSG((exp), L"")
#define BAIL_IF(exp)				ASSERT_MSG(!(exp), L"")
#define BAIL_IF_MSG(exp, msg)		ASSERT_MSG(!(exp), (msg))

#define ASSERT_TRUE(exp)			ASSERT(exp)
#define ASSERT_TRUE_MSG(exp, msg)	ASSERT_MSG(exp, msg)
#define ASSERT_FALSE(exp)			BAIL_IF(exp)
#define ASSERT_FALSE_MSG(exp, msg)	BAIL_IF_MSG(exp, msg)

#endif
