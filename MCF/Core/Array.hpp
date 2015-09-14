// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ARRAY_HPP_
#define MCF_CORE_ARRAY_HPP_

#include "ArrayObserver.hpp"
#include "../Containers/_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "Exception.hpp"
#include <utility>
#include <initializer_list>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<typename ElementT, std::size_t kSize>
class Array {
	static_assert(kSize > 0, "An array shall have a non-zero size.");

public:
	using ConstObserver = ArrayObserver<const ElementT>;
	using Observer      = ArrayObserver<      ElementT>;

public:
	ElementT m_aStorage[kSize];

public:
	// 整体仿造容器，唯独没有 Clear()。
	using Element         = ElementT;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <Array>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <Array>;

	constexpr bool IsEmpty() const noexcept {
		return false;
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
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	Element *GetPrev(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		--uOffset;
		return pBegin + uOffset;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == kSize){
			return nullptr;
		}
		return pBegin + uOffset;
	}
	Element *GetNext(Element *pPos) noexcept {
		ASSERT(pPos);

		const auto pBegin = GetBegin();
		auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		++uOffset;
		if(uOffset == kSize){
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

	void Swap(Array &rhs) noexcept(noexcept(std::swap(std::declval<ElementT (&)[kSize]>(), std::declval<ElementT (&)[kSize]>()))) {
		std::swap(m_aStorage, rhs.m_aStorage);
	}

	// Array 需求。
	const Element *GetData() const noexcept {
		return m_aStorage;
	}
	Element *GetData() noexcept {
		return m_aStorage;
	}
	static constexpr std::size_t GetSize() noexcept {
		return kSize;
	}

	const Element *GetBegin() const noexcept {
		return GetData();
	}
	Element *GetBegin() noexcept {
		return GetData();
	}
	const Element *GetConstBegin() const noexcept {
		return GetBegin();
	}
	const Element *GetEnd() const noexcept {
		return GetData() + kSize;
	}
	Element *GetEnd() noexcept {
		return GetData() + kSize;
	}
	const Element *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Element &Get(std::size_t uIndex) const {
		if(uIndex >= kSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	Element &Get(std::size_t uIndex){
		if(uIndex >= kSize){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	const Element &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < kSize);

		return GetData()[uIndex];
	}
	Element &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < kSize);

		return GetData()[uIndex];
	}

	const Element &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Element &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

public:
	ConstObserver GetObserver() const noexcept {
		return ConstObserver(GetData(), GetSize());
	}
	Observer GetObserver() noexcept {
		return Observer(GetData(), GetSize());
	}

public:
	operator ConstObserver() const noexcept {
		return GetObserver();
	}
	operator Observer() noexcept {
		return GetObserver();
	}

	explicit operator const ElementT *() const noexcept {
		return GetData();
	}
	explicit operator ElementT *() noexcept {
		return GetData();
	}
};

template<typename ElementT, std::size_t kCapacity>
void swap(Array<ElementT, kCapacity> &lhs, Array<ElementT, kCapacity> &rhs) noexcept(noexcept(lhs.Swap(rhs))) {
	lhs.Swap(rhs);
}

template<typename ElementT, std::size_t kCapacity>
decltype(auto) begin(const Array<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) begin(Array<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) cbegin(const Array<ElementT, kCapacity> &rhs) noexcept {
	return begin(rhs);
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) end(const Array<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) end(Array<ElementT, kCapacity> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename ElementT, std::size_t kCapacity>
decltype(auto) cend(const Array<ElementT, kCapacity> &rhs) noexcept {
	return end(rhs);
}

}

#endif
