// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_FUNCTION_HPP_
#define MCF_FUNCTION_FUNCTION_HPP_

#include "Invoke.hpp"
#include "_ForwardedParam.hpp"
#include "../SmartPointers/IntrusivePtr.hpp"
#include <type_traits>

namespace MCF {

template<typename PrototypeT>
class Function {
	static_assert(((void)sizeof(PrototypeT *), false), "Class template Function instantiated with non-function template type parameter.");
};

template<typename RetT, typename ...ParamsT>
class Function<RetT (ParamsT...)> : public IntrusiveBase<Function<RetT (ParamsT...)>> {
public:
	virtual ~Function();

public:
	virtual RetT operator()(ParamsT ...vParams) const = 0;
};

template<typename RetT, typename ...ParamsT>
Function<RetT (ParamsT...)>::~Function(){
}

namespace Impl_Function {
	template<typename PrototypeT, typename ActionT>
	class ConcreteFunction;

	template<typename RetT, typename ...ParamsT, typename ActionT>
	class ConcreteFunction<RetT (ParamsT...), ActionT> : public Function<RetT (ParamsT...)> {
	private:
		std::decay_t<ActionT> x_vAction;

	public:
		explicit ConcreteFunction(ActionT &vAction)
			: x_vAction(std::forward<ActionT>(vAction))
		{
		}
		~ConcreteFunction() override;

	public:
		RetT operator()(ParamsT ...vParams) const override {
			return DesignatedInvoke<std::remove_cv_t<RetT>>(x_vAction, std::forward<ParamsT>(vParams)...); // 值形参当作右值引用传递。
		}
	};

	template<typename RetT, typename ...ParamsT, typename ActionT>
	ConcreteFunction<RetT (ParamsT...), ActionT>::~ConcreteFunction(){
	}
}

template<typename PrototypeT, typename ActionT>
IntrusivePtr<Function<PrototypeT>> MakeFunction(ActionT &&vAction){
	return MakeIntrusive<Impl_Function::ConcreteFunction<PrototypeT, ActionT>>(vAction);
}

}

#endif
