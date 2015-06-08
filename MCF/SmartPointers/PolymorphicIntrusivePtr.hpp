// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_INTRUSIVE_PTR_HPP_

#include "IntrusivePtr.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

struct PolymorphicIntrusiveDeletableBase : public IntrusiveBase<PolymorphicIntrusiveDeletableBase> {
	virtual ~PolymorphicIntrusiveDeletableBase();

	virtual void *MCF_Impl_IntrusiveClone_() const = 0;
};

template<typename ObjectT>
class PolymorphicIntrusiveBase : public PolymorphicIntrusiveDeletableBase {
private:
	template<typename TestObjectT = ObjectT,
		std::enable_if_t<
			std::is_copy_constructible<TestObjectT>::value,
			int> = 0>
	TestObjectT *xDoClone() const {
		return new TestObjectT(
			*Impl_IntrusivePtr::StaticOrDynamicCast<const TestObjectT>(this));
	}
	template<typename TestObjectT = ObjectT,
		std::enable_if_t<
			!std::is_copy_constructible<TestObjectT>::value,
			int> = 0>
	[[noreturn]]
	TestObjectT *xDoClone() const {
		DEBUG_THROW(Exception, "Class is not copy-constructible", ERROR_INVALID_PARAMETER);
	}

public:
	void *MCF_Impl_IntrusiveClone_() const override {
		return xDoClone();
	}
};

template<typename ObjectT>
using PolymorphicIntrusivePtr = IntrusivePtr<ObjectT, DefaultDeleter<PolymorphicIntrusiveDeletableBase>>;

template<typename ObjectT, typename ...ParamsT>
auto MakePolymorphicIntrusive(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	return PolymorphicIntrusivePtr<ObjectT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename ObjectT>
auto DynamicClone(const PolymorphicIntrusivePtr<ObjectT> &rhs){
	PolymorphicIntrusivePtr<std::remove_cv_t<ObjectT>> pNew;
	if(rhs){
		pNew.Reset(static_cast<std::remove_cv_t<ObjectT> *>(rhs->MCF_Impl_IntrusiveClone_()));
	}
	return pNew;
}

}

#endif
