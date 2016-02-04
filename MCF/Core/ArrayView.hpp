// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_VIEW_HPP_
#define MCF_CORE_ARRAY_VIEW_HPP_

#include "Exception.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/AddressOf.hpp"
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class ArrayView {
public:
	using Element = ElementT;

private:
	Element *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayView(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayView(Element &rhs) noexcept
		: x_pBegin(AddressOf(rhs)), x_uSize(1)
	{
	}
	constexpr ArrayView(Element *pBegin, std::size_t uSize) noexcept
		: x_pBegin(pBegin), x_uSize(uSize)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayView(Element (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}

public:
	constexpr Element *GetBegin() const noexcept {
		return x_pBegin;
	}
	constexpr Element *GetEnd() const noexcept {
		return x_pBegin + x_uSize;
	}
	constexpr Element *GetData() const noexcept {
		return x_pBegin;
	}
	constexpr std::size_t GetSize() const noexcept {
		return x_uSize;
	}

	Element &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "ArrayView: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return x_pBegin[uIndex];
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator Element *() const noexcept {
		return GetData();
	}
	Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}

	friend decltype(auto) begin(const ArrayView &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) cbegin(const ArrayView &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) end(const ArrayView &rhs) noexcept {
		return rhs.GetEnd();
	}
	friend decltype(auto) cend(const ArrayView &rhs) noexcept {
		return rhs.GetEnd();
	}
};

template<typename ElementT>
class ArrayView<const ElementT> {
public:
	using Element = const ElementT;

private:
	Element *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayView(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayView(Element &rhs) noexcept
		: x_pBegin(AddressOf(rhs)), x_uSize(1)
	{
	}
	constexpr ArrayView(Element *pBegin, std::size_t uSize) noexcept
		: x_pBegin(pBegin), x_uSize(uSize)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayView(Element (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	constexpr ArrayView(const ArrayView<Element> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayView(std::initializer_list<Element> rhs) noexcept
		: x_pBegin(rhs.begin()), x_uSize(rhs.size())
	{
	}

public:
	constexpr Element *GetBegin() const noexcept {
		return x_pBegin;
	}
	constexpr Element *GetEnd() const noexcept {
		return x_pBegin + x_uSize;
	}
	constexpr Element *GetData() const noexcept {
		return x_pBegin;
	}
	constexpr std::size_t GetSize() const noexcept {
		return x_uSize;
	}

	Element &Get(std::size_t uIndex) const {
		if(uIndex >= x_uSize){
			DEBUG_THROW(Exception, ERROR_ACCESS_DENIED, "ArrayView: Subscript out of range"_rcs);
		}
		return UncheckedGet(uIndex);
	}
	Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < x_uSize);

		return x_pBegin[uIndex];
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator Element *() const noexcept {
		return GetData();
	}
	Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}

	friend decltype(auto) begin(const ArrayView &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) cbegin(const ArrayView &rhs) noexcept {
		return rhs.GetBegin();
	}
	friend decltype(auto) end(const ArrayView &rhs) noexcept {
		return rhs.GetEnd();
	}
	friend decltype(auto) cend(const ArrayView &rhs) noexcept {
		return rhs.GetEnd();
	}
};

}

#endif
