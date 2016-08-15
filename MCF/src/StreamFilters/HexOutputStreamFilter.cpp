// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "HexOutputStreamFilter.hpp"
#include "../Core/Endian.hpp"

namespace MCF {

HexOutputStreamFilter::~HexOutputStreamFilter(){
	try {
		X_FlushPlainBuffer(true);
	} catch(...){
	}
}

void HexOutputStreamFilter::X_FlushPlainBuffer(bool bForceFlushAll){
	static constexpr unsigned char kHexTable[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
	};

	for(;;){
		const int nRead = x_sbufPlain.Peek();
		if(nRead < 0){
			break;
		}
		const auto uByte = static_cast<unsigned>(nRead);

		unsigned char abyResult[2];
		abyResult[0] = kHexTable[(uByte >> 4) % 16];
		abyResult[1] = kHexTable[(uByte     ) % 16];
		y_vStream.BufferedPut(abyResult, 2);
		x_sbufPlain.Discard(1);
	};

	y_vStream.Flush(bForceFlushAll ? y_vStream.kFlushBufferNow : y_vStream.kFlushBufferAuto);
}

void HexOutputStreamFilter::Put(unsigned char byData){
	x_sbufPlain.Put(byData);
	X_FlushPlainBuffer(false);
}
void HexOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_sbufPlain.Put(pData, uSize);
	X_FlushPlainBuffer(false);
}
void HexOutputStreamFilter::Flush(bool bHard){
	X_FlushPlainBuffer(true);

	y_vStream.Flush(bHard);
}

}
