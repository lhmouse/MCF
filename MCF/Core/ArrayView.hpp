// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_VIEW_HPP_
#define MCF_CORE_ARRAY_VIEW_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/AddressOf.hpp"
#include <type_traits>
#include <initializer_list>
#include <cstddef>
#include "StringView.hpp"

namespace MCF {

template<typename ElementT>
class Vector;

template<class ElementT>
class ArrayView {
private:
	ElementT *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayView(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayView(ElementT &rhs) noexcept
		: x_pBegin(AddressOf(rhs)), x_uSize(1)
	{
	}
	constexpr ArrayView(ElementT *pBegin, std::size_t uSize) noexcept
		: x_pBegin(pBegin), x_uSize(uSize)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayView(ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<StringType kTypeT,
		std::enable_if_t<
			std::is_same<typename StringView<kTypeT>::Char, std::remove_cv_t<ElementT>>::value,
			int> = 0>
	constexpr ArrayView(StringView<kTypeT> rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayView(Vector<ElementT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}

public:
	constexpr ElementT *GetBegin() const noexcept {
		return x_pBegin;
	}
	constexpr ElementT *GetEnd() const noexcept {
		return x_pBegin + x_uSize;
	}
	constexpr ElementT *GetData() const noexcept {
		return x_pBegin;
	}
	constexpr std::size_t GetSize() const noexcept {
		return x_uSize;
	}

public:
	explicit constexpr operator ElementT *() const noexcept {
		return GetData();
	}
	constexpr ElementT &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData();
	}
};

template<class ElementT>
class ArrayView<const ElementT> {
private:
	const ElementT *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayView(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayView(const ElementT &rhs) noexcept
		: x_pBegin(AddressOf(rhs)), x_uSize(1)
	{
	}
	constexpr ArrayView(const ElementT *pBegin, std::size_t uSize) noexcept
		: x_pBegin(pBegin), x_uSize(uSize)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayView(const ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayView(ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<StringType kTypeT,
		std::enable_if_t<
			std::is_same<typename StringView<kTypeT>::Char, std::remove_cv_t<ElementT>>::value,
			int> = 0>
	constexpr ArrayView(StringView<kTypeT> rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayView(const Vector<const ElementT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayView(const Vector<ElementT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayView(std::initializer_list<const ElementT> rhs) noexcept
		: x_pBegin(rhs.begin()), x_uSize(rhs.size())
	{
	}
	constexpr ArrayView(std::initializer_list<ElementT> rhs) noexcept
		: x_pBegin(rhs.begin()), x_uSize(rhs.size())
	{
	}

public:
	constexpr const ElementT *GetBegin() const noexcept {
		return x_pBegin;
	}
	constexpr const ElementT *GetEnd() const noexcept {
		return x_pBegin + x_uSize;
	}
	constexpr const ElementT *GetData() const noexcept {
		return x_pBegin;
	}
	constexpr std::size_t GetSize() const noexcept {
		return x_uSize;
	}

public:
	explicit constexpr operator const ElementT *() const noexcept {
		return GetData();
	}
	constexpr const ElementT &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetData();
	}
};

template<class ElementT>
decltype(auto) begin(const ArrayView<ElementT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<class ElementT>
decltype(auto) cbegin(const ArrayView<ElementT> &rhs) noexcept {
	return begin(rhs);
}
template<class ElementT>
decltype(auto) end(const ArrayView<ElementT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<class ElementT>
decltype(auto) cend(const ArrayView<ElementT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
