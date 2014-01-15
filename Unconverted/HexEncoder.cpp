// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "HexEncoder.hpp"
using namespace MCF;

// 构造函数和析构函数。
HexEncoder::HexEncoder(bool bUpperCase) :
	xm_bUpperCase(bUpperCase)
{
}

// 其他非静态成员函数。
void HexEncoder::xInitialize(void *pOutput){
	UNREF_PARAM(pOutput);
}
void HexEncoder::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	const BYTE *const pbyEnd = pbyData + (std::ptrdiff_t)uSize;
	for(const BYTE *pbyCur = pbyData; pbyCur < pbyEnd; ++pbyCur){
		const BYTE by = *pbyCur;

		auto PushDigit = [&](BYTE byDigit){
			ASSERT(byDigit < 0x10);

			((std::vector<BYTE> *)pOutput)->emplace_back(
				(byDigit < 0x0A) ? (BYTE)(byDigit + 0x30)
				: (xm_bUpperCase ? (BYTE)(byDigit + 0x37) : (BYTE)(byDigit + 0x57))
			);
		};

		PushDigit((BYTE)(by >> 4));
		PushDigit((BYTE)(by & 0x0F));
	}
}
void HexEncoder::xFinalize(void *pOutput){
	((std::vector<BYTE> *)pOutput)->push_back(0);
}

void HexEncoder::Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	pvecOutput->reserve(uSize * 2);
	AbstractTransformerBaseClass::xTransform(pvecOutput, pbyData, uSize, bIsEndOfStream);
}
