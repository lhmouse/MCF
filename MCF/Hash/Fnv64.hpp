// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_HASH_FNV64_HPP_
#define MCF_HASH_FNV64_HPP_

#include <cstddef>
#include <cstdint>

namespace MCF {

class Fnv64 {
private:
	bool $bInited;
	std::uint64_t $u64Reg;

public:
	Fnv64() noexcept;

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize) noexcept;
	std::uint64_t Finalize() noexcept;
};

}

#endif
