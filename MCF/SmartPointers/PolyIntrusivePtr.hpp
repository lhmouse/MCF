// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLY_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"

namespace MCF {

class PolyIntrusiveBase : public virtual IntrusiveBase<PolyIntrusiveBase> {
public:
	~PolyIntrusiveBase() override;

public:
	template<typename ObjectT>
	IntrusivePtr<const volatile ObjectT, DefaultDeleter<PolyIntrusiveBase>> Share() const volatile {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkShared<const volatile ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusivePtr<const ObjectT, DefaultDeleter<PolyIntrusiveBase>> Share() const {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkShared<const ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusivePtr<volatile ObjectT, DefaultDeleter<PolyIntrusiveBase>> Share() volatile {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkShared<volatile ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusivePtr<ObjectT, DefaultDeleter<PolyIntrusiveBase>> Share(){
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkShared<ObjectT>(this);
	}

	template<typename ObjectT>
	IntrusiveWeakPtr<const volatile ObjectT, DefaultDeleter<PolyIntrusiveBase>> Weaken() const volatile {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkWeak<const volatile ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusiveWeakPtr<const ObjectT, DefaultDeleter<PolyIntrusiveBase>> Weaken() const {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkWeak<const ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusiveWeakPtr<volatile ObjectT, DefaultDeleter<PolyIntrusiveBase>> Weaken() volatile {
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkWeak<volatile ObjectT>(this);
	}
	template<typename ObjectT>
	IntrusiveWeakPtr<ObjectT, DefaultDeleter<PolyIntrusiveBase>> Weaken(){
		return IntrusiveBase<PolyIntrusiveBase>::X_ForkWeak<ObjectT>(this);
	}
};

template<typename ObjectT>
using PolyIntrusivePtr         = IntrusivePtr     <ObjectT, DefaultDeleter<PolyIntrusiveBase>>;
template<typename ObjectT>
using PolyIntrusiveWeakPtr     = IntrusiveWeakPtr <ObjectT, DefaultDeleter<PolyIntrusiveBase>>;

template<typename ObjectT>
using PolyIntrusivePtrView     = const PolyIntrusivePtr<ObjectT> &;
template<typename ObjectT>
using PolyIntrusiveWeakPtrView = const PolyIntrusiveWeakPtr<ObjectT> &;

}

#endif
