// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "CriticalSection.hpp"
#include "Thread.hpp"
#include "_WinHandle.hpp"
#include "../Core/Exception.hpp"
#include "../Core/StringObserver.hpp"
using namespace MCF;

namespace {

class CriticalSectionDelegate : CONCRETE(CriticalSection) {
private:
	Impl::UniqueWinHandle xm_hSemaphore;
	unsigned long xm_ulSpinCount;

	volatile DWORD xm_dwOwner;
	std::size_t xm_uRecurCount;
	volatile std::size_t xm_uWaiting;

public:
	CriticalSectionDelegate(unsigned long ulSpinCount)
		: xm_ulSpinCount	(ulSpinCount)
		, xm_dwOwner		(0)
		, xm_uRecurCount	(0)
		, xm_uWaiting		(0)
	{
		xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, 0, 1, nullptr));
		if(!xm_hSemaphore){
			MCF_THROW(::GetLastError(), L"CreateSemaphoreW() 失败。"_wso);
		}

		SYSTEM_INFO vSystemInfo;
		::GetSystemInfo(&vSystemInfo);
		if(vSystemInfo.dwNumberOfProcessors == 1){
			xm_ulSpinCount = 0;
		}
	}

private:
	// http://wiki.osdev.org/Spinlock
	std::size_t xLockSpin() noexcept {
		std::size_t uWaiting;
		for(;;){
			uWaiting = __atomic_exchange_n(&xm_uWaiting, (std::size_t)-1, __ATOMIC_ACQ_REL | __ATOMIC_HLE_ACQUIRE);
			if(EXPECT_NOT(uWaiting != (std::size_t)-1)){
				break;
			}
			do {
				__asm__ __volatile__("pause \n");
			} while(EXPECT(__atomic_load_n(&xm_uWaiting, __ATOMIC_ACQUIRE) == (std::size_t)-1));
		}
		return uWaiting;
	}
	void xUnlockSpin(std::size_t uNewWaiting) noexcept {
		__atomic_store_n(&xm_uWaiting, uNewWaiting, __ATOMIC_RELEASE | __ATOMIC_HLE_RELEASE);
	}

public:
	bool IsLockedByCurrentThread() const noexcept {
		return __atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) == Thread::GetCurrentId();
	}

	void Enter() noexcept {
		const DWORD dwThreadId = Thread::GetCurrentId();

		if(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) != dwThreadId){
			for(;;){
				auto i = xm_ulSpinCount;
				for(;;){
					DWORD dwOldOwner = 0;
					if(EXPECT_NOT(__atomic_compare_exchange_n(
						&xm_dwOwner, &dwOldOwner, dwThreadId,
						false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE
					))){
						goto jAcquired;
					}
					if(EXPECT_NOT(i)){
						break;
					}
					--i;
				}

				auto uWaiting = xLockSpin();
				++uWaiting;
				xUnlockSpin(uWaiting);

				::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
			}
		}

	jAcquired:
		++xm_uRecurCount;
	}
	void Leave() noexcept {
		ASSERT(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) == Thread::GetCurrentId());

		if(--xm_uRecurCount == 0){
			__atomic_store_n(&xm_dwOwner, 0, __ATOMIC_RELEASE);

			auto uWaiting = xLockSpin();
			if(uWaiting){
				::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr);

				--uWaiting;
			}
			xUnlockSpin(uWaiting);
		}
	}
};

}

namespace MCF {

namespace Impl {
	template<>
	void CriticalSection::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->Enter();
	}
	template<>
	void CriticalSection::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionDelegate *>(xm_pOwner));

		static_cast<CriticalSectionDelegate *>(xm_pOwner)->Leave();
	}
}

}

// 静态成员函数。
std::unique_ptr<CriticalSection> CriticalSection::Create(unsigned long ulSpinCount){
	return std::make_unique<CriticalSectionDelegate>(ulSpinCount);
}

// 其他非静态成员函数。
bool CriticalSection::IsLockedByCurrentThread() const noexcept {
	ASSERT(dynamic_cast<const CriticalSectionDelegate *>(this));

	return static_cast<const CriticalSectionDelegate *>(this)->IsLockedByCurrentThread();
}

CriticalSection::Lock CriticalSection::GetLock() noexcept {
	return Lock(this);
}
