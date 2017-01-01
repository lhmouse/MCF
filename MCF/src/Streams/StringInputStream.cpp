// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "StringInputStream.hpp"
#include "../Core/MinMax.hpp"
#include "../Core/CopyMoveFill.hpp"

namespace MCF {

StringInputStream::~StringInputStream(){
}

int StringInputStream::Peek(){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return -1;
	}
	const int nRet = static_cast<unsigned char>(x_vString.UncheckedGet(uOffset));
	return nRet;
}
int StringInputStream::Get(){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return -1;
	}
	const int nRet = static_cast<unsigned char>(x_vString.UncheckedGet(uOffset));
	x_uOffset = uOffset + 1;
	return nRet;
}
bool StringInputStream::Discard(){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return false;
	}
	x_uOffset = uOffset + 1;
	return true;
}
std::size_t StringInputStream::Peek(void *pData, std::size_t uSize){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return false;
	}
	const auto uBytesToCopy = Min(uSize, uSizeAvail - uOffset);
	CopyN(static_cast<char *>(pData), x_vString.GetBegin() + uOffset, uBytesToCopy);
	return uBytesToCopy;
}
std::size_t StringInputStream::Get(void *pData, std::size_t uSize){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return false;
	}
	const auto uBytesToCopy = Min(uSize, uSizeAvail - uOffset);
	CopyN(static_cast<char *>(pData), x_vString.GetBegin() + uOffset, uBytesToCopy);
	x_uOffset = uOffset + uBytesToCopy;
	return uBytesToCopy;
}
std::size_t StringInputStream::Discard(std::size_t uSize){
	const auto uOffset = x_uOffset;
	const auto uSizeAvail = x_vString.GetSize();
	if(uOffset >= uSizeAvail){
		return false;
	}
	const auto uBytesToCopy = Min(uSize, uSizeAvail - uOffset);
	x_uOffset = uOffset + uBytesToCopy;
	return uBytesToCopy;
}

}
