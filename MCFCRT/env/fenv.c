// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "fenv.h"

#define RND_NEAREST		0				// 四舍六入五凑双。
#define RND_DOWN		1				// 向负无穷舍入。
#define RND_UP			2				// 向正无穷舍入。
#define RND_ZERO		3				// 向零（截断）舍入。

#define PRCS_SINGLE		0				// 24 位。
#define PRCS_DOUBLE		2				// 53 位。
#define PRCS_EXTENDED	3				// 64 位。

#define EXCEPT_PM		(1 << 5)		// 精度损失异常。
#define EXCEPT_UM		(1 << 4)		// 下溢异常。
#define EXCEPT_OM		(1 << 3)		// 上溢异常。
#define EXCEPT_ZM		(1 << 2)		// 除以零异常。
#define EXCEPT_DM		(1 << 1)		// 非规格化数异常。
#define EXCEPT_IM		(1 << 0)		// 无效操作异常。

#define ROUNDING		RND_ZERO
#define PRECISION		PRCS_EXTENDED
#define EXCEPT_MASK		(EXCEPT_PM | EXCEPT_DM)

static const uint16_t FPU_CONTROL_WORD	= (ROUNDING << 10) | (PRECISION << 8) | (EXCEPT_MASK);
static const uint32_t MXCSR_REGISTER	= (ROUNDING << 13) |                    (EXCEPT_MASK << 7);

void __MCF_CRT_FEnvInitialize(){
	__asm__ __volatile__("fldcw word ptr[%0]		\n" : : "m"(FPU_CONTROL_WORD));
	__asm__ __volatile__("ldmxcsr dword ptr[%0]		\n" : : "m"(MXCSR_REGISTER));
}
