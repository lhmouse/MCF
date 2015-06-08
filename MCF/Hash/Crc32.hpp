// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_HASH_CRC32_HPP_
#define MCF_HASH_CRC32_HPP_

#include <cstddef>
#include <cstdint>

namespace MCF {

class Crc32 {
public:
	enum : std::uint32_t {
		kDivisorIeee802_3	= 0xEDB88320,
		kDivisorCastagnoli	= 0x82F63B78,
	};

private:
	std::uint32_t x_au32Table[0x100];

	bool x_bInited;
	std::uint32_t x_u32Reg;

public:
	explicit Crc32(std::uint32_t u32Divisor = kDivisorIeee802_3) noexcept;

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize) noexcept;
	std::uint32_t Finalize() noexcept;
};

}

#endif
