// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_MATHASM_H_
#define MCF_CRT_MATHASM_H_

#include "../../env/_crtdef.h"

#if defined(__x86_64__)

// x64 ABI 中需要将返回的浮点型数据从 st 拷贝到 SSE 寄存器 xmm0 中。
#	define __MCF_FLT_RET_ST(tmp)		"fstp dword ptr[" tmp "] \n"	\
										"movss xmm0, dword ptr[" tmp "] \n"
#	define __MCF_FLT_RET_CONS(v)		"=Yz"(v)

#	define __MCF_DBL_RET_ST(tmp)		"fstp qword ptr[" tmp "] \n"	\
										"movsd xmm0, qword ptr[" tmp "] \n"
#	define __MCF_DBL_RET_CONS(v)		"=Yz"(v)

// x64 ABI 中 long double 使用引用返回。必须定义一个函数接受隐式传递的返回地址以便应用 RVO。
#	define __MCF_LDBL_DECL(name, ...)	long double name(__VA_ARGS__) __attribute__((alias("__MCF_CRT_Ldbl_" #name)));	\
										static __attribute__((cdecl)) long double *__MCF_CRT_Ldbl_##name(long double *__MCF_LdblRetAddr, __VA_ARGS__)
#	define __MCF_LDBL_RETURN(v)			return __MCF_LdblRetAddr
#	define __MCF_LDBL_RET_ST()			"fstp tbyte ptr[%[__MCF_LdblRetAddr]] \n"
#	define __MCF_LDBL_RET_MEM(mem, tmp)	"mov r" tmp "x, qword ptr[" mem "] \n"	\
										"mov qword ptr[%[__MCF_LdblRetAddr]], r" tmp "x \n"	\
										"movzx r" tmp "x, word ptr[" mem " + 8] \n"	\
										"mov word ptr[%[__MCF_LdblRetAddr] + 8], " tmp "x \n"
#	define __MCF_LDBL_RET_CONS(v)		"=*X"(v)
#	define __MCF_LDBL_RET_CONS_IN()		[__MCF_LdblRetAddr]"r"(__MCF_LdblRetAddr)

#elif defined(__i386__)

// x86 ABI 中保留 st 即可。
#	define __MCF_FLT_RET_ST(tmp)
#	define __MCF_FLT_RET_CONS(v)		"=t"(v)

#	define __MCF_DBL_RET_ST(tmp)
#	define __MCF_DBL_RET_CONS(v)		"=t"(v)

// x86 ABI 中 long double 使用 st 返回。
#	define __MCF_LDBL_DECL(name, ...)	long double name(__VA_ARGS__)
#	define __MCF_LDBL_RETURN(v)			return v
#	define __MCF_LDBL_RET_ST()
#	define __MCF_LDBL_RET_MEM(mem, tmp)	"fld tbyte ptr[" mem "] \n"
#	define __MCF_LDBL_RET_CONS(v)		"=t"(v)
#	define __MCF_LDBL_RET_CONS_IN()		"i"(0)

#else

#	error Oops, this platform is not supported.

#endif

#endif
