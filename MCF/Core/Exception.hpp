// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXCEPTION_HPP__
#define __MCF_EXCEPTION_HPP__

#include <cstddef>

namespace MCF {

struct Exception {
	unsigned long ulCode;
#ifndef NDEBUG
	struct {
		const char *pszFile;
		std::size_t uLine;
	} DebugInfo;
#endif
};

}

#ifdef NDEBUG
#define MCF_THROW(code)		throw ::MCF::Exception{code}
#else
#define MCF_THROW(code)		throw ::MCF::Exception{code, __FILE__, __LINE__}
#endif

#endif
