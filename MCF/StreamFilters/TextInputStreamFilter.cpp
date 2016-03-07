// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "TextInputStreamFilter.hpp"
#include "../Utilities/MinMax.hpp"

namespace MCF {

TextInputStreamFilter::~TextInputStreamFilter(){
}

void TextInputStreamFilter::X_PopulateDecodedBuffer(std::size_t uExpected){
	if(x_vecDecoded.GetSize() >= uExpected){
		return;
	}
	x_vecDecoded.Reserve(uExpected);

	for(;;){
		int nCur = y_vStream.Get();
		if(nCur < 0){
			break;
		}
		if(nCur == '\r'){
			int nNext = y_vStream.Peek();
			if(nNext < 0){
				break;
			}
			if(nNext == '\n'){
				y_vStream.Discard();
				nCur = '\n';
			}
		}
		x_vecDecoded.UncheckedPush(static_cast<char>(nCur));
	}
}

int TextInputStreamFilter::Peek(){
	X_PopulateDecodedBuffer(1);
	if(x_vecDecoded.IsEmpty()){
		return -1;
	}
	const int nRet = x_vecDecoded[0];
	return nRet;
}
int TextInputStreamFilter::Get(){
	X_PopulateDecodedBuffer(1);
	if(x_vecDecoded.IsEmpty()){
		return -1;
	}
	const int nRet = x_vecDecoded[0];
	x_vecDecoded.Erase(x_vecDecoded.GetBegin());
	return nRet;
}
bool TextInputStreamFilter::Discard(){
	X_PopulateDecodedBuffer(1);
	if(x_vecDecoded.IsEmpty()){
		return false;
	}
	x_vecDecoded.Erase(x_vecDecoded.GetBegin());
	return true;
}

std::size_t TextInputStreamFilter::Peek(void *pData, std::size_t uSize){
	X_PopulateDecodedBuffer(uSize);
	const auto uBytesRead = Min(x_vecDecoded.GetSize(), uSize);
	if(uBytesRead == 0){
		return 0;
	}
	std::memcpy(pData, x_vecDecoded.GetData(), uBytesRead);
	return uBytesRead;
}
std::size_t TextInputStreamFilter::Get(void *pData, std::size_t uSize){
	X_PopulateDecodedBuffer(uSize);
	const auto uBytesRead = Min(x_vecDecoded.GetSize(), uSize);
	if(uBytesRead == 0){
		return 0;
	}
	std::memcpy(pData, x_vecDecoded.GetData(), uBytesRead);
	x_vecDecoded.Erase(x_vecDecoded.GetBegin() + uBytesRead);
	return uBytesRead;
}
std::size_t TextInputStreamFilter::Discard(std::size_t uSize){
	X_PopulateDecodedBuffer(uSize);
	const auto uBytesRead = Min(x_vecDecoded.GetSize(), uSize);
	if(uBytesRead == 0){
		return 0;
	}
	x_vecDecoded.Erase(x_vecDecoded.GetBegin() + uBytesRead);
	return uBytesRead;
}

}
