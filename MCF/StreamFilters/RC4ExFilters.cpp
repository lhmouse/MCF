// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "RC4ExFilters.hpp"
#include "../Hash/SHA256.hpp"
using namespace MCF;

// ========== StreamFilterBase ==========
// 构造函数和析构函数。
RC4ExFilterBase::RC4ExFilterBase(const void *pKey, std::size_t uKeyLen) noexcept {
	union {
		struct {
			unsigned char abyMagic1[16];
			unsigned char abyKeyHash[32];
			unsigned char abyMagic2[16];
		};
		unsigned char abyBytes[64];
	} vRC4Key;

	std::memset(vRC4Key.abyMagic1, 0xC5, sizeof(vRC4Key.abyMagic1));
	std::memset(vRC4Key.abyMagic2, 0x3A, sizeof(vRC4Key.abyMagic2));

	Sha256 vSha256;
	vSha256.Update(pKey, uKeyLen);
	vSha256.Finalize(vRC4Key.abyKeyHash);

	for(std::size_t i = 0; i < 256; ++i){
		xm_abyInitBox[i] = i;
	}
	unsigned char j = 0;
	for(std::size_t i = 0; i < 256; ++i){
		const auto b0 = xm_abyInitBox[i];
		j += b0 + vRC4Key.abyBytes[i % sizeof(vRC4Key.abyBytes)];
		const auto b1 = xm_abyInitBox[j];
		xm_abyInitBox[i] = b1;
		xm_abyInitBox[j] = b0;
	}

	xInit();
}

void RC4ExFilterBase::xInit() noexcept {
	BCopy(xm_abyBox, xm_abyInitBox);
	xm_i = 0;
	xm_j = 0;
}

void RC4ExFilterBase::Abort() noexcept {
	xInit();
	StreamFilterBase::Abort();
}
void RC4ExFilterBase::Finalize(){
	xInit();
	StreamFilterBase::Finalize();
}

// ========== RC4ExEncoder ==========
// 构造函数和析构函数。
RC4ExEncoder::RC4ExEncoder(const void *pKey, std::size_t uKeyLen) noexcept
	: CONCRETE_INIT(RC4ExFilterBase, pKey, uKeyLen)
{
}

// 其他非静态成员函数。
void RC4ExEncoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		++xm_i;
		const auto b0 = xm_abyBox[xm_i];
		xm_j += b0;
		const auto b1 = xm_abyBox[xm_j];
		xm_abyBox[xm_i] = b1;
		xm_abyBox[xm_j] = b0;

		register auto by = *pbyRead;
		__asm__ __volatile__(
			"rol %b0, cl \n"
			: "+q"(by)
			: "c"(b1 & 7)
		);
		by ^= xm_abyBox[(unsigned char)(b0 + b1)];
		xm_j += by;
		xOutput(by);

		RC4ExFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}

// ========== RC4ExDecoder ==========
// 构造函数和析构函数。
RC4ExDecoder::RC4ExDecoder(const void *pKey, std::size_t uKeyLen) noexcept
	: CONCRETE_INIT(RC4ExFilterBase, pKey, uKeyLen)
{
}

// 其他非静态成员函数。
void RC4ExDecoder::Update(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	for(std::size_t i = 0; i < uSize; ++i){
		++xm_i;
		const auto b0 = xm_abyBox[xm_i];
		xm_j += b0;
		const auto b1 = xm_abyBox[xm_j];
		xm_abyBox[xm_i] = b1;
		xm_abyBox[xm_j] = b0;

		register auto by = *pbyRead;
		xm_j += by;
		by ^= xm_abyBox[(unsigned char)(b0 + b1)];
		__asm__ __volatile__(
			"ror %b0, cl \n"
			: "+q"(by)
			: "c"(b1 & 7)
		);
		xOutput(by);

		RC4ExFilterBase::Update(pbyRead, 1);
		++pbyRead;
	}
}
