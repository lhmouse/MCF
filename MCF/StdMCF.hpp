// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_STD_MCF_HPP__
#define __MCF_STD_MCF_HPP__

#ifdef UNICODE
#	define _UNICODE
#endif

#include "MCFCRT/MCFCRT.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <climits>
#include <cassert>

#include <memory>
#include <atomic>
#include <algorithm>
#include <limits>
#include <utility>
#include <functional>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <tchar.h>

#undef	NULL
#define	NULL	nullptr

#endif
