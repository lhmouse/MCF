// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_MCFWIN_H_
#define MCFBUILD_MCFWIN_H_

#include "common.h"

#undef WINVER
#undef _WIN32_WINNT
#undef WIN32_LEAN_AND_MEAN

#define WINVER                      0x0601
#define _WIN32_WINNT                0x0601
#define WIN32_LEAN_AND_MEAN         1

#include <windows.h>

#endif
