// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef __MCFCRT_MATH_MATH_ASM_H_
#define __MCFCRT_MATH_MATH_ASM_H_

#include "../../env/_crtdef.h"

#ifdef _WIN64

// x64 ABI 中需要将返回的浮点型数据从 st 拷贝到 SSE 寄存器 xmm0 中。
#	define __MCFCRT_FLT_RET_ST(__tmp_)      "fstp dword ptr[" __tmp_ "]; movss xmm0, dword ptr[" __tmp_ "]; \n"
#	define __MCFCRT_FLT_RET_CONS(__v_)      "=Yz"(__v_)

#	define __MCFCRT_DBL_RET_ST(__tmp_)      "fstp qword ptr[" __tmp_ "]; movsd xmm0, qword ptr[" __tmp_ "]; \n"
#	define __MCFCRT_DBL_RET_CONS(__v_)      "=Yz"(__v_)

// x64 ABI 中 long double 使用引用返回。我们把返回值放在 st 里，然后让编译器处理其余的事情。
#	define __MCFCRT_LDBL_RET_ST(__tmp_)     "; \n"
#	define __MCFCRT_LDBL_RET_CONS(__v_)     "=t"(__v_)

#else

// x86 ABI 中保留 st 即可。
#	define __MCFCRT_FLT_RET_ST(__tmp_)      "; \n"
#	define __MCFCRT_FLT_RET_CONS(__v_)      "=t"(__v_)

#	define __MCFCRT_DBL_RET_ST(__tmp_)      "; \n"
#	define __MCFCRT_DBL_RET_CONS(__v_)      "=t"(__v_)

#	define __MCFCRT_LDBL_RET_ST(__tmp_)     "; \n"
#	define __MCFCRT_LDBL_RET_CONS(__v_)     "=t"(__v_)

#endif

#endif
