// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DEFER_HPP_
#define MCF_CORE_DEFER_HPP_

#include <cstddef>
#include <utility>
#include <type_traits>
#include <exception>

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
/*
	inline std::size_t GetUncaughtExceptionCount() noexcept {
		return static_cast<std::size_t>(std::uncaught_exceptions()); // FIXME: 需要 c++1z 的支持。
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
	};*/
}

template<typename CallbackT>
auto Defer(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallback<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}
/*
template<typename CallbackT>
auto DeferOnNormalExit(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallbackOnNormalExit<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}

template<typename CallbackT>
auto DeferOnException(CallbackT &&vCallback){
	return Impl_Defer::DeferredCallbackOnException<std::remove_cv_t<CallbackT>>(std::forward<CallbackT>(vCallback));
}
*/
}

#endif
