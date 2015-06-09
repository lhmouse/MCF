// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>

namespace MCF {

struct PolymorphicIntrusiveDeleteableBase : public IntrusiveBase<PolymorphicIntrusiveDeleteableBase> {
	virtual ~PolymorphicIntrusiveDeleteableBase();

	virtual UniquePtr<PolymorphicIntrusiveDeleteableBase> MCF_Impl_IntrusiveClone_() const = 0;
};

namespace Impl_PolymorphicIntrusivePtr {
	template<typename ObjectT, bool kIsCopyConstructible = std::is_copy_constructible<ObjectT>::value>
	struct VirtualCloner {
		UniquePtr<PolymorphicIntrusiveDeleteableBase> operator()(const ObjectT &src) const {
			return MakeUnique<ObjectT>(src);
		}
	};
	template<typename ObjectT>
	struct VirtualCloner<ObjectT, false> {
		[[noreturn]]
		UniquePtr<PolymorphicIntrusiveDeleteableBase> operator()(const ObjectT &) const {
			DEBUG_THROW(Exception, "Class is not copy-constructible", ERROR_INVALID_PARAMETER);
		}
	};
}

template<typename ObjectT>
struct PolymorphicIntrusiveBase : public virtual PolymorphicIntrusiveDeleteableBase {
	UniquePtr<PolymorphicIntrusiveDeleteableBase> MCF_Impl_IntrusiveClone_() const override {
		return Impl_PolymorphicIntrusivePtr::VirtualCloner<ObjectT>()(
			Impl_IntrusivePtr::StaticOrDynamicCast<const ObjectT &>(*this));
	}
};

template<typename ObjectT>
using PolymorphicIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<PolymorphicIntrusiveDeleteableBase>>;

using PolymorphicIntrusiveUnknownPtr = PolymorphicIntrusivePtr<PolymorphicIntrusiveDeleteableBase>;

template<typename ObjectT, typename ...ParamsT>
PolymorphicIntrusivePtr<ObjectT> MakePolymorphicIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return PolymorphicIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename ObjectT>
PolymorphicIntrusivePtr<ObjectT> DynamicClone(const PolymorphicIntrusivePtr<ObjectT> &rhs){
	PolymorphicIntrusivePtr<ObjectT> pRet;
	if(rhs){
		const auto pBase = static_cast<const PolymorphicIntrusiveDeleteableBase *>(rhs.Get());
		auto pUnknown = pBase->MCF_Impl_IntrusiveClone_();
		pRet.Reset(Impl_IntrusivePtr::StaticOrDynamicCast<ObjectT *>(pUnknown.Get()));
		ASSERT(pRet);
		pUnknown.Release();
	}
	return pRet;
}

}

#endif
