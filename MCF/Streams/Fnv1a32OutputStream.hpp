// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FNV1A32_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_FNV1A32_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include <cstdint>

namespace MCF {

// http://www.isthe.com/chongo/tech/comp/fnv/

class Fnv1a32OutputStream : public AbstractOutputStream {
private:
	int x_nChunkOffset;
	std::uint8_t x_abyChunk[8];
	std::uint32_t x_u32Reg;

public:
	Fnv1a32OutputStream() noexcept
		: x_nChunkOffset(-1)
	{
	}
	~Fnv1a32OutputStream() override;

	Fnv1a32OutputStream(Fnv1a32OutputStream &&) noexcept = default;
	Fnv1a32OutputStream &operator=(Fnv1a32OutputStream &&) noexcept = default;

private:
	void X_Initialize() noexcept;
	void X_Update(const std::uint8_t (&abyChunk)[8]) noexcept;
	void X_Finalize(std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;

	void Flush(bool bHard) override;

	void Reset() noexcept;
	std::uint32_t Finalize() noexcept;

	void Swap(Fnv1a32OutputStream &rhs) noexcept {
		using std::swap;
		swap(x_nChunkOffset, rhs.x_nChunkOffset);
		swap(x_abyChunk,     rhs.x_abyChunk);
		swap(x_u32Reg,       rhs.x_u32Reg);
	}

public:
	friend void swap(Fnv1a32OutputStream &lhs, Fnv1a32OutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
