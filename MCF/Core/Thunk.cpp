// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "UniqueHandle.hpp"
#include "Exception.hpp"
using namespace MCF;

namespace {

struct HeapDestroyer {
	constexpr HANDLE operator()() const {
		return NULL;
	}
	void operator()(HANDLE hHeap) const {
		::HeapDestroy(hHeap);
	}
};

struct ThunkHeap {
	UniqueHandle<HeapDestroyer>	m_hHeap;

	ThunkHeap(){
		m_hHeap.Reset(::HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0));
		if(!m_hHeap){
			MCF_THROW(::GetLastError(), L"HeapCreate() 失败。");
		}

		ULONG ulLFHFlag = 2;
		::HeapSetInformation(m_hHeap.Get(), HeapCompatibilityInformation, &ulLFHFlag, sizeof(ulLFHFlag));

#if WINVER >= 0x0600
		if(LOBYTE(LOWORD(::GetVersion())) >= 6){
			::HeapSetInformation(m_hHeap.Get(), HeapEnableTerminationOnCorruption, nullptr, 0);
		}
#endif
	}
} g_vThunkHeap;

struct ThunkDeallocator {
	constexpr void *operator()() const noexcept {
		return nullptr;
	}
	void operator()(void *pThunk) const noexcept {
		if(pThunk){
			::HeapFree(g_vThunkHeap.m_hHeap.Get(), 0, pThunk);
		}
	}
};

}

namespace MCF {

std::shared_ptr<void> AllocateThunk(std::size_t uSize){
	UniqueHandle<ThunkDeallocator> pThunk;
	pThunk.Reset(::HeapAlloc(g_vThunkHeap.m_hHeap.Get(), 0, uSize));
	if(!pThunk){
		MCF_THROW(::GetLastError(), L"HeapAlloc() 失败。");
	}
	std::shared_ptr<void> pRet(pThunk.Get(), ThunkDeallocator());
	pThunk.Release();
	return std::move(pRet);
}

}
