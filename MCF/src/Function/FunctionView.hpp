// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_VIEW_HPP_
#define MCF_FUNCTION_FUNCTION_VIEW_HPP_

#include "Invoke.hpp"
#include "TupleManipulation.hpp"
#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {
/*
template<typename PrototypeT>
class FunctionView {
	static_assert((sizeof(PrototypeT *), false), "Class template FunctionView instantiated with a non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class FunctionView<RetT (ParamsT ...)> {
protected:
	using X_ReturnType      = RetT;
	using X_ForwardedParams = std::tuple<ParamsT &&...>;

protected:
	constexpr FunctionView() noexcept {
	}
};











template<typename RetT, typename ...ParamsT>
class FunctionView<RetT (ParamsT...)> {
private:
	RetT (*x_pfnLambda)(const void *, Impl_ForwardedParam::ForwardedParam<ParamsT> ...);
	const void *x_pContext;

public:
	constexpr FunctionView(std::nullptr_t = nullptr) noexcept
		: x_pfnLambda(nullptr), x_pContext(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			!std::is_base_of<FunctionView, std::decay_t<FuncT>>::value &&
				(std::is_convertible<decltype(DeclVal<FuncT>()(DeclVal<ParamsT>()...)), RetT>::value || std::is_void<RetT>::value),
			int> = 0>
	FunctionView(const FuncT &vFunc) noexcept
		: x_pfnLambda([](auto pContext, auto ...vParams){ return DesignatedInvoke<RetT>(static_cast<const FuncT *>(pContext)[0], std::forward<ParamsT>(vParams)...); }), x_pContext(AddressOf(vFunc))
	{
	}

public:
	FunctionView &Reset(std::nullptr_t = nullptr) noexcept {
		FunctionView().Swap(*this);
		return *this;
	}
	template<typename FuncT>
	FunctionView &Reset(const FuncT &vFunc){
		FunctionView(vFunc).Swap(*this);
		return *this;
	}

	void Swap(FunctionView &vOther) noexcept {
		using std::swap;
		swap(x_pfnLambda, vOther.x_pfnLambda);
		swap(x_pContext,  vOther.x_pContext);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pfnLambda;
	}
	RetT operator()(ParamsT ...vParams) const {
		MCF_DEBUG_CHECK(x_pfnLambda);

		return (*x_pfnLambda)(x_pContext, std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}

	friend void swap(FunctionView &vSelf, FunctionView &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};
*/
}

#endif
