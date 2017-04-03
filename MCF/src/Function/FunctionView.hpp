// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_VIEW_HPP_
#define MCF_FUNCTION_FUNCTION_VIEW_HPP_

#include "../Core/AddressOf.hpp"
#include "TupleManipulation.hpp"
#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {

template<typename ObjectT, class DeleterT>
class UniquePtr;

template<typename ObjectT, class DeleterT>
class IntrusivePtr;

template<typename PrototypeT>
class FunctionView {
	static_assert((sizeof(PrototypeT *), false), "Class template FunctionView instantiated with a non-function template type parameter.");
};

template<typename ReturnT, typename ...ParamsT>
class FunctionView<ReturnT (ParamsT ...)> {
protected:
	using X_ReturnType      = ReturnT;
	using X_ForwardedParams = std::tuple<ParamsT &&...>;

protected:
	X_ReturnType (*x_pfnSpringboard)(const void *, X_ForwardedParams &&) = nullptr;
	const void *x_pSource = nullptr;

public:
	constexpr FunctionView() noexcept {
	}
	template<typename FunctionT>
	FunctionView(const FunctionT *pFunction) noexcept
		: x_pfnSpringboard(pFunction ? +[](const void *pSource, X_ForwardedParams &&tupParams) -> X_ReturnType { return DesignatedSqueeze<X_ReturnType>(*static_cast<const FunctionT *>(pSource), std::move(tupParams)); }
		                             : nullptr)
		, x_pSource(pFunction)
	{
	}
	template<typename FunctionT>
	FunctionView(const UniquePtr<FunctionT> &pFunction) noexcept
		: FunctionView(pFunction.Get())
	{
	}
	template<typename FunctionT>
	FunctionView(const IntrusivePtr<FunctionT> &pFunction) noexcept
		: FunctionView(pFunction.Get())
	{
	}
	template<typename FunctionT,
		std::enable_if_t<
			!std::is_base_of<FunctionView, std::decay_t<FunctionT>>::value &&
				(std::is_convertible<decltype(std::declval<FunctionT>()(std::declval<ParamsT>()...)), ReturnT>::value || std::is_void<ReturnT>::value),
			int> = 0>
	FunctionView(FunctionT &vFunction) noexcept
		: FunctionView(AddressOf(vFunction))
	{
	}

public:
	FunctionView &Reset(std::nullptr_t = nullptr) noexcept {
		FunctionView().Swap(*this);
		return *this;
	}
	template<typename FunctionT>
	FunctionView &Reset(FunctionT &vFunction) noexcept {
		FunctionView(vFunction).Swap(*this);
		return *this;
	}

	void Swap(FunctionView &vOther) noexcept {
		using std::swap;
		swap(x_pfnSpringboard, vOther.x_pfnSpringboard);
		swap(x_pSource,        vOther.x_pSource);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pfnSpringboard;
	}
	ReturnT operator()(ParamsT ...vParams) const {
		MCF_DEBUG_CHECK(x_pfnSpringboard);
		return Invoke(*x_pfnSpringboard, x_pSource, std::forward_as_tuple(std::forward<ParamsT>(vParams)...));
	}

	friend void swap(FunctionView &vSelf, FunctionView &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};

}

#endif
