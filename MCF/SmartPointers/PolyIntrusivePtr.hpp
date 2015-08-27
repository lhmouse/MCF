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
struct PolyIntrusiveBase : public virtual Impl_PolyIntrusivePtr::UnknownBase {
};

template<typename ObjectT>
using PolyIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::UnknownBase>>;

using PolyIntrusivePtrConstVolatileUnknown	= PolyIntrusivePtr<const volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrConstUnknown			= PolyIntrusivePtr<const Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrVolatileUnknown		= PolyIntrusivePtr<volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusivePtrUnknown				= PolyIntrusivePtr<Impl_PolyIntrusivePtr::UnknownBase>;

template<typename ObjectT>
using PolyIntrusiveWeakPtr = IntrusiveWeakPtr<ObjectT, DefaultDeleter<Impl_PolyIntrusivePtr::UnknownBase>>;

using PolyIntrusiveWeakPtrConstVolatileUnknown	= PolyIntrusiveWeakPtr<const volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrConstUnknown			= PolyIntrusiveWeakPtr<const Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrVolatileUnknown		= PolyIntrusiveWeakPtr<volatile Impl_PolyIntrusivePtr::UnknownBase>;
using PolyIntrusiveWeakPtrUnknown				= PolyIntrusiveWeakPtr<Impl_PolyIntrusivePtr::UnknownBase>;

template<typename ObjectT, typename ...ParamsT>
PolyIntrusivePtr<ObjectT> MakePolyIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return PolyIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
