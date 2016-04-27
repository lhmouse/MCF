// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_BIND_HPP_
#define MCF_FUNCTION_BIND_HPP_

#include <type_traits>
#include <utility>
#include <tuple>
#include "../Utilities/Invoke.hpp"
#include "RefWrapper.hpp"

namespace MCF {

namespace Impl_Bind {
	template<typename FuncT, bool kIsLazyT, typename ...ParamsT>
	class BindResult;

	template<std::size_t kIndexT>
	struct Placeholder {
	};

	template<typename ...ParamsAddT>
	class ParamSelector {
	private:
		std::tuple<ParamsAddT &&...> x_tupParamsAdd;

	public:
		explicit ParamSelector(ParamsAddT &&...vParamsAdd) noexcept
			: x_tupParamsAdd(std::forward<ParamsAddT>(vParamsAdd)...)
		{
		}

	private:
		template<std::size_t ...kParamIndicesT, typename LazyT>
		decltype(auto) X_ForwardLazyParams(std::index_sequence<kParamIndicesT...>, const LazyT &vLazy) const {
			return vLazy(std::forward<std::tuple_element_t<kParamIndicesT, decltype(x_tupParamsAdd)>>(std::get<kParamIndicesT>(x_tupParamsAdd))...);
		}

	public:
		template<typename ParamT>
		decltype(auto) operator()(ParamT &vParam) noexcept {
			return vParam;
		}
		template<std::size_t kIndexT>
		decltype(auto) operator()(const Placeholder<kIndexT> &) noexcept {
			return std::forward<std::tuple_element_t<kIndexT - 1, decltype(x_tupParamsAdd)>>(std::get<kIndexT - 1>(x_tupParamsAdd));
		}
		template<typename ParamT>
		decltype(auto) operator()(const RefWrapper<ParamT> &vParam) noexcept {
			return vParam.Get();
		}
		template<typename FuncT, typename ...ParamsT>
		decltype(auto) operator()(const BindResult<FuncT, true, ParamsT...> &vLazy) noexcept {
			return X_ForwardLazyParams(std::index_sequence_for<ParamsAddT...>(), vLazy);
		}
	};

	template<typename FuncT, bool kIsLazyT, typename ...ParamsT>
	class BindResult {
	private:
		FuncT x_vFunc;
		std::tuple<ParamsT...> x_tupParams;

	public:
		BindResult(FuncT vFunc, ParamsT ...vParams)
			: x_vFunc(std::move(vFunc)), x_tupParams(std::move(vParams)...)
		{
		}

	private:
		template<std::size_t ...kParamIndicesT, typename ...ParamsAddT>
		decltype(auto) X_DispatchParams(std::index_sequence<kParamIndicesT...>, ParamsAddT &&...vParamsAdd) const {
			return Invoke(x_vFunc, ParamSelector<ParamsAddT...>(std::forward<ParamsAddT>(vParamsAdd)...)(std::get<kParamIndicesT>(x_tupParams))...);
		}

	public:
		template<typename ...ParamsAddT>
		decltype(auto) operator()(ParamsAddT &&...vParamsAdd) const {
			return X_DispatchParams(std::index_sequence_for<ParamsT...>(), std::forward<ParamsAddT>(vParamsAdd)...);
		}
	};
}

constexpr Impl_Bind::Placeholder< 1>  _1;
constexpr Impl_Bind::Placeholder< 2>  _2;
constexpr Impl_Bind::Placeholder< 3>  _3;
constexpr Impl_Bind::Placeholder< 4>  _4;
constexpr Impl_Bind::Placeholder< 5>  _5;
constexpr Impl_Bind::Placeholder< 6>  _6;
constexpr Impl_Bind::Placeholder< 7>  _7;
constexpr Impl_Bind::Placeholder< 8>  _8;
constexpr Impl_Bind::Placeholder< 9>  _9;
constexpr Impl_Bind::Placeholder<10> _10;

constexpr Impl_Bind::Placeholder<11> _11;
constexpr Impl_Bind::Placeholder<12> _12;
constexpr Impl_Bind::Placeholder<13> _13;
constexpr Impl_Bind::Placeholder<14> _14;
constexpr Impl_Bind::Placeholder<15> _15;
constexpr Impl_Bind::Placeholder<16> _16;
constexpr Impl_Bind::Placeholder<17> _17;
constexpr Impl_Bind::Placeholder<18> _18;
constexpr Impl_Bind::Placeholder<19> _19;
constexpr Impl_Bind::Placeholder<20> _20;

constexpr Impl_Bind::Placeholder<21> _21;
constexpr Impl_Bind::Placeholder<22> _22;
constexpr Impl_Bind::Placeholder<23> _23;
constexpr Impl_Bind::Placeholder<24> _24;
constexpr Impl_Bind::Placeholder<25> _25;
constexpr Impl_Bind::Placeholder<26> _26;
constexpr Impl_Bind::Placeholder<27> _27;
constexpr Impl_Bind::Placeholder<28> _28;
constexpr Impl_Bind::Placeholder<29> _29;
constexpr Impl_Bind::Placeholder<30> _30;

constexpr Impl_Bind::Placeholder<31> _31;
constexpr Impl_Bind::Placeholder<32> _32;
constexpr Impl_Bind::Placeholder<33> _33;
constexpr Impl_Bind::Placeholder<34> _34;
constexpr Impl_Bind::Placeholder<35> _35;
constexpr Impl_Bind::Placeholder<36> _36;
constexpr Impl_Bind::Placeholder<37> _37;
constexpr Impl_Bind::Placeholder<38> _38;
constexpr Impl_Bind::Placeholder<39> _39;
constexpr Impl_Bind::Placeholder<40> _40;

constexpr Impl_Bind::Placeholder<41> _41;
constexpr Impl_Bind::Placeholder<42> _42;
constexpr Impl_Bind::Placeholder<43> _43;
constexpr Impl_Bind::Placeholder<44> _44;
constexpr Impl_Bind::Placeholder<45> _45;
constexpr Impl_Bind::Placeholder<46> _46;
constexpr Impl_Bind::Placeholder<47> _47;
constexpr Impl_Bind::Placeholder<48> _48;
constexpr Impl_Bind::Placeholder<49> _49;
constexpr Impl_Bind::Placeholder<50> _50;

template<typename FuncT, typename ...ParamsT>
Impl_Bind::BindResult<std::decay_t<FuncT>, false, std::decay_t<ParamsT>...> Bind(FuncT &&vFunc, ParamsT &&...vParams){
	return { std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)... };
}
template<typename FuncT, typename ...ParamsT>
Impl_Bind::BindResult<std::decay_t<FuncT>, true, std::decay_t<ParamsT>...> LazyBind(FuncT &&vFunc, ParamsT &&...vParams){
	return { std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)... };
}

}

#endif
