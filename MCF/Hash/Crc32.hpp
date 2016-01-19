// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_HASH_CRC32_HPP_
#define MCF_HASH_CRC32_HPP_

#include <cstddef>
#include <cstdint>

namespace MCF {

// 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。

class Crc32 {
private:
	bool x_bInited;
	std::uint32_t x_u32Reg;

public:
	Crc32() noexcept;

public:
	void Abort() noexcept;
	void Update(const void *pData, std::size_t uSize) noexcept;
	std::uint32_t Finalize() noexcept;
};

}

#endif
