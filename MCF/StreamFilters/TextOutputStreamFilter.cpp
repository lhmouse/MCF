// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextOutputStreamFilter.hpp"

namespace MCF {

TextOutputStreamFilter::~TextOutputStreamFilter(){
	try {
		X_FlushPlainBuffer(true);
	} catch(...){
	}
}

void TextOutputStreamFilter::X_FlushPlainBuffer(bool bForceFlushAll){
	bool bShouldFlushStream = bForceFlushAll;
	for(;;){
		const int nChar = x_sbufPlain.Peek();
		if(nChar < 0){
			break;
		}
		if(nChar == '\n'){
			y_vStream.BufferedPut("\r\n", 2);
			bShouldFlushStream = true;
		} else {
			y_vStream.BufferedPut(static_cast<unsigned char>(nChar));
		}
		x_sbufPlain.Discard();
	}
	if(bShouldFlushStream){
		y_vStream.Flush(y_vStream.kFlushBufferAll);
	}
}

void TextOutputStreamFilter::Put(unsigned char byData){
	x_sbufPlain.Put(byData);
	X_FlushPlainBuffer(false);
}

void TextOutputStreamFilter::Put(const void *pData, std::size_t uSize){
	x_sbufPlain.Put(pData, uSize);
	X_FlushPlainBuffer(false);
}

void TextOutputStreamFilter::Flush(bool bHard){
	X_FlushPlainBuffer(true);

	y_vStream.Flush(bHard);
}

}
