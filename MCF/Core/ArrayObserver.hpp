// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_OBSERVER_HPP_
#define MCF_CORE_ARRAY_OBSERVER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include <type_traits>
#include <initializer_list>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <cstddef>
#include "StringObserver.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

template<class ElementT>
class ArrayObserver {
private:
	ElementT *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayObserver(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayObserver(ElementT &rhs) noexcept
		: x_pBegin(std::addressof(rhs)), x_uSize(1)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<typename TraitsT, typename AllocatorT>
	constexpr ArrayObserver(std::basic_string<ElementT, TraitsT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<typename AllocatorT>
	constexpr ArrayObserver(std::vector<ElementT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(std::array<ElementT, kSizeT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<StringType kTypeT,
		std::enable_if_t<
			std::is_same<typename StringObserver<kTypeT>::Char, ElementT>::value,
			int> = 0>
	constexpr ArrayObserver(StringObserver<kTypeT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayObserver(Vector<ElementT> &rhs) noexcept
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
class ArrayObserver<const ElementT> {
private:
	const ElementT *x_pBegin;
	std::size_t x_uSize;

public:
	constexpr ArrayObserver(std::nullptr_t = nullptr) noexcept
		: x_pBegin(nullptr), x_uSize(0)
	{
	}
	constexpr ArrayObserver(const ElementT &rhs) noexcept
		: x_pBegin(std::addressof(rhs)), x_uSize(1)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(const ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(ElementT (&rhs)[kSizeT]) noexcept
		: x_pBegin(rhs), x_uSize(kSizeT)
	{
	}
	template<typename TraitsT, typename AllocatorT>
	constexpr ArrayObserver(const std::basic_string<const ElementT, TraitsT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<typename TraitsT, typename AllocatorT>
	constexpr ArrayObserver(const std::basic_string<ElementT, TraitsT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<typename AllocatorT>
	constexpr ArrayObserver(const std::vector<const ElementT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<typename AllocatorT>
	constexpr ArrayObserver(const std::vector<ElementT, AllocatorT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(const std::array<const ElementT, kSizeT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<std::size_t kSizeT>
	constexpr ArrayObserver(const std::array<ElementT, kSizeT> &rhs) noexcept
		: x_pBegin(rhs.data()), x_uSize(rhs.size())
	{
	}
	template<StringType kTypeT,
		std::enable_if_t<
			std::is_same<typename StringObserver<kTypeT>::Char, ElementT>::value,
			int> = 0>
	constexpr ArrayObserver(const StringObserver<kTypeT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayObserver(const Vector<const ElementT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayObserver(const Vector<ElementT> &rhs) noexcept
		: x_pBegin(rhs.GetData()), x_uSize(rhs.GetSize())
	{
	}
	constexpr ArrayObserver(std::initializer_list<const ElementT> rhs) noexcept
		: x_pBegin(rhs.begin()), x_uSize(rhs.size())
	{
	}
	constexpr ArrayObserver(std::initializer_list<ElementT> rhs) noexcept
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
auto begin(const ArrayObserver<ElementT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<class ElementT>
auto cbegin(const ArrayObserver<ElementT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<class ElementT>
auto end(const ArrayObserver<ElementT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<class ElementT>
auto cend(const ArrayObserver<ElementT> &rhs) noexcept {
	return rhs.GetEnd();
}

}

#endif
