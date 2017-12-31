// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FNV1A64_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_FNV1A64_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include <cstdint>

namespace MCF {

// http://www.isthe.com/chongo/tech/comp/fnv/

class Fnv1a64OutputStream : public AbstractOutputStream {
private:
	int x_nChunkOffset = -1;
	std::uint8_t x_abyChunk[8];
	std::uint64_t x_u64Reg;

public:
	Fnv1a64OutputStream() noexcept = default;
	~Fnv1a64OutputStream() override;

private:
	void X_Initialize() noexcept;
	void X_Update(const std::uint8_t (&abyChunk)[8]) noexcept;
	void X_Finalize(std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept;

public:
	void Put(unsigned char byData) noexcept override;
	void Put(const void *pData, std::size_t uSize) noexcept override;
	void Flush(bool bHard) noexcept override;

	void Reset() noexcept;
	std::uint64_t Finalize() noexcept;
};

}

#endif
