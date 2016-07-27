// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_VIEW_HPP_
#define MCF_CORE_ARRAY_VIEW_HPP_

#include "_Enumerator.hpp"
#include "Exception.hpp"
#include "Assert.hpp"
#include "CountOf.hpp"
#include "AddressOf.hpp"
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

template<typename ElementT>
class ArrayView {
public:
	// 容器需求。
	using Element         = ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <ArrayView>;
	using Enumerator      = Impl_Enumerator::Enumerator      <ArrayView>;

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
	// 容器需求。
	constexpr bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear(){
		x_uSize = 0;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		for(std::size_t i = 0; i < x_uSize; ++i){
			*itOutput = std::move(x_pBegin[i]);
			++itOutput;
		}
		return itOutput;
	}

	constexpr const Element *GetFirst() const noexcept {
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		return GetBegin();
	}
	constexpr const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	constexpr const Element *GetLast() const noexcept {
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		return GetEnd() - 1;
	}
	constexpr const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(ArrayView &rhs) noexcept {
		using std::swap;
		swap(x_pBegin, rhs.x_pBegin);
		swap(x_uSize,  rhs.x_uSize);
	}

	// ArrayView 需求。
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
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"ArrayView: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_ASSERT(uIndex < x_uSize);

		return x_pBegin[uIndex];
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
	// 容器需求。
	using Element         = const ElementT;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <ArrayView>;
	using Enumerator      = Impl_Enumerator::Enumerator      <ArrayView>;

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
	// 容器需求。
	constexpr bool IsEmpty() const noexcept {
		return x_uSize == 0;
	}
	void Clear(){
		x_uSize = 0;
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		for(std::size_t i = 0; i < x_uSize; ++i){
			*itOutput = std::move(x_pBegin[i]);
			++itOutput;
		}
		return itOutput;
	}

	constexpr const Element *GetFirst() const noexcept {
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		return GetBegin();
	}
	constexpr const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	constexpr const Element *GetLast() const noexcept {
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		return GetEnd() - 1;
	}
	constexpr const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		MCF_ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == x_uSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(ArrayView &rhs) noexcept {
		using std::swap;
		swap(x_pBegin, rhs.x_pBegin);
		swap(x_uSize,  rhs.x_uSize);
	}

	// ArrayView 需求。
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
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"ArrayView: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Element &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_ASSERT(uIndex < x_uSize);

		return x_pBegin[uIndex];
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
