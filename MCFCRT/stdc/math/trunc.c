// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float truncf(float x){
	register float ret;
	uint64_t temp[2];
	__asm__ volatile (
		"fstcw word ptr[%1] \n"
		"fld dword ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__MCFCRT_FLT_RET_ST("%2")
		"fldcw word ptr[%1] \n"
		: __MCFCRT_FLT_RET_CONS(ret), "=m"(temp)
		: "m"(x)
		: "ax", "cx"
	);
	return ret;
}

double trunc(double x){
	register double ret;
	uint64_t temp[2];
	__asm__ volatile (
		"fstcw word ptr[%1] \n"
		"fld qword ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__MCFCRT_DBL_RET_ST("%2")
		"fldcw word ptr[%1] \n"
		: __MCFCRT_DBL_RET_CONS(ret), "=m"(temp)
		: "m"(x)
		: "ax", "cx"
	);
	return ret;
}

long double truncl(long double x){
	register long double ret;
	uint64_t temp[2];
	__asm__ volatile (
		"fstcw word ptr[%1] \n"
		"fld tbyte ptr[%2] \n"
		"movzx eax, word ptr[%1] \n"
		"mov ecx, eax \n"
		"or ecx, 0x0C00 \n"
		"mov word ptr[%1], cx \n"
		"fldcw word ptr[%1] \n"
		"frndint \n"
		"mov word ptr[%1], ax \n"
		__MCFCRT_LDBL_RET_ST("%2")
		"fldcw word ptr[%1] \n"
		: __MCFCRT_LDBL_RET_CONS(ret), "=m"(temp)
		: "m"(x)
		: "ax", "cx"
	);
	return ret;
}
