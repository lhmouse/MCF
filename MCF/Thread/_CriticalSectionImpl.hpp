// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CRITICAL_SECTION_IMPL_HPP_
#define MCF_THREAD_CRITICAL_SECTION_IMPL_HPP_

#include "Thread.hpp"
#include "_WinHandle.hpp"
#include "_LockRaiiTemplate.hpp"
#include "../Utilities/NoCopy.hpp"
#include "../Utilities/Assert.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace Impl {
	class CriticalSectionImpl : NO_COPY {
	public:
		enum class Result {
			TRY_FAILED,
			STATE_CHANGED,
			RECURSIVE
		};

		using Lock = LockRaiiTemplate<CriticalSectionImpl>;

	private:
		Impl::UniqueWinHandle xm_hSemaphore;
		unsigned long xm_ulSpinCount;

		volatile DWORD xm_dwLocking;
		std::size_t xm_uRecurCount;
		volatile std::size_t xm_uWaiting;

	public:
		explicit CriticalSectionImpl(unsigned long ulSpinCount)
			: xm_ulSpinCount(ulSpinCount)
			, xm_dwLocking(0), xm_uRecurCount(0), xm_uWaiting(0)
		{
			if(!xm_hSemaphore.Reset(::CreateSemaphoreW(nullptr, 0, 1, nullptr))){
				DEBUG_THROW(SystemError, "CreateSemaphoreW");
			}

			SYSTEM_INFO vSystemInfo;
			::GetSystemInfo(&vSystemInfo);
			if(vSystemInfo.dwNumberOfProcessors == 1){
				xm_ulSpinCount = 0;
			}
		}
		~CriticalSectionImpl(){
			ASSERT(__atomic_load_n(&xm_dwLocking, __ATOMIC_ACQUIRE) == 0);
		}

	private:
		// http://wiki.osdev.org/Spinlock
//		std::size_t xLockSpin() noexcept {	// FIXME: g++ 4.9.2 ICE
		std::size_t xLockSpin() throw() {	//
			std::size_t uWaiting;
			for(;;){
				uWaiting = __atomic_exchange_n(&xm_uWaiting, (std::size_t)-1, __ATOMIC_ACQ_REL);
				if(EXPECT_NOT(uWaiting != (std::size_t)-1)){
					break;
				}
				do {
					__builtin_ia32_pause();
				} while(EXPECT(__atomic_load_n(&xm_uWaiting, __ATOMIC_ACQUIRE) == (std::size_t)-1));
			}
			return uWaiting;
		}
		void xUnlockSpin(std::size_t uNewWaiting) noexcept {
			__atomic_store_n(&xm_uWaiting, uNewWaiting, __ATOMIC_RELEASE);
		}

	public:
		unsigned long ImplGetSpinCount() const noexcept {
			return __atomic_load_n(&xm_ulSpinCount, __ATOMIC_RELAXED);
		}
		void ImplSetSpinCount(unsigned long ulSpinCount) noexcept {
			__atomic_store_n(&xm_ulSpinCount, ulSpinCount, __ATOMIC_RELAXED);
		}

		bool ImplIsLockedByCurrentThread() const noexcept {
			return __atomic_load_n(&xm_dwLocking, __ATOMIC_ACQUIRE) == Thread::GetCurrentId();
		}

		Result ImplTry() noexcept {
			const DWORD dwThreadId = Thread::GetCurrentId();

			Result eResult;
			DWORD dwOldOwner = __atomic_load_n(&xm_dwLocking, __ATOMIC_ACQUIRE);
			if(dwOldOwner == dwThreadId){
				++xm_uRecurCount;
				eResult = Result::RECURSIVE;
			} else if((dwOldOwner == 0) &&
				__atomic_compare_exchange_n(&xm_dwLocking, &dwOldOwner, dwThreadId,
					false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
			{
				++xm_uRecurCount;
				eResult = Result::STATE_CHANGED;
			} else {
				eResult = Result::TRY_FAILED;
			}
			return eResult;
		}
		Result ImplEnter() noexcept {
			const DWORD dwThreadId = Thread::GetCurrentId();

			Result eResult;
			DWORD dwOldOwner = __atomic_load_n(&xm_dwLocking, __ATOMIC_ACQUIRE);
			if(dwOldOwner == dwThreadId){
				++xm_uRecurCount;
				eResult = Result::RECURSIVE;
			} else {
				for(;;){
					auto i = ImplGetSpinCount();
					for(;;){
						dwOldOwner = 0;
						if(EXPECT_NOT(__atomic_compare_exchange_n(&xm_dwLocking, &dwOldOwner, dwThreadId,
							false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)))
						{
							goto jAcquired;
						}
						if(EXPECT_NOT(i == 0)){
							break;
						}
						--i;
						__builtin_ia32_pause();
					}

					auto uWaiting = xLockSpin();
					++uWaiting;
					xUnlockSpin(uWaiting);

					if(::WaitForSingleObject(xm_hSemaphore.Get(), INFINITE) == WAIT_FAILED){
						ASSERT_MSG(false, L"WaitForSingleObject() 失败。");
					}
				}
			jAcquired:
				++xm_uRecurCount;
				eResult = Result::STATE_CHANGED;
			}
			return eResult;
		}
		Result ImplLeave() noexcept {
			ASSERT(__atomic_load_n(&xm_dwLocking, __ATOMIC_ACQUIRE) == Thread::GetCurrentId());

			Result eResult;
			if(--xm_uRecurCount == 0){
				__atomic_store_n(&xm_dwLocking, 0, __ATOMIC_RELEASE);

				auto uWaiting = xLockSpin();
				if(uWaiting != 0){
					if(!::ReleaseSemaphore(xm_hSemaphore.Get(), 1, nullptr)){
						ASSERT_MSG(false, L"ReleaseSemaphore() 失败。");
					}
					--uWaiting;
				}
				xUnlockSpin(uWaiting);

				eResult = Result::STATE_CHANGED;
			} else {
				eResult = Result::RECURSIVE;
			}
			return eResult;
		}
	};

	template<>
	inline bool CriticalSectionImpl::Lock::xDoTry() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionImpl *>(xm_pOwner));

		return static_cast<CriticalSectionImpl *>(xm_pOwner)->ImplTry() != CriticalSectionImpl::Result::TRY_FAILED;
	}
	template<>
	inline void CriticalSectionImpl::Lock::xDoLock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionImpl *>(xm_pOwner));

		static_cast<CriticalSectionImpl *>(xm_pOwner)->ImplEnter();
	}
	template<>
	inline void CriticalSectionImpl::Lock::xDoUnlock() const noexcept {
		ASSERT(dynamic_cast<CriticalSectionImpl *>(xm_pOwner));

		static_cast<CriticalSectionImpl *>(xm_pOwner)->ImplLeave();
	}
}

}

#endif
