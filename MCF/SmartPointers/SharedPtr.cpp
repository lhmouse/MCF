// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SharedPtr.hpp"
#include "../../MCFCRT/ext/expect.h"
#include "../Thread/Atomic.hpp"
#include "../Utilities/Noncopyable.hpp"

namespace MCF {

namespace {
	union alignas(std::max_align_t) PooledSharedControl {
		PooledSharedControl *pNext;
		char achDummy[sizeof(Impl_SharedPtr::SharedControl)];
	};

	class Pool : NONCOPYABLE {
	private:
		PooledSharedControl *volatile x_pHead;

	public:
		Pool() noexcept
			: x_pHead(nullptr)
		{
		}
		~Pool(){
			auto pHead = AtomicLoad(x_pHead, MemoryModel::RELAXED);
			while(pHead){
				if(EXPECT(AtomicCompareExchange(x_pHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
					delete pHead;
				}
			}
		}

	public:
		PooledSharedControl *Alloc(){
			auto pHead = AtomicLoad(x_pHead, MemoryModel::RELAXED);
			while(pHead){
				if(EXPECT_NOT(AtomicCompareExchange(x_pHead, pHead, pHead->pNext, MemoryModel::RELAXED))){
					return pHead;
				}
			}
			return new PooledSharedControl;
		}
		void Dealloc(PooledSharedControl *pControl) noexcept {
			auto pHead = AtomicLoad(x_pHead, MemoryModel::RELAXED);
			do {
				pControl->pNext = pHead;
			} while(EXPECT(!AtomicCompareExchange(x_pHead, pHead, pControl, MemoryModel::RELAXED)));
		}
	} g_vPool __attribute__((__init_priority__(101)));
}

void *Impl_SharedPtr::SharedControl::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(PooledSharedControl));
	(void)uSize;
	return g_vPool.Alloc();
}
void Impl_SharedPtr::SharedControl::operator delete(void *pData) noexcept {
	if(!pData){
		return;
	}
	g_vPool.Dealloc(static_cast<PooledSharedControl *>(pData));
}

}
