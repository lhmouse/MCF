// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "random.h"
#include "../env/clocks.h"

uint32_t _MCFCRT_GetRandomUint32(void){
	static volatile uint64_t s_u64Seed = 0;
	uint64_t u64OldSeed, u64Seed;
	u64OldSeed = __atomic_load_n(&s_u64Seed, __ATOMIC_RELAXED);
	do {
		u64Seed = (u64OldSeed ^ _MCFCRT_ReadTimeStampCounter32()) * 6364136223846793005u + 1442695040888963407u;
	} while(!__atomic_compare_exchange_n(&s_u64Seed, &u64OldSeed, u64Seed, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	return (uint32_t)(u64Seed >> 32);
}
uint64_t _MCFCRT_GetRandomUint64(void){
	return ((uint64_t)_MCFCRT_GetRandomUint32() << 32) | _MCFCRT_GetRandomUint32();
}
double _MCFCRT_GetRandomDouble(void){
	return (double)(int64_t)(_MCFCRT_GetRandomUint64() >> 1) / 0x1p63;
}
long double _MCFCRT_GetRandomLongDouble(void){
	return (long double)(int64_t)(_MCFCRT_GetRandomUint64() >> 1) / 0x1p63;
}
