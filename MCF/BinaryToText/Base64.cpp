// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64.hpp"
#include <iterator>
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
}

// 静态成员变量。
const char *__MCF::Base64Base::ENCODED_CHARS_MIME	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
const char *__MCF::Base64Base::ENCODED_CHARS_URL	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";
const char *__MCF::Base64Base::ENCODED_CHARS_REGEXP	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-=";

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const char *pchEncodedChars)
	: xm_fnDataCallback(std::move(fnDataCallback))
	, xm_bInited(false)
{
	__builtin_memcpy(xm_achTable, pchEncodedChars, sizeof(xm_achTable));
}

// 其他非静态成员函数。
void Base64Encoder::Abort() noexcept{
	xm_bInited = false;
}
void Base64Encoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_uState = 0;

		xm_bInited = true;
	}

	std::uint32_t u32Word = xm_u32Word;
	std::size_t uState = xm_uState;

	alignas(16) char achBuffer[64];
	auto pchWrite = std::begin(achBuffer);

	auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;
	while(pbyRead != pbyEnd){
		u32Word = (u32Word << 8) | *(pbyRead++);
		if(++uState != 3){
			continue;
		}
		uState = 0;

		pchWrite[0] = xm_achTable[(u32Word >> 18) & 0x3F];
		pchWrite[1] = xm_achTable[(u32Word >> 12) & 0x3F];
		pchWrite[2] = xm_achTable[(u32Word >>  6) & 0x3F];
		pchWrite[3] = xm_achTable[ u32Word        & 0x3F];
		pchWrite += 4;

		if(pchWrite == std::end(xm_achTable)){
			CopyOut(xm_fnDataCallback, achBuffer, sizeof(achBuffer));
			pchWrite = std::begin(xm_achTable);
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pchWrite - std::begin(achBuffer));
	if(uBytesInBuffer > 0){
		CopyOut(xm_fnDataCallback, achBuffer, uBytesInBuffer);
	}

	xm_u32Word = u32Word;
	xm_uState = uState;
}
void Base64Encoder::Finalize(){
	if(xm_bInited){
		char achTemp[4];

		switch(xm_uState){
		case 1:
			if(xm_achTable[64] == 0){
				achTemp[0] = xm_achTable[(xm_u32Word >>  2) & 0x3F];
				achTemp[1] = xm_achTable[(xm_u32Word <<  4) & 0x30];
				CopyOut(xm_fnDataCallback, achTemp, 2);
			} else {
				achTemp[0] = xm_achTable[(xm_u32Word >>  2) & 0x3F];
				achTemp[1] = xm_achTable[(xm_u32Word <<  4) & 0x30];
				achTemp[2] = xm_achTable[64];
				achTemp[3] = xm_achTable[64];
				CopyOut(xm_fnDataCallback, achTemp, 4);
			}
			break;
		case 2:
			if(xm_achTable[64] == 0){
				achTemp[0] = xm_achTable[(xm_u32Word >> 10) & 0x3F];
				achTemp[1] = xm_achTable[(xm_u32Word >>  4) & 0x3F];
				achTemp[2] = xm_achTable[(xm_u32Word <<  2) & 0x3C];
				CopyOut(xm_fnDataCallback, achTemp, 3);
			} else {
				achTemp[0] = xm_achTable[(xm_u32Word >> 10) & 0x3F];
				achTemp[1] = xm_achTable[(xm_u32Word >>  4) & 0x3F];
				achTemp[2] = xm_achTable[(xm_u32Word <<  2) & 0x3C];
				achTemp[3] = xm_achTable[64];
				CopyOut(xm_fnDataCallback, achTemp, 4);
			}
			break;
		}

		xm_bInited = false;
	}
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(std::function<std::pair<void *, std::size_t> (std::size_t)> fnDataCallback, const char *pchEncodedChars)
	: xm_fnDataCallback(std::move(fnDataCallback))
	, xm_bInited(false)
{
	__builtin_memset(xm_achTable, 0xFF, sizeof(xm_achTable));
	for(std::size_t i = 0; i < 64; ++i){
		xm_achTable[(unsigned char)pchEncodedChars[i]] = i;
	}
}

// 其他非静态成员函数。
void Base64Decoder::Abort() noexcept{
	xm_bInited = false;
}
void Base64Decoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_uState = 0;

		xm_bInited = true;
	}

	std::uint32_t u32Word = xm_u32Word;
	std::size_t uState = xm_uState;

	alignas(16) unsigned char abyBuffer[48];
	auto pbyWrite = std::begin(abyBuffer);

	auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;
	while(pbyRead != pbyEnd){
		const auto byDecoded = xm_achTable[*(pbyRead++)];
		if(byDecoded >= 64){
			continue;
		}
		u32Word = (u32Word << 6) | byDecoded;
		if(++uState != 4){
			continue;
		}
		uState = 0;

		pbyWrite[0] = (u32Word >> 16) & 0xFF;
		pbyWrite[1] = (u32Word >>  8) & 0xFF;
		pbyWrite[2] =  u32Word        & 0xFF;
		pbyWrite += 3;

		if(pbyWrite == std::end(abyBuffer)){
			CopyOut(xm_fnDataCallback, abyBuffer, sizeof(abyBuffer));
			pbyWrite = std::begin(abyBuffer);
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pbyWrite - std::begin(abyBuffer));
	if(uBytesInBuffer > 0){
		CopyOut(xm_fnDataCallback, abyBuffer, uBytesInBuffer);
	}

	xm_u32Word = u32Word;
	xm_uState = uState;
}
void Base64Decoder::Finalize(){
	if(xm_bInited){
		char achTemp[4];

		switch(xm_uState){
		case 2:
			achTemp[0] = (xm_u32Word >>  4) & 0xFF;
			CopyOut(xm_fnDataCallback, achTemp, 1);
			break;
		case 3:
			achTemp[0] = (xm_u32Word >> 10) & 0xFF;
			achTemp[1] = (xm_u32Word >>  2) & 0xFF;
			CopyOut(xm_fnDataCallback, achTemp, 1);
			break;
		}

		xm_bInited = false;
	}
}
