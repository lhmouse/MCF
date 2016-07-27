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
		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FuncT pFunc, ObjectT &vObject, ParamsT &...vParams){
			return ((*std::forward<ObjectT>(vObject)).*pFunc)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct ReflexivityChecker<true> {
		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FuncT pFunc, ObjectT &vObject, ParamsT &...vParams){
			return (std::forward<ObjectT>(vObject).*pFunc)(std::forward<ParamsT>(vParams)...);
		}
	};

	template<bool>
	struct MemberFunctionPointerChecker {
		template<typename FuncT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FuncT &vFunc, ParamsT &...vParams){
			return std::forward<FuncT>(vFunc)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct MemberFunctionPointerChecker<true> {
		template<typename C, typename M>
		static constexpr C GetClassFromPointerToMember(M C::*) noexcept;

		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		static constexpr decltype(auto) DoIt(FuncT pFunc, ObjectT &vObject, ParamsT &...vParams){
			using Next = ReflexivityChecker<std::is_base_of<decltype(GetClassFromPointerToMember(pFunc)), std::decay_t<ObjectT>>::value>;
			return Next::template DoIt<FuncT, ObjectT, ParamsT...>(pFunc, vObject, vParams...);
		}
	};
}

template<typename FuncT, typename ...ParamsT>
constexpr decltype(auto) Invoke(FuncT &&vFunc, ParamsT &&...vParams){
	using Next = Impl_Invoke::MemberFunctionPointerChecker<std::is_member_pointer<std::decay_t<FuncT>>::value>;
	return Next::template DoIt<FuncT, ParamsT...>(vFunc, vParams...);
}

}

#endif
