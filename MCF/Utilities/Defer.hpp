// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_DEFER_HPP_
#define MCF_UTILITIES_DEFER_HPP_

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
	template<typename LvalueRefOrPrvalueT>
	class DeferredCallbackOnNormalExit {
	private:
		int x_nOldExceptionCount;
		LvalueRefOrPrvalueT x_fnCallback;

	public:
		explicit DeferredCallbackOnNormalExit(LvalueRefOrPrvalueT &&fnCallback)
			: x_nOldExceptionCount(std::uncaught_exceptions())
			, x_fnCallback(std::forward<LvalueRefOrPrvalueT>(fnCallback))
		{
		}
		~DeferredCallbackOnNormalExit() noexcept(false) {
			if(std::uncaught_exceptions() <= x_nOldExceptionCount){
				x_fnCallback();
			}
		}
	};

	template<typename LvalueRefOrPrvalueT>
	class DeferredCallbackOnException {
	private:
		int x_nOldExceptionCount;
		LvalueRefOrPrvalueT x_fnCallback;

	public:
		explicit DeferredCallbackOnException(LvalueRefOrPrvalueT &&fnCallback)
			: x_nOldExceptionCount(std::uncaught_exceptions())
			, x_fnCallback(std::forward<LvalueRefOrPrvalueT>(fnCallback))
		{
		}
		~DeferredCallbackOnException() noexcept(false) {
			if(std::uncaught_exceptions() > x_nOldExceptionCount){
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
