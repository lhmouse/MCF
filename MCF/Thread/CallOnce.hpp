// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include "Atomic.hpp"
#include "Mutex.hpp"
#include <utility>

namespace MCF {

class OnceFlag {
private:
	Atomic<bool> x_bInitialized;
	Mutex x_mtxGuard;

public:
	constexpr OnceFlag() noexcept
		: x_bInitialized(false), x_mtxGuard()
	{
	}

public:
	template<typename FuncT, typename ...ParamsT>
	bool Call(FuncT &&vFunc, ParamsT &&...vParams){
		if(x_bInitialized.Load(kAtomicRelaxed)){
			return false;
		}
		const auto vLock = x_mtxGuard.GetLock();
		if(x_bInitialized.Load(kAtomicRelaxed)){
			return false;
		}
		std::forward<FuncT>(vFunc)(std::forward<ParamsT>(vParams)...);
		AtomicFence(kAtomicRelease);
		x_bInitialized.Store(true, kAtomicRelaxed);
		return true;
	}
};

template<typename FuncT, typename ...ParamsT>
bool CallOnce(OnceFlag &vFlag, FuncT &&vFunc, ParamsT &&...vParams){
	return vFlag.Call(std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)...);
}

}

#endif
