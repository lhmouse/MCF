// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm.h"

float remquof(float x, float y, int *quo){
	register float ret;
	__asm__ volatile (
		"fld dword ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"movzx eax, ax \n"
		"shr eax, 6 \n"
		"mov ecx, eax \n"
		"and ecx, 4 \n"
		"shr eax, 3 \n"
		"and eax, 0x21 \n"
		"or ecx, eax \n"
		"shr eax, 4 \n"
		"or ecx, eax \n"
		"and ecx, 7 \n"
		"mov eax, dword ptr[%2] \n"
		"xor eax, dword ptr[%1] \n"
		"fstp st(1) \n"
		"sar eax, 31 \n"
		"xor ecx, eax \n"
		"sub ecx, eax \n"
		__MCFCRT_FLT_RET_ST("%1")
		"mov dword ptr[%3], ecx \n"
		: __MCFCRT_FLT_RET_CONS(ret)
		: "m"(x), "m"(y), "r"(quo)
		: "ax", "cx"
	);
	return ret;
}

double remquo(double x, double y, int *quo){
	register double ret;
	__asm__ volatile (
		"fld qword ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"movzx eax, ax \n"
		"shr eax, 6 \n"
		"mov ecx, eax \n"
		"and ecx, 4 \n"
		"shr eax, 3 \n"
		"and eax, 0x21 \n"
		"or ecx, eax \n"
		"shr eax, 4 \n"
		"or ecx, eax \n"
		"and ecx, 7 \n"
		"mov eax, dword ptr[%2] \n"
		"xor eax, dword ptr[%1] \n"
		"fstp st(1) \n"
		"sar eax, 31 \n"
		"xor ecx, eax \n"
		"sub ecx, eax \n"
		__MCFCRT_DBL_RET_ST("%1")
		"mov dword ptr[%3], ecx \n"
		: __MCFCRT_DBL_RET_CONS(ret)
		: "m"(x), "m"(y), "r"(quo)
		: "ax", "cx"
	);
	return ret;
}

long double remquol(long double x, long double y, int *quo){
	register long double ret;
	__asm__ volatile (
		"fld tbyte ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"1: \n"
		"	fprem1 \n"
		"	fstsw ax \n"
		"	test ah, 4 \n"
		"jnz 1b \n"
		"movzx eax, ax \n"
		"shr eax, 6 \n"
		"mov ecx, eax \n"
		"and ecx, 4 \n"
		"shr eax, 3 \n"
		"and eax, 0x21 \n"
		"or ecx, eax \n"
		"shr eax, 4 \n"
		"or ecx, eax \n"
		"and ecx, 7 \n"
		"movzx eax, word ptr[%2 + 8] \n"
		"xor ax, word ptr[%1 + 8] \n"
		"movsx eax, ax \n"
		"fstp st(1) \n"
		"sar eax, 31 \n"
		"xor ecx, eax \n"
		"sub ecx, eax \n"
		__MCFCRT_LDBL_RET_ST("%1")
		"mov dword ptr[%3], ecx \n"
		: __MCFCRT_LDBL_RET_CONS(ret)
		: "m"(x), "m"(y), "r"(quo)
		: "ax", "cx"
	);
	return ret;
}
