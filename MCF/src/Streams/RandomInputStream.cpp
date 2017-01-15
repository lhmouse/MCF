// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "RandomInputStream.hpp"
#include "../Core/Random.hpp"

namespace MCF {

RandomInputStream::~RandomInputStream(){
}

int RandomInputStream::Peek() noexcept {
	return GetRandomUint32() & 0xFF;
}
int RandomInputStream::Get() noexcept {
	return GetRandomUint32() & 0xFF;
}
bool RandomInputStream::Discard() noexcept {
	return true;
}
std::size_t RandomInputStream::Peek(void *pData, std::size_t uSize) noexcept {
	for(std::size_t i = 0; i < uSize; ++i){
		static_cast<unsigned char *>(pData)[i] = GetRandomUint32() & 0xFF;
	}
	return uSize;
}
std::size_t RandomInputStream::Get(void *pData, std::size_t uSize) noexcept {
	for(std::size_t i = 0; i < uSize; ++i){
		static_cast<unsigned char *>(pData)[i] = GetRandomUint32() & 0xFF;
	}
	return uSize;
}
std::size_t RandomInputStream::Discard(std::size_t uSize) noexcept {
	return uSize;
}
void RandomInputStream::Invalidate() noexcept {
}

}
