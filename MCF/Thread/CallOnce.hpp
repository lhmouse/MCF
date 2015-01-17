// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include <utility>
#include <cstddef>
#include "Atomic.hpp"
#include "../Utilities/Defer.hpp"
#include "../../MCFCRT/env/global_mutex.h"

namespace MCF {

namespace Impl {
	class OnceFlag {
	private:
		bool xm_bFlag;

	public:
		constexpr OnceFlag() noexcept
			: xm_bFlag(false)
		{
		}

		OnceFlag(const OnceFlag &) = delete;
		OnceFlag &operator=(const OnceFlag &) = delete;

	public:
		operator const volatile bool &() const volatile & noexcept {
			return xm_bFlag;
		}
		operator volatile bool &() volatile & noexcept {
			return xm_bFlag;
		}
	};
}

using OnceFlag = volatile Impl::OnceFlag;

template<typename FunctionT, typename ...ParamsT>
bool CallOnce(OnceFlag &vFlag, FunctionT &&vFunction, ParamsT &&...vParams){
	auto &bFlag = static_cast<volatile bool &>(vFlag);

	if(AtomicLoad(bFlag, MemoryModel::ACQUIRE)){
		return false;
	} else {
		::MCF_CRT_GlobalMutexLock();
		DEFER(::MCF_CRT_GlobalMutexUnlock);

		if(AtomicLoad(bFlag, MemoryModel::ACQUIRE)){
			return false;
		}
		std::forward<FunctionT>(vFunction)(std::forward<ParamsT>(vParams)...);
		AtomicStore(bFlag, true, MemoryModel::RELEASE);
		return true;
	}
}

}

#endif
