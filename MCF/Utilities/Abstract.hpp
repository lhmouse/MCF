// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_ABSTRACT_HPP_
#define MCF_UTILITIES_ABSTRACT_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl {
	class AbstractBase {
	protected:
		virtual ~AbstractBase();

	private:
		virtual void MCF_PureAbstract_() noexcept = 0;
	};

	template<typename RealBaseT>
	class ConcreteBase : public RealBaseT {
	protected:
		template<typename ...BaseParams>
		explicit ConcreteBase(BaseParams &&...vBaseParams)
			noexcept(std::is_nothrow_constructible<RealBaseT, BaseParams &&...>::value)
			: RealBaseT(std::forward<BaseParams>(vBaseParams)...)
		{
		}

	private:
		void MCF_PureAbstract_() noexcept override {
		}
	};
}

}

#define ABSTRACT					private ::MCF::Impl::AbstractBase
#define CONCRETE(base_)				public ::MCF::Impl::ConcreteBase<base_>

#define CONCRETE_INIT(base_, ...)	::MCF::Impl::ConcreteBase<base_>(__VA_ARGS__)

#endif
