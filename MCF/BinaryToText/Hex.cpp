// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Hex.hpp"
#include <cstring>
using namespace MCF;

// ========== HexEncoder ==========
// 构造函数和析构函数。
HexEncoder::HexEncoder(std::function<void *(std::size_t)> fnDataCallback, bool bUpperCase)
	: xm_fnDataCallback(std::move(fnDataCallback))
	, xm_byDelta((bUpperCase ? 'A' : 'a') - ('9' + 1))
{
}

// 其他非静态成员函数。
void HexEncoder::Update(const void *pData, std::size_t uSize){
	__attribute__((aligned(16))) char achBuffer[64];
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
			pchWrite = std::begin(achBuffer);
			__builtin_memcpy(xm_fnDataCallback(sizeof(achBuffer)), achBuffer, sizeof(achBuffer));
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pchWrite - std::begin(achBuffer));
	if(uBytesInBuffer > 0){
		std::memcpy(xm_fnDataCallback(uBytesInBuffer), achBuffer, uBytesInBuffer);
	}
}
void HexEncoder::Finalize(){
}

// ========== HexDecoder ==========
// 构造函数和析构函数。
HexDecoder::HexDecoder(std::function<void *(std::size_t)> fnDataCallback)
	: xm_fnDataCallback(std::move(fnDataCallback))
{
	xm_bInited = false;
}

// 其他非静态成员函数。
void HexDecoder::Update(const void *pData, std::size_t uSize){
	if(!xm_bInited){
		xm_bInited = true;
		xm_uchLastDigit = 0xFF;
	}

	auto uchLastDigit = xm_uchLastDigit;

	__attribute__((aligned(16))) unsigned char abyBuffer[64];
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
		*pbyWrite = (uchLastDigit << 4) | uchDecoded;
		uchLastDigit = 0xFF;

		if(++pbyWrite == std::end(abyBuffer)){
			pbyWrite = std::begin(abyBuffer);
			__builtin_memcpy(xm_fnDataCallback(sizeof(abyBuffer)), abyBuffer, sizeof(abyBuffer));
		}
	}

	const auto uBytesInBuffer = (std::size_t)(pbyWrite - std::begin(abyBuffer));
	if(uBytesInBuffer > 0){
		std::memcpy(xm_fnDataCallback(uBytesInBuffer), abyBuffer, uBytesInBuffer);
	}

	xm_uchLastDigit = uchLastDigit;
}
void HexDecoder::Finalize(){
	if(xm_bInited){
		xm_bInited = false;
	}
}
