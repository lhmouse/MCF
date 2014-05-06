// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "Utilities.hpp"
#include "CriticalSection.hpp"
#include "MultiIndexedMap.hpp"
#include "Exception.hpp"
#include <exception>
using namespace MCF;

namespace {
/*
union ChunkHeader {
	struct {
		std::size_t uRefCount;
		std::size_t uChunkSize;
	};
	unsigned char abyPadding[16];
};

union ThunkInfo {
	struct {
		ChunkHeader *pChunkHeader;
		std::size_t uSize;
	};
	unsigned char abyPadding[16];
};

const auto g_pcsLock = CriticalSection::Create();
MultiIndexedMap<ThunkInfo *, std::uintptr_t> g_mapPool;

const void *DoAllocateThunk(const void *pInit, std::size_t uSize) noexcept {
	CRITICAL_SECTION_SCOPE(g_pcsLock){
	}
	return nullptr;
}
void DoDeallocateThunk(const void *pThunk) noexcept {
	CRITICAL_SECTION_SCOPE(g_pcsLock){
	}
}
*/
}

namespace MCF {

std::shared_ptr<const void> AllocateThunk(const void *pInit, std::size_t uSize){
	/*const auto pThunk = DoAllocateThunk(pInit, uSize);
	if(!pThunk){
		throw std::bad_alloc();
	}
	return std::shared_ptr<const void>(pThunk, &DoDeallocateThunk);*/
	return std::make_shared<int>(123);
}

}
