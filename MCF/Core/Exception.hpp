// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXCEPTION_HPP__
#define __MCF_EXCEPTION_HPP__

#include <cstddef>

namespace MCF {

struct Exception {
	const char *pszFunction;
	unsigned long ulCode;
	const wchar_t *pwszMessage;
};

static inline void __attribute__((noreturn, always_inline)) Throw(
	const char *pszFunction,
	unsigned long ulCode,
	const wchar_t *pwszMessage = L""
){
	throw ::MCF::Exception{pszFunction, ulCode, pwszMessage};
}

}

#define MCF_THROW(...)	::MCF::Throw(__PRETTY_FUNCTION__, __VA_ARGS__)

#endif
