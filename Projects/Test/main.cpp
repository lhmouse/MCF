#include <MCF/StdMCF.hpp>
#include <cstdio>
#include <cmath>

#define RND_NEAREST     (0ul)            // 四舍六入五凑双。
#define RND_DOWN        (1ul)            // 向负无穷舍入。
#define RND_UP          (2ul)            // 向正无穷舍入。
#define RND_ZERO        (3ul)            // 向零舍入。

#define PRCS_SINGLE     (0ul)            // 24 位。
#define PRCS_DOUBLE     (2ul)            // 53 位。
#define PRCS_EXTENDED   (3ul)            // 64 位。

#define EXCEPT_PM       (1ul << 5)       // 精度损失异常。
#define EXCEPT_UM       (1ul << 4)       // 下溢异常。
#define EXCEPT_OM       (1ul << 3)       // 上溢异常。
#define EXCEPT_ZM       (1ul << 2)       // 除以零异常。
#define EXCEPT_DM       (1ul << 1)       // 非规格化数异常。
#define EXCEPT_IM       (1ul << 0)       // 无效操作异常。

#define ROUNDING        (RND_NEAREST)
#define PRECISION       (PRCS_EXTENDED)
#define EXCEPT_MASK     0xFF

static const uint16_t kFpCsr = (ROUNDING << 10) | (PRECISION << 8) | (EXCEPT_MASK << 0);
static const uint32_t kMxCsr = (ROUNDING << 13) |                    (EXCEPT_MASK << 7);

extern "C" unsigned _MCFCRT_Main(void) noexcept {
	__asm__ volatile (
		"fnclex \n"
		"fldcw word ptr[%0] \n"
		"ldmxcsr dword ptr[%1] \n"
		:
		: "m"(kFpCsr), "m"(kMxCsr)
	);

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
	std::printf("%g\n", std::pow(+0.0, -5.0));      // inf
	std::printf("%g\n", std::pow(-0.0, -5.0));      // -inf
	std::printf("%g\n", std::pow(+0.0, -5.5));      // inf
	std::printf("%g\n", std::pow(-0.0, -5.5));      // inf
	std::printf("%g\n", std::pow(+0.0, -INFINITY)); // inf
	std::printf("%g\n", std::pow(-0.0, -INFINITY)); // inf
	std::printf("%g\n", std::pow(+0.0, +5.0));      // 0
	std::printf("%g\n", std::pow(-0.0, +5.0));      // -0
	std::printf("%g\n", std::pow(+0.0, +5.5));      // 0
	std::printf("%g\n", std::pow(-0.0, +5.5));      // 0
	std::printf("%g\n", std::pow(-1.0, +INFINITY)); // 1
	std::printf("%g\n", std::pow(-1.0, -INFINITY)); // 1
	std::printf("%g\n", std::pow(+1.0, +INFINITY)); // 1
	std::printf("%g\n", std::pow(+1.0, -INFINITY)); // 1
	std::printf("%g\n", std::pow(+INFINITY, +0.0)); // 1
	std::printf("%g\n", std::pow(-INFINITY, +0.0)); // 1
	std::printf("%g\n", std::pow(+INFINITY, -0.0)); // 1
	std::printf("%g\n", std::pow(-INFINITY, -0.0)); // 1
	std::printf("%g\n", std::pow(+125, +1.0/3));    // 5
	std::printf("%g\n", std::pow(+125, -1.0/3));    // 0.2
	std::printf("%g\n", std::pow(-125, +1.0/3));    // nan
	std::printf("%g\n", std::pow(-125, -1.0/3));    // nan
	std::printf("%g\n", std::pow(+0.5, -INFINITY)); // inf
	std::printf("%g\n", std::pow(-0.5, -INFINITY)); // inf
	std::printf("%g\n", std::pow(+1.5, -INFINITY)); // 0
	std::printf("%g\n", std::pow(-1.5, -INFINITY)); // 0
	std::printf("%g\n", std::pow(+0.5, +INFINITY)); // 0
	std::printf("%g\n", std::pow(-0.5, +INFINITY)); // 0
	std::printf("%g\n", std::pow(+1.5, +INFINITY)); // inf
	std::printf("%g\n", std::pow(-1.5, +INFINITY)); // inf
	std::printf("%g\n", std::pow(-INFINITY, -5.0)); // -0
	std::printf("%g\n", std::pow(-INFINITY, -5.5)); // 0
	std::printf("%g\n", std::pow(-INFINITY, +5.0)); // -inf
	std::printf("%g\n", std::pow(-INFINITY, +5.5)); // inf
	std::printf("%g\n", std::pow(+INFINITY, -5.5)); // 0
	std::printf("%g\n", std::pow(+INFINITY, +5.0)); // inf
	return 0;
}
