// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

#include "../Core/Assert.hpp"
#include "../Core/Comparators.hpp"
#include "DefaultDeleter.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename ObjectT, class DeleterT = DefaultDeleter<ObjectT>>
class UniquePtr;

template<typename ObjectT, class DeleterT>
class UniquePtr {
	template<typename, class>
	friend class UniquePtr;

public:
	using Element = std::remove_extent_t<ObjectT>;
	using Deleter = DeleterT;

	static_assert(noexcept(Deleter()(std::declval<std::remove_cv_t<Element> *>())), "Deleter must not throw.");

private:
	Element *x_pElement;

private:
	void X_Dispose() noexcept {
		const auto pElement = x_pElement;
#ifndef NDEBUG
		x_pElement = (Element *)(std::uintptr_t)0xDEADBEEFDEADBEEF;
#endif
		if(pElement){
			Deleter()(const_cast<std::remove_cv_t<Element> *>(pElement));
		}
	}

public:
	constexpr UniquePtr(std::nullptr_t = nullptr) noexcept
		: x_pElement(nullptr)
	{
	}
	explicit UniquePtr(Element *rhs) noexcept
		: x_pElement(rhs)
	{
	}
	template<typename OtherObjectT, typename OtherDeleterT,
		std::enable_if_t<
			std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Element *, Element *>::value &&
				std::is_convertible<typename UniquePtr<OtherObjectT, OtherDeleterT>::Deleter, Deleter>::value,
			int> = 0>
	UniquePtr(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept
		: UniquePtr(rhs.Release())
	{
	}
	UniquePtr(UniquePtr &&rhs) noexcept
		: UniquePtr(rhs.Release())
	{
	}
	UniquePtr &operator=(UniquePtr &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~UniquePtr(){
		X_Dispose();
	}

	UniquePtr(const UniquePtr &) = delete;
	UniquePtr &operator=(const UniquePtr &) = delete;

public:
	constexpr bool IsNull() const noexcept {
		return !x_pElement;
	}
	constexpr Element *Get() const noexcept {
		return x_pElement;
	}
	Element *Release() noexcept {
		return std::exchange(x_pElement, nullptr);
	}

	UniquePtr &Reset(std::nullptr_t = nullptr) noexcept {
		UniquePtr().Swap(*this);
		return *this;
	}
	UniquePtr &Reset(Element *rhs) noexcept {
		UniquePtr(rhs).Swap(*this);
		return *this;
	}
	template<typename OtherObjectT, typename OtherDeleterT>
	UniquePtr &Reset(UniquePtr<OtherObjectT, OtherDeleterT> &&rhs) noexcept {
		UniquePtr(std::move(rhs)).Swap(*this);
		return *this;
	}
	UniquePtr &Reset(UniquePtr &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}

	void Swap(UniquePtr &rhs) noexcept {
		using std::swap;
		swap(x_pElement, rhs.x_pElement);
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsNull();
	}
	explicit constexpr operator Element *() const noexcept {
		return Get();
	}

	template<typename T = ObjectT>
	constexpr std::enable_if_t<
		!std::is_void<T>::value && !std::is_array<T>::value,
		Element> & operator*() const noexcept
	{
		MCF_ASSERT(!IsNull());

		return *Get();
	}
	template<typename T = ObjectT>
	constexpr std::enable_if_t<
		!std::is_void<T>::value && !std::is_array<T>::value,
		Element> * operator->() const noexcept
	{
		MCF_ASSERT(!IsNull());

		return Get();
	}
	template<typename T = ObjectT>
	constexpr std::enable_if_t<
		std::is_array<T>::value,
		Element> & operator[](std::size_t uIndex) const noexcept
	{
		MCF_ASSERT(!IsNull());

		return Get()[uIndex];
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator==(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return Equal()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator==(OtherObjectT *rhs) const noexcept {
		return Equal()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator==(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return Equal()(lhs, rhs.x_pElement);
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator!=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return Unequal()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator!=(OtherObjectT *rhs) const noexcept {
		return Unequal()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator!=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return Unequal()(lhs, rhs.x_pElement);
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator<(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return Less()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator<(OtherObjectT *rhs) const noexcept {
		return Less()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return Less()(lhs, rhs.x_pElement);
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator>(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return Greater()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator>(OtherObjectT *rhs) const noexcept {
		return Greater()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return Greater()(lhs, rhs.x_pElement);
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator<=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return LessEqual()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator<=(OtherObjectT *rhs) const noexcept {
		return LessEqual()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return LessEqual()(lhs, rhs.x_pElement);
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator>=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return GreaterEqual()(x_pElement, rhs.x_pElement);
	}
	template<typename OtherObjectT>
	constexpr bool operator>=(OtherObjectT *rhs) const noexcept {
		return GreaterEqual()(x_pElement, rhs);
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return GreaterEqual()(lhs, rhs.x_pElement);
	}

	friend void swap(UniquePtr &lhs, UniquePtr &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

template<typename ObjectT, typename DeleterT = DefaultDeleter<ObjectT>, typename ...ParamsT>
UniquePtr<ObjectT, DeleterT> MakeUnique(ParamsT &&...vParams){
	static_assert(!std::is_array<ObjectT>::value, "ObjectT shall not be an array type.");
	static_assert(!std::is_reference<ObjectT>::value, "ObjectT shall not be a reference type.");

	return UniquePtr<ObjectT, DeleterT>(new ObjectT(std::forward<ParamsT>(vParams)...));
}

template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> StaticPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	return UniquePtr<DstT, DeleterT>(static_cast<DstT *>(pSrc.Release()));
}
template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> ConstPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	return UniquePtr<DstT, DeleterT>(const_cast<DstT *>(pSrc.Release()));
}

template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> DynamicPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	const auto pTest = dynamic_cast<DstT *>(pSrc.Get());
	if(!pTest){
		return nullptr;
	}
	pSrc.Release();
	return UniquePtr<DstT, DeleterT>(pTest);
}

}

#endif
