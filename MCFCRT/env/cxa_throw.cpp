// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/hooks.h"
#include "../env/_crtdef.hpp"
#include "../ext/unref_param.h"
#include <cxxabi.h>
#include <typeinfo>

// weak
void MCF_OnException(void *, const std::type_info &, const void *) noexcept {
}

#pragma GCC diagnostic ignored "-Wattributes"

extern "C" [[noreturn]]
void __real___cxa_throw(void *, std::type_info *, void (_GLIBCXX_CDTOR_CALLABI *)(void *));

extern "C" [[noreturn]] __attribute__((__noinline__))
void __wrap___cxa_throw(void *pException, std::type_info *pTypeInfo, void (_GLIBCXX_CDTOR_CALLABI *pfnDtor)(void *)){
	MCF_OnException(pException, *pTypeInfo, __builtin_return_address(0));

	::__real___cxa_throw(pException, pTypeInfo, pfnDtor);
}
