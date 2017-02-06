// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_BIND_HPP_
#define MCF_FUNCTION_BIND_HPP_

#include "../Core/RefWrapper.hpp"
#include "Invoke.hpp"
#include "TupleManipulation.hpp"
#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {

namespace Impl_Bind {
	template<bool kIsLazyT, typename FunctionT, typename ...ParamsT>
	class BindResult;

	template<std::size_t kIndexT>
	struct Placeholder {
	};

	template<typename LazyParamTupleT, typename ParamT>
	decltype(auto) SelectParam(LazyParamTupleT &&, ParamT &&vParam){
		return std::forward<ParamT>(vParam);
	}
	template<typename LazyParamTupleT, typename ParamT>
	decltype(auto) SelectParam(LazyParamTupleT &&, const RefWrapper<ParamT> &vRef){
		return vRef.Get();
	}
	template<typename LazyParamTupleT, std::size_t kPlaceholderIndexT>
	decltype(auto) SelectParam(LazyParamTupleT &&tupLazyParams, const Placeholder<kPlaceholderIndexT> &){
		return std::get<kPlaceholderIndexT - 1>(std::forward<LazyParamTupleT>(tupLazyParams));
	}
	template<typename LazyParamTupleT, typename FunctionT, typename ...ParamsT>
	decltype(auto) SelectParam(LazyParamTupleT &&tupLazyParams, const BindResult<true, FunctionT, ParamsT...> &vBindResult){
		return Squeeze<>(vBindResult, std::forward<LazyParamTupleT>(tupLazyParams));
	}

	template<typename FunctionT, typename ParamTupleT, std::size_t ...kIndicesT, typename LazyParamTupleT>
	decltype(auto) SelectParamAndInvoke(const FunctionT &vFunction, const ParamTupleT &tupParams, const std::index_sequence<kIndicesT...> &, LazyParamTupleT &&tupLazyParams){
		return Invoke(vFunction, SelectParam<>(std::forward<LazyParamTupleT>(tupLazyParams), std::get<kIndicesT>(tupParams))...);
	}

	template<bool kIsLazyT, typename FunctionT, typename ...ParamsT>
	struct BindResult {
		std::decay_t<FunctionT> vFunction;
		std::tuple<std::decay_t<ParamsT>...> tupParams;

		template<typename ...LazyParamsT>
		decltype(auto) operator()(LazyParamsT &&...vLazyParams) const {
			return SelectParamAndInvoke<>(vFunction, tupParams, std::make_index_sequence<sizeof...(ParamsT)>(), std::forward_as_tuple(std::forward<LazyParamsT>(vLazyParams)...));
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

template<typename FunctionT, typename ...ParamsT>
Impl_Bind::BindResult<false, FunctionT, ParamsT...> Bind(FunctionT &&vFunction, ParamsT &&...vParams){
	return { std::forward<FunctionT>(vFunction), std::forward_as_tuple(std::forward<ParamsT>(vParams)...) };
}
template<typename FunctionT, typename ...ParamsT>
Impl_Bind::BindResult<true, FunctionT, ParamsT...> LazyBind(FunctionT &&vFunction, ParamsT &&...vParams){
	return { std::forward<FunctionT>(vFunction), std::forward_as_tuple(std::forward<ParamsT>(vParams)...) };
}

}

#endif
