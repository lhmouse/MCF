// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FUNCTION_HPP_
#define MCF_CORE_FUNCTION_HPP_

#include "Exception.hpp"
#include "Assert.hpp"
#include "DeclVal.hpp"
#include "Atomic.hpp"
#include "_MagicalInvoker.hpp"
#include "_ForwardedParam.hpp"
#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl_Function {
	struct FunctorCopier {
		template<typename FuncT>
		static FuncT *DoIt(const FuncT &vFunc){
			return new auto(vFunc);
		}
	};
	struct DummyFunctorCopier {
		template<typename FuncT>
		[[noreturn]] static FuncT *DoIt(const FuncT &){
			MCF_THROW(Exception, ERROR_CALL_NOT_IMPLEMENTED, Rcntws::View(L"Function: 该函数对象不允许复制构造。"));
		}
	};

	template<typename RetT, typename ...ParamsT>
	class RefCountedFunctorBase {
	private:
		mutable Atomic<std::size_t> x_uRef;

	public:
		constexpr RefCountedFunctorBase() noexcept
			: x_uRef(1)
		{
		}
		constexpr RefCountedFunctorBase(const RefCountedFunctorBase &) noexcept
			: RefCountedFunctorBase()
		{
		}
		RefCountedFunctorBase &operator=(const RefCountedFunctorBase &) noexcept {
			return *this;
		}
		virtual ~RefCountedFunctorBase(){
		}

	public:
		virtual RetT Dispatch(Impl_ForwardedParam::ForwardedParam<ParamsT> ...vParams) const = 0;
		virtual RefCountedFunctorBase *VirtualNew() const = 0;

		bool IsUnique() const volatile noexcept {
			return GetRef() == 1;
		}
		std::size_t GetRef() const volatile noexcept {
			return x_uRef.Load(kAtomicRelaxed);
		}
		void AddRef() const volatile noexcept {
			MCF_DEBUG_CHECK(static_cast<std::ptrdiff_t>(x_uRef.Load(kAtomicRelaxed)) > 0);

			x_uRef.Increment(kAtomicRelaxed);
		}
		bool DropRef() const volatile noexcept {
			MCF_DEBUG_CHECK(static_cast<std::ptrdiff_t>(x_uRef.Load(kAtomicRelaxed)) > 0);

			return x_uRef.Decrement(kAtomicRelaxed) == 0;
		}
	};

	template<typename FuncT, typename RetT, typename ...ParamsT>
	class Functor : public RefCountedFunctorBase<RetT, ParamsT...> {
	private:
		std::decay_t<FuncT> x_vFunc;

	public:
		explicit Functor(FuncT &vFunc)
			: x_vFunc(std::forward<FuncT>(vFunc))
		{
		}

	public:
		RetT Dispatch(Impl_ForwardedParam::ForwardedParam<ParamsT> ...vParams) const override {
			return Impl_MagicalInvoker::MagicalInvoker<RetT>()(x_vFunc, std::forward<ParamsT>(vParams)...);
		}
		Functor *VirtualNew() const override {
			return std::conditional_t<std::is_copy_constructible<std::decay_t<FuncT>>::value, FunctorCopier, DummyFunctorCopier>::DoIt(*this);
		}
	};
}

template<typename PrototypeT>
class Function {
	static_assert((sizeof(PrototypeT), false), "Class template Function instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class Function<RetT (ParamsT...)> {
private:
	struct X_AdoptionTag { };
	using X_Functor = Impl_Function::RefCountedFunctorBase<std::remove_cv_t<RetT>, ParamsT...>;

private:
	const X_Functor *x_pFunctor;

private:
	const X_Functor *X_Fork() const noexcept {
		const auto pFunctor = x_pFunctor;
		if(pFunctor){
			pFunctor->AddRef();
		}
		return pFunctor;
	}
	const X_Functor *X_Release() noexcept {
		const auto pFunctor = x_pFunctor;
		x_pFunctor = nullptr;
		return pFunctor;
	}
	void X_Dispose() noexcept {
		const auto pFunctor = x_pFunctor;
#ifndef NDEBUG
		__builtin_memset(&x_pFunctor, 0xEA, sizeof(x_pFunctor));
#endif
		if(pFunctor){
			if(pFunctor->DropRef()){
				delete pFunctor;
			}
		}
	}

private:
	constexpr Function(const X_AdoptionTag &, const X_Functor *pFunctor) noexcept
		: x_pFunctor(pFunctor)
	{
	}

public:
	constexpr Function(std::nullptr_t = nullptr) noexcept
		: x_pFunctor(nullptr)
	{
	}
	template<typename FuncT,
		std::enable_if_t<
			!std::is_base_of<Function, std::decay_t<FuncT>>::value &&
				(std::is_convertible<decltype(Invoke(DeclVal<FuncT>(), DeclVal<ParamsT>()...)), RetT>::value || std::is_void<RetT>::value),
			int> = 0>
	Function(FuncT &&vFunc)
		: Function(X_AdoptionTag(), new Impl_Function::Functor<FuncT, std::remove_cv_t<RetT>, ParamsT...>(vFunc))
	{
	}
	Function(const Function &rhs) noexcept
		: Function(X_AdoptionTag(), rhs.X_Fork())
	{
	}
	Function(Function &&rhs) noexcept
		: Function(X_AdoptionTag(), rhs.X_Release())
	{
	}
	Function &operator=(const Function &rhs) noexcept {
		return Reset(rhs);
	}
	Function &operator=(Function &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~Function(){
		X_Dispose();
	}

public:
	bool IsNull() const noexcept {
		return !x_pFunctor;
	}
	std::size_t GetRefCount() const noexcept {
		const auto pFunctor = x_pFunctor;
		if(!pFunctor){
			return 0;
		}
		return pFunctor->GetRef();
	}

	Function &Reset(std::nullptr_t = nullptr) noexcept {
		Function().Swap(*this);
		return *this;
	}
	Function &Reset(const Function &vFunc){
		Function(vFunc).Swap(*this);
		return *this;
	}
	Function &Reset(Function &&vFunc){
		vFunc.Swap(*this);
		return *this;
	}
	template<typename FuncT>
	Function &Reset(FuncT &&vFunc){
		Function(std::forward<FuncT>(vFunc)).Swap(*this);
		return *this;
	}

	// 后置条件：GetRefCount() <= 1
	void Clone(){
		const auto pFunctor = x_pFunctor;
		if(!pFunctor || (pFunctor->GetRef() <= 1)){
			return;
		}
		Function(X_AdoptionTag(), pFunctor->VirtualNew()).Swap(*this);
	}

	void Swap(Function &rhs) noexcept {
		using std::swap;
		swap(x_pFunctor, rhs.x_pFunctor);
	}

public:
	explicit operator bool() const noexcept {
		return !IsNull();
	}
	std::remove_cv_t<RetT> operator()(ParamsT ...vParams) const {
		const auto pFunctor = x_pFunctor;
		MCF_DEBUG_CHECK(pFunctor);
		return pFunctor->Dispatch(std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
	}

	friend void swap(Function &lhs, Function &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
