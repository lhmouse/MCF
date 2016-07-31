// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "Base64OutputStreamFilter.hpp"
#include "../Core/Endian.hpp"

namespace MCF {

Base64OutputStreamFilter::~Base64OutputStreamFilter(){
	try {
		X_FlushPlainBuffer(true);
	} catch(...){
	}
}

void Base64OutputStreamFilter::X_FlushPlainBuffer(bool bForceFlushAll){
	static constexpr unsigned char kBase64Table[64] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/',
	};

	for(;;){
		std::uint32_t u32WordInBigEndian = 0;
		const auto uBytesRead = x_sbufPlain.Peek(&u32WordInBigEndian, 3);
		if((uBytesRead == 0) || (!bForceFlushAll && (uBytesRead < 3))){
			break;
		}
		const std::uint_fast32_t u32Word = LoadBe(u32WordInBigEndian);

		unsigned char abyResult[4];
		if(uBytesRead == 3){
			abyResult[0] = kBase64Table[(u32Word >> 26) % 64];
			abyResult[1] = kBase64Table[(u32Word >> 20) % 64];
			abyResult[2] = kBase64Table[(u32Word >> 14) % 64];
			abyResult[3] = kBase64Table[(u32Word >>  8) % 64];
		} else if(uBytesRead == 2){
			abyResult[0] = kBase64Table[(u32Word >> 26) % 64];
			abyResult[1] = kBase64Table[(u32Word >> 20) % 64];
			abyResult[2] = kBase64Table[(u32Word >> 14) % 64];
			abyResult[3] = '=';
		} else {
			abyResult[0] = kBase64Table[(u32Word >> 26) % 64];
			abyResult[1] = kBase64Table[(u32Word >> 20) % 64];
			abyResult[2] = '=';
			abyResult[3] = '=';
		}

		y_vStream.BufferedPut(abyResult, 4);
		x_sbufPlain.Discard(3);
	}
	y_vStream.Flush(bForceFlushAll ? y_vStream.kFlushBufferNow : y_vStream.kFlushBufferAuto);
}

void Base64OutputStreamFilter::Put(unsigned char byData){
	x_sbufPlain.Put(byData);
	X_FlushPlainBuffer(false);
}
void Base64OutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_sbufPlain.Put(pData, uSize);
	X_FlushPlainBuffer(false);
}
void Base64OutputStreamFilter::Flush(bool bHard){
	X_FlushPlainBuffer(true);

	y_vStream.Flush(bHard);
}

}
