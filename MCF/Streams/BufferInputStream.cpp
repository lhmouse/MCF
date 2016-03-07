// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "BufferInputStream.hpp"

namespace MCF {

BufferInputStream::~BufferInputStream(){
}

int BufferInputStream::Peek(){
	return x_vBuffer.Peek();
}
int BufferInputStream::Get(){
	return x_vBuffer.Get();
}
bool BufferInputStream::Discard(){
	return x_vBuffer.Discard();
}

std::size_t BufferInputStream::Peek(void *pData, std::size_t uSize){
	return x_vBuffer.Peek(pData, uSize);
}
std::size_t BufferInputStream::Get(void *pData, std::size_t uSize){
	return x_vBuffer.Get(pData, uSize);
}
std::size_t BufferInputStream::Discard(std::size_t uSize){
	return x_vBuffer.Discard(uSize);
}

}
