// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

static const double POS_HALF = 0.5;
static const double NEG_HALF = -0.5;

float roundf(float x){
	register float ret;
	uintptr_t unused;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%3] \n"
#ifdef _WIN64
		"movsx rdx, dword ptr[%2] \n"
		"sar rdx, 63 \n"
		"xor %4, %5 \n"
		"and %4, rdx \n"
#else
		"mov edx, dword ptr[%2] \n"
		"sar edx, 31 \n"
		"xor %4, %5 \n"
		"and %4, edx \n"
#endif
		"xor %4, %5 \n"
		"fld dword ptr[%2] \n"
		"movzx eax, word ptr[%3] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"fadd qword ptr[%4] \n"
		"mov word ptr[%3], cx \n"
		"fldcw word ptr[%3] \n"
		"frndint \n"
		"mov word ptr[%3], ax \n"
		__FLT_RET_ST("%2")
		"fldcw word ptr[%3] \n"
		: __FLT_RET_CONS(ret), "=r"(unused)
		: "m"(x), "m"(fcw), "1"(&NEG_HALF), "r"(&POS_HALF)
		: "ax", "cx", "dx"
	);
	return ret;
}

double round(double x){
	register double ret;
	uintptr_t unused;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%3] \n"
#ifdef _WIN64
		"movsx rdx, dword ptr[%2 + 4] \n"
		"sar rdx, 63 \n"
		"xor %4, %5 \n"
		"and %4, rdx \n"
#else
		"mov edx, dword ptr[%2 + 4] \n"
		"sar edx, 31 \n"
		"xor %4, %5 \n"
		"and %4, edx \n"
#endif
		"xor %4, %5 \n"
		"fld qword ptr[%2] \n"
		"movzx eax, word ptr[%3] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"fadd qword ptr[%4] \n"
		"mov word ptr[%3], cx \n"
		"fldcw word ptr[%3] \n"
		"frndint \n"
		"mov word ptr[%3], ax \n"
		__DBL_RET_ST("%2")
		"fldcw word ptr[%3] \n"
		: __DBL_RET_CONS(ret), "=r"(unused)
		: "m"(x), "m"(fcw), "1"(&NEG_HALF), "r"(&POS_HALF)
		: "ax", "cx", "dx"
	);
	return ret;
}

__LDBL_DECL(roundl, long double x){
	register long double ret;
	uintptr_t unused;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%3] \n"
#ifdef _WIN64
		"movsx rdx, word ptr[%2 + 8] \n"
		"sar rdx, 63 \n"
		"xor %4, %5 \n"
		"and %4, rdx \n"
#else
		"movsx edx, word ptr[%2 + 8] \n"
		"sar edx, 31 \n"
		"xor %4, %5 \n"
		"and %4, edx \n"
#endif
		"xor %4, %5 \n"
		"fld tbyte ptr[%2] \n"
		"movzx eax, word ptr[%3] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"fadd qword ptr[%4] \n"
		"mov word ptr[%3], cx \n"
		"fldcw word ptr[%3] \n"
		"frndint \n"
		"mov word ptr[%3], ax \n"
		__LDBL_RET_ST()
		"fldcw word ptr[%3] \n"
		: __LDBL_RET_CONS(ret), "=r"(unused)
		: "m"(x), "m"(fcw), "1"(&NEG_HALF), "r"(&POS_HALF), __LDBL_RET_CONS_IN()
		: "ax", "cx", "dx"
	);
	__LDBL_RETURN(ret);
}
