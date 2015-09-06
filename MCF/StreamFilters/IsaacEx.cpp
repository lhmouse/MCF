// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacEx.hpp"
#include "../Utilities/Endian.hpp"
#include "../Hash/Sha256.hpp"

namespace MCF {

namespace {
	IsaacFilterKeyHash GenerateKeyHash(const void *pKey, std::size_t uKeyLen) noexcept {
		union {
			unsigned char aby[32];
			std::uint32_t au32[8];
		} unHash;
		Sha256 vShaHasher;
		vShaHasher.Update(pKey, uKeyLen);
		vShaHasher.Finalize(unHash.aby);

		IsaacFilterKeyHash vKeyHash;
		for(std::size_t i = 0; i < 8; ++i){
			vKeyHash.au32Words[i] = LoadBe(unHash.au32[i]);
		}
		return vKeyHash;
	}
}

// ========== IsaacExEncoder ==========
// 构造函数和析构函数。
IsaacExEncoder::IsaacExEncoder(const void *pKey, std::size_t uKeyLen) noexcept
	: x_vKeyHash(GenerateKeyHash(pKey, uKeyLen))
{
}

// 其他非静态成员函数。
void IsaacExEncoder::XDoInit(){
	x_vIsaacGenerator.Init(x_vKeyHash.au32Words);
	x_byLastEncoded = 0;
	x_lLastHighWord = -1;
}
void IsaacExEncoder::XDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	const auto EncodeByte = [&](unsigned uSeed){
		register auto by = *pbyRead;
		++pbyRead;
		const unsigned char byRot = x_byLastEncoded & 7;

		by ^= uSeed;
		__asm__ __volatile__("rol %b0, cl \n" : "+q"(by) : "c"(byRot));
		x_byLastEncoded = by ^ (uSeed >> 8);

		XOutput(by);
	};

	if(uSize > 4){
		unsigned uSeed;
		if(x_lLastHighWord != -1){
			uSeed = (std::uint16_t)x_lLastHighWord;
			x_lLastHighWord = -1;
			EncodeByte(uSeed);
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / 2;
		while(i != 0){
			uSeed = x_vIsaacGenerator.Get();
			EncodeByte(uSeed);
			EncodeByte(uSeed >> 16);
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		unsigned uSeed;
		if(x_lLastHighWord == -1){
			const auto u32Word = x_vIsaacGenerator.Get();
			uSeed = u32Word;
			x_lLastHighWord = (long)(u32Word >> 16);
		} else {
			uSeed = (std::uint16_t)x_lLastHighWord;
			x_lLastHighWord = -1;
		}
		EncodeByte(uSeed);
	}
}
void IsaacExEncoder::XDoFinalize(){
}

// ========== IsaacExDecoder ==========
// 构造函数和析构函数。
IsaacExDecoder::IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept
	: x_vKeyHash(GenerateKeyHash(pKey, uKeyLen))
{
}

// 其他非静态成员函数。
void IsaacExDecoder::XDoInit(){
	x_vIsaacGenerator.Init(x_vKeyHash.au32Words);
	x_byLastEncoded = 0;
	x_lLastHighWord = -1;
}
void IsaacExDecoder::XDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	const auto DecodeByte = [&](unsigned uSeed){
		register auto by = *pbyRead;
		++pbyRead;
		const unsigned char byRot = x_byLastEncoded & 7;

		x_byLastEncoded = by ^ (uSeed >> 8);
		__asm__ __volatile__("ror %b0, cl \n" : "+q"(by) : "c"(byRot));
		by ^= uSeed;

		XOutput(by);
	};

	if(uSize > 4){
		unsigned uSeed;
		if(x_lLastHighWord != -1){
			uSeed = (std::uint16_t)x_lLastHighWord;
			x_lLastHighWord = -1;
			DecodeByte(uSeed);
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / 2;
		while(i != 0){
			uSeed = x_vIsaacGenerator.Get();
			DecodeByte(uSeed);
			DecodeByte(uSeed >> 16);
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		unsigned uSeed;
		if(x_lLastHighWord == -1){
			const auto u32Word = x_vIsaacGenerator.Get();
			uSeed = u32Word;
			x_lLastHighWord = (long)(u32Word >> 16);
		} else {
			uSeed = (std::uint16_t)x_lLastHighWord;
			x_lLastHighWord = -1;
		}
		DecodeByte(uSeed);
	}
}
void IsaacExDecoder::XDoFinalize(){
}

}
