// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SharedPtr.hpp"
#include "../../MCFCRT/ext/expect.h"
#include "../Thread/Atomic.hpp"
using namespace MCF;

namespace {

union alignas(std::max_align_t) PooledSharedControl {
	PooledSharedControl *pNext;
	char achDummy[sizeof(Impl::SharedControl)];
};

PooledSharedControl *volatile g_pPoolHead = nullptr;

struct PoolClearer {
	~PoolClearer(){
		auto pHead = AtomicLoad(g_pPoolHead, MemoryModel::RELAXED);
		while(pHead){
			if(EXPECT(AtomicCompareExchange(g_pPoolHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
				delete pHead;
			}
		}
	}
} const g_vPoolClearer __attribute__((__init_priority__(101)));

}

void *Impl::SharedControl::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(PooledSharedControl));

	auto pHead = AtomicLoad(g_pPoolHead, MemoryModel::RELAXED);
	while(pHead){
		if(EXPECT_NOT(AtomicCompareExchange(g_pPoolHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
			break;
		}
	}
	if(!pHead){
		pHead = new(PooledSharedControl);
	}
	return pHead;
}
void Impl::SharedControl::operator delete(void *pData) noexcept {
	if(!pData){
		return;
	}
	const auto pToPool = static_cast<PooledSharedControl *>(pData);

	auto pHead = AtomicLoad(g_pPoolHead, MemoryModel::RELAXED);
	do {
		pToPool->pNext = pHead;
	} while(EXPECT(!AtomicCompareExchange(g_pPoolHead, pHead, pToPool, MemoryModel::RELAXED)));
}
