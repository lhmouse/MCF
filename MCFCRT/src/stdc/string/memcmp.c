// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "../../env/expect.h"

#define UINTPTR_BYTES     (sizeof(uintptr_t))

#undef memcmp

int memcmp(const void *s1, const void *s2, size_t n){
	const unsigned char *rp1 = s1;
	const unsigned char *rp2 = s2;
	size_t cnt = n / UINTPTR_BYTES;
	switch(cnt % 32){
		uintptr_t w, c;
#define STEP(k_)	\
			__builtin_memcpy(&w, rp1, UINTPTR_BYTES);	\
			__builtin_memcpy(&c, rp2, UINTPTR_BYTES);	\
			if(_MCFCRT_EXPECT_NOT(w != c)){	\
				cnt = UINTPTR_BYTES;	\
				goto diff_byte;	\
			}	\
			rp1 += UINTPTR_BYTES;	\
			rp2 += UINTPTR_BYTES;	\
			--cnt;	\
			__attribute__((__fallthrough__));	\
	case (k_):	\
			;
//=============================================================================
		do {
	STEP(037)  STEP(036)  STEP(035)  STEP(034)  STEP(033)  STEP(032)  STEP(031)  STEP(030)
	STEP(027)  STEP(026)  STEP(025)  STEP(024)  STEP(023)  STEP(022)  STEP(021)  STEP(020)
	STEP(017)  STEP(016)  STEP(015)  STEP(014)  STEP(013)  STEP(012)  STEP(011)  STEP(010)
	STEP(007)  STEP(006)  STEP(005)  STEP(004)  STEP(003)  STEP(002)  STEP(001)  STEP(000)
		} while(_MCFCRT_EXPECT(cnt != 0));
//=============================================================================
#undef STEP
	}
	cnt = n % UINTPTR_BYTES;
diff_byte:
	while(_MCFCRT_EXPECT(cnt != 0)){
		if(*rp1 != *rp2){
			return (*rp1 < *rp2) ? -1 : 1;
		}
		++rp1;
		++rp2;
		--cnt;
	}
	return 0;
}
