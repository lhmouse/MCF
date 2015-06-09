// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

// 1) 构造函数（包含复制构造函数、转移构造函数以及构造函数模板）、赋值运算符和析构函数应当调用 Reset()；
// 2) Reset() 的形参若具有 UniquePtr 的某模板类类型，则禁止传值，必须传引用。

#include "../Utilities/Assert.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<std::remove_cv_t<ObjectT>>>
class UniquePtr {
	static_assert(noexcept(DeleterT()(DeleterT()())), "Deleter must not throw.");

public:
	using ElementType = std::remove_extent_t<ObjectT>;

private:
	ElementType *x_pElement;

public:
	constexpr UniquePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit constexpr UniquePtr(ElementType *pElement) noexcept
		: x_pElement(pElement)
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	UniquePtr(UniquePtr<OtherT, OtherDeleterT> &&rhs) noexcept
		: UniquePtr()
	{
		Reset(std::move(rhs));
	}
	UniquePtr &operator=(std::nullptr_t) noexcept {
		Reset();
		return *this;
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	UniquePtr &operator=(UniquePtr<OtherT, OtherDeleterT> rhs) noexcept {
		Reset(std::move(rhs));
		return *this;
	}
	~UniquePtr(){
		Reset();
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	bool IsNonnull() const noexcept {
		return Get() != nullptr;
	}
	ElementType *Get() const noexcept {
		return x_pElement;
	}
	ElementType *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}

	UniquePtr &Reset(ElementType *pElement = nullptr) noexcept {
		const auto pOld = std::exchange(x_pElement, pElement);
		if(pOld){
			DeleterT()(const_cast<std::remove_cv_t<ElementType> *>(pOld));
		}
		return *this;
	}
	template<typename OtherT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherT, OtherDeleterT>::ElementType *, ElementType *>::value &&
				std::is_convertible<OtherDeleterT, DeleterT>::value,
			int> = 0>
	UniquePtr &Reset(UniquePtr<OtherT, OtherDeleterT> &&rhs) noexcept {
		return Reset(static_cast<ElementType *>(rhs.Release()));
	}

	void Swap(UniquePtr &rhs) noexcept {
		std::swap(x_pElement, rhs.x_pElement);
	}

public:
	explicit operator bool() const noexcept {
		return IsNonnull();
	}
	explicit operator ElementType *() const noexcept {
		return Get();
	}

	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, ElementType> &operator*() const noexcept {
		ASSERT(IsNonnull());

		return *Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<!std::is_void<TestT>::value && !std::is_array<TestT>::value, ElementType> *operator->() const noexcept {
		ASSERT(IsNonnull());

		return Get();
	}
	template<typename TestT = ObjectT>
	std::enable_if_t<std::is_array<TestT>::value, ElementType> &operator[](std::size_t uIndex) const noexcept {
		ASSERT(IsNonnull());

		return Get()[uIndex];
	}
};

#define MCF_SMART_POINTERS_DECLARE_TEMPLATE_PARAMETERS_	template<typename ObjectT, class DeleterT>
#define MCF_SMART_POINTERS_INVOKE_TEMPLATE_PARAMETERS_	UniquePtr<ObjectT, DeleterT>
#include "_RationalAndSwap.hpp"

template<typename ObjectT, typename ...ParamsT>
UniquePtr<ObjectT, DefaultDeleter<ObjectT>> MakeUnique(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return UniquePtr<ObjectT, DefaultDeleter<ObjectT>>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

}

#endif
