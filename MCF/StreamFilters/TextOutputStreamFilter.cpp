// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "TextOutputStreamFilter.hpp"

namespace MCF {

TextOutputStreamFilter::~TextOutputStreamFilter(){
	try {
		X_FlushPlainBuffer(true);
	} catch(...){
	}
}

void TextOutputStreamFilter::X_FlushPlainBuffer(bool bForceFlushAll){
	for(;;){
		const int nChar = x_sbufPlain.Peek();
		if(nChar < 0){
			break;
		}
		if(nChar == '\n'){
			y_vStream.BufferedPut("\r\n", 2);
			bForceFlushAll = true;
		} else {
			y_vStream.BufferedPut(static_cast<unsigned char>(nChar));
		}
		x_sbufPlain.Discard();
	}
	y_vStream.Flush(bForceFlushAll ? y_vStream.kFlushBufferNow : y_vStream.kFlushBufferAuto);
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
