// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "BufferOutputStream.hpp"

namespace MCF {

BufferOutputStream::~BufferOutputStream(){
}

void BufferOutputStream::Put(unsigned char byData){
	x_vBuffer.Put(byData);
}
void BufferOutputStream::Put(const void *pData, std::size_t uSize){
	x_vBuffer.Put(pData, uSize);
}
void BufferOutputStream::Flush(bool bHard){
	(void)bHard;
}

}
