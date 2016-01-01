// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#ifndef NDEBUG
#	undef DEBUG
#	define DEBUG    1
#endif

#undef USE_DL_PREFIX
#define USE_DL_PREFIX   1

#include "../../External/dlmalloc/malloc.c"
