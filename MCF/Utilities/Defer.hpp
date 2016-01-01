// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_DEFER_HPP_
#define MCF_UTILITIES_DEFER_HPP_

#include <utility>
#include <type_traits>

namespace MCF {

namespace Impl_Defer {
	template<typename CallbackT>
	class DeferredCallback {
	private:
		CallbackT x_vCallback;

	public:
		explicit DeferredCallback(CallbackT vCallback)
			: x_vCallback(std::move(vCallback))
		{
		}
		~DeferredCallback() noexcept(false) {
			x_vCallback();
		}
	};

	template<typename CallbackT>
	auto CreateDeferredCallback(CallbackT &&vCallback){
		return DeferredCallback<std::decay_t<CallbackT>>(std::forward<CallbackT>(vCallback));
	}
}

}

#define DEFER_UNIQUE_ID_2_(cnt_)    MCF_DeferredCallback_ ## cnt_ ## X_
#define DEFER_UNIQUE_ID_(cnt_)      DEFER_UNIQUE_ID_2_(cnt_)

#define DEFER(func_)                const auto DEFER_UNIQUE_ID_(__COUNTER__) = ::MCF::Impl_Defer::CreateDeferredCallback(func_)

#endif
