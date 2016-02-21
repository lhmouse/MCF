// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_CRC32_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_CRC32_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include <cstdint>

namespace MCF {

// 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。

class Crc32OutputStream : public AbstractOutputStream {
private:
	bool x_bInited;
	std::uint32_t x_u32Reg;

public:
	constexpr Crc32OutputStream() noexcept
		: x_bInited(false), x_u32Reg()
	{
	}
	~Crc32OutputStream() override;

	Crc32OutputStream(Crc32OutputStream &&) noexcept = default;
	Crc32OutputStream& operator=(Crc32OutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush() const override;

	void Abort() noexcept;
	std::uint32_t Finalize() noexcept;

	void Swap(Crc32OutputStream &rhs) noexcept {
		using std::swap;
		swap(x_bInited, rhs.x_bInited);
		swap(x_u32Reg,  rhs.x_u32Reg);
	}

	friend void swap(Crc32OutputStream &lhs, Crc32OutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
