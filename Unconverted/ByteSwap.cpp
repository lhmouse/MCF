// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ByteSwap.hpp"
using namespace MCF;

// 构造函数和析构函数。
ByteSwapper::ByteSwapper(UNIT eUnit) :
	xm_eUnit(eUnit)
{
}

// 其他非静态成员函数。
void ByteSwapper::xPushByte(std::vector<BYTE> *pvecOutput, BYTE byData){
	xm_abyHistoryBytes[xm_uHistoryByteCount++] = byData;

	if(xm_uHistoryByteCount >= (std::size_t)(1 << (std::size_t)xm_eUnit)){
		switch(xm_eUnit){
			case U_WORD:
				{
					const std::uint16_t w = ::_byteswap_ushort(*(const std::uint16_t *)xm_abyHistoryBytes);
					pvecOutput->insert(pvecOutput->end(), (const BYTE *)&w, (const BYTE *)&w + sizeof(std::uint16_t));
				}
				break;
			case U_DWORD:
				{
					const std::uint32_t dw = ::_byteswap_ulong(*(const std::uint32_t *)xm_abyHistoryBytes);
					pvecOutput->insert(pvecOutput->end(), (const BYTE *)&dw, (const BYTE *)&dw + sizeof(std::uint32_t));
				}
				break;
			case U_QWORD:
				{
					const std::uint64_t qw = ::_byteswap_uint64(*(const std::uint64_t *)xm_abyHistoryBytes);
					pvecOutput->insert(pvecOutput->end(), (const BYTE *)&qw, (const BYTE *)&qw + sizeof(std::uint64_t));
				}
				break;
			default:
				ASSERT(FALSE);
		}

		xm_uHistoryByteCount = 0;
	}
}

void ByteSwapper::xInitialize(void *pOutput){
	UNREF_PARAM(pOutput);

	xm_uHistoryByteCount = 0;
}
void ByteSwapper::xUpdate(void *pOutput, const BYTE *pbyData, std::size_t uSize){
	std::vector<BYTE> &vecOutput = *((std::vector<BYTE> *)pOutput);

	const BYTE *pbyCur = pbyData;
	const BYTE *const pbyEnd = pbyData + (std::ptrdiff_t)uSize;

	const std::size_t uUnitSize = (std::size_t)(1 << (std::size_t)xm_eUnit);
	if(uSize >= uUnitSize){
		while(xm_uHistoryByteCount != 0){
			xPushByte((std::vector<BYTE> *)pOutput, *(pbyCur++));
		}
		while((std::size_t)(pbyEnd - pbyCur) >= uUnitSize){
			switch(xm_eUnit){
				case U_WORD:
					{
						const std::uint16_t w = ::_byteswap_ushort(*(const std::uint16_t *)pbyCur);
						vecOutput.insert(vecOutput.end(), (const BYTE *)&w, (const BYTE *)&w + sizeof(std::uint16_t));
					}
					break;
				case U_DWORD:
					{
						const std::uint32_t dw = ::_byteswap_ulong(*(const std::uint32_t *)pbyCur);
						vecOutput.insert(vecOutput.end(), (const BYTE *)&dw, (const BYTE *)&dw + sizeof(std::uint32_t));
					}
					break;
				case U_QWORD:
					{
						const std::uint64_t qw = ::_byteswap_uint64(*(const std::uint64_t *)pbyCur);
						vecOutput.insert(vecOutput.end(), (const BYTE *)&qw, (const BYTE *)&qw + sizeof(std::uint64_t));
					}
					break;
				default:
					ASSERT(FALSE);
			}

			pbyCur += uUnitSize;
		}
	}
	while(pbyCur < pbyEnd){
		xPushByte(&vecOutput, *(pbyCur++));
	}
}
void ByteSwapper::xFinalize(void *pOutput){
	while(xm_uHistoryByteCount != 0){
		xPushByte((std::vector<BYTE> *)pOutput, 0);
	}
}

void ByteSwapper::Transform(std::vector<BYTE> *pvecOutput, const BYTE *pbyData, std::size_t uSize, bool bIsEndOfStream){
	pvecOutput->reserve((uSize + 1) & ((1 << (std::size_t)xm_eUnit) - 1));
	AbstractTransformerBaseClass::xTransform(pvecOutput, pbyData, uSize, bIsEndOfStream);
}
