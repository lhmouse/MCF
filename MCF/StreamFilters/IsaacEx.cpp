// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacEx.hpp"
#include "../Utilities/Endian.hpp"
#include "../Hash/Sha256.hpp"
using namespace MCF;

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
	: xm_vKeyHash(GenerateKeyHash(pKey, uKeyLen))
{
}

// 其他非静态成员函数。
void IsaacExEncoder::xDoInit(){
	xm_vIsaacGenerator.Init(xm_vKeyHash.au32Words);
	xm_byLastEncoded = 0;
	xm_lLastHighWord = -1;
}
void IsaacExEncoder::xDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	const auto EncodeByte = [&](unsigned uSeed){
		register auto by = *pbyRead;
		++pbyRead;
		const unsigned char byRot = xm_byLastEncoded & 7;

		by ^= uSeed;
		__asm__ __volatile__("rol %b0, cl \n" : "+q"(by) : "c"(byRot));
		xm_byLastEncoded = by ^ (uSeed >> 8);

		xOutput(by);
	};

	if(uSize > 4){
		unsigned uSeed;
		if(xm_lLastHighWord != -1){
			uSeed = (std::uint16_t)xm_lLastHighWord;
			xm_lLastHighWord = -1;
			EncodeByte(uSeed);
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / 2;
		while(i != 0){
			uSeed = xm_vIsaacGenerator.Get();
			EncodeByte(uSeed);
			EncodeByte(uSeed >> 16);
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		unsigned uSeed;
		if(xm_lLastHighWord == -1){
			const auto u32Word = xm_vIsaacGenerator.Get();
			uSeed = u32Word;
			xm_lLastHighWord = (long)(u32Word >> 16);
		} else {
			uSeed = (std::uint16_t)xm_lLastHighWord;
			xm_lLastHighWord = -1;
		}
		EncodeByte(uSeed);
	}
}
void IsaacExEncoder::xDoFinalize(){
}

// ========== IsaacExDecoder ==========
// 构造函数和析构函数。
IsaacExDecoder::IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept
	: xm_vKeyHash(GenerateKeyHash(pKey, uKeyLen))
{
}

// 其他非静态成员函数。
void IsaacExDecoder::xDoInit(){
	xm_vIsaacGenerator.Init(xm_vKeyHash.au32Words);
	xm_byLastEncoded = 0;
	xm_lLastHighWord = -1;
}
void IsaacExDecoder::xDoUpdate(const void *pData, std::size_t uSize){
	auto pbyRead = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyRead + uSize;

	const auto DecodeByte = [&](unsigned uSeed){
		register auto by = *pbyRead;
		++pbyRead;
		const unsigned char byRot = xm_byLastEncoded & 7;

		xm_byLastEncoded = by ^ (uSeed >> 8);
		__asm__ __volatile__("ror %b0, cl \n" : "+q"(by) : "c"(byRot));
		by ^= uSeed;

		xOutput(by);
	};

	if(uSize > 4){
		unsigned uSeed;
		if(xm_lLastHighWord != -1){
			uSeed = (std::uint16_t)xm_lLastHighWord;
			xm_lLastHighWord = -1;
			DecodeByte(uSeed);
		}
		register auto i = (std::size_t)(pbyEnd - pbyRead) / 2;
		while(i != 0){
			uSeed = xm_vIsaacGenerator.Get();
			DecodeByte(uSeed);
			DecodeByte(uSeed >> 16);
			--i;
		}
	}
	while(pbyRead != pbyEnd){
		unsigned uSeed;
		if(xm_lLastHighWord == -1){
			const auto u32Word = xm_vIsaacGenerator.Get();
			uSeed = u32Word;
			xm_lLastHighWord = (long)(u32Word >> 16);
		} else {
			uSeed = (std::uint16_t)xm_lLastHighWord;
			xm_lLastHighWord = -1;
		}
		DecodeByte(uSeed);
	}
}
void IsaacExDecoder::xDoFinalize(){
}
