// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "IntrusivePtr.hpp"
#include "../Thread/Atomic.hpp"

namespace MCF {

using ViewPoolElement = Impl_IntrusivePtr::ViewPoolElement;

namespace {
	Atomic<ViewPoolElement *> g_pPoolHead;

	__attribute__((__destructor__(101)))
	void PoolDestructor() noexcept {
		auto pCur = g_pPoolHead.Exchange(nullptr, kAtomicRelaxed);
		while(pCur){
			const auto pNext = pCur->pNext;
			::operator delete(pCur);
			pCur = pNext;
		}
	}
}

void *ViewPoolElement::operator new(std::size_t uSize){
	ASSERT(uSize == sizeof(ViewPoolElement));

	auto pCur = g_pPoolHead.Load(kAtomicRelaxed);
	for(;;){
		if(!pCur){
			pCur = static_cast<ViewPoolElement *>(::operator new(sizeof(ViewPoolElement)));
			break;
		}
		const auto pNext = pCur->pNext;
		if(g_pPoolHead.CompareExchange(pCur, pNext, kAtomicRelaxed, kAtomicRelaxed)){
			break;
		}
	}
	return pCur;
}
void ViewPoolElement::operator delete(void *pRaw) noexcept {
	if(!pRaw){
		return;
	}
	const auto pCur = static_cast<ViewPoolElement *>(pRaw);

	auto pNext = g_pPoolHead.Load(kAtomicRelaxed);
	for(;;){
		pCur->pNext = pNext;
		if(g_pPoolHead.CompareExchange(pNext, pCur, kAtomicRelaxed, kAtomicRelaxed)){
			break;
		}
	}
}

}
