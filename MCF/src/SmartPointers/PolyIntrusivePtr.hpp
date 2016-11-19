// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

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
	IntrusivePtr<const volatile OtherT, DefaultDeleter<PolyIntrusiveBase>> Share() const volatile {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkShared<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<const OtherT, DefaultDeleter<PolyIntrusiveBase>> Share() const {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkShared<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<volatile OtherT, DefaultDeleter<PolyIntrusiveBase>> Share() volatile {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkShared<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusivePtr<OtherT, DefaultDeleter<PolyIntrusiveBase>> Share(){
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkShared<OtherT>(this);
	}

	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const volatile OtherT, DefaultDeleter<PolyIntrusiveBase>> Weaken() const volatile {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkWeak<const volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<const OtherT, DefaultDeleter<PolyIntrusiveBase>> Weaken() const {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkWeak<const OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<volatile OtherT, DefaultDeleter<PolyIntrusiveBase>> Weaken() volatile {
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkWeak<volatile OtherT>(this);
	}
	template<typename OtherT = ObjectT>
	IntrusiveWeakPtr<OtherT, DefaultDeleter<PolyIntrusiveBase>> Weaken(){
		return IntrusiveBase<Impl_PolyIntrusivePtr::Deletable>::Y_ForkWeak<OtherT>(this);
	}
};

template<typename ObjectT>
using PolyIntrusivePtr         = IntrusivePtr     <ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;
template<typename ObjectT>
using PolyIntrusiveWeakPtr     = IntrusiveWeakPtr <ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;

}

#endif
