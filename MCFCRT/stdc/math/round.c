// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"
#include "_constants.h"

float roundf(float x){
	register float ret;
	uint64_t temp[2];
	__asm__ __volatile__(
		"fstcw word ptr[%1] \n"
#ifdef _WIN64
		"movsx rdx, dword ptr[%2] \n"
		"sar rdx, 63 \n"
		"xor %3, %4 \n"
		"and %3, rdx \n"
#else
		"mov edx, dword ptr[%2] \n"
		"sar edx, 31 \n"
		"xor %3, %4 \n"
		"and %3, edx \n"
#endif
		"xor %3, %4 \n"
		"fld dword ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"fadd qword ptr[%3] \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__FLT_RET_ST("%2")
		"fldcw word ptr[%1] \n"
		: __FLT_RET_CONS(ret), "=m"(temp)
		: "m"(x), "r"(&__MCF_CRT_kMath_Neg_0_5), "r"(&__MCF_CRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}

double round(double x){
	register double ret;
	uint64_t temp[2];
	__asm__ __volatile__(
		"fstcw word ptr[%1] \n"
#ifdef _WIN64
		"movsx rdx, dword ptr[%2 + 4] \n"
		"sar rdx, 63 \n"
		"xor %3, %4 \n"
		"and %3, rdx \n"
#else
		"mov edx, dword ptr[%2 + 4] \n"
		"sar edx, 31 \n"
		"xor %3, %4 \n"
		"and %3, edx \n"
#endif
		"xor %3, %4 \n"
		"fld qword ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"fadd qword ptr[%3] \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__DBL_RET_ST("%2")
		"fldcw word ptr[%1] \n"
		: __DBL_RET_CONS(ret), "=m"(temp)
		: "m"(x), "r"(&__MCF_CRT_kMath_Neg_0_5), "r"(&__MCF_CRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}

long double roundl(long double x){
	register long double ret;
	uint64_t temp[2];
	__asm__ __volatile__(
		"fstcw word ptr[%1] \n"
#ifdef _WIN64
		"movsx rdx, word ptr[%2 + 8] \n"
		"sar rdx, 63 \n"
		"xor %3, %4 \n"
		"and %3, rdx \n"
#else
		"movsx edx, word ptr[%2 + 8] \n"
		"sar edx, 31 \n"
		"xor %3, %4 \n"
		"and %3, edx \n"
#endif
		"xor %3, %4 \n"
		"fld tbyte ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"fadd qword ptr[%3] \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__LDBL_RET_ST("%1")
		"fldcw word ptr[%1] \n"
		: __LDBL_RET_CONS(ret), "=m"(temp)
		: "m"(x), "r"(&__MCF_CRT_kMath_Neg_0_5), "r"(&__MCF_CRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}
