// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"

namespace MCF {

namespace Impl_PolyIntrusivePtr {
	// 避免 Unknown 作非虚基类时的二义性。
	class Deletable : public IntrusiveBase<Deletable> {
	public:
		~Deletable() override;
	};

	class Unknown : public virtual Deletable {
	public:
		~Unknown() override;
	};
}

template<typename ObjectT>
class PolyIntrusiveBase : public Impl_PolyIntrusivePtr::Unknown {
public:
	template<typename OtherT = ObjectT>
	IntrusivePtr<const volatile OtherT, DefaultDeleter<Deletable>> Share() const volatile {
		return IntrusiveBase<Deletable>::Y_ForkShared<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DefaultDeleter<Deletable>> Share() const {
		return IntrusiveBase<Deletable>::Y_ForkShared<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DefaultDeleter<Deletable>> Share() volatile {
		return IntrusiveBase<Deletable>::Y_ForkShared<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DefaultDeleter<Deletable>> Share(){
		return IntrusiveBase<Deletable>::Y_ForkShared<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const volatile OtherT, DefaultDeleter<Deletable>> Weaken() const volatile {
		return IntrusiveBase<Deletable>::Y_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const OtherT, DefaultDeleter<Deletable>> Weaken() const {
		return IntrusiveBase<Deletable>::Y_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<volatile OtherT, DefaultDeleter<Deletable>> Weaken() volatile {
		return IntrusiveBase<Deletable>::Y_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<OtherT, DefaultDeleter<Deletable>> Weaken(){
		return IntrusiveBase<Deletable>::Y_ForkWeak<OtherT>(this);
	}
};

template<typename ObjectT>
using PolyIntrusivePtr             = IntrusivePtr     <ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;
template<typename ObjectT>
using PolyIntrusiveWeakPtr         = IntrusiveWeakPtr <ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;

using PolyIntrusivePtrUnknown      = PolyIntrusivePtr     <Impl_PolyIntrusivePtr::Unknown>;
using PolyIntrusiveWeakPtrUnknown  = PolyIntrusiveWeakPtr <Impl_PolyIntrusivePtr::Unknown>;

}

#endif
