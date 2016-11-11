// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "random.h"
#include "../env/clocks.h"

static volatile uint64_t g_u64RandSeed = 0;

uint32_t _MCFCRT_GetRandomUint32(void){
	uint64_t u64OldSeed, u64Seed;

	u64OldSeed = __atomic_load_n(&g_u64RandSeed, __ATOMIC_RELAXED);
	do {
		u64Seed = u64OldSeed ^ _MCFCRT_ReadTimeStampCounter32();
		u64Seed *= 6364136223846793005ull;
		u64Seed += 1442695040888963407ull;
	} while(!__atomic_compare_exchange_n(&g_u64RandSeed, &u64OldSeed, u64Seed, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

	return (uint32_t)(u64Seed >> 32);
}
uint64_t _MCFCRT_GetRandomUint64(void){
	return ((uint64_t)_MCFCRT_GetRandomUint32() << 32) | _MCFCRT_GetRandomUint32();
}
double _MCFCRT_GetRandomDouble(void){
	return (double)(int64_t)(_MCFCRT_GetRandomUint64() >> 1) / 0x1p63;
}
