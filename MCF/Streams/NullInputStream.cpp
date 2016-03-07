// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "NullInputStream.hpp"

namespace MCF {

NullInputStream::~NullInputStream(){
}

int NullInputStream::Peek() noexcept {
	return -1;
}
int NullInputStream::Get() noexcept {
	return -1;
}
bool NullInputStream::Discard() noexcept {
	return false;
}

std::size_t NullInputStream::Peek(void * /* pData */, std::size_t /* uSize */) noexcept {
	return 0;
}
std::size_t NullInputStream::Get(void * /* pData */, std::size_t /* uSize */) noexcept {
	return 0;
}
std::size_t NullInputStream::Discard(std::size_t /* uSize */) noexcept {
	return 0;
}

}
