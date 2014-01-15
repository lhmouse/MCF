// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "HexDecoder.hpp"
using namespace MCF;

// 构造函数和析构函数。
HexDecoder::HexDecoder(){
	xm_byFirstHex = 0xFF;
}

// 其他非静态成员函数。
void inline __attribute__((always_inline)) HexDecoder::xPushDigit(std::vector<BYTE> *pvecOutput, BYTE byDigit){
	auto OnHexValue = [&](BYTE byHexValue){
		ASSERT(byHexValue < 0x10);

		if(xm_byFirstHex == 0xFF){
			xm_byFirstHex = byHexValue;
		} else {
			pvecOutput->emplace_back((BYTE)((xm_byFirstHex << 4) | byHexValue));
			xm_byFirstHex = 0xFF;
		}
	};

	if(('0' <= byDigit) && (byDigit <= '9')){
		OnHexValue((BYTE)(byDigit - '0'));
	} else if(('a' <= byDigit) && (byDigit <= 'f')){
		OnHexValue((BYTE)(byDigit - 'a'));
	} else if(('A' <= byDigit) && (byDigit <= 'F')){
		OnHexValue((BYTE)(byDigit - 'A'));
	}
}

void HexDecoder::xInitialize(void *pOutput){
	UNREF_PARAM(pOutput);
}
void HexDecoder::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	const BYTE *const pbyEnd = pbyData + (std::ptrdiff_t)uSize;
	for(const BYTE *pbyCur = pbyData; pbyCur < pbyEnd; ++pbyCur){
		xPushDigit((std::vector<BYTE> *)pOutput, *pbyCur);
	}
}
void HexDecoder::xFinalize(void *pOutput){
	if(xm_byFirstHex != 0){
		xPushDigit((std::vector<BYTE> *)pOutput, '0');
	}
}

void HexDecoder::Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	pvecOutput->reserve(uSize / 2);
	AbstractTransformerBaseClass::xTransform(pvecOutput, pbyData, uSize, bIsEndOfStream);
}
