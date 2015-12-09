// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>

namespace MCF {

namespace Impl_PolyIntrusivePtr {
	struct UnknownBase : public IntrusiveBase<UnknownBase> {
		virtual ~UnknownBase();
	};
}

template<typename ObjectT>
using PolyIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::UnknownBase>>;

using PolyIntrusivePtrConstVolatileUnknown     = PolyIntrusivePtr<const volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrConstUnknown             = PolyIntrusivePtr<const Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrVolatileUnknown          = PolyIntrusivePtr<volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrUnknown                  = PolyIntrusivePtr<Impl_PolyIntrusivePtr::UnknownBase>;

template<typename ObjectT>
using PolyIntrusiveWeakPtr = IntrusiveWeakPtr<ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::UnknownBase>>;

using PolyIntrusiveWeakPtrConstVolatileUnknown = PolyIntrusiveWeakPtr<const volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrConstUnknown         = PolyIntrusiveWeakPtr<const Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrVolatileUnknown      = PolyIntrusiveWeakPtr<volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrUnknown              = PolyIntrusiveWeakPtr<Impl_PolyIntrusivePtr::UnknownBase>;

template<typename ObjectT>
class PolyIntrusiveBase : public virtual Impl_PolyIntrusivePtr::UnknownBase {
private:
	template<typename CvOtherT, typename CvThisT>
	static PolyIntrusivePtr<CvOtherT> X_ForkShared(CvThisT *pThis) noexcept {
		const auto pOther = Impl_IntrusivePtr::StaticCastOrDynamicCast<CvOtherT *>(static_cast<Impl_PolyIntrusivePtr::UnknownBase *>(pThis));
		if(!pOther){
			return nullptr;
		}
		static_cast<const volatile Impl_IntrusivePtr::RefCountBase *>(pThis)->AddRef();
		return PolyIntrusivePtr<CvOtherT>(pOther);
	}
	template<typename CvOtherT, typename CvThisT>
	static PolyIntrusiveWeakPtr<CvOtherT> X_ForkWeak(CvThisT *pThis){
		const auto pOther = Impl_IntrusivePtr::StaticCastOrDynamicCast<CvOtherT *>(static_cast<Impl_PolyIntrusivePtr::UnknownBase *>(pThis));
		if(!pOther){
			return nullptr;
		}
		return PolyIntrusiveWeakPtr<CvOtherT>(pOther);
	}

public:
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const volatile OtherT> Share() const volatile noexcept {
		return X_ForkShared<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const OtherT> Share() const noexcept {
		return X_ForkShared<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<volatile OtherT> Share() volatile noexcept {
		return X_ForkShared<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<OtherT> Share() noexcept {
		return X_ForkShared<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	PolyIntrusiveWeakPtr<const volatile OtherT> Weaken() const volatile {
		return X_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusiveWeakPtr<const OtherT> Weaken() const {
		return X_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusiveWeakPtr<volatile OtherT> Weaken() volatile {
		return X_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusiveWeakPtr<OtherT> Weaken(){
		return X_ForkWeak<OtherT>(this);
	}
};

template<typename ObjectT, typename ...ParamsT>
PolyIntrusivePtr<ObjectT> MakePolyIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return PolyIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
