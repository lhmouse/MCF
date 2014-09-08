// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_TUPLE_HELPERS_HPP_
#define MCF_TUPLE_HELPERS_HPP_

#include <type_traits>
#include <tuple>
#include <utility>
#include <memory>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<std::size_t CUR_T, std::size_t END_T>
	struct CallOnTupleHelper {
		template<typename FunctionT, typename ...TupleParamsT, typename ...UnpackedT>
		static decltype(auto) Do(
			FunctionT &&vFunction, const std::tuple<TupleParamsT...> &vTuple,
			const UnpackedT &...vUnpacked
		){
			return CallOnTupleHelper<CUR_T + 1, END_T>::Do(
				vFunction, vTuple,
				vUnpacked..., std::get<CUR_T>(vTuple)
			);
		}
		template<typename FunctionT, typename ...TupleParamsT, typename ...UnpackedT>
		static decltype(auto) Do(
			FunctionT &&vFunction, std::tuple<TupleParamsT...> &&vTuple,
			UnpackedT &&...vUnpacked
		){
			return CallOnTupleHelper<CUR_T + 1, END_T>::Do(
				vFunction, std::move(vTuple),
				std::move(vUnpacked)..., std::move(std::get<CUR_T>(vTuple))
			);
		}
	};
	template<std::size_t END_T>
	struct CallOnTupleHelper<END_T, END_T> {
		template<typename FunctionT, typename ...TupleParamsT, typename ...UnpackedT>
		static decltype(auto) Do(
			FunctionT &&vFunction, const std::tuple<TupleParamsT...> &,
			const UnpackedT &...vUnpacked
		){
			return vFunction(vUnpacked...);
		}
		template<typename FunctionT, typename ...TupleParamsT, typename ...UnpackedT>
		static decltype(auto) Do(
			FunctionT &&vFunction, std::tuple<TupleParamsT...> &&,
			UnpackedT &&...vUnpacked
		){
			return vFunction(std::move(vUnpacked)...);
		}
	};
}

template<typename FunctionT, typename ...ParamsT>
decltype(auto) CallOnTuple(FunctionT &&vFunction, const std::tuple<ParamsT...> &vTuple)
	noexcept(noexcept(
		std::declval<FunctionT>()(std::declval<const ParamsT &>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		std::forward<FunctionT>(vFunction), vTuple
	);
}
template<typename FunctionT, typename ...ParamsT>
decltype(auto) CallOnTuple(FunctionT &&vFunction, std::tuple<ParamsT...> &&vTuple)
	noexcept(noexcept(
		std::declval<FunctionT>()(std::declval<ParamsT &&>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		std::forward<FunctionT>(vFunction), std::move(vTuple)
	);
}

template<class ObjectT, typename ...ParamsT>
auto MakeFromTuple(const std::tuple<ParamsT...> &vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<ObjectT, const ParamsT &...>::value
		&& std::is_nothrow_move_constructible<ObjectT>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](const ParamsT &...vParams){
			return ObjectT(vParams...);
		},
		vTuple
	);
}
template<class ObjectT, typename ...ParamsT>
auto MakeFromTuple(std::tuple<ParamsT...> &&vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value
		&& std::is_nothrow_move_constructible<ObjectT>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](ParamsT &&...vParams){
			return ObjectT(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class ObjectT, typename ...ParamsT>
auto MakeUniqueFromTuple(const std::tuple<ParamsT...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](const ParamsT &...vParams){
			return std::make_unique<ObjectT>(vParams...);
		},
		vTuple
	);
}
template<class ObjectT, typename ...ParamsT>
auto MakeUniqueFromTuple(std::tuple<ParamsT...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](ParamsT &&...vParams){
			return std::make_unique<ObjectT>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class ObjectT, typename ...ParamsT>
auto MakeSharedFromTuple(const std::tuple<ParamsT...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](const ParamsT &...vParams){
			return std::make_shared<ObjectT>(vParams...);
		},
		vTuple
	);
}
template<class ObjectT, typename ...ParamsT>
auto MakeSharedFromTuple(std::tuple<ParamsT...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(ParamsT)>::Do(
		[](ParamsT &&...vParams){
			return std::make_shared<ObjectT>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

}

#endif
