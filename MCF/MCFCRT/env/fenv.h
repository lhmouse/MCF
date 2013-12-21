// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_FENV_H__
#define __MCF_CRT_FENV_H__

#include "_crtdef.h"

__MCF_CRT_EXTERN void __MCF_CRTFEnvInitialize();
// __MCF_CRT_EXTERN void __MCF_CRTFEnvUninitialize();
#define __MCF_CRTFEnvUninitialize()		((void)0)

#endif
