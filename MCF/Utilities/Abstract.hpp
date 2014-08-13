// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_ABSTRACT_HPP_
#define MCF_ABSTRACT_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl {
	class AbstractBase {
	protected:
		virtual ~AbstractBase() noexcept = default;

	private:
		virtual void MCF_PureAbstract_() noexcept = 0;
	};

	template<typename RealBase>
	class ConcreteBase : public RealBase {
	protected:
		template<typename... BaseParams>
		explicit ConcreteBase(BaseParams &&... vBaseParams)
			noexcept(std::is_nothrow_constructible<RealBase, BaseParams &&...>::value)
			: RealBase(std::forward<BaseParams>(vBaseParams)...)
		{
		}

	private:
		void MCF_PureAbstract_() noexcept override {
		}
	};
}

}

#define ABSTRACT					private ::MCF::Impl::AbstractBase
#define CONCRETE(base)				public ::MCF::Impl::ConcreteBase<base>

#define CONCRETE_INIT(base, ...)	::MCF::Impl::ConcreteBase<base>(__VA_ARGS__)

#endif
