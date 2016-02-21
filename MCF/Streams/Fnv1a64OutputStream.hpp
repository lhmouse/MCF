// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FNV1A_64_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_FNV1A_64_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include <cstdint>

namespace MCF {

// http://www.isthe.com/chongo/tech/comp/fnv/

class Fnv1a64OutputStream : public AbstractOutputStream {
private:
	int x_nChunkOffset;
	std::uint8_t x_abyChunk[8];
	std::uint64_t x_u64Reg;

public:
	constexpr Fnv1a64OutputStream() noexcept
		: x_nChunkOffset(-1), x_abyChunk(), x_u64Reg()
	{
	}
	~Fnv1a64OutputStream() override;

	Fnv1a64OutputStream(Fnv1a64OutputStream &&) noexcept = default;
	Fnv1a64OutputStream& operator=(Fnv1a64OutputStream &&) noexcept = default;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush() const override;

	void Reset() noexcept;
	std::uint64_t Finalize() noexcept;

	void Swap(Fnv1a64OutputStream &rhs) noexcept {
		using std::swap;
		swap(x_nChunkOffset, rhs.x_nChunkOffset);
		swap(x_abyChunk,     rhs.x_abyChunk);
		swap(x_u64Reg,       rhs.x_u64Reg);
	}

	friend void swap(Fnv1a64OutputStream &lhs, Fnv1a64OutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
