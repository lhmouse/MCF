// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DEFER_HPP_
#define MCF_CORE_DEFER_HPP_

#include <utility>
#include <type_traits>
#include <exception>

namespace MCF {

namespace Impl_Defer {
	template<typename CallbackT, typename PredictorT>
	struct DeferredCallback {
		std::decay_t<CallbackT> vCallback;
		std::decay_t<PredictorT> vPredictor;

		~DeferredCallback() noexcept(false) {
			if(std::forward<PredictorT>(vPredictor)()){
				std::forward<CallbackT>(vCallback)();
			}
		}
	};

	template<typename CallbackT, typename PredictorT>
	DeferredCallback<CallbackT, PredictorT> DeferCallback(CallbackT &&vCallback, PredictorT &&vPredictor){
		return { std::forward<CallbackT>(vCallback), std::forward<PredictorT>(vPredictor) };
	}
}

template<typename CallbackT>
decltype(auto) Defer(CallbackT &&vCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(vCallback),
		[]{ return true; });
}
template<typename CallbackT>
decltype(auto) DeferOnNormalExit(CallbackT &&vCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(vCallback),
		[c = std::uncaught_exceptions()]{ return std::uncaught_exceptions() <= c; });
}
template<typename CallbackT>
decltype(auto) DeferOnException(CallbackT &&vCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(vCallback),
		[c = std::uncaught_exceptions()]{ return std::uncaught_exceptions() > c; });
}

}

#endif
