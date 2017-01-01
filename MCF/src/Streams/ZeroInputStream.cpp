// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "ZeroInputStream.hpp"

namespace MCF {

ZeroInputStream::~ZeroInputStream(){
}

int ZeroInputStream::Peek() noexcept {
	return 0;
}
int ZeroInputStream::Get() noexcept {
	return 0;
}
bool ZeroInputStream::Discard() noexcept {
	return true;
}
std::size_t ZeroInputStream::Peek(void *pData, std::size_t uSize) noexcept {
	std::memset(pData, 0, uSize);
	return uSize;
}
std::size_t ZeroInputStream::Get(void *pData, std::size_t uSize) noexcept {
	std::memset(pData, 0, uSize);
	return uSize;
}
std::size_t ZeroInputStream::Discard(std::size_t uSize) noexcept {
	return uSize;
}

}
