// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#include "StringOutputStream.hpp"

namespace MCF {

StringOutputStream::~StringOutputStream(){ }

void StringOutputStream::Put(unsigned char byData){
	StringOutputStream::Put(&byData, 1);
}
void StringOutputStream::Put(const void *pData, std::size_t uSize){
	const auto uBytesTotal = uSize;
	const auto uWriteEnd = x_uOffset + uBytesTotal;
	const auto uStringSizeOld = x_vString.GetSize();
	if(uStringSizeOld < uWriteEnd){
		x_vString.Append('\0', uWriteEnd - uStringSizeOld);
	}
	std::memcpy(x_vString.GetData() + x_uOffset, pData, uBytesTotal);
	x_uOffset += uBytesTotal;
}
void StringOutputStream::Flush(bool bHard){
	(void)bHard;
}

}
