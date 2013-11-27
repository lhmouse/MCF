// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EXEROOT_H__
#define __MCF_EXEROOT_H__

#include "../MCFCRT/MCFCRT.h"

#ifdef _UNICODE
__MCF_CRT_EXTERN unsigned int MCFMain(__MCF_STD size_t argc, const wchar_t *const *argv);
#else
__MCF_CRT_EXTERN unsigned int MCFMain(__MCF_STD size_t argc, const char *const *argv);
#endif

#endif
