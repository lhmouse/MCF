// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextInputStreamFilter.hpp"

namespace MCF {

TextInputStreamFilter::~TextInputStreamFilter(){
}

void TextInputStreamFilter::X_PopulatePlainBuffer(std::size_t uExpected){
	for(;;){
		if(x_sbufPlain.GetSize() >= uExpected){
			break;
		}

		unsigned char abyTemp[2];
		y_vStream.PopulateBuffer(2);
		const auto uBytesRead = y_vStream.BufferedPeek(abyTemp, 2);
		if(uBytesRead == 0){
			break;
		}
		if(uBytesRead == 1){
			if(abyTemp[0] != '\r'){
				x_sbufPlain.Put(abyTemp[0]);
				y_vStream.BufferedDiscard();
			}
			break;
		}
		if((abyTemp[0] == '\r') && (abyTemp[1] == '\n')){
			x_sbufPlain.Put('\n');
			y_vStream.BufferedDiscard(2);
		} else if(abyTemp[1] == '\r'){
			x_sbufPlain.Put(abyTemp[0]);
			y_vStream.BufferedDiscard();
		} else {
			x_sbufPlain.Put(abyTemp, 2);
			y_vStream.BufferedDiscard(2);
		}
	}
}

int TextInputStreamFilter::Peek(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Peek();
}
int TextInputStreamFilter::Get(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Get();
}
bool TextInputStreamFilter::Discard(){
	X_PopulatePlainBuffer(1);
	return x_sbufPlain.Discard();
}
std::size_t TextInputStreamFilter::Peek(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Peek(pData, uSize);
}
std::size_t TextInputStreamFilter::Get(void *pData, std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Get(pData, uSize);
}
std::size_t TextInputStreamFilter::Discard(std::size_t uSize){
	X_PopulatePlainBuffer(uSize);
	return x_sbufPlain.Discard(uSize);
}

}
