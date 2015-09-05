// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_HPP_
#define MCF_FUNCTION_FUNCTION_HPP_

#include "../Core/Exception.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/Invoke.hpp"
#include "_ForwardedParam.hpp"
#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Function {
	template<typename ElementT, typename = void>
	struct CopyOrThrowHelper {
		[[noreturn]]
		IntrusivePtr<ElementT> operator()(const ElementT & /* vElement */) const {
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "CopyOrThrowHelper::operator()()");
		}
	};
	template<typename ElementT>
	struct CopyOrThrowHelper<ElementT,
		std::enable_if_t<std::is_copy_constructible<ElementT>::value>>
	{
		IntrusivePtr<ElementT> operator()(const ElementT &vElement) const {
			return IntrusivePtr<ElementT>(new ElementT(vElement));
		}
	};

	template<typename RetT, typename ...ParamsT>
	class FunctorBase : public IntrusiveBase<FunctorBase<RetT, ParamsT...>> {
	public:
		virtual ~FunctorBase() = default;

	public:
		virtual RetT Dispatch(ForwardedParam<ParamsT>...vParams) const = 0;
		virtual IntrusivePtr<FunctorBase> Fork() const = 0;
	};

	template<typename FuncT, typename RetT, typename ...ParamsT>
	class Functor : public FunctorBase<RetT, ParamsT...> {
	private:
		const std::remove_reference_t<FuncT> $vFunc;

	public:
		explicit Functor(FuncT &&vFunc)
			: $vFunc(std::forward<FuncT>(vFunc))
		{
		}

	public:
		RetT Dispatch(ForwardedParam<ParamsT>...vParams) const override {
			return Invoke($vFunc, std::forward<ParamsT>(vParams)...);
		}
		IntrusivePtr<FunctorBase<RetT, ParamsT...>> Fork() const override {
			return CopyOrThrowHelper<Functor>()(*this);
		}
	};
}

template<typename FuncT>
class Function {
	static_assert((sizeof(FuncT), false), "Class template Function instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class Function<RetT (ParamsT...)> {
private:
	IntrusivePtr<const Impl_Function::FunctorBase<RetT, ParamsT...>> $pFunctor;

public:
	constexpr Function(std::nullptr_t = nullptr) noexcept
		: $pFunctor(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			std::is_convertible<std::result_of_t<FuncT && (ForwardedParam<ParamsT>...)>, RetT>::value &&
				!std::is_same<std::decay_t<FuncT>, Function>::value,
			int> = 0>
	Function(FuncT &&vFunc)
		: $pFunctor(new Impl_Function::Functor<FuncT, RetT, ParamsT...>(std::forward<FuncT>(vFunc)))
	{
	}

public:
	bool IsNonnull() const noexcept {
		return $pFunctor.IsNonnull();
	}
	std::size_t GetRefCount() const noexcept {
		return $pFunctor.GetRefCount();
	}

	Function &Reset(std::nullptr_t = nullptr) noexcept {
		Function().Swap(*this);
		return *this;
	}
	template<typename FuncT>
	Function &Reset(FuncT &&vFunc){
		Function(std::forward<FuncT>(vFunc)).Swap(*this);
		return *this;
	}

	// 后置条件：GetRefCount() <= 1
	void Fork(){
		if($pFunctor.GetRefCount() > 1){
			$pFunctor = $pFunctor->Fork();
		}
	}

	void Swap(Function &rhs) noexcept {
		$pFunctor.Swap(rhs.$pFunctor);
	}

public:
	explicit operator bool() const noexcept {
		return !!$pFunctor;
	}
	RetT operator()(ParamsT ...vParams) const {
		ASSERT($pFunctor);

		return $pFunctor->Dispatch(std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}

	bool operator==(const Function &rhs) const noexcept {
		return $pFunctor == rhs.$pFunctor;
	}
	bool operator!=(const Function &rhs) const noexcept {
		return $pFunctor != rhs.$pFunctor;
	}
	bool operator<(const Function &rhs) const noexcept {
		return $pFunctor < rhs.$pFunctor;
	}
	bool operator>(const Function &rhs) const noexcept {
		return $pFunctor > rhs.$pFunctor;
	}
	bool operator<=(const Function &rhs) const noexcept {
		return $pFunctor <= rhs.$pFunctor;
	}
	bool operator>=(const Function &rhs) const noexcept {
		return $pFunctor >= rhs.$pFunctor;
	}
};

template<typename RetT, typename ...ParamsT>
void swap(Function<RetT (ParamsT...)> &lhs, Function<RetT (ParamsT...)> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
