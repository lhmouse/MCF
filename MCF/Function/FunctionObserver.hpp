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
	static_assert(((void)sizeof(FuncT), false), "Class template FunctionObserver instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class FunctionObserver<RetT (ParamsT...)> {
private:
	RetT (*$pfnLambda)(const void *, ParamsT &&...);
	const void *$pContext;

public:
	constexpr FunctionObserver(std::nullptr_t = nullptr) noexcept
		: $pfnLambda(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<const FuncT & (ForwardedParam<ParamsT>...)>, RetT>::value,
			int> = 0>
	FunctionObserver(const FuncT &vFunc) noexcept
		: $pfnLambda([](const void *pContext, ParamsT &&...vParams){ return Invoke(*static_cast<const FuncT *>(pContext), std::forward<ParamsT>(vParams)...);  })
		, $pContext(std::addressof(vFunc))
	{
	}

public:
	FunctionObserver &Reset(std::nullptr_t = nullptr) noexcept {
		FunctionObserver().Swap(*this);
		return *this;
	}
	template<typename FuncT>
	FunctionObserver &Reset(const FuncT &vFunc){
		FunctionObserver(vFunc).Swap(*this);
		return *this;
	}

	void Swap(FunctionObserver &rhs) noexcept {
		std::swap($pfnLambda, rhs.$pfnLambda);
		std::swap($pContext, rhs.$pContext);
	}

public:
	explicit operator bool() const noexcept {
		return !!$pfnLambda;
	}
	RetT operator()(ParamsT ...vParams) const {
		ASSERT($pfnLambda);

		return (*$pfnLambda)($pContext, std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}
};

}

#endif
