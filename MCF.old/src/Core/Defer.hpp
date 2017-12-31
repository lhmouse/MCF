// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_DEFER_HPP_
#define MCF_CORE_DEFER_HPP_

#include <utility>
#include <type_traits>
#include <exception>

namespace MCF {

namespace Impl_Defer {
	template<typename CallbackT, typename PredictorT>
	struct DeferredCallback {
		std::decay_t<CallbackT> fnCallback;
		std::decay_t<PredictorT> fnPredictor;

		~DeferredCallback() noexcept(false) {
			if(std::forward<PredictorT>(fnPredictor)()){
				std::forward<CallbackT>(fnCallback)();
			}
		}
	};

	template<typename CallbackT, typename PredictorT>
	DeferredCallback<CallbackT, PredictorT> DeferCallback(CallbackT &&fnCallback, PredictorT &&fnPredictor){
		return { std::forward<CallbackT>(fnCallback), std::forward<PredictorT>(fnPredictor) };
	}
}

template<typename CallbackT>
decltype(auto) Defer(CallbackT &&fnCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(fnCallback),
		[]{ return true; });
}
template<typename CallbackT>
decltype(auto) DeferOnNormalExit(CallbackT &&fnCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(fnCallback),
		[c = std::uncaught_exceptions()]{ return std::uncaught_exceptions() <= c; });
}
template<typename CallbackT>
decltype(auto) DeferOnException(CallbackT &&fnCallback){
	return Impl_Defer::DeferCallback(std::forward<CallbackT>(fnCallback),
		[c = std::uncaught_exceptions()]{ return std::uncaught_exceptions() > c; });
}

}

#endif
