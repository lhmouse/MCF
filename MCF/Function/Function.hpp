// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_HPP_
#define MCF_FUNCTION_FUNCTION_HPP_

#include <type_traits>
#include <utility>
#include <functional>
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/Invoke.hpp"

namespace MCF {

namespace Impl {
	template<typename T>
		using ForwardedNonScalar = std::conditional_t<std::is_scalar<T>::value, std::decay_t<T>, T &&>;

	template<typename RetT, typename ...ParamsT>
	class FunctorBase : public IntrusiveBase<FunctorBase<RetT, ParamsT...>> {
	public:
		virtual ~FunctorBase() = default;

	public:
		virtual RetT Dispatch(ForwardedNonScalar<ParamsT>...vParams) const = 0;
	};

	template<typename FuncT, typename RetT, typename ...ParamsT>
	class Functor : public FunctorBase<RetT, ParamsT...> {
	private:
		const std::remove_reference_t<FuncT> x_vFunc;

	public:
		explicit Functor(FuncT &&vFunc)
			: x_vFunc(std::forward<FuncT>(vFunc))
		{
		}

	public:
		RetT Dispatch(ForwardedNonScalar<ParamsT>...vParams) const override {
			return Invoke(x_vFunc, std::forward<ParamsT>(vParams)...);
		}
	};
}

template<typename FuncT>
class Function {
	static_assert((std::declval<FuncT &>(), false), "Class template Function instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class Function<RetT (ParamsT...)> {
private:
	IntrusivePtr<const Impl::FunctorBase<RetT, ParamsT...>> x_pFunctor;

public:
	constexpr Function() = default;

	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (Impl::ForwardedNonScalar<ParamsT>...)>, RetT>::value,
			int> = 0>
	Function(FuncT &&x_vFunc){
		Reset(std::forward<FuncT>(x_vFunc));
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (Impl::ForwardedNonScalar<ParamsT>...)>, RetT>::value,
			int> = 0>
	Function &operator=(FuncT &&x_vFunc){
		Reset(std::forward<FuncT>(x_vFunc));
		return *this;
	}

public:
	void Reset() noexcept {
		x_pFunctor.Reset();
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (Impl::ForwardedNonScalar<ParamsT>...)>, RetT>::value,
			int> = 0>
	void Reset(FuncT &&x_vFunc){
		x_pFunctor.Reset(new Impl::Functor<FuncT, RetT, ParamsT...>(std::forward<FuncT>(x_vFunc)));
	}

	void Swap(Function &rhs) noexcept {
		x_pFunctor.Swap(rhs.x_pFunctor);
	}

public:
	explicit operator bool() const noexcept {
		return !!x_pFunctor;
	}
	RetT operator()(ParamsT ...vParams) const {
		ASSERT(x_pFunctor);

		return x_pFunctor->Dispatch(std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}
};

template<typename RetT, typename ...ParamsT>
void swap(Function<RetT (ParamsT...)> &lhs, Function<RetT (ParamsT...)> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
