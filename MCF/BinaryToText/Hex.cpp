// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Hex.hpp"
#include <cstring>
using namespace MCF;

namespace {

inline void CopyOut(const std::function<std::pair<void *, std::size_t> (std::size_t)> &fnDataCallback, const void *pSrc, std::size_t uBytesToCopy){
	std::size_t uBytesCopied = 0;
	while(uBytesCopied < uBytesToCopy){
		const std::size_t uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto vResult = fnDataCallback(uBytesRemaining);
		const std::size_t uBytesToCopyThisTime = std::min(vResult.second, uBytesRemaining);
		__builtin_memcpy(vResult.first, (const unsigned char *)pSrc + uBytesCopied, uBytesToCopyThisTime);
		uBytesCopied += uBytesToCopyThisTime;
	}
}

class HexEncoderDelegate : CONCRETE(HexEncoder) {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	const unsigned char xm_byDelta;

public:
	HexEncoderDelegate(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, bool bUpperCase)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_byDelta((bUpperCase ? 'A' : 'a') - ('9' + 1))
	{
	}

public:
	void Abort() noexcept{
	}
	void Update(const void *pData, std::size_t uSize){
		alignas(16) char achBuffer[64];
		auto pchWrite = std::begin(achBuffer);

		auto pbyRead = (const unsigned char *)pData;
		const auto pbyEnd = pbyRead + uSize;
		while(pbyRead != pbyEnd){
			const auto by = *(pbyRead++);

			auto byEncoded = (by >> 4) + '0';
			if(byEncoded >= '9' + 1){
				byEncoded += xm_byDelta;
			}
			pchWrite[0] = byEncoded;
			byEncoded = (by & 0x0F) + '0';
			if(byEncoded >= '9' + 1){
				byEncoded += xm_byDelta;
			}
			pchWrite[1] = byEncoded;
			pchWrite += 2;

			if(pchWrite == std::end(achBuffer)){
				CopyOut(xm_fnDataCallback, achBuffer, sizeof(achBuffer));
				pchWrite = std::begin(achBuffer);
			}
		}

		const auto uBytesInBuffer = (std::size_t)(pchWrite - std::begin(achBuffer));
		if(uBytesInBuffer > 0){
			CopyOut(xm_fnDataCallback, achBuffer, uBytesInBuffer);
		}
	}
	void Finalize(){
	}
};

class HexDecoderDelegate : CONCRETE(HexDecoder) {
private:
	const std::function<std::pair<void *, std::size_t> (std::size_t)> xm_fnDataCallback;
	bool xm_bInited;

	unsigned char xm_uchLastDigit;

public:
	HexDecoderDelegate(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback)
		: xm_fnDataCallback(std::move(fnDataCallback))
		, xm_bInited(false)
	{
	}

public:
	void Abort() noexcept{
		xm_bInited = false;
	}
	void Update(const void *pData, std::size_t uSize){
		if(!xm_bInited){
			xm_uchLastDigit = 0xFF;

			xm_bInited = true;
		}

		auto uchLastDigit = xm_uchLastDigit;

		alignas(16) unsigned char abyBuffer[64];
		auto pbyWrite = std::begin(abyBuffer);

		auto pbyRead = (const unsigned char *)pData;
		const auto pbyEnd = pbyRead + uSize;
		while(pbyRead != pbyEnd){
			unsigned char uchDecoded;
			const auto by = *(pbyRead++);
			if(by <= 'F'){
				if('A' <= by){
					uchDecoded = by - 'A' + 0x0A;
				} else if(('0' <= by) && (by <= '9')){
					uchDecoded = by - '0';
				} else {
					continue;
				}
			} else if(('a' <= by) && (by <= 'f')){
				uchDecoded = by - 'a' + 0x0A;
			} else {
				continue;
			}
			if(uchLastDigit == 0xFF){
				uchLastDigit = uchDecoded;
				continue;
			}
			*(pbyWrite++) = (uchLastDigit << 4) | uchDecoded;
			uchLastDigit = 0xFF;

			if(pbyWrite == std::end(abyBuffer)){
				CopyOut(xm_fnDataCallback, abyBuffer, sizeof(abyBuffer));
				pbyWrite = std::begin(abyBuffer);
			}
		}

		const auto uBytesInBuffer = (std::size_t)(pbyWrite - std::begin(abyBuffer));
		if(uBytesInBuffer > 0){
			CopyOut(xm_fnDataCallback, abyBuffer, uBytesInBuffer);
		}

		xm_uchLastDigit = uchLastDigit;
	}
	void Finalize(){
		xm_bInited = false;
	}
};

}

// ========== HexEncoder ==========
// 静态成员函数。
std::unique_ptr<HexEncoder> HexEncoder::Create(
	std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback,
	bool bUpperCase
){
	return std::make_unique<HexEncoderDelegate>(std::move(fnDataCallback), bUpperCase);
}

// 其他非静态成员函数。
void HexEncoder::Abort() noexcept {
	ASSERT(dynamic_cast<HexEncoderDelegate *>(this));

	((HexEncoderDelegate *)this)->Abort();
}
void HexEncoder::Update(const void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<HexEncoderDelegate *>(this));

	((HexEncoderDelegate *)this)->Update(pData, uSize);
}
void HexEncoder::Finalize(){
	ASSERT(dynamic_cast<HexEncoderDelegate *>(this));

	((HexEncoderDelegate *)this)->Finalize();
}

// ========== HexDecoder ==========
// 静态成员函数。
std::unique_ptr<HexDecoder> HexDecoder::Create(
	std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback
){
	return std::make_unique<HexDecoderDelegate>(std::move(fnDataCallback));
}

// 其他非静态成员函数。
void HexDecoder::Abort() noexcept {
	ASSERT(dynamic_cast<HexDecoderDelegate *>(this));

	((HexDecoderDelegate *)this)->Abort();
}
void HexDecoder::Update(const void *pData, std::size_t uSize){
	ASSERT(dynamic_cast<HexDecoderDelegate *>(this));

	((HexDecoderDelegate *)this)->Update(pData, uSize);
}
void HexDecoder::Finalize(){
	ASSERT(dynamic_cast<HexDecoderDelegate *>(this));

	((HexDecoderDelegate *)this)->Finalize();
}
