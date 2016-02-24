// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Fnv1a64OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Utilities/Endian.hpp"

// http://www.isthe.com/chongo/tech/comp/fnv/

namespace MCF {

namespace {
	void InitializeFnv1a64(std::uint64_t &u64Reg) noexcept {
		u64Reg = 14695981039346656037u;
	}
	void UpdateFnv1a64(std::uint64_t &u64Reg, const std::uint8_t (&abyChunk)[8]) noexcept {
		register auto u64Word = LoadLe(reinterpret_cast<const std::uint64_t *>(abyChunk)[0]);
		for(unsigned i = 0; i < sizeof(u64Word); ++i){
			const unsigned uLow = static_cast<unsigned char>(u64Word);
			u64Word >>= 8;
			u64Reg ^= uLow;
			u64Reg *= 1099511628211u;
		}
	}
	void FinalizeFnv1a64(std::uint64_t &u64Reg, std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept {
		for(unsigned i = 0; i < uBytesInChunk; ++i){
			const unsigned uLow = abyChunk[i];
			u64Reg ^= uLow;
			u64Reg *= 1099511628211u;
		}
	}
}

Fnv1a64OutputStream::~Fnv1a64OutputStream(){
}

void Fnv1a64OutputStream::Put(unsigned char byData){
	Put(&byData, 1);
}

void Fnv1a64OutputStream::Put(const void *pData, std::size_t uSize){
	if(x_nChunkOffset < 0){
		InitializeFnv1a64(x_u64Reg);
		x_nChunkOffset = 0;
	}

	auto pbyRead = static_cast<const unsigned char *>(pData);
	auto uBytesRemaining = uSize;
	const auto uChunkAvail = sizeof(x_abyChunk) - static_cast<unsigned>(x_nChunkOffset);
	if(uBytesRemaining >= uChunkAvail){
		if(x_nChunkOffset != 0){
			std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uChunkAvail);
			pbyRead += uChunkAvail;
			uBytesRemaining -= uChunkAvail;
			UpdateFnv1a64(x_u64Reg, x_abyChunk);
			x_nChunkOffset = 0;
		}
		while(uBytesRemaining >= sizeof(x_abyChunk)){
			UpdateFnv1a64(x_u64Reg, reinterpret_cast<const decltype(x_abyChunk) *>(pbyRead)[0]);
			pbyRead += sizeof(x_abyChunk);
			uBytesRemaining -= (int)sizeof(x_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uBytesRemaining);
		x_nChunkOffset += (int)uBytesRemaining;
	}
}

void Fnv1a64OutputStream::Flush(bool /* bHard */) const {
}

void Fnv1a64OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
std::uint64_t Fnv1a64OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		FinalizeFnv1a64(x_u64Reg, x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
		x_nChunkOffset = -1;
	}
	return x_u64Reg;
}

}
