// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_THUNK_BLOCK_HPP__
#define __MCF_THUNK_BLOCK_HPP__

#include <memory>

namespace MCF {

extern std::shared_ptr<void> AllocateThunk(std::size_t uSize);

}

#endif
