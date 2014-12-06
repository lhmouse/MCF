// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CRITICAL_SECTION_IMPL_INL_
#define MCF_THREAD_CRITICAL_SECTION_IMPL_INL_

#include "../StdMCF.hpp"
#include "LockRaiiTemplate.hpp"
#include "Thread.hpp"
#include "../Utilities/NoCopy.hpp"
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
		volatile unsigned long xm_ulSpinCount;
		CRITICAL_SECTION xm_vCriticalSection;
		volatile DWORD xm_dwLockingThread;
		std::size_t xm_uRecursionCount;

	public:
		explicit CriticalSectionImpl(unsigned long ulSpinCount)
			: xm_ulSpinCount(ulSpinCount), xm_dwLockingThread(0), xm_uRecursionCount(0)
		{
			if(!::InitializeCriticalSectionEx(&xm_vCriticalSection, ulSpinCount,
#ifdef NDEBUG
				CRITICAL_SECTION_NO_DEBUG_INFO
#else
				0
#endif
				))
			{
				DEBUG_THROW(SystemError, "InitializeCriticalSectionEx");
			}
		}
		~CriticalSectionImpl(){
			ASSERT(__atomic_load_n(&xm_dwLockingThread, __ATOMIC_RELAXED) == 0);
			::DeleteCriticalSection(&xm_vCriticalSection);
		}

	public:
		unsigned long ImplGetSpinCount() const noexcept {
			return __atomic_load_n(&xm_ulSpinCount, __ATOMIC_RELAXED);
		}
		void ImplSetSpinCount(unsigned long ulSpinCount) noexcept {
			::SetCriticalSectionSpinCount(&xm_vCriticalSection, ulSpinCount);
			__atomic_store_n(&xm_ulSpinCount, ulSpinCount, __ATOMIC_RELAXED);
		}

		bool ImplIsLockedByCurrentThread() const noexcept {
			return __atomic_load_n(&xm_dwLockingThread, __ATOMIC_RELAXED) == Thread::GetCurrentId();
		}

		Result ImplTry() noexcept {
			Result eResult;
			if(!::TryEnterCriticalSection(&xm_vCriticalSection)){
				return Result::TRY_FAILED;
			}
			if(++xm_uRecursionCount == 1){
				__atomic_store_n(&xm_dwLockingThread, Thread::GetCurrentId(), __ATOMIC_RELAXED);
				eResult = Result::STATE_CHANGED;
			} else {
				eResult = Result::RECURSIVE;
			}
			return eResult;
		}
		Result ImplEnter() noexcept {
			Result eResult;
			::EnterCriticalSection(&xm_vCriticalSection);
			if(++xm_uRecursionCount == 1){
				__atomic_store_n(&xm_dwLockingThread, Thread::GetCurrentId(), __ATOMIC_RELAXED);
				eResult = Result::STATE_CHANGED;
			} else {
				eResult = Result::RECURSIVE;
			}
			return eResult;
		}
		Result ImplLeave() noexcept {
			Result eResult;
			if(--xm_uRecursionCount == 0){
				__atomic_store_n(&xm_dwLockingThread, 0, __ATOMIC_RELAXED);
				eResult = Result::STATE_CHANGED;
			} else {
				eResult = Result::RECURSIVE;
			}
			::LeaveCriticalSection(&xm_vCriticalSection);
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
