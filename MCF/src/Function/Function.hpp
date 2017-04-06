// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_HPP_
#define MCF_FUNCTION_FUNCTION_HPP_

#include "../SmartPointers/UniquePtr.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include "../Core/AddressOf.hpp"
#include "TupleManipulation.hpp"
#include <type_traits>
#include <utility>
#include <tuple>

namespace MCF {

template<typename PrototypeT>
class Function {
	static_assert((sizeof(PrototypeT *), false), "Class template Function instantiated with a non-function template type parameter.");
};

template<typename ReturnT, typename ...ParamsT>
class Function<ReturnT (ParamsT ...)> : public IntrusiveBase<Function<ReturnT (ParamsT ...)>> {
protected:
	using X_ReturnType      = ReturnT;
	using X_ForwardedParams = std::tuple<ParamsT &&...>;

public:
	constexpr Function() noexcept {
	}
	virtual ~Function();

protected:
	virtual X_ReturnType X_Forward(X_ForwardedParams &&tupParams) const = 0;

public:
	ReturnT operator()(ParamsT ...vParams) const {
		return X_Forward(std::forward_as_tuple(std::forward<ParamsT>(vParams)...));
	}
};

template<typename ReturnT, typename ...ParamsT>
Function<ReturnT (ParamsT ...)>::~Function(){
}

namespace Impl_Function {
	template<typename PrototypeT, typename FunctionT>
	class ConcreteFunction final : public Function<PrototypeT> {
	protected:
		using X_ReturnType      = typename Function<PrototypeT>::X_ReturnType;
		using X_ForwardedParams = typename Function<PrototypeT>::X_ForwardedParams;

	private:
		std::decay_t<FunctionT> x_vFunction;

	public:
		explicit ConcreteFunction(FunctionT &vFunction)
			: x_vFunction(std::forward<FunctionT>(vFunction))
		{
		}
		~ConcreteFunction() override;

	protected:
		X_ReturnType X_Forward(X_ForwardedParams &&tupParams) const override {
			return DesignatedSqueeze<X_ReturnType>(x_vFunction, std::move(tupParams));
		}
	};

	template<typename PrototypeT, typename FunctionT>
	ConcreteFunction<PrototypeT, FunctionT>::~ConcreteFunction(){
	}
}

template<typename PrototypeT, typename FunctionT>
UniquePtr<Impl_Function::ConcreteFunction<PrototypeT, FunctionT>> MakeFunction(FunctionT &&vFunction){
	return MakeUnique<Impl_Function::ConcreteFunction<PrototypeT, FunctionT>>(vFunction);
}

}

#endif
