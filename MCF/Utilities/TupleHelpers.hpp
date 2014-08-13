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
	template<std::size_t CUR, std::size_t END>
	struct CallOnTupleHelper {
		template<typename Function, typename... TupleParams, typename... Unpacked>
		static decltype(auto) Do(
			Function &&vFunction, const std::tuple<TupleParams...> &vTuple,
			const Unpacked &... vUnpacked
		){
			return CallOnTupleHelper<CUR + 1, END>::Do(
				vFunction, vTuple,
				vUnpacked..., std::get<CUR>(vTuple)
			);
		}
		template<typename Function, typename... TupleParams, typename... Unpacked>
		static decltype(auto) Do(
			Function &&vFunction, std::tuple<TupleParams...> &&vTuple,
			Unpacked &&... vUnpacked
		){
			return CallOnTupleHelper<CUR + 1, END>::Do(
				vFunction, std::move(vTuple),
				std::move(vUnpacked)..., std::move(std::get<CUR>(vTuple))
			);
		}
	};
	template<std::size_t END>
	struct CallOnTupleHelper<END, END> {
		template<typename Function, typename... TupleParams, typename... Unpacked>
		static decltype(auto) Do(
			Function &&vFunction, const std::tuple<TupleParams...> &,
			const Unpacked &... vUnpacked
		){
			return vFunction(vUnpacked...);
		}
		template<typename Function, typename... TupleParams, typename... Unpacked>
		static decltype(auto) Do(
			Function &&vFunction, std::tuple<TupleParams...> &&,
			Unpacked &&... vUnpacked
		){
			return vFunction(std::move(vUnpacked)...);
		}
	};
}

template<typename Function, typename... Params>
decltype(auto) CallOnTuple(Function &&vFunction, const std::tuple<Params...> &vTuple)
	noexcept(noexcept(
		std::declval<Function>()(std::declval<const Params &>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		std::forward<Function>(vFunction), vTuple
	);
}
template<typename Function, typename... Params>
decltype(auto) CallOnTuple(Function &&vFunction, std::tuple<Params...> &&vTuple)
	noexcept(noexcept(
		std::declval<Function>()(std::declval<Params &&>()...)
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		std::forward<Function>(vFunction), std::move(vTuple)
	);
}

template<class Object, typename... Params>
auto MakeFromTuple(const std::tuple<Params...> &vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object, const Params &...>::value
		&& std::is_nothrow_move_constructible<Object>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](const Params &... vParams){
			return Object(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeFromTuple(std::tuple<Params...> &&vTuple)
	noexcept(noexcept(
		std::is_nothrow_constructible<Object, Params &&...>::value
		&& std::is_nothrow_move_constructible<Object>::value
	))
{
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](Params &&... vParams){
			return Object(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object, typename... Params>
auto MakeUniqueFromTuple(const std::tuple<Params...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](const Params &... vParams){
			return std::make_unique<Object>(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeUniqueFromTuple(std::tuple<Params...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](Params &&... vParams){
			return std::make_unique<Object>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

template<class Object, typename... Params>
auto MakeSharedFromTuple(const std::tuple<Params...> &vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](const Params &... vParams){
			return std::make_shared<Object>(vParams...);
		},
		vTuple
	);
}
template<class Object, typename... Params>
auto MakeSharedFromTuple(std::tuple<Params...> &&vTuple){
	return Impl::CallOnTupleHelper<0u, sizeof...(Params)>::Do(
		[](Params &&... vParams){
			return std::make_shared<Object>(std::move(vParams)...);
		},
		std::move(vTuple)
	);
}

}

#endif
