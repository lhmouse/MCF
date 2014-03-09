// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRC32_HPP__
#define __MCF_CRC32_HPP__

#include <cstddef>
#include <cstdint>

namespace MCF {

class CRC32 {
public:
	enum : std::uint32_t {
		DIVISOR_IEEE_802_3 	= 0xEDB88320,
		DIVISOR_CASTAGNOLI	= 0x82F63B78
	};

private:
	std::uint32_t xm_au32Table[0x100];

	bool xm_bInited;
	std::uint32_t xm_u32Reg;

public:
	CRC32(std::uint32_t u32Divisor = DIVISOR_IEEE_802_3) noexcept;

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize) noexcept;
	std::uint32_t Finalize() noexcept;
};

}

#endif
