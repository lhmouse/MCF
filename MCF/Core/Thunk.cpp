// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "../../MCFCRT/env/thunk.h"
#include <new>

namespace MCF {

void ThunkDeleter::operator()(void *pThunk) const noexcept {
	::MCF_CRT_DeallocateThunk(pThunk, true);
}

ThunkPtr CreateThunk(const void *pInit, std::size_t uSize){
	ThunkPtr pThunk;
	if(!pThunk.Reset(::MCF_CRT_AllocateThunk(pInit, uSize))){
		throw std::bad_alloc();
	}
	return pThunk;
}

}
