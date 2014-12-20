// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"

namespace MCF {

struct PolymorphicIntrusiveBase : public IntrusiveBase<PolymorphicIntrusiveBase> {
	virtual ~PolymorphicIntrusiveBase() = 0;
};

template<typename ObjectT>
using PolymorphicIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<PolymorphicIntrusiveBase>>;

template<typename ObjectT, typename ...ParamsT>
auto MakePolymorphicIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return PolymorphicIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
