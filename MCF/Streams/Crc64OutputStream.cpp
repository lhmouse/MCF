// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Crc64OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Utilities/Endian.hpp"

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 CRC 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 CRC 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

// 按照 ECMA-182 描述的算法，除数为 0xC96C5795D7870F42。

namespace MCF {

namespace {
	template<unsigned kRoundT, std::uint64_t kRegT>
	struct ElementGenerator {
		static constexpr std::uint64_t kValue = ElementGenerator<kRoundT + 1, (kRegT >> 1) ^ ((kRegT & 1) ? 0xC96C5795D7870F42 : 0)>::kValue;
	};
	template<std::uint64_t kRegT>
	struct ElementGenerator<8, kRegT> {
		static constexpr std::uint64_t kValue = kRegT;
	};

	template<std::size_t ...kIndices>
	constexpr Array<std::uint64_t, sizeof...(kIndices)> GenerateTable(std::index_sequence<kIndices...>) noexcept {
		return { ElementGenerator<0, kIndices>::kValue... };
	}

	constexpr auto kCrcTable = GenerateTable(std::make_index_sequence<256>());

	void InitializeCrc64(std::uint64_t &u64Reg) noexcept {
		u64Reg = static_cast<std::uint64_t>(-1);
	}
	void UpdateCrc64(std::uint64_t &u64Reg, const std::uint8_t (&abyChunk)[8]) noexcept {
		register auto u64Word = LoadLe(reinterpret_cast<const std::uint64_t *>(abyChunk)[0]);
		for(unsigned i = 0; i < sizeof(u64Word); ++i){
			const unsigned uLow = static_cast<unsigned char>(u64Word);
			u64Word >>= 8;
			u64Reg = kCrcTable[(u64Reg ^ uLow) & 0xFF] ^ (u64Reg >> 8);
		}
	}
	void FinalizeCrc64(std::uint64_t &u64Reg, std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept {
		for(unsigned i = 0; i < uBytesInChunk; ++i){
			const unsigned uLow = abyChunk[i];
			u64Reg = kCrcTable[(u64Reg ^ uLow) & 0xFF] ^ (u64Reg >> 8);
		}
		u64Reg = ~u64Reg;
	}
}

Crc64OutputStream::~Crc64OutputStream(){
}

void Crc64OutputStream::Put(unsigned char byData){
	Put(&byData, 1);
}

void Crc64OutputStream::Put(const void *pData, std::size_t uSize){
	if(x_nChunkOffset < 0){
		InitializeCrc64(x_u64Reg);
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
			UpdateCrc64(x_u64Reg, x_abyChunk);
			x_nChunkOffset = 0;
		}
		while(uBytesRemaining >= sizeof(x_abyChunk)){
			UpdateCrc64(x_u64Reg, reinterpret_cast<const decltype(x_abyChunk) *>(pbyRead)[0]);
			pbyRead += sizeof(x_abyChunk);
			uBytesRemaining -= (int)sizeof(x_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uBytesRemaining);
		x_nChunkOffset += (int)uBytesRemaining;
	}
}

void Crc64OutputStream::Flush(bool /* bHard */) const {
}

void Crc64OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
std::uint64_t Crc64OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		FinalizeCrc64(x_u64Reg, x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
		x_nChunkOffset = -1;
	}
	return x_u64Reg;
}

}
