// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "random.h"
#include "../env/clocks.h"

uint32_t _MCFCRT_GetRandom_uint32(void){
	static volatile uint64_t s_seed;

	uint64_t old_seed, new_seed;
	{
		old_seed = __atomic_load_n(&s_seed, __ATOMIC_RELAXED);
		do {
			new_seed = old_seed;
			new_seed ^= _MCFCRT_ReadTimeStampCounter32();
			new_seed *= 6364136223846793005u;
			new_seed += 1442695040888963407u;
		} while(!__atomic_compare_exchange_n(&s_seed, &old_seed, new_seed, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED));
	}
	return (uint32_t)(new_seed >> 32);
}
uint64_t _MCFCRT_GetRandom_uint64(void){
	return ((uint64_t)_MCFCRT_GetRandom_uint32() << 32) + _MCFCRT_GetRandom_uint32();
}
double _MCFCRT_GetRandom_double(void){
	return (double)(int64_t)(_MCFCRT_GetRandom_uint64() >> 1) / 0x1p63;
}
long double _MCFCRT_GetRandom_long_double(void){
	return (long double)(int64_t)(_MCFCRT_GetRandom_uint64() >> 1) / 0x1p63;
}
