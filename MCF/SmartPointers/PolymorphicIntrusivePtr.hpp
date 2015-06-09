// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"
#include "../Core/Exception.hpp"
#include <type_traits>

namespace MCF {

namespace Impl_IntrusivePtr {
	struct DeletableBase : public IntrusiveBase<DeletableBase> {
		virtual ~DeletableBase();
	};
}

template<typename ObjectT>
struct PolymorphicIntrusiveBase : public virtual Impl_IntrusivePtr::DeletableBase {
};

template<typename ObjectT>
using PolymorphicIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<Impl_IntrusivePtr::DeletableBase>>;

using PolymorphicIntrusiveUnknownPtr = PolymorphicIntrusivePtr<Impl_IntrusivePtr::DeletableBase>;

template<typename ObjectT, typename ...ParamsT>
PolymorphicIntrusivePtr<ObjectT> MakePolymorphicIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return PolymorphicIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
