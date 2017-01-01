// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_SHA256_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_SHA256_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/Array.hpp"
#include <cstdint>

namespace MCF {

class Sha256OutputStream : public AbstractOutputStream {
private:
	int x_nChunkOffset;
	std::uint8_t x_abyChunk[64];
	Array<std::uint32_t, 8> x_au32Reg;
	std::uint64_t x_u64BytesTotal;

public:
	Sha256OutputStream() noexcept
		: x_nChunkOffset(-1)
	{
	}
	~Sha256OutputStream() override;

	Sha256OutputStream(Sha256OutputStream &&) noexcept = default;
	Sha256OutputStream &operator=(Sha256OutputStream &&) noexcept = default;

private:
	void X_Initialize() noexcept;
	void X_Update(const std::uint8_t (&abyChunk)[64]) noexcept;
	void X_Finalize(std::uint8_t (&abyChunk)[64], unsigned uBytesInChunk) noexcept;

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	void Reset() noexcept;
	Array<std::uint8_t, 32> Finalize() noexcept;

	void Swap(Sha256OutputStream &rhs) noexcept {
		using std::swap;
		swap(x_nChunkOffset,  rhs.x_nChunkOffset);
		swap(x_abyChunk,      rhs.x_abyChunk);
		swap(x_au32Reg,       rhs.x_au32Reg);
		swap(x_u64BytesTotal, rhs.x_u64BytesTotal);
	}

public:
	friend void swap(Sha256OutputStream &lhs, Sha256OutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
