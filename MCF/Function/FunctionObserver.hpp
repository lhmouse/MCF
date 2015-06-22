// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_OBSERVER_HPP_
#define MCF_FUNCTION_FUNCTION_OBSERVER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/Invoke.hpp"
#include "_ForwardedParam.hpp"
#include <type_traits>
#include <utility>
#include <memory>

namespace MCF {

template<typename FuncT>
class FunctionObserver {
	static_assert((sizeof(FuncT), false), "Class template FunctionObserver instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class FunctionObserver<RetT (ParamsT...)> {
private:
	RetT (*x_pfnLambda)(const void *, ParamsT &&...);
	const void *x_pContext;

public:
	FunctionObserver(const FunctionObserver &) noexcept = default;
	FunctionObserver(FunctionObserver &&) noexcept = default;
	FunctionObserver &operator=(const FunctionObserver &) noexcept = default;
	FunctionObserver &operator=(FunctionObserver &&) noexcept = default;

	constexpr FunctionObserver(std::nullptr_t = nullptr) noexcept
		: x_pfnLambda(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (ForwardedParam<ParamsT>...)>, RetT>::value,
			int> = 0>
	FunctionObserver(const FuncT &vFunc) noexcept
		: FunctionObserver()
	{
		Reset(vFunc);
	}

public:
	FunctionObserver &Reset(std::nullptr_t = nullptr) noexcept {
		x_pfnLambda = nullptr;
		return *this;
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (ForwardedParam<ParamsT>...)>, RetT>::value,
			int> = 0>
	FunctionObserver &Reset(const FuncT &vFunc){
		x_pfnLambda = [](const void *pContext, ParamsT &&...vParams){ return Invoke(*static_cast<const FuncT *>(pContext), std::forward<ParamsT>(vParams)...); };
		x_pContext = std::addressof(vFunc);
		return *this;
	}

	void Swap(FunctionObserver &rhs) noexcept {
		std::swap(x_pfnLambda, rhs.x_pfnLambda);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pfnLambda;
	}
	RetT operator()(ParamsT ...vParams) const {
		ASSERT(x_pfnLambda);

		return (*x_pfnLambda)(x_pContext, std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}
};

}

#endif
