// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_INVOKE_HPP_
#define MCF_CORE_INVOKE_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Invoke {
	template<bool>
	struct ReflexivityChecker {
		template<typename FunctionT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
			return ((*std::forward<ObjectT>(vObject)).*pFunction)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct ReflexivityChecker<true> {
		template<typename FunctionT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
			return (std::forward<ObjectT>(vObject).*pFunction)(std::forward<ParamsT>(vParams)...);
		}
	};

	template<bool>
	struct MemberFunctionPointerChecker {
		template<typename FunctionT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FunctionT &vFunction, ParamsT &...vParams){
			return std::forward<FunctionT>(vFunction)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct MemberFunctionPointerChecker<true> {
		template<typename C, typename M>
		static C GetClassFromPointerToMember(M C::*) noexcept;

		template<typename FunctionT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FunctionT pFunction, ObjectT &vObject, ParamsT &...vParams){
			using Next = ReflexivityChecker<std::is_base_of<decltype(GetClassFromPointerToMember(pFunction)), std::decay_t<ObjectT>>::value>;
			return Next::template DoIt<FunctionT, ObjectT, ParamsT...>(pFunction, vObject, vParams...);
		}
	};
}

template<typename FunctionT, typename ...ParamsT>
constexpr decltype(auto) Invoke(FunctionT &&vFunction, ParamsT &&...vParams){
	using Next = Impl_Invoke::MemberFunctionPointerChecker<std::is_member_pointer<std::decay_t<FunctionT>>::value>;
	return Next::template DoIt<FunctionT, ParamsT...>(vFunction, vParams...);
}

namespace Impl_Invoke {
	template<typename FunctionT, typename ...ParamsT>
	constexpr decltype(auto) RealDesignatedInvoke(const std::false_type &, FunctionT &vFunction, ParamsT &...vParams){
		return Invoke(std::forward<FunctionT>(vFunction), std::forward<ParamsT>(vParams)...);
	}
	template<typename FunctionT, typename ...ParamsT>
	constexpr void RealDesignatedInvoke(const std::true_type &, FunctionT &vFunction, ParamsT &...vParams){
		Invoke(std::forward<FunctionT>(vFunction), std::forward<ParamsT>(vParams)...);
	}
}

template<typename ReturnT, typename FunctionT, typename ...ParamsT>
constexpr decltype(auto) DesignatedInvoke(FunctionT &&vFunction, ParamsT &&...vParams){
	return Impl_Invoke::RealDesignatedInvoke<FunctionT, ParamsT...>(std::is_void<ReturnT>(), vFunction, vParams...);
}

}

#endif
