// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

// 我们使用了 libstdc++ 的一个内部结构体（__cxa_eh_globals）。
// ----------------------------------------------------------------------------
// GCC is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// GCC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#ifndef MCF_UTILITIES_DEFER_HPP_
#define MCF_UTILITIES_DEFER_HPP_

#include <cstddef>
#include <utility>
#include <type_traits>
#include <c++/cxxabi.h> // FIXME: std::uncaught_exceptions() 需要 c++1z 的支持。 // #include <exception>

// 下面这个结构是从 gcc 的工程目录里的 libstdc++-v3/libsupc++/unwind-cxx.h 中复制过来的。
namespace __cxxabiv1 {
// Each thread in a C++ program has access to a __cxa_eh_globals object.
struct __cxa_eh_globals
{
  __cxa_exception *caughtExceptions;
  unsigned int uncaughtExceptions;
#ifdef __ARM_EABI_UNWINDER__
  __cxa_exception* propagatingExceptions;
#endif
};
}

namespace MCF {

namespace Impl_Defer {
	template<typename LvalueRefOrPrvalueT>
	class DeferredCallback {
	private:
		LvalueRefOrPrvalueT x_fnCallback;

	public:
		explicit DeferredCallback(LvalueRefOrPrvalueT &&fnCallback)
			: x_fnCallback(std::forward<LvalueRefOrPrvalueT>(fnCallback))
		{
		}
		~DeferredCallback() noexcept(false) {
			x_fnCallback();
		}
	};

	inline std::size_t GetUncaughtExceptionCount() noexcept {
		// return std::uncaught_exceptions(); // FIXME: 需要 c++1z 的支持。
		return ::__cxxabiv1::__cxa_get_globals()->uncaughtExceptions;
	}

	template<typename LvalueRefOrPrvalueT>
	class DeferredCallbackOnNormalExit {
	private:
		std::size_t x_uOldExceptionCount;
		LvalueRefOrPrvalueT x_fnCallback;

	public:
		explicit DeferredCallbackOnNormalExit(LvalueRefOrPrvalueT &&fnCallback)
			: x_uOldExceptionCount(GetUncaughtExceptionCount())
			, x_fnCallback(std::forward<LvalueRefOrPrvalueT>(fnCallback))
		{
		}
		~DeferredCallbackOnNormalExit() noexcept(false) {
			if(GetUncaughtExceptionCount() <= x_uOldExceptionCount){
				x_fnCallback();
			}
		}
	};

	template<typename LvalueRefOrPrvalueT>
	class DeferredCallbackOnException {
	private:
		std::size_t x_uOldExceptionCount;
		LvalueRefOrPrvalueT x_fnCallback;

	public:
		explicit DeferredCallbackOnException(LvalueRefOrPrvalueT &&fnCallback)
			: x_uOldExceptionCount(GetUncaughtExceptionCount())
			, x_fnCallback(std::forward<LvalueRefOrPrvalueT>(fnCallback))
		{
		}
		~DeferredCallbackOnException() noexcept(false) {
			if(GetUncaughtExceptionCount() > x_uOldExceptionCount){
				x_fnCallback();
			}
		}
	};
}

template<typename CallbackT>
auto Defer(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallback<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}

template<typename CallbackT>
auto DeferOnNormalExit(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallbackOnNormalExit<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}

template<typename CallbackT>
auto DeferOnException(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallbackOnException<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}

}

#endif
