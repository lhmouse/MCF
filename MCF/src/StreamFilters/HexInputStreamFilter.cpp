// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "HexInputStreamFilter.hpp"

namespace MCF {

HexInputStreamFilter::~HexInputStreamFilter(){
}

void HexInputStreamFilter::X_PopulatePlainBuffer(std::size_t uExpected){
	static constexpr signed char kHexReverseTable[256] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	};

	for(;;){
		if(x_sbufPlain.GetSize() >= uExpected){
			break;
		}

		unsigned char abyTemp[2];
		y_vStream.PopulateBuffer(2);
		const auto uBytesRead = y_vStream.BufferedPeek(abyTemp, 2);
		if(uBytesRead < 2){
			break;
		}

		const auto DecodeOne = [&](unsigned char by){
			const int nDecoded = kHexReverseTable[by];
			if(nDecoded < 0){
				MCF_THROW(Exception, ERROR_INVALID_DATA, Rcntws::View(L"HexInputStreamFilter: 输入的数据无效。"));
			}
			MCF_ASSERT(nDecoded < 16);
			return static_cast<std::uint_fast32_t>(nDecoded);
		};

		unsigned uByte = 0;
		uByte |= (DecodeOne(abyTemp[0]) << 4);
		uByte |= (DecodeOne(abyTemp[1])     );
		x_sbufPlain.Put(static_cast<unsigned char>(uByte));
		y_vStream.BufferedDiscard(2);
	}
}

int HexInputStreamFilter::Peek(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Peek();
}
int HexInputStreamFilter::Get(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Get();
}
bool HexInputStreamFilter::Discard(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Discard();
}
std::size_t HexInputStreamFilter::Peek(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Peek(pData, uSize);
}
std::size_t HexInputStreamFilter::Get(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Get(pData, uSize);
}
std::size_t HexInputStreamFilter::Discard(std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Discard(uSize);
}

}
