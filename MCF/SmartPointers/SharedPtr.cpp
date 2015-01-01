// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SharedPtr.hpp"
#include "../Thread/Mutex.hpp"
using namespace MCF;

namespace {

union alignas(std::max_align_t) PooledSharedControl {
	PooledSharedControl *pNext;
	char achDummy[sizeof(Impl::SharedControl)];
};

class Pool : NONCOPYABLE {
private:
	Mutex xm_vMutex;
	PooledSharedControl *xm_pHead;

public:
	Pool()
		: xm_pHead(nullptr)
	{
	}
	~Pool(){
		auto pCur = xm_pHead;
		while(pCur){
			const auto pNext = pCur->pNext;
			delete pCur;
			pCur = pNext;
		}
	}

public:
	PooledSharedControl *Allocate(){
		{
			const auto vLock = xm_vMutex.GetLock();
			if(xm_pHead){
				return std::exchange(xm_pHead, xm_pHead->pNext);
			}
		}
		return new PooledSharedControl;
	}
	void Deallocate(PooledSharedControl *pPooled) noexcept {
		const auto vLock = xm_vMutex.GetLock();
		pPooled->pNext = std::exchange(xm_pHead, pPooled);
	}
} g_vPool;

}

void *Impl::SharedControl::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(PooledSharedControl));
	return g_vPool.Allocate();
}
void Impl::SharedControl::operator delete(void *pData) noexcept {
	g_vPool.Deallocate(static_cast<PooledSharedControl *>(pData));
}
