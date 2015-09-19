// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include <utility>
#include <cstddef>
#include "Atomic.hpp"
#include "../Utilities/Defer.hpp"

namespace MCF {

namespace Impl_CallOnce {
	extern void OnceMutexLock() noexcept;
	extern void OnceMutexUnlock() noexcept;

	class OnceFlag {
	public:
		enum State {
			kUninitialized = 0,
			kInitializing  = 1,
			kInitialized   = 2,
		};

	private:
		Atomic<State> x_eState;

	public:
		constexpr OnceFlag() noexcept
			: x_eState(kUninitialized)
		{
		}

		OnceFlag(const OnceFlag &) = delete;
		OnceFlag &operator=(const OnceFlag &) = delete;

	public:
		State Load(MemoryModel eModel) const volatile noexcept {
			return x_eState.Load(eModel);
		}
		void Store(State eState, MemoryModel eModel) volatile noexcept {
			x_eState.Store(eState, eModel);
		}
	};
}

using OnceFlag = volatile Impl_CallOnce::OnceFlag;

template<typename FunctionT, typename ...ParamsT>
bool CallOnce(OnceFlag &vFlag, FunctionT &&vFunction, ParamsT &&...vParams){
	if(vFlag.Load(kAtomicConsume) == OnceFlag::kInitialized){
		return false;
	}

	Impl_CallOnce::OnceMutexLock();
	DEFER([&]{ Impl_CallOnce::OnceMutexUnlock(); });

	if(vFlag.Load(kAtomicRelaxed) == OnceFlag::kInitialized){
		return false;
	}

	vFlag.Store(OnceFlag::kInitializing, kAtomicRelaxed);
	try {
		std::forward<FunctionT>(vFunction)(std::forward<ParamsT>(vParams)...);
	} catch(...){
		// vFlag.Store(OnceFlag::kUninitialized, kAtomicRelease);
		vFlag.Store(OnceFlag::kUninitialized, kAtomicRelaxed);
		throw;
	}
	// vFlag.Store(OnceFlag::kInitialized, kAtomicRelease);
	vFlag.Store(OnceFlag::kInitialized, kAtomicRelaxed);
	return true;
}

}

#endif
