// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRITICAL_SECTION_IMPL_HPP_
#define MCF_CRITICAL_SECTION_IMPL_HPP_

#include "Thread.hpp"
#include "_WinHandle.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace Impl {
	class CriticalSectionImpl {
	private:
		struct xWaitingThread {
			xWaitingThread *volatile pNext;
		};

	private:
		Impl::UniqueWinHandle xm_hSemaphore;
		unsigned long xm_ulSpinCount;

		volatile DWORD xm_dwOwner;
		std::size_t xm_uRecurCount;

		volatile std::size_t xm_uWaiting;
		xWaitingThread *xm_pFirstWaiting;
		xWaitingThread *xm_pLastWaiting;

	public:
		explicit CriticalSectionImpl(unsigned long ulSpinCount)
			: xm_ulSpinCount	(ulSpinCount)
			, xm_dwOwner		(0)
			, xm_uRecurCount	(0)
			, xm_uWaiting		(0)
			, xm_pFirstWaiting	(nullptr)
			, xm_pLastWaiting	(nullptr)
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
		unsigned long ImplGetSpinCount() const noexcept {
			return __atomic_load_n(&xm_ulSpinCount, __ATOMIC_RELAXED);
		}
		void ImplSetSpinCount(unsigned long ulSpinCount) noexcept {
			__atomic_store_n(&xm_ulSpinCount, ulSpinCount, __ATOMIC_RELAXED);
		}

		bool ImplIsLockedByCurrentThread() const noexcept {
			return __atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) == Thread::GetCurrentId();
		}

		// 首次进入临界区则返回 1，重入返回 2，否则返回 0。
		int ImplTry() noexcept {
			int nResult = 0;

			const DWORD dwThreadId = Thread::GetCurrentId();
			if(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) != dwThreadId){
				DWORD dwOldOwner = 0;
				if(__atomic_compare_exchange_n(
					&xm_dwOwner, &dwOldOwner, dwThreadId,
					false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE
				)){
					nResult = 1;
				}
			} else {
				nResult = 2;
			}

			return nResult;
		}
		// 首次进入临界区返回 true，否则返回 false。
		bool ImplEnter() noexcept {
			bool bNotRecur = false;

			const DWORD dwThreadId = Thread::GetCurrentId();
			if(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) != dwThreadId){
				bNotRecur = true;

				for(;;){
					auto i = ImplGetSpinCount();
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

					xWaitingThread vWaiting = { nullptr };
					const auto pCurrent = &vWaiting;

					auto uWaiting = xLockSpin();
					{
						++uWaiting;

						if(xm_pLastWaiting){
							xm_pLastWaiting->pNext = pCurrent;
						} else {
							xm_pFirstWaiting = pCurrent;
						}
						xm_pLastWaiting = pCurrent;
					}
					xUnlockSpin(uWaiting);

					for(;;){
						::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE);
						if(xm_pFirstWaiting == pCurrent){
							if(!(xm_pFirstWaiting = pCurrent->pNext)){
								xm_pLastWaiting = nullptr;
							}
							break;
						}
						::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr);
					}
				}
			}

		jAcquired:
			++xm_uRecurCount;

			return bNotRecur;
		}
		// 临界区被释放返回 true，否则返回 false。
		bool ImplLeave() noexcept {
			ASSERT(__atomic_load_n(&xm_dwOwner, __ATOMIC_ACQUIRE) == Thread::GetCurrentId());

			bool bReleased = false;

			if(--xm_uRecurCount == 0){
				bReleased = true;

				__atomic_store_n(&xm_dwOwner, 0, __ATOMIC_RELEASE);

				auto uWaiting = xLockSpin();
				{
					if(uWaiting){
						if(!::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr)){
							ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
						}
						--uWaiting;
					}
				}
				xUnlockSpin(uWaiting);
			}

			return bReleased;
		}
	};
}

}

#endif
