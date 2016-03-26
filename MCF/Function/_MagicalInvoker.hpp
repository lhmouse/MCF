// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_MAGICAL_INVOKER_HPP_
#define MCF_FUNCTION_MAGICAL_INVOKER_HPP_

#include <type_traits>
#include <utility>
#include "../Utilities/Invoke.hpp"

namespace MCF {

namespace Impl_MagicalInvoker {
	template<bool kReturnsVoidT>
	struct MagicalInvokerImpl {
		template<typename FuncT, typename ...ParamsT>
		decltype(auto) operator()(FuncT &&vFunc, ParamsT &&...vParams){
			return Invoke(std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct MagicalInvokerImpl<true> {
		template<typename FuncT, typename ...ParamsT>
		void operator()(FuncT &&vFunc, ParamsT &&...vParams){
			Invoke(std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)...);
		}
	};

	template<typename RetT>
	using MagicalInvoker = MagicalInvokerImpl<std::is_void<RetT>::value>;
}

}

#endif
