// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.hpp"

// 定义于 ../stdc/stdlib/atexit.c。
extern "C" int __wrap_atexit(void (*func)(void)) noexcept;

extern "C++" int atexit(void (*func)(void)) noexcept {
	return ::__wrap_atexit(func);
}
