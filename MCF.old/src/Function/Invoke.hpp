// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_INVOKE_HPP_
#define MCF_FUNCTION_INVOKE_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Invoke {
	template<typename C, typename M>
	C &&GetClassFromPointerToMember(M C::*) noexcept;

	template<typename FunctionT, typename ObjectT, typename ...ParamsT>
	constexpr decltype(auto) CheckReflexivity(const std::false_type &, FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
		return ((*std::forward<ObjectT>(vObject)).*pFunction)(std::forward<ParamsT>(vParams)...);
	}
	template<typename FunctionT, typename ObjectT, typename ...ParamsT>
	constexpr decltype(auto) CheckReflexivity(const std::true_type &, FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
		return (std::forward<ObjectT>(vObject).*pFunction)(std::forward<ParamsT>(vParams)...);
	}

	template<typename FunctionT, typename ...ParamsT>
	constexpr decltype(auto) CheckMemberPointer(const std::false_type &, FunctionT &vFunction, ParamsT &...vParams){
		return std::forward<FunctionT>(vFunction)(std::forward<ParamsT>(vParams)...);
	}
	template<typename FunctionT, typename ObjectT, typename ...ParamsT>
	constexpr decltype(auto) CheckMemberPointer(const std::true_type &, FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
		return CheckReflexivity<FunctionT, ObjectT, ParamsT...>(std::is_base_of<std::decay_t<decltype(GetClassFromPointerToMember<>(pFunction))>, std::decay_t<ObjectT>>(), pFunction, vObject, vParams...);
	}

	template<typename FunctionT, typename ...ParamsT>
	constexpr decltype(auto) RealInvoke(FunctionT &vFunction, ParamsT &...vParams){
		return CheckMemberPointer<FunctionT, ParamsT...>(std::is_member_pointer<std::decay_t<FunctionT>>(), vFunction, vParams...);
	}
}

template<typename FunctionT, typename ...ParamsT>
constexpr decltype(auto) Invoke(FunctionT &&vFunction, ParamsT &&...vParams){
	return Impl_Invoke::RealInvoke<FunctionT, ParamsT...>(vFunction, vParams...);
}

namespace Impl_Invoke {
	template<typename FunctionT, typename ...ParamsT>
	constexpr decltype(auto) CheckVoidReturn(const std::false_type &, FunctionT &vFunction, ParamsT &...vParams){
		return RealInvoke<FunctionT, ParamsT...>(vFunction, vParams...);
	}
	template<typename FunctionT, typename ...ParamsT>
	constexpr void CheckVoidReturn(const std::true_type &, FunctionT &vFunction, ParamsT &...vParams){
		RealInvoke<FunctionT, ParamsT...>(vFunction, vParams...);
	}

	template<typename ReturnT, typename FunctionT, typename ...ParamsT>
	constexpr decltype(auto) RealDesignatedInvoke(FunctionT &vFunction, ParamsT &...vParams){
		return CheckVoidReturn<FunctionT, ParamsT...>(std::is_void<ReturnT>(), vFunction, vParams...);
	}
}

template<typename ReturnT, typename FunctionT, typename ...ParamsT>
constexpr ReturnT DesignatedInvoke(FunctionT &&vFunction, ParamsT &&...vParams){
	return Impl_Invoke::RealDesignatedInvoke<ReturnT, FunctionT, ParamsT...>(vFunction, vParams...);
}

}

#endif
