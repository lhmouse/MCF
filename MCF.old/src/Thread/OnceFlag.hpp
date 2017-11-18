// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include "../Core/Assert.hpp"
#include <MCFCRT/env/once_flag.h>
#include <utility>
#include <cstdint>

namespace MCF {

class OnceFlag {
public:
	enum OnceResult {
		kTimedOut  = ::_MCFCRT_kOnceResultTimedOut,
		kInitial   = ::_MCFCRT_kOnceResultInitial,
		kFinished  = ::_MCFCRT_kOnceResultFinished,
	};

private:
	::_MCFCRT_OnceFlag x_vOnce;

public:
	constexpr OnceFlag() noexcept
		: x_vOnce{ 0 }
	{ }

	OnceFlag(const OnceFlag &) = delete;
	OnceFlag &operator=(const OnceFlag &) = delete;

public:
	template<typename FuncT>
	OnceResult CallOnce(FuncT &&vFunc, std::uint64_t u64UntilFastMonoClock){
		const auto eResult = ::_MCFCRT_WaitForOnceFlag(&x_vOnce, u64UntilFastMonoClock);
		if(eResult == ::_MCFCRT_kOnceResultInitial){
			try {
				std::forward<FuncT>(vFunc)();
			} catch(...){
				::_MCFCRT_SignalOnceFlagAsAborted(&x_vOnce);
				throw;
			}
			::_MCFCRT_SignalOnceFlagAsFinished(&x_vOnce);
		}
		return static_cast<OnceResult>(eResult);
	}
	template<typename FuncT>
	OnceResult CallOnce(FuncT &&vFunc){
		const auto eResult = ::_MCFCRT_WaitForOnceFlagForever(&x_vOnce);
		MCF_ASSERT(eResult != ::_MCFCRT_kOnceResultTimedOut);
		if(eResult == ::_MCFCRT_kOnceResultInitial){
			try {
				std::forward<FuncT>(vFunc)();
			} catch(...){
				::_MCFCRT_SignalOnceFlagAsAborted(&x_vOnce);
				throw;
			}
			::_MCFCRT_SignalOnceFlagAsFinished(&x_vOnce);
		}
		return static_cast<OnceResult>(eResult);
	}
};

}

#endif
