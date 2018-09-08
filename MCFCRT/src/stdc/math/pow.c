// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_asm_fpu.h"

#undef powf
#undef pow
#undef powl

static inline long double fpu_pow(long double x, long double y){
	// ISO/IEC C11 N1570
	// F.10.4.4 The pow functions
	//  1. pow(±0, y) returns ±∞ and raises the "divide-by-zero" floating-point exception for y an odd integer < 0.
	//  2. pow(±0, y) returns +∞ and raises the "divide-by-zero" floating-point exception for y < 0, finite, and not an odd integer.
	//  3. pow(±0, -∞) returns +∞ and may raise the "divide-by-zero" floating-point exception.
	//  4. pow(±0, y) returns ±0 for y an odd integer > 0.
	//  5. pow(±0, y) returns +0 for y > 0 and not an odd integer.
	//  6. pow(-1, ±∞) returns 1.
	//  7. pow(+1, y) returns 1 for any y, even a NaN.
	//  8. pow(x, ±0) returns 1 for any x, even a NaN.
	//  9. pow(x, y) returns a NaN and raises the "invalid" floating-point exception for finite x < 0 and finite non-integer y.
	// 10. pow(x, -∞) returns +∞ for |x| < 1.
	// 11. pow(x, -∞) returns +0 for |x| > 1.
	// 12. pow(x, +∞) returns +0 for |x| < 1.
	// 13. pow(x, +∞) returns +∞ for |x| > 1.
	// 14. pow(-∞, y) returns -0 for y an odd integer < 0.
	// 15. pow(-∞, y) returns +0 for y < 0 and not an odd integer.
	// 16. pow(-∞, y) returns -∞ for y an odd integer > 0.
	// 17. pow(-∞, y) returns +∞ for y > 0 and not an odd integer.
	// 18. pow(+∞, y) returns +0 for y < 0.
	// 19. pow(+∞, y) returns +∞ for y > 0.

	if(x == 1){
		return 1; // Case 7.
	}
	bool ysign;
	const __MCFCRT_FpuExamine yexam = __MCFCRT_fxam(&ysign, y);
	if(yexam == __MCFCRT_kFpuExamineZero){
		return 1; // Case 8.
	}
	bool xsign;
	const __MCFCRT_FpuExamine xexam = __MCFCRT_fxam(&xsign, x);
	if(xexam == __MCFCRT_kFpuExamineNaN){
		return x;
	}
	if(yexam == __MCFCRT_kFpuExamineNaN){
		return y;
	}
	if(xexam == __MCFCRT_kFpuExamineZero){
		if(ysign){
			if(yexam == __MCFCRT_kFpuExamineInfinity){
				return 1 / __MCFCRT_fldz(); // Case 3. Raises the exception.
			}
			bool bits[3];
			if(__MCFCRT_fmod(&bits, y, 2) == -1){
				long double sign_unit = 1;
				if(xsign){
					sign_unit = __MCFCRT_fchs(sign_unit);
				}
				return sign_unit / __MCFCRT_fldz(); // Case 1.
			}
			return 1 / __MCFCRT_fldz(); // Case 2.
		}
		bool bits[3];
		if(__MCFCRT_fmod(&bits, y, 2) == 1){
			return x; // Case 4. Note that x is zero.
		}
		return __MCFCRT_fldz(); // Case 5.
	}
	if(xexam == __MCFCRT_kFpuExamineInfinity){
		if(xsign){
			if(ysign){
				bool bits[3];
				if(__MCFCRT_fmod(&bits, y, 2) == -1){
					return __MCFCRT_fchs(__MCFCRT_fldz()); // Case 14.
				}
				return __MCFCRT_fldz(); // Case 15.
			}
			bool bits[3];
			if(__MCFCRT_fmod(&bits, y, 2) == 1){
				return x; // Case 16. Note that x is -∞.
			}
			return __MCFCRT_fchs(x); // Case 17. Note that x is -∞.
		}
		if(ysign){
			return __MCFCRT_fldz(); // Case 18.
		}
		return x; // Case 19. Note that x is +∞.
	}
	const long double xabs = __MCFCRT_fabs(x);
	if(yexam == __MCFCRT_kFpuExamineInfinity){
		if(xabs == 1){
			return 1; // Case 6. Note that x cannot be 1.
		}
		if(ysign){
			if(xabs < 1){
				return __MCFCRT_fchs(y); // Case 10. Note that y is -∞.
			}
			return __MCFCRT_fldz(); // Case 11.
		}
		if(xabs < 1){
			return __MCFCRT_fldz(); // Case 12.
		}
		return y; // Case 13. Note that y is +∞.
	}
	bool rsign = false;
	if(xsign){
		if(__MCFCRT_frndintany(y) != y){
			return __builtin_nansl("0x4D43463A3A706F77") + __MCFCRT_fldz(); // Case 9.
		}
		bool bits[3];
		rsign = (__MCFCRT_fmod(&bits, y, 2) != 0);
	}
	// x^y = 2^(y*log2(x))
	const long double ylog2x = __MCFCRT_fyl2x(y, xabs);
	const long double i = __MCFCRT_frndintany(ylog2x), m = ylog2x - i;
	long double ret = __MCFCRT_fscale(1, i) * (__MCFCRT_f2xm1(m) + 1);
	if(rsign){
		ret = __MCFCRT_fchs(ret);
	}
	return ret;
}

float powf(float x, float y){
	return (float)fpu_pow(x, y);
}
double pow(double x, double y){
	return (double)fpu_pow(x, y);
}
long double powl(long double x, long double y){
	return fpu_pow(x, y);
}
