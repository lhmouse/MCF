// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IntrusivePtr.hpp"
#include "../Thread/Mutex.hpp"

namespace MCF {

using ViewPoolElement = Impl_IntrusivePtr::ViewPoolElement;

namespace {
	Mutex g_mtxPoolMutex;
	ViewPoolElement *g_pPoolHead;

	__attribute__((__destructor__(101)))
	void PoolDestructor() noexcept {
		auto pCur = g_pPoolHead;
		while(pCur){
			const auto pPrev = pCur->pPrev;
			::operator delete(pCur);
			pCur = pPrev;
		}
		g_pPoolHead = nullptr;
	}
}

void *ViewPoolElement::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(ViewPoolElement));

	ViewPoolElement *pCur;
	{
		const Mutex::UniqueLock vLock(g_mtxPoolMutex);
		pCur = g_pPoolHead;
	}
	if(!pCur){
		pCur = static_cast<ViewPoolElement *>(::operator new(sizeof(ViewPoolElement)));
	}
	return pCur;
}
void ViewPoolElement::operator delete(void *pRaw) noexcept {
	if(!pRaw){
		return;
	}
	const auto pCur = static_cast<ViewPoolElement *>(pRaw);

	const Mutex::UniqueLock vLock(g_mtxPoolMutex);
	pCur->pPrev = g_pPoolHead;
	g_pPoolHead = pCur;
}

}
