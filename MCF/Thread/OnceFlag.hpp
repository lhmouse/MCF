// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_CALL_ONCE_HPP_
#define MCF_THREAD_CALL_ONCE_HPP_

#include "../../MCFCRT/env/once_flag.h"
#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/Assert.hpp"
#include <utility>
#include <cstdint>

namespace MCF {

class OnceFlag : MCF_NONCOPYABLE {
public:
	enum OnceResult {
		kTimedOut     = ::_MCFCRT_kOnceResultTimedOut,
		kInitialized  = ::_MCFCRT_kOnceResultInitial,
		kFinished     = ::_MCFCRT_kOnceResultFinished,
	};

private:
	::_MCFCRT_OnceFlag x_vFlag;

public:
	constexpr OnceFlag() noexcept
		: x_vFlag(_MCFCRT_ONCE_FLAG_INITIALIZER)
	{
	}

public:
	template<typename FuncT>
	OnceResult CallOnce(FuncT &&vFunc, std::uint64_t u64UntilFastMonoClock){
		const auto eResult = ::_MCFCRT_WaitForOnceFlag(&x_vFlag, u64UntilFastMonoClock);
		if(eResult == ::_MCFCRT_kOnceResultInitial){
			try {
				std::forward<FuncT>(vFunc)();
			} catch(...){
				::_MCFCRT_SignalOnceFlagAsAborted(&x_vFlag);
				throw;
			}
			::_MCFCRT_SignalOnceFlagAsFinished(&x_vFlag);
		}
		return static_cast<OnceResult>(eResult);
	}
	template<typename FuncT>
	OnceResult CallOnce(FuncT &&vFunc){
		const auto eResult = ::_MCFCRT_WaitForOnceFlagForever(&x_vFlag);
		MCF_ASSERT(eResult != ::_MCFCRT_kOnceResultTimedOut);
		if(eResult == ::_MCFCRT_kOnceResultInitial){
			try {
				std::forward<FuncT>(vFunc)();
			} catch(...){
				::_MCFCRT_SignalOnceFlagAsAborted(&x_vFlag);
				throw;
			}
			::_MCFCRT_SignalOnceFlagAsFinished(&x_vFlag);
		}
		return static_cast<OnceResult>(eResult);
	}
};

}

#endif
