// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "../env/clocks.h"

static volatile uint64_t g_u64RandSeed = 0;

uint32_t MCFCRT_GetRandomUint32(){
	uint64_t u64OldSeed, u64Seed;

	u64OldSeed = __atomic_load_n(&g_u64RandSeed, __ATOMIC_RELAXED);
	do {
		u64Seed = u64OldSeed ^ MCFCRT_ReadTimestampCounter32();
		u64Seed *= 6364136223846793005ull;
		u64Seed += 1442695040888963407ull;
	} while(!__atomic_compare_exchange_n(&g_u64RandSeed, &u64OldSeed, u64Seed, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));

	return (uint32_t)(u64Seed >> 32);
}
uint64_t MCFCRT_GetRandomUint64(){
	return ((uint64_t)MCFCRT_GetRandomUint32() << 32) | MCFCRT_GetRandomUint32();
}
double MCFCRT_GetRandomDouble(){
	return (double)((int64_t)MCFCRT_GetRandomUint64() & 0x7FFFFFFFFFFFFFFFll) / 0x1p63;
}
