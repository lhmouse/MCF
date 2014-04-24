// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

float truncf(float x){
	register float ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld dword ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__FLT_RET_ST("%1")
		"fldcw word ptr[%2] \n"
		: __FLT_RET_CONS(ret)
		: "m"(x), "m"(fcw)
		: "ax", "cx"
	);
	return ret;
}

double trunc(double x){
	register double ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld qword ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__DBL_RET_ST("%1")
		"fldcw word ptr[%2] \n"
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(fcw)
		: "ax", "cx"
	);
	return ret;
}

__LDBL_DECL(truncl, long double x){
	register long double ret;
	uint16_t fcw;
	__asm__ __volatile__(
		"fstcw word ptr[%2] \n"
		"fld tbyte ptr[%1] \n"
		"movzx eax, word ptr[%2] \n"
		"mov ecx, eax \n"
		"or ch, 0x0C \n"
		"mov word ptr[%2], cx \n"
		"fldcw word ptr[%2] \n"
		"frndint \n"
		"mov word ptr[%2], ax \n"
		__LDBL_RET_ST()
		"fldcw word ptr[%2] \n"
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(fcw), __LDBL_RET_CONS_IN()
		: "ax", "cx"
	);
	__LDBL_RETURN(ret);
}
