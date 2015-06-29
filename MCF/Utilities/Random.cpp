// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Random.hpp"
#include "../Core/Time.hpp"
#include "../Thread/Atomic.hpp"

namespace MCF {

namespace {
	volatile std::uint64_t g_u64RandSeed = 0;
}

std::uint32_t GetRandomUint32() noexcept {
	std::uint64_t u64OldSeed, u64Seed;
	u64OldSeed = AtomicLoad(g_u64RandSeed, MemoryModel::kConsume);
	do {
		u64Seed = u64OldSeed ^ ReadTimestampCounter();
		u64Seed *= 6364136223846793005ull;
		u64Seed += 1442695040888963407ull;
	} while(!AtomicCompareExchange(g_u64RandSeed, u64OldSeed, u64Seed, MemoryModel::kAcqRel, MemoryModel::kConsume));
	return u64Seed >> 32;
}

}
