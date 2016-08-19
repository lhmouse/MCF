// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StringOutputStream.hpp"

namespace MCF {

StringOutputStream::~StringOutputStream(){
}

void StringOutputStream::Put(unsigned char byData){
	x_vString.Append(static_cast<char>(byData), 1);
}
void StringOutputStream::Put(const void *pData, std::size_t uSize){
	x_vString.Append(static_cast<const char *>(pData), uSize);
}
void StringOutputStream::Flush(bool bHard){
	(void)bHard;
}

}
