// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"

#pragma GCC diagnostic ignored "-Wswitch-unreachable"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#define UINTPTR_BYTES     (sizeof(uintptr_t))

#undef memcmp

int memcmp(const void *s1, const void *s2, size_t n){
	const unsigned char *rp1 = s1;
	const unsigned char *rp2 = s2;
	const unsigned char *const erp2 = rp2 + n;
	switch((size_t)(erp2 - rp2) / UINTPTR_BYTES % 8){
		do {
#define COMPARE_STEP_(k_)	\
		if(_MCFCRT_EXPECT_NOT(*(const uintptr_t *)rp1 != *(const uintptr_t *)rp2)){	\
			break;	\
		}	\
		rp1 += UINTPTR_BYTES;	\
		rp2 += UINTPTR_BYTES;	\
	case (k_):	\
		;
//=============================================================================
	COMPARE_STEP_(7)  COMPARE_STEP_(6)  COMPARE_STEP_(5)  COMPARE_STEP_(4)
	COMPARE_STEP_(3)  COMPARE_STEP_(2)  COMPARE_STEP_(1)  COMPARE_STEP_(0)
//=============================================================================
#undef COMPARE_STEP_
		} while(_MCFCRT_EXPECT((size_t)(erp2 - rp2) / UINTPTR_BYTES != 0));
	}
	switch((size_t)(erp2 - rp2) % UINTPTR_BYTES){
#define COMPARE_STEP_(k_)	\
		if(_MCFCRT_EXPECT_NOT(*rp1 != *rp2)){	\
			return (*rp1 < *rp2) ? -1 : 1;	\
		}	\
		++rp1;	\
		++rp2;	\
	case (k_):	\
		;
//=============================================================================
	COMPARE_STEP_(7)  COMPARE_STEP_(6)  COMPARE_STEP_(5)  COMPARE_STEP_(4)
	COMPARE_STEP_(3)  COMPARE_STEP_(2)  COMPARE_STEP_(1)  COMPARE_STEP_(0)
//=============================================================================
#undef COMPARE_STEP_
	}
	return 0;
}
