// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_THUNK_HPP_
#define MCF_THUNK_HPP_

#include <memory>
#include <cstddef>

namespace MCF {

// 返回的指针指向 Thunk 首地址，该地址总是对齐到 16 字节边界的。
// 出于安全原因，Thunk 是一段可执行但是不可写的内存，一旦初始化就不能更改它的内容。
std::shared_ptr<const void> AllocateThunk(const void *pInit, std::size_t uSize);

}

#endif
