// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "RC4Ex.hpp"
#include "../Hash/SHA256.hpp"
using namespace MCF;

namespace {
	// http://en.wikipedia.org/wiki/RC4
	// 1. 基本算法为 RC4；
	// 2. 输入 RC4 的密钥为原始密钥的散列值；
	// 3. 原文输入 RC4 前被循环移位；
	// 4. 加密输出的字节参与伪随机数生成。
	void GenInitBox(unsigned char (&abyOutput)[256], const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce) noexcept {
		SHA256 Hasher;
		union {
			struct {
				unsigned char abyNoncedKeyHash[32];
				unsigned char abyKeyHash[32];
			};
			unsigned char abyBytes[64];
		} RC4Key;

		Hasher.Update(&u64Nonce, sizeof(u64Nonce));
		Hasher.Update(pKey, uKeyLen);
		Hasher.Finalize(RC4Key.abyNoncedKeyHash);

		Hasher.Update(pKey, uKeyLen);
		Hasher.Finalize(RC4Key.abyKeyHash);

		for(std::size_t i = 0; i < 256; ++i){
			abyOutput[i] = (unsigned char)i;
		}
		unsigned char j = 0;
		for(std::size_t i = 0; i < 256; ++i){
			const auto b0 = abyOutput[i];
			j += b0 + RC4Key.abyBytes[i % sizeof(RC4Key.abyBytes)];
			const auto b1 = abyOutput[j];
			abyOutput[i] = b1;
			abyOutput[j] = b0;
		}
	}

	void Encode(void *pOut, const void *pIn, std::size_t uSize, unsigned char *pbyBox, unsigned char *pbyI, unsigned char *pbyJ) noexcept {
		auto pbyRead = (const unsigned char *)pIn;
		auto pbyWrite = (unsigned char *)pOut;

		auto i = *pbyI;
		auto j = *pbyJ;

		for(std::size_t k = 0; k < uSize; ++k){
			++i;
			const auto b0 = pbyBox[i];
			j += b0;
			const auto b1 = pbyBox[j];
			pbyBox[i] = b1;
			pbyBox[j] = b0;

			register auto ch = *(pbyRead++);
			__asm__ __volatile__(
				"rol %b0, cl \n"
				: "+q"(ch)
				: "c"(b1 & 7)
			);
			ch ^= pbyBox[(unsigned char)(b0 + b1)];
			j += ch;
			*(pbyWrite++) = ch;
		}

		*pbyI = i;
		*pbyJ = j;
	}
	void Decode(void *pOut, const void *pIn, std::size_t uSize, unsigned char *pbyBox, unsigned char *pbyI, unsigned char *pbyJ) noexcept {
		auto pbyRead = (const unsigned char *)pIn;
		auto pbyWrite = (unsigned char *)pOut;

		auto i = *pbyI;
		auto j = *pbyJ;

		for(std::size_t k = 0; k < uSize; ++k){
			++i;
			const auto b0 = pbyBox[i];
			j += b0;
			const auto b1 = pbyBox[j];
			pbyBox[i] = b1;
			pbyBox[j] = b0;

			register auto ch = *(pbyRead++);
			j += ch;
			ch ^= pbyBox[(unsigned char)(b0 + b1)];
			__asm__ __volatile__(
				"ror %b0, cl \n"
				: "+q"(ch)
				: "c"(b1 & 7)
			);
			*(pbyWrite++) = ch;
		}

		*pbyI = i;
		*pbyJ = j;
	}
}

// ========== RC4ExEncoder ==========
// 构造函数和析构函数。
RC4ExEncoder::RC4ExEncoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce)
	: xm_fnDataCallback(std::move(fnDataCallback))
	, xm_bInited(false)
{
	GenInitBox(xm_abyInitBox, pKey, uKeyLen, u64Nonce);
}

// 其他非静态成员函数。
void RC4ExEncoder::Abort() noexcept{
	xm_bInited = false;
}
void RC4ExEncoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		__builtin_memcpy(xm_abyBox, xm_abyInitBox, sizeof(xm_abyBox));
		xm_byI = 0;
		xm_byJ = 0;

		xm_bInited = true;
	}

	auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;
	while(pbyRead != pbyEnd){
		const std::size_t uBytesRemaining = pbyEnd - pbyRead;
		const auto vResult = xm_fnDataCallback(uBytesRemaining);
		const std::size_t uBytesToProcessThisTime = std::min(vResult.second, uBytesRemaining);
		Encode(vResult.first, pbyRead, uBytesToProcessThisTime, xm_abyBox, &xm_byI, &xm_byJ);
		pbyRead += uBytesToProcessThisTime;
	}
}
void RC4ExEncoder::Finalize(){
	xm_bInited = false;
}

// ========== RC4ExDecoder ==========
// 构造函数和析构函数。
RC4ExDecoder::RC4ExDecoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const void *pKey, std::size_t uKeyLen, std::uint64_t u64Nonce)
	: xm_fnDataCallback(std::move(fnDataCallback))
	, xm_bInited(false)
{
	GenInitBox(xm_abyInitBox, pKey, uKeyLen, u64Nonce);
}

// 其他非静态成员函数。
void RC4ExDecoder::Abort() noexcept{
	xm_bInited = false;
}
void RC4ExDecoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		__builtin_memcpy(xm_abyBox, xm_abyInitBox, sizeof(xm_abyBox));
		xm_byI = 0;
		xm_byJ = 0;

		xm_bInited = true;
	}

	auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;
	while(pbyRead != pbyEnd){
		const std::size_t uBytesRemaining = pbyEnd - pbyRead;
		const auto vResult = xm_fnDataCallback(uBytesRemaining);
		const std::size_t uBytesToProcessThisTime = std::min(vResult.second, uBytesRemaining);
		Decode(vResult.first, pbyRead, uBytesToProcessThisTime, xm_abyBox, &xm_byI, &xm_byJ);
		pbyRead += uBytesToProcessThisTime;
	}
}
void RC4ExDecoder::Finalize(){
	xm_bInited = false;
}
