// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_VIRTUAL_SHARED_FROM_THIS_HPP_
#define MCF_VIRTUAL_SHARED_FROM_THIS_HPP_

#include <memory>
#include <type_traits>

namespace MCF {

namespace Impl {
	template<class Base, class Derived, typename TraitHelper = void>
	struct IsStaticCastable : public std::false_type {
	};
	template<class Base, class Derived>
	struct IsStaticCastable<Base, Derived,
		decltype(static_cast<Derived *>(std::declval<Base *>()), (void)0)>
		: public std::true_type
	{
	};

	template<class Base, class Derived, typename TraitHelper = void>
	struct IsDynamicCastable : public std::false_type {
	};
	template<class Base, class Derived>
	struct IsDynamicCastable<Base, Derived,
		decltype(dynamic_cast<Derived *>(std::declval<Base *>()), (void)0)>
		: public std::true_type
	{
	};
}

class VirtualSharedFromThis
	: public std::enable_shared_from_this<VirtualSharedFromThis>
{
public:
	// 定义在别处。参考源文件中的注释。
	virtual ~VirtualSharedFromThis() = 0;

public:
	template<typename Derived>
	std::shared_ptr<const Derived> GetSharedFromThis() const {
		static_assert(std::is_base_of<VirtualSharedFromThis, Derived>::value &&
				!Impl::IsStaticCastable<VirtualSharedFromThis, Derived>::value &&
				Impl::IsDynamicCastable<VirtualSharedFromThis, Derived>::value,
			"Please virtually derive from VirtualSharedFromThis.");

		return std::dynamic_pointer_cast<const Derived>(shared_from_this());
	}
	template<typename Derived>
	std::shared_ptr<Derived> GetSharedFromThis(){
		static_assert(std::is_base_of<VirtualSharedFromThis, Derived>::value &&
				!Impl::IsStaticCastable<VirtualSharedFromThis, Derived>::value &&
				Impl::IsDynamicCastable<VirtualSharedFromThis, Derived>::value,
			"Please virtually derive from VirtualSharedFromThis.");

		return std::dynamic_pointer_cast<Derived>(shared_from_this());
	}

	template<typename Derived>
	std::weak_ptr<const Derived> GetWeakFromThis() const {
		return GetSharedFromThis<const Derived>();
	}
	template<typename Derived>
	std::weak_ptr<Derived> GetWeakFromThis(){
		return GetSharedFromThis<Derived>();
	}
};

}

#endif
