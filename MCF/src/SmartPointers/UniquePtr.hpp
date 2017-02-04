// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_SMART_POINTERS_UNIQUE_PTR_HPP_
#define MCF_SMART_POINTERS_UNIQUE_PTR_HPP_

#include "../Core/Assert.hpp"
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
		const auto pElement = x_pElement;
#ifndef NDEBUG
		__builtin_memset(&x_pElement, 0xEF, sizeof(x_pElement));
#endif
		if(pElement){
			Deleter()(const_cast<std::remove_cv_t<Element> *>(pElement));
		}
	}

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
		UniquePtr(std::move(rhs)).Swap(*this);
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
		MCF_DEBUG_CHECK(!IsNull());

		return *Get();
	}
	template<typename T = ObjectT>
	constexpr std::enable_if_t<
		!std::is_void<T>::value && !std::is_array<T>::value,
		Element> * operator->() const noexcept
	{
		MCF_DEBUG_CHECK(!IsNull());

		return Get();
	}
	template<typename T = ObjectT>
	constexpr std::enable_if_t<
		std::is_array<T>::value,
		Element> & operator[](std::size_t uIndex) const noexcept
	{
		MCF_DEBUG_CHECK(!IsNull());

		return Get()[uIndex];
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator==(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement == rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator==(OtherObjectT *rhs) const noexcept {
		return x_pElement == rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator==(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs == rhs.x_pElement;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator!=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement != rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator!=(OtherObjectT *rhs) const noexcept {
		return x_pElement != rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator!=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs != rhs.x_pElement;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator<(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement < rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator<(OtherObjectT *rhs) const noexcept {
		return x_pElement < rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs < rhs.x_pElement;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator>(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement > rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator>(OtherObjectT *rhs) const noexcept {
		return x_pElement > rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs > rhs.x_pElement;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator<=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement <= rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator<=(OtherObjectT *rhs) const noexcept {
		return x_pElement <= rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator<=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs <= rhs.x_pElement;
	}

	template<typename OtherObjectT, class OtherDeleterT>
	constexpr bool operator>=(const UniquePtr<OtherObjectT, OtherDeleterT> &rhs) const noexcept {
		return x_pElement >= rhs.x_pElement;
	}
	template<typename OtherObjectT>
	constexpr bool operator>=(OtherObjectT *rhs) const noexcept {
		return x_pElement >= rhs;
	}
	template<typename OtherObjectT>
	friend constexpr bool operator>=(OtherObjectT *lhs, const UniquePtr &rhs) noexcept {
		return lhs >= rhs.x_pElement;
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
	const auto pTest = static_cast<DstT *>(pSrc.Get());
	pSrc.Release();
	return UniquePtr<DstT, DeleterT>(pTest);
}
template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> DynamicPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	const auto pTest = dynamic_cast<DstT *>(pSrc.Get());
	if(pTest){
		pSrc.Release();
	}
	return UniquePtr<DstT, DeleterT>(pTest);
}
template<typename DstT, typename SrcT, class DeleterT>
UniquePtr<DstT, DeleterT> ConstPointerCast(UniquePtr<SrcT, DeleterT> &&pSrc) noexcept {
	const auto pTest = const_cast<DstT *>(pSrc.Get());
	pSrc.Release();
	return UniquePtr<DstT, DeleterT>(pTest);
}

}

#endif
