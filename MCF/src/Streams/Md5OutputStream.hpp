// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_MD5_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_MD5_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/Array.hpp"
#include <cstdint>

namespace MCF {

class Md5OutputStream : public AbstractOutputStream {
private:
	int x_nChunkOffset = -1;
	std::uint8_t x_abyChunk[64];
	Array<std::uint32_t, 4> x_au32Reg;
	std::uint64_t x_u64BytesTotal;

public:
	Md5OutputStream() noexcept {
	}
	~Md5OutputStream() override;

private:
	void X_Initialize() noexcept;
	void X_Update(const std::uint8_t (&abyChunk)[64]) noexcept;
	void X_Finalize(std::uint8_t (&abyChunk)[64], unsigned uBytesInChunk) noexcept;

public:
	void Put(unsigned char byData) noexcept override;
	void Put(const void *pData, std::size_t uSize) noexcept override;
	void Flush(bool bHard) noexcept override;

	void Reset() noexcept;
	Array<std::uint8_t, 16> Finalize() noexcept;
};

}

#endif
