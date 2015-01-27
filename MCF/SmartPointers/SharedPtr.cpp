// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SharedPtr.hpp"
#include "../../MCFCRT/ext/expect.h"
#include "../Thread/Atomic.hpp"
#include "../Utilities/Noncopyable.hpp"
using namespace MCF;

namespace {

union alignas(std::max_align_t) PooledSharedControl {
	PooledSharedControl *pNext;
	char achDummy[sizeof(Impl::SharedControl)];
};

class Pool : NONCOPYABLE {
private:
	PooledSharedControl *volatile xm_pHead;

public:
	Pool() noexcept
		: xm_pHead(nullptr)
	{
	}
	~Pool(){
		auto pHead = AtomicLoad(xm_pHead, MemoryModel::RELAXED);
		while(pHead){
			if(EXPECT(AtomicCompareExchange(xm_pHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
				delete pHead;
			}
		}
	}

public:
	PooledSharedControl *Alloc(){
		auto pHead = AtomicLoad(xm_pHead, MemoryModel::RELAXED);
		while(pHead){
			if(EXPECT_NOT(AtomicCompareExchange(xm_pHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
				return pHead;
			}
		}
		return new PooledSharedControl;
	}
	void Dealloc(PooledSharedControl *pControl) noexcept {
		auto pHead = AtomicLoad(xm_pHead, MemoryModel::RELAXED);
		do {
			pControl->pNext = pHead;
		} while(EXPECT(!AtomicCompareExchange(xm_pHead, pHead, pControl, MemoryModel::RELAXED)));
	}
} g_vPool __attribute__((__init_priority__(101)));

}

void *Impl::SharedControl::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(PooledSharedControl));
	(void)uSize;
	return g_vPool.Alloc();
}
void Impl::SharedControl::operator delete(void *pData) noexcept {
	if(!pData){
		return;
	}
	g_vPool.Dealloc(static_cast<PooledSharedControl *>(pData));
}
