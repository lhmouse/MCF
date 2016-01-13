// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_constants.h"

long long llroundf(float x){
	long long ret;
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
		"fistp qword ptr[%0] \n"
		"fldcw word ptr[%1] \n"
		: "=m"(ret), "=m"(temp)
		: "m"(x), "r"(&__MCFCRT_kMath_Neg_0_5), "r"(&__MCFCRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}

long long llround(double x){
	long long ret;
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
		"fistp qword ptr[%0] \n"
		"fldcw word ptr[%1] \n"
		: "=m"(ret), "=m"(temp)
		: "m"(x), "r"(&__MCFCRT_kMath_Neg_0_5), "r"(&__MCFCRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}

long long llroundl(long double x){
	long long ret;
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
		"fistp qword ptr[%0] \n"
		"fldcw word ptr[%1] \n"
		: "=m"(ret), "=m"(temp)
		: "m"(x), "r"(&__MCFCRT_kMath_Neg_0_5), "r"(&__MCFCRT_kMath_Pos_0_5)
		: "ax", "cx", "dx"
	);
	return ret;
}
