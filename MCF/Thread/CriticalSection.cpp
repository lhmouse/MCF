// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "Semaphore.hpp"
using namespace MCF;

namespace {

class CriticalSectionDelegate : CONCRETE(CriticalSection) {
private:
	unsigned long xm_ulSpinCount;
	volatile DWORD xm_dwOwner;
	std::size_t xm_uRecurCount;
	const std::unique_ptr<Semaphore> xm_psemExclusive;
	volatile std::size_t xm_uWaiting;

public:
	CriticalSectionDelegate(unsigned long ulSpinCount) noexcept
		: xm_ulSpinCount	(ulSpinCount)
		, xm_dwOwner		(0)
		, xm_uRecurCount	(0)
		, xm_psemExclusive	(Semaphore::Create(0, 1))
		, xm_uWaiting		(0)
	{
		SYSTEM_INFO vSystemInfo;
		::GetSystemInfo(&vSystemInfo);
		if(vSystemInfo.dwNumberOfProcessors == 1){
			xm_ulSpinCount = 0;
		}
	}

public:
	void Enter() noexcept {
		const DWORD dwThreadId = ::GetCurrentThreadId();

		if(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) != dwThreadId){
			for(;;){
				auto i = xm_ulSpinCount;
				for(;;){
					DWORD dwOldOwner = 0;
					if(EXPECT_NOT(__atomic_compare_exchange_n(&xm_dwOwner, &dwOldOwner, dwThreadId, false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))){
						goto jAcquired;
					}
					if(EXPECT_NOT(i)){
						break;
					}
				}

				std::size_t uWaiting;
				for(;;){
					uWaiting = __atomic_exchange_n(&xm_uWaiting, (std::size_t)-1, __ATOMIC_ACQ_REL);
					if(EXPECT_NOT(uWaiting != (std::size_t)-1)){
						break;
					}
					::SwitchToThread();
				}

				++uWaiting;
				__atomic_store_n(&xm_uWaiting, uWaiting, __ATOMIC_RELEASE);
				xm_psemExclusive->Wait();
			}
		}

	jAcquired:
		++xm_uRecurCount;
	}
	void Leave() noexcept {
		ASSERT(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) == ::GetCurrentThreadId());

		if(--xm_uRecurCount == 0){
			__atomic_store_n(&xm_dwOwner, 0, __ATOMIC_RELEASE);

			std::size_t uWaiting;
			for(;;){
				uWaiting = __atomic_exchange_n(&xm_uWaiting, (std::size_t)-1, __ATOMIC_ACQ_REL);
				if(EXPECT_NOT(uWaiting != (std::size_t)-1)){
					break;
				}
				::SwitchToThread();
			}

			if(uWaiting){
				xm_psemExclusive->Signal();
				--uWaiting;
			}
			__atomic_store_n(&xm_uWaiting, uWaiting, __ATOMIC_RELEASE);
		}
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	void CriticalSection::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		((CriticalSectionDelegate *)xm_pOwner)->Enter();
	}
	template<>
	void CriticalSection::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		((CriticalSectionDelegate *)xm_pOwner)->Leave();
	}
}

}

// 静态成员函数。
std::unique_ptr<CriticalSection> CriticalSection::Create(unsigned long ulSpinCount){
	return std::make_unique<CriticalSectionDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(this);
}
