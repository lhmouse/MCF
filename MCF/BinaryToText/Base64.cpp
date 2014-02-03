// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Base64.hpp"
#include <iterator>
#include <cstring>
using namespace MCF;

// 静态成员变量。
const char *__MCF::Base64Base::ENCODED_CHARS_MIME	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
const char *__MCF::Base64Base::ENCODED_CHARS_URL	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789*-";
const char *__MCF::Base64Base::ENCODED_CHARS_REGEXP	= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!-=";

// ========== Base64Encoder ==========
// 构造函数和析构函数。
Base64Encoder::Base64Encoder(std::function<void *(std::size_t)> fnDataCallback, const char *pchEncodedChars) noexcept
	: xm_fnDataCallback(std::move(fnDataCallback))
{
	__builtin_memcpy(xm_achTable, pchEncodedChars, sizeof(xm_achTable));
	xm_bInited = false;
}

// 其他非静态成员函数。
void Base64Encoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_bInited = true;
		xm_uState = 0;
	}

	std::uint32_t u32Word = xm_u32Word;
	std::size_t uState = xm_uState;

	__attribute__((aligned(16))) char achBuffer[64];
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
			pchWrite = std::begin(xm_achTable);
			__builtin_memcpy(xm_fnDataCallback(sizeof(achBuffer)), achBuffer, sizeof(achBuffer));
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pchWrite - std::begin(achBuffer));
	if(uBytesInBuffer > 0){
		std::memcpy(xm_fnDataCallback(uBytesInBuffer), achBuffer, uBytesInBuffer);
	}

	xm_u32Word = u32Word;
	xm_uState = uState;
}
void Base64Encoder::Finalize(){
	if(xm_bInited){
		xm_bInited = false;

		switch(xm_uState){
		case 1:
			if(xm_achTable[64] == 0){
				const auto pchWrite = (char *)xm_fnDataCallback(2);
				pchWrite[0] = xm_achTable[(xm_u32Word >>  2) & 0x3F];
				pchWrite[1] = xm_achTable[(xm_u32Word <<  4) & 0x30];
			} else {
				const auto pchWrite = (char *)xm_fnDataCallback(4);
				pchWrite[0] = xm_achTable[(xm_u32Word >>  2) & 0x3F];
				pchWrite[1] = xm_achTable[(xm_u32Word <<  4) & 0x30];
				pchWrite[2] = xm_achTable[64];
				pchWrite[3] = xm_achTable[64];
			}
			break;
		case 2:
			if(xm_achTable[64] == 0){
				const auto pchWrite = (char *)xm_fnDataCallback(3);
				pchWrite[0] = xm_achTable[(xm_u32Word >> 10) & 0x3F];
				pchWrite[1] = xm_achTable[(xm_u32Word >>  4) & 0x3F];
				pchWrite[2] = xm_achTable[(xm_u32Word <<  2) & 0x3C];
			} else {
				const auto pchWrite = (char *)xm_fnDataCallback(4);
				pchWrite[0] = xm_achTable[(xm_u32Word >> 10) & 0x3F];
				pchWrite[1] = xm_achTable[(xm_u32Word >>  4) & 0x3F];
				pchWrite[2] = xm_achTable[(xm_u32Word <<  2) & 0x3C];
				pchWrite[3] = xm_achTable[64];
			}
			break;
		}
	}
}

// ========== Base64Decoder ==========
// 构造函数和析构函数。
Base64Decoder::Base64Decoder(std::function<void *(std::size_t)> fnDataCallback, const char *pchEncodedChars) noexcept
	: xm_fnDataCallback(std::move(fnDataCallback))
{
	__builtin_memset(xm_achTable, 0xFF, sizeof(xm_achTable));
	for(std::size_t i = 0; i < 64; ++i){
		xm_achTable[(unsigned char)pchEncodedChars[i]] = i;
	}
	xm_bInited = false;
}

// 其他非静态成员函数。
void Base64Decoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_bInited = true;
		xm_uState = 0;
	}

	std::uint32_t u32Word = xm_u32Word;
	std::size_t uState = xm_uState;

	__attribute__((aligned(16))) unsigned char abyBuffer[48];
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
			pbyWrite = std::begin(abyBuffer);
			__builtin_memcpy(xm_fnDataCallback(sizeof(abyBuffer)), abyBuffer, sizeof(abyBuffer));
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pbyWrite - std::begin(abyBuffer));
	if(uBytesInBuffer > 0){
		std::memcpy(xm_fnDataCallback(uBytesInBuffer), abyBuffer, uBytesInBuffer);
	}

	xm_u32Word = u32Word;
	xm_uState = uState;
}
void Base64Decoder::Finalize(){
	if(xm_bInited){
		xm_bInited = false;

		switch(xm_uState){
		case 2:
			{
				const auto pbyWrite = (unsigned char *)xm_fnDataCallback(1);
				pbyWrite[0] = (xm_u32Word >>  4) & 0xFF;
			}
			break;
		case 3:
			{
				const auto pbyWrite = (unsigned char *)xm_fnDataCallback(2);
				pbyWrite[0] = (xm_u32Word >> 10) & 0xFF;
				pbyWrite[1] = (xm_u32Word >>  2) & 0xFF;
			}
			break;
		}
	}
}
