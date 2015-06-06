// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_HPP_
#define MCF_FUNCTION_FUNCTION_HPP_

#include <type_traits>
#include <utility>
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Utilities/Assert.hpp"

namespace MCF {

template<typename FuncT>
class Function {
	static_assert((std::declval<FuncT &>(), false), "Class template Function instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class Function<RetT (ParamsT...)> {
private:
	template<typename T>
		using xForwardedNonScalar = std::conditional_t<std::is_scalar<T>::value, std::decay_t<T>, std::decay_t<T> &&>;

	struct xCallableBase : IntrusiveBase<xCallableBase> {
		virtual ~xCallableBase() = default;

		virtual RetT Invoke(xForwardedNonScalar<ParamsT>...vParams) const = 0;
	};

private:
	IntrusivePtr<const xCallableBase> x_pCallable;

public:
	constexpr Function() = default;

	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (xForwardedNonScalar<ParamsT>...)>, RetT>::value,
			int> = 0>
	explicit Function(FuncT &&vFunc){
		Reset(std::forward<FuncT>(vFunc));
	}

public:
	void Reset() noexcept {
		x_pCallable.Reset();
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (xForwardedNonScalar<ParamsT>...)>, RetT>::value,
			int> = 0>
	void Reset(FuncT &&vFunc){
		struct Callable : xCallableBase {
			const std::remove_reference_t<FuncT> vCallableFunc;

			explicit Callable(FuncT &&vFunc)
				: vCallableFunc(std::forward<FuncT>(vFunc))
			{
			}

			RetT Invoke(xForwardedNonScalar<ParamsT>...vParams) const override {
				return vCallableFunc(std::forward<ParamsT>(vParams)...);
			}
		};

		x_pCallable.Reset(new Callable(std::forward<FuncT>(vFunc)));
	}

	void Swap(Function &rhs) noexcept {
		x_pCallable.Swap(rhs.x_pCallable);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pCallable;
	}
	RetT operator()(ParamsT ...vParams) const {
		ASSERT(x_pCallable);

		return x_pCallable->Invoke(std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}
};

template<typename RetT, typename ...ParamsT>
void swap(Function<RetT (ParamsT...)> &lhs, Function<RetT (ParamsT...)> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
