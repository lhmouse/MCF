// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "NullOutputStream.hpp"

namespace MCF {

NullOutputStream::~NullOutputStream(){
}

void NullOutputStream::Put(unsigned char /* byData */) noexcept {
}

void NullOutputStream::Put(const void * /* pData */, std::size_t /* uSize */) noexcept {
}

void NullOutputStream::Flush(bool /* bHard */) const noexcept {
}

}
