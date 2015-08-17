// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include <utility>
#include <cstddef>
#include "Atomic.hpp"
#include "Mutex.hpp"

namespace MCF {

namespace Impl_CallOnce {
	class OnceFlag {
	public:
		static Mutex &GetMutex() noexcept;

	private:
		Atomic<bool> x_bFlag;

	public:
		constexpr OnceFlag() noexcept
			: x_bFlag(false)
		{
		}

		OnceFlag(const OnceFlag &) = delete;
		OnceFlag &operator=(const OnceFlag &) = delete;

	public:
		bool Load(MemoryModel eModel) const volatile noexcept {
			return x_bFlag.Load(eModel);
		}
		void Store(bool bOperand, MemoryModel eModel) volatile noexcept {
			x_bFlag.Store(bOperand, eModel);
		}
	};
}

using OnceFlag = volatile Impl_CallOnce::OnceFlag;

template<typename FunctionT, typename ...ParamsT>
bool CallOnce(OnceFlag &vFlag, FunctionT &&vFunction, ParamsT &&...vParams){
	if(vFlag.Load(MemoryModel::kConsume)){
		return false;
	}
	{
		const auto vLock = Impl_CallOnce::OnceFlag::GetMutex().GetLock();
		if(vFlag.Load(MemoryModel::kRelaxed)){
			return false;
		}
		std::forward<FunctionT>(vFunction)(std::forward<ParamsT>(vParams)...);
		vFlag.Store(true, MemoryModel::kRelease);
	}
	return true;
}

}

#endif
