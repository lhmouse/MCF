// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float floorf(float x){
	register float ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"and ecx, 0xF3FF \n"
		"or ecx, 0x0400 \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__MCF_FLT_RET_ST("%1")
		"fldcw word ptr[%2] \n"
		: __MCF_FLT_RET_CONS(ret)
		: "m"(x), "m"(fcw)
		: "ax", "cx"
	);
	return ret;
}

double floor(double x){
	register double ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"and ecx, 0xF3FF \n"
		"or ecx, 0x0400 \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__MCF_DBL_RET_ST("%1")
		"fldcw word ptr[%2] \n"
		: __MCF_DBL_RET_CONS(ret)
		: "m"(x), "m"(fcw)
		: "ax", "cx"
	);
	return ret;
}

long double floorl(long double x){
	register long double ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"and ecx, 0xF3FF \n"
		"or ecx, 0x0400 \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__MCF_LDBL_RET_ST("%1")
		"fldcw word ptr[%2] \n"
		: __MCF_LDBL_RET_CONS(ret)
		: "m"(x), "m"(fcw)
		: "ax", "cx"
	);
	return ret;
}
