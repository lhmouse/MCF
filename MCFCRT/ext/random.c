// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../env/_crtdef.h"
#include "../env/clocks.h"

static volatile uint64_t g_u64RandSeed = 0;

uint32_t MCF_GetRandomUint32(){
	uint64_t u64OldSeed, u64Seed;

	u64OldSeed = __atomic_load_n(&g_u64RandSeed, __ATOMIC_CONSUME);
	do {
		u64Seed = u64OldSeed ^ MCF_ReadTimestampCounter64();
		u64Seed *= 6364136223846793005ull;
		u64Seed += 1442695040888963407ull;
	} while(!__atomic_compare_exchange_n(&g_u64RandSeed, &u64OldSeed, u64Seed, false, __ATOMIC_ACQ_REL, __ATOMIC_CONSUME));

	return u64Seed >> 32;
}
uint64_t MCF_GetRandomUint64(){
	return ((uint64_t)MCF_GetRandomUint32() << 32) | MCF_GetRandomUint32();
}
double MCF_GetRandomDouble(){
	return ((int64_t)MCF_GetRandomUint64() & 0x7FFFFFFFFFFFFFFFll) / 0x1p63;
}
