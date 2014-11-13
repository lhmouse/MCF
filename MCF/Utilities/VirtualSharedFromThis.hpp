// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_VIRTUAL_SHARED_FROM_THIS_HPP_
#define MCF_CORE_VIRTUAL_SHARED_FROM_THIS_HPP_

#include <memory>
#include <stdexcept>
#include <type_traits>

namespace MCF {

namespace Impl {
	template<class BaseT, class DerivedT, typename TraitHelperT = void>
	struct IsStaticCastable : public std::false_type {
	};
	template<class BaseT, class DerivedT>
	struct IsStaticCastable<BaseT, DerivedT,
		decltype(static_cast<DerivedT *>(std::declval<BaseT *>()), (void)0)>
		: public std::true_type
	{
	};

	template<class BaseT, class DerivedT, typename TraitHelperT = void>
	struct IsDynamicCastable : public std::false_type {
	};
	template<class BaseT, class DerivedT>
	struct IsDynamicCastable<BaseT, DerivedT,
		decltype(dynamic_cast<DerivedT *>(std::declval<BaseT *>()), (void)0)>
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
	template<typename DerivedT>
	std::shared_ptr<const DerivedT> GetSharedFromThis() const {
		static_assert(
			std::is_base_of<VirtualSharedFromThis, DerivedT>::value &&
				!Impl::IsStaticCastable<VirtualSharedFromThis, DerivedT>::value &&
				Impl::IsDynamicCastable<VirtualSharedFromThis, DerivedT>::value,
			"Please virtually derive from VirtualSharedFromThis.");

		const auto pVirtualThis = dynamic_cast<const DerivedT *>(this);
		if(!pVirtualThis){
			throw std::bad_cast();
		}
		return std::shared_ptr<const DerivedT>(shared_from_this(), pVirtualThis);
	}
	template<typename DerivedT>
	std::shared_ptr<DerivedT> GetSharedFromThis(){
		static_assert(
			std::is_base_of<VirtualSharedFromThis, DerivedT>::value &&
				!Impl::IsStaticCastable<VirtualSharedFromThis, DerivedT>::value &&
				Impl::IsDynamicCastable<VirtualSharedFromThis, DerivedT>::value,
			"Please virtually derive from VirtualSharedFromThis.");

		const auto pVirtualThis = dynamic_cast<DerivedT *>(this);
		if(!pVirtualThis){
			throw std::bad_cast();
		}
		return std::shared_ptr<DerivedT>(shared_from_this(), pVirtualThis);
	}

	template<typename DerivedT>
	std::weak_ptr<const DerivedT> GetWeakFromThis() const {
		return GetSharedFromThis<const DerivedT>();
	}
	template<typename DerivedT>
	std::weak_ptr<DerivedT> GetWeakFromThis(){
		return GetSharedFromThis<DerivedT>();
	}
};

}

#endif
