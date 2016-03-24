// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_VIEW_HPP_
#define MCF_FUNCTION_FUNCTION_VIEW_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/AddressOf.hpp"
#include "../Utilities/Invoke.hpp"
#include "../Utilities/DeclVal.hpp"
#include "_ForwardedParam.hpp"
#include <type_traits>
#include <utility>

namespace MCF {

template<typename FuncT>
class FunctionView {
	static_assert(((void)sizeof(FuncT), false), "Class template FunctionView instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class FunctionView<RetT (ParamsT...)> {
private:
	RetT (*x_pfnLambda)(const void *, Impl_ForwardedParam::ForwardedParam<ParamsT> ...);
	const void *x_pContext;

public:
	constexpr FunctionView(std::nullptr_t = nullptr) noexcept
		: x_pfnLambda(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			!std::is_base_of<FunctionView, std::decay_t<FuncT>>::value &&
				std::is_convertible<decltype(DeclVal<FuncT>()(DeclVal<ParamsT>()...)), RetT>::value || std::is_void<RetT>::value,
			int> = 0>
	FunctionView(const FuncT &vFunc) noexcept
		: x_pfnLambda([](const void *pContext, Impl_ForwardedParam::ForwardedParam<ParamsT> ...vParams){
			return Invoke(*static_cast<const FuncT *>(pContext), std::forward<ParamsT>(vParams)...);
		})
		, x_pContext(AddressOf(vFunc))
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

	void Swap(FunctionView &rhs) noexcept {
		using std::swap;
		swap(x_pfnLambda, rhs.x_pfnLambda);
		swap(x_pContext,  rhs.x_pContext);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pfnLambda;
	}
	RetT operator()(ParamsT ...vParams) const {
		MCF_ASSERT(x_pfnLambda);

		return (*x_pfnLambda)(x_pContext, std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}

	friend void swap(FunctionView &lhs, FunctionView &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
