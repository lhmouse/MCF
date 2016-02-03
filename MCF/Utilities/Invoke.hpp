// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_INVOKE_HPP_
#define MCF_UTILITIES_INVOKE_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Invoke {
	template<typename T> struct ClassGetter;

	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const volatile   > { using Type = const volatile C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const volatile & > { using Type = const volatile C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const volatile &&> { using Type = const volatile C; };

	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const            > { using Type = const          C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const          & > { using Type = const          C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...) const          &&> { using Type = const          C; };

	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)       volatile   > { using Type =       volatile C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)       volatile & > { using Type =       volatile C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)       volatile &&> { using Type =       volatile C; };

	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)                  > { using Type =                C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)                & > { using Type =                C; };
	template<typename C, typename R, typename ...ParamsT> struct ClassGetter<R (C::*)(ParamsT...)                &&> { using Type =                C; };

	template<bool>
	struct ReflexivityChecker {
		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		decltype(auto) operator()(FuncT pFunc, ObjectT &&vObject, ParamsT &&...vParams) const {
			return ((*std::forward<ObjectT>(vObject)).*pFunc)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct ReflexivityChecker<true> {
		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		decltype(auto) operator()(FuncT pFunc, ObjectT &&vObject, ParamsT &&...vParams) const {
			return (std::forward<ObjectT>(vObject).*pFunc)(std::forward<ParamsT>(vParams)...);
		}
	};

	template<bool>
	struct MemberFunctionPointerChecker {
		template<typename FuncT, typename ...ParamsT>
		decltype(auto) operator()(FuncT &&vFunc, ParamsT &&...vParams) const {
			return std::forward<FuncT>(vFunc)(std::forward<ParamsT>(vParams)...);
		}
	};
	template<>
	struct MemberFunctionPointerChecker<true> {
		template<typename FuncT, typename ObjectT, typename ...ParamsT>
		decltype(auto) operator()(FuncT pFunc, ObjectT &&vObject, ParamsT &&...vParams) const {
			return ReflexivityChecker<std::is_base_of<typename ClassGetter<FuncT>::Type, std::decay_t<ObjectT>>::value>()(pFunc, std::forward<ObjectT>(vObject), std::forward<ParamsT>(vParams)...);
		}
	};
}

template<typename FuncT, typename ...ParamsT>
decltype(auto) Invoke(FuncT &&vFunc, ParamsT &&...vParams){
	return Impl_Invoke::MemberFunctionPointerChecker<std::is_member_function_pointer<std::decay_t<FuncT>>::value>()(std::forward<FuncT>(vFunc), std::forward<ParamsT>(vParams)...);
}

}

#endif
