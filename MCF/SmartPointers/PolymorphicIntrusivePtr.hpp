// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>

namespace MCF {

struct PolymorphicIntrusiveDeleteableBase : public virtual IntrusiveBase<PolymorphicIntrusiveDeleteableBase> {
	virtual ~PolymorphicIntrusiveDeleteableBase();

	virtual PolymorphicIntrusiveDeleteableBase *MCF_Impl_IntrusiveClone_() const = 0;
};

namespace Impl_PolymorphicIntrusivePtr {
	template<typename ObjectT, bool kIsCopyConstructible = std::is_copy_constructible<std::decay_t<ObjectT>>::value>
	struct VirtualCloner {
		PolymorphicIntrusiveDeleteableBase *operator()(const std::decay_t<ObjectT> &src) const {
			return new std::decay_t<ObjectT>(src);
		}
	};
	template<typename ObjectT>
	struct VirtualCloner<ObjectT, false> {
		[[noreturn]]
		PolymorphicIntrusiveDeleteableBase *operator()(const std::decay_t<ObjectT> &src) const {
			DEBUG_THROW(Exception, "Class is not copy-constructible", ERROR_INVALID_PARAMETER);
		}
	};
}

template<typename ObjectT>
struct PolymorphicIntrusiveBase : public PolymorphicIntrusiveDeleteableBase {
	PolymorphicIntrusiveDeleteableBase *MCF_Impl_IntrusiveClone_() const override {
		return Impl_PolymorphicIntrusivePtr::VirtualCloner<ObjectT>()(
			Impl_IntrusivePtr::StaticOrDynamicCast<const std::decay_t<ObjectT> &>(*this));
	}
};

template<typename ObjectT>
using PolymorphicIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<PolymorphicIntrusiveDeleteableBase>>;

template<typename ObjectT, typename ...ParamsT>
auto MakePolymorphicIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return PolymorphicIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename ObjectT>
auto DynamicClone(const PolymorphicIntrusivePtr<ObjectT> &rhs){
	PolymorphicIntrusivePtr<std::decay_t<ObjectT>> pNew;
	if(rhs){
		pNew.Reset(Impl_IntrusivePtr::StaticOrDynamicCast<std::decay_t<ObjectT> *>(rhs->MCF_Impl_IntrusiveClone_()));
		ASSERT(pNew);
	}
	return pNew;
}

}

#endif
