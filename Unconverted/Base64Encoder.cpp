// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Base64Encoder.hpp"
using namespace MCF;

// 静态成员变量。
const BYTE Base64Encoder::xENCODE_TABLE_DRAFT[0x40] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '?', '?'
};

// 构造函数和析构函数。
Base64Encoder::Base64Encoder(const PARAMS &Params){
	std::memcpy(xm_abyEncodeTable, xENCODE_TABLE_DRAFT, sizeof(xm_abyEncodeTable));
	xm_abyEncodeTable[0x3E] = (BYTE)Params.ch63rd;
	xm_abyEncodeTable[0x3F] = (BYTE)Params.ch64th;
	xm_byPadding = (BYTE)Params.chPadding;

	xm_eState = INIT;
}
Base64Encoder::~Base64Encoder(){
}

// 其他非静态成员函数。
void Base64Encoder::xInitialize(void *pOutput){
	UNREF_PARAM(pOutput);
}
void Base64Encoder::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	std::vector<BYTE> &vecOutput = *((std::vector<BYTE> *)pOutput);

	const BYTE *pbyCur = pbyData;
	const BYTE *const pbyEnd = pbyData + (std::ptrdiff_t)uSize;
	auto PushByte = [&]{
		ASSERT(pbyCur < pbyEnd);

		switch(xm_eState){
			case INIT:
				xm_dwHistory = ((DWORD)*(pbyCur++)) << 16;

				xm_eState = BYTE_1;
				break;
			case BYTE_1:
				xm_dwHistory |= ((DWORD)*(pbyCur++)) << 8;

				xm_eState = BYTE_2;
				break;
			case BYTE_2:
				xm_dwHistory |= ((DWORD)*(pbyCur++));
				vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 18) & 0x3F]);
				vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 12) & 0x3F]);
				vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 6) & 0x3F]);
				vecOutput.emplace_back(xm_abyEncodeTable[xm_dwHistory & 0x3F]);

				xm_eState = INIT;
				break;
			default:
				ASSERT(false);
		}
	};

	if((std::size_t)(pbyEnd - pbyCur) >= 3){
		while(xm_eState != INIT){
			PushByte();
		}
		while((std::size_t)(pbyEnd - pbyCur) >= 3){
			const DWORD dwFull = (((DWORD)pbyCur[0]) << 16) | (((DWORD)pbyCur[1]) << 8) | ((DWORD)pbyCur[2]);
			vecOutput.emplace_back(xm_abyEncodeTable[(dwFull >> 18) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[(dwFull >> 12) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[(dwFull >> 6) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[dwFull & 0x3F]);

			pbyCur += 3;
		}
	}
	while(pbyCur < pbyEnd){
		PushByte();
	}
}
void Base64Encoder::xFinalize(void *pOutput){
	std::vector<BYTE> &vecOutput = *((std::vector<BYTE> *)pOutput);

	switch(xm_eState){
		case INIT:
			break;
		case BYTE_1:
			vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 18) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 12) & 0x3F]);
			if(xm_byPadding != 0){
				vecOutput.emplace_back(xm_byPadding);
				vecOutput.emplace_back(xm_byPadding);
			}
			break;
		case BYTE_2:
			vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 18) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 12) & 0x3F]);
			vecOutput.emplace_back(xm_abyEncodeTable[(xm_dwHistory >> 6) & 0x3F]);
			if(xm_byPadding != 0){
				vecOutput.emplace_back(xm_byPadding);
			}
			break;
		default:
			ASSERT(false);
	}

	vecOutput.push_back(0);
}

void Base64Encoder::Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	pvecOutput->reserve(pvecOutput->size() + (uSize + 2) / 3 * 4 + 1);
	AbstractTransformerBaseClass::xTransform(pvecOutput, pbyData, uSize, bIsEndOfStream);
}
