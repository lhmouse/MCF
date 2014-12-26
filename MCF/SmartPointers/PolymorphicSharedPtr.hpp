// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_
#define MCF_SMART_POINTERS_POLYMORPHIC_SHARED_PTR_HPP_

#include "SharedPtr.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

namespace Impl {
	struct PolymorphicSharedPtrContainerBase {
		virtual ~PolymorphicSharedPtrContainerBase() = 0;

		virtual std::pair<PolymorphicSharedPtrContainerBase *, void *> Clone() const = 0;
	};

	template<typename ObjectT>
	class PolymorphicSharedPtrContainer : public PolymorphicSharedPtrContainerBase {
	public:
		ObjectT m_vObjectT;

	public:
		template<typename ...ParamsT>
		explicit PolymorphicSharedPtrContainer(ParamsT &&...vParams)
			: m_vObjectT(std::forward<ParamsT>(vParams)...)
		{
		}

	private:
		template<typename TestContainerT = PolymorphicSharedPtrContainer,
			std::enable_if_t<
				std::is_copy_constructible<TestContainerT>::value,
				int> = 0>
		PolymorphicSharedPtrContainer *xCloneSelf() const {
			return new TestContainerT(*this);
		}
		template< typename TestContainerT = PolymorphicSharedPtrContainer,
			std::enable_if_t<
				!std::is_copy_constructible<TestContainerT>::value,
				int> = 0>
		[[noreturn]]
		PolymorphicSharedPtrContainer *xCloneSelf() const {
			DEBUG_THROW(Exception, "Class is not copy-constructible", ERROR_INVALID_PARAMETER);
		}

	public:
		std::pair<PolymorphicSharedPtrContainerBase *, void *> Clone() const override {
			const auto pNewContainer = xCloneSelf();
			return std::make_pair(pNewContainer, &(pNewContainer->m_vObjectT));
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

	const auto pNewContainer =
		new Impl::PolymorphicSharedPtrContainer<std::remove_cv_t<ObjectT>>(
			std::forward<ParamsT>(vParams)...);
	return PolymorphicSharedPtr<ObjectT>(
		SharedPtr<Impl::PolymorphicSharedPtrContainerBase,
			DefaultDeleter<Impl::PolymorphicSharedPtrContainerBase>>(pNewContainer),
		&(pNewContainer->m_vObjectT));
}

template<typename DstT, typename SrcT>
auto DynamicPointerCast(PolymorphicSharedPtr<SrcT> rhs) noexcept {
	static_assert((std::is_const<DstT>::value == std::is_const<SrcT>::value) &&
		(std::is_volatile<DstT>::value == std::is_volatile<SrcT>::value), "cv-qualifiers mismatch.");

	const auto pContainer =
		dynamic_cast<Impl::PolymorphicSharedPtrContainer<std::remove_cv_t<DstT>> *>(rhs.GetRaw());
	return PolymorphicSharedPtr<DstT>(std::move(rhs), pContainer ? &(pContainer->m_vObjectT) : nullptr);
}

template<typename ObjectT>
auto DynamicClone(const PolymorphicSharedPtr<ObjectT> &rhs){
	PolymorphicSharedPtr<std::remove_cv_t<ObjectT>> pNew;
	const auto pContainer = rhs.GetRaw();
	if(pContainer){
		const auto vResult = pContainer->Clone();
		pNew.Reset(vResult.first, vResult.second);
	}
	return pNew;
}

}

#endif
