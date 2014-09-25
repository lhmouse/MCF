// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IsaacExFilters.hpp"
#include "../Utilities/ByteSwap.hpp"
#include "../Hash/Sha256.hpp"
#include "../Random/IsaacRng.hpp"
using namespace MCF;

// ========== IsaacExEncoder ==========
// 构造函数和析构函数。
IsaacExEncoder::IsaacExEncoder(const void *pKey, std::size_t uKeyLen) noexcept {
	Sha256 vShaHasher;
	vShaHasher.Update(pKey, uKeyLen);
	vShaHasher.Finalize(xm_vKeyHash.aby);
	for(auto &u32 : xm_vKeyHash.au32){
		u32 = BYTE_SWAP_IF_BE(u32);
	}
}
IsaacExEncoder::~IsaacExEncoder() noexcept {
}

// 其他非静态成员函数。
void IsaacExEncoder::Abort() noexcept {
	xm_pIsaacRng.reset();

	StreamFilterBase::Abort();
}
void IsaacExEncoder::Update(const void *pData, std::size_t uSize){
	if(!xm_pIsaacRng){
		xm_pIsaacRng = std::make_unique<IsaacRng>(xm_vKeyHash.au32);
		xm_abyLastEncoded = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		register auto by = *pbyRead;
		const unsigned uSeed = xm_pIsaacRng->Get();

		by ^= uSeed;
		__asm__ __volatile__(
			"rol %b0, cl \n"
			: "+q"(by)
			: "c"(xm_abyLastEncoded & 7)
		);
		xm_abyLastEncoded = by ^ (uSeed >> 8);

		xOutput(by);
		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
void IsaacExEncoder::Finalize(){
	xm_pIsaacRng.reset();

	StreamFilterBase::Finalize();
}

// ========== IsaacExDecoder ==========
// 构造函数和析构函数。
IsaacExDecoder::IsaacExDecoder(const void *pKey, std::size_t uKeyLen) noexcept {
	Sha256 vShaHasher;
	vShaHasher.Update(pKey, uKeyLen);
	vShaHasher.Finalize(xm_vKeyHash.aby);
	for(auto &u32 : xm_vKeyHash.au32){
		u32 = BYTE_SWAP_IF_BE(u32);
	}
}
IsaacExDecoder::~IsaacExDecoder() noexcept {
}

// 其他非静态成员函数。
void IsaacExDecoder::Abort() noexcept {
	xm_pIsaacRng.reset();

	StreamFilterBase::Abort();
}
void IsaacExDecoder::Update(const void *pData, std::size_t uSize){
	if(!xm_pIsaacRng){
		xm_pIsaacRng = std::make_unique<IsaacRng>(xm_vKeyHash.au32);
		xm_abyLastEncoded = 0;
	}

	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		register auto by = *pbyRead;
		const unsigned uSeed = xm_pIsaacRng->Get();

		__asm__ __volatile__(
			"ror %b0, cl \n"
			: "+q"(by)
			: "c"(xm_abyLastEncoded & 7)
		);
		xm_abyLastEncoded = *pbyRead ^ (uSeed >> 8);
		by ^= uSeed;

		xOutput(by);
		StreamFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
void IsaacExDecoder::Finalize(){
	xm_pIsaacRng.reset();

	StreamFilterBase::Finalize();
}
