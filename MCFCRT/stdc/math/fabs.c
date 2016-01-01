// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

#ifdef _WIN64
static alignas(16) uint32_t kFloatMask [] = { 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF };
static alignas(16) uint64_t kDoubleMask[] = { 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF };
#endif

float fabsf(float x){
	register float ret;
	__asm__ __volatile__(
#ifdef _WIN64
		"movaps xmm1, xmmword ptr[%2] \n"
		"andps xmm0, xmm1 \n"
#else
		"mov eax, dword ptr[%1] \n"
		"shl eax, 1 \n"
		"shr eax, 1 \n"
		"mov dword ptr[%1], eax \n"
		"fld dword ptr[%1] \n"
#endif
		: __FLT_RET_CONS(ret)
#ifdef _WIN64
		: "Yz"(x), "m"(kFloatMask)
#else
		: "m"(x)
#endif
	);
	return ret;
}

double fabs(double x){
	register double ret;
	__asm__ __volatile__(
#ifdef _WIN64
		"movapd xmm1, xmmword ptr[%2] \n"
		"andpd xmm0, xmm1 \n"
#else
		"mov eax, dword ptr[%1 + 4] \n"
		"shl eax, 1 \n"
		"shr eax, 1 \n"
		"mov dword ptr[%1 + 4], eax \n"
		"fld qword ptr[%1] \n"
#endif
		: __DBL_RET_CONS(ret)
#ifdef _WIN64
		: "Yz"(x), "m"(kDoubleMask)
#else
		: "m"(x)
#endif
	);
	return ret;
}

long double fabsl(long double x){
	register long double ret;
	__asm__ __volatile__(
#ifdef _WIN64
		"movzx edx, word ptr[%1 + 8] \n"
		"shl edx, 17 \n"
		"shr edx, 17 \n"
		"mov word ptr[%1 + 8], dx \n"
#else
		"movzx edx, word ptr[%1 + 8] \n"
		"shl edx, 17 \n"
		"shr edx, 17 \n"
		"mov word ptr[%1 + 8], dx \n"
		"fld tbyte ptr[%1] \n"
#endif
		: __LDBL_RET_CONS(ret)
		: "m"(x)
		: "dx"
	);
	return ret;
}
