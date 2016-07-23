// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "../../MCFCRT/ext/thunk.h"
#include <new>

namespace MCF {

void ThunkDeleter::operator()(const void *pThunk) const noexcept {
	::_MCFCRT_DeallocateThunk(pThunk, true);
}

UniquePtr<const void, ThunkDeleter> CreateThunk(const void *pInit, std::size_t uSize){
	UniquePtr<const void, ThunkDeleter> pThunk;
	if(!pThunk.Reset(::_MCFCRT_AllocateThunk(pInit, uSize))){
		throw std::bad_alloc();
	}
	return pThunk;
}

}
