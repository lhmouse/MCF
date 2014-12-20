// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_

#include "SharedPtr.hpp"

namespace MCF {

namespace Impl {
	struct PolymorphicSharedPtrContainerBase {
		virtual ~PolymorphicSharedPtrContainerBase() = 0;
	};

	template<typename ObjectT>
	struct PolymorphicSharedPtrContainer : public PolymorphicSharedPtrContainerBase {
		ObjectT m_vObjectT;

		template<typename ...ParamsT>
		explicit PolymorphicSharedPtrContainer(ParamsT &&...vParams)
			: m_vObjectT(std::forward<ParamsT>(vParams)...)
		{
		}
	};
}

template<typename ObjectT>
using PolymorphicSharedPtr = SharedPtr<ObjectT, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template<typename ObjectT>
using PolymorphicWeakPtr = WeakPtr<ObjectT, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

template<typename ObjectT, typename ...ParamsT>
auto MakePolymorphicShared(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");

	using SharedContainerPtr = SharedPtr<Impl::PolymorphicSharedPtrContainer<ObjectT>, DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>;

	SharedContainerPtr pContainer(new Impl::PolymorphicSharedPtrContainer<ObjectT>(std::forward<ParamsT>(vParams)...));
	return PolymorphicSharedPtr<ObjectT>(std::move(pContainer), &(pContainer->m_vObjectT));
}

}

#endif
