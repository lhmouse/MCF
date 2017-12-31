// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "StringInputStream.hpp"
#include "../Core/MinMax.hpp"

namespace MCF {

StringInputStream::~StringInputStream(){ }

int StringInputStream::Peek(){
	int nRet = -1;
	unsigned char byData;
	if(StringInputStream::Peek(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
int StringInputStream::Get(){
	int nRet = -1;
	unsigned char byData;
	if(StringInputStream::Get(&byData, 1) >= 1){
		nRet = byData;
	}
	return nRet;
}
bool StringInputStream::Discard(){
	bool bRet = false;
	if(StringInputStream::Discard(1) >= 1){
		bRet = true;
	}
	return bRet;
}
std::size_t StringInputStream::Peek(void *pData, std::size_t uSize){
	std::size_t uBytesTotal = 0;
	const auto uStringSize = x_vString.GetSize();
	if(x_uOffset < uStringSize){
		const auto uBytesRead = Min(uSize, uStringSize - x_uOffset);
		std::memcpy(pData, x_vString.GetData() + x_uOffset, uBytesRead);
		uBytesTotal += uBytesRead;
	}
	return uBytesTotal;
}
std::size_t StringInputStream::Get(void *pData, std::size_t uSize){
	const auto uBytesTotal = StringInputStream::Peek(pData, uSize);
	x_uOffset += uBytesTotal;
	return uBytesTotal;
}
std::size_t StringInputStream::Discard(std::size_t uSize){
	std::size_t uBytesTotal = 0;
	const auto uStringSize = x_vString.GetSize();
	if(x_uOffset < uStringSize){
		const auto uBytesDiscarded = Min(uSize, uStringSize - x_uOffset);
		uBytesTotal += uBytesDiscarded;
	}
	x_uOffset += uBytesTotal;
	return uBytesTotal;
}
void StringInputStream::Invalidate(){ }

}
