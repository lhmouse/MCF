// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_mathasm.h"

static const double NEG_THRESHOLD = -0.292892;
static const double POS_THRESHOLD =  0.292892;

float log1pf(float x){
	register float ret;
	__asm__ __volatile__(
		"fldln2 \n"
		"fld dword ptr[%1] \n"
		"fcom qword ptr[%2] \n"
		"fstsw ax \n"
		"test ah, 0x41 \n"
		"jnz 1f \n"
		"fcom qword ptr[%3] \n"
		"fstsw ax \n"
		"test ah, 0x01 \n"
		"jz 1f \n"
		"	fyl2xp1 \n"
		"	jmp 2f \n"
		"1: \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fyl2x \n"
		"2: \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x), "m"(NEG_THRESHOLD), "m"(POS_THRESHOLD)
		: "ax"
	);
	return ret;
}

double log1p(double x){
	register double ret;
	__asm__ __volatile__(
		"fldln2 \n"
		"fld qword ptr[%1] \n"
		"fcom qword ptr[%2] \n"
		"fstsw ax \n"
		"test ah, 0x41 \n"
		"jnz 1f \n"
		"fcom qword ptr[%3] \n"
		"fstsw ax \n"
		"test ah, 0x01 \n"
		"jz 1f \n"
		"	fyl2xp1 \n"
		"	jmp 2f \n"
		"1: \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fyl2x \n"
		"2: \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x), "m"(NEG_THRESHOLD), "m"(POS_THRESHOLD)
		: "ax"
	);
	return ret;
}

__LDBL_DECL(log1pl, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fldln2 \n"
		"fld tbyte ptr[%1] \n"
		"fcom qword ptr[%2] \n"
		"fstsw ax \n"
		"test ah, 0x41 \n"
		"jnz 1f \n"
		"fcom qword ptr[%3] \n"
		"fstsw ax \n"
		"test ah, 0x01 \n"
		"jz 1f \n"
		"	fyl2xp1 \n"
		"	jmp 2f \n"
		"1: \n"
		"fld1 \n"
		"faddp st(1), st \n"
		"fyl2x \n"
		"2: \n"
		__LDBL_RET_ST()
		: __LDBL_RET_CONS(ret)
		: "m"(x), "m"(NEG_THRESHOLD), "m"(POS_THRESHOLD), __LDBL_RET_CONS_IN()
		: "ax"
	);
	__LDBL_RETURN(ret);
}
