// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"

namespace MCF {

namespace Impl_PolyIntrusivePtr {
	class UnknownBase : public IntrusiveBase<UnknownBase> {
	public:
		virtual ~UnknownBase();
	};
}

template<typename ObjectT>
using PolyIntrusivePtr     = IntrusivePtr     <ObjectT>;
template<typename ObjectT>
using PolyIntrusiveWeakPtr = IntrusiveWeakPtr <ObjectT>;

template<typename ObjectT>
class PolyIntrusiveBase : public virtual Impl_PolyIntrusivePtr::UnknownBase {
public:
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const volatile OtherT> Share() const volatile noexcept {
		return IntrusiveBase<UnknownBase>::template Y_ForkStrong<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const OtherT> Share() const noexcept {
		return IntrusiveBase<UnknownBase>::template Y_ForkStrong<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<volatile OtherT> Share() volatile noexcept {
		return IntrusiveBase<UnknownBase>::template Y_ForkStrong<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<OtherT> Share() noexcept {
		return IntrusiveBase<UnknownBase>::template Y_ForkStrong<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const volatile OtherT> Weaken() const volatile {
		return IntrusiveBase<UnknownBase>::template Y_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<const OtherT> Weaken() const {
		return IntrusiveBase<UnknownBase>::template Y_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<volatile OtherT> Weaken() volatile {
		return IntrusiveBase<UnknownBase>::template Y_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	PolyIntrusivePtr<OtherT> Weaken(){
		return IntrusiveBase<UnknownBase>::template Y_ForkWeak<OtherT>(this);
	}
};

extern template class IntrusivePtr     <Impl_PolyIntrusivePtr::UnknownBase>;
extern template class IntrusiveWeakPtr <Impl_PolyIntrusivePtr::UnknownBase>;

using PolyIntrusivePtrUnknownBase      = PolyIntrusivePtr     <Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrUnknownBase  = PolyIntrusiveWeakPtr <Impl_PolyIntrusivePtr::UnknownBase>;

}

#endif
