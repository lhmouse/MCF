// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ATOMIC_HPP_
#define MCF_CORE_ATOMIC_HPP_

#include <type_traits>
#include <cstddef>
#include "AddressOf.hpp"

namespace MCF {

enum MemoryModel {
	kAtomicRelaxed = __ATOMIC_RELAXED,
	kAtomicConsume = __ATOMIC_CONSUME,
	kAtomicAcquire = __ATOMIC_ACQUIRE,
	kAtomicRelease = __ATOMIC_RELEASE,
	kAtomicAcqRel  = __ATOMIC_ACQ_REL,
	kAtomicSeqCst  = __ATOMIC_SEQ_CST,
};

template<typename ElementT>
class Atomic {
public:
	using Element = ElementT;

	static_assert(std::is_object<Element>::value,          "Only object types are supported.");
	static_assert(std::is_trivial<Element>::value,         "This object type is not trivial.");
	static_assert(sizeof(Element)  <= 2 * sizeof(void *),  "The size of this object type is too large.");
	static_assert(alignof(Element) <= 2 * alignof(void *), "The alignment of this object type is too strict.");

private:
	Element x_vElement;

public:
	constexpr Atomic() noexcept
		: x_vElement()
	{ }
	explicit constexpr Atomic(const Element &vElement) noexcept
		: x_vElement(vElement)
	{ }

	Atomic(const Atomic &) noexcept = delete;
	Atomic &operator=(const Atomic &) noexcept = delete;

public:
	Element Load(MemoryModel eModel) const volatile noexcept {
		Element vRet;
		__atomic_load(AddressOf(x_vElement), AddressOf(vRet), static_cast<int>(eModel));
		return vRet;
	}
	void Store(Element vOperand, MemoryModel eModel) volatile noexcept {
		__atomic_store(AddressOf(x_vElement), AddressOf(vOperand), static_cast<int>(eModel));
	}

	bool CompareExchange(Element &vComparand, Element vExchangeWith, MemoryModel eSuccessModel, MemoryModel eFailureModel) volatile noexcept {
		return __atomic_compare_exchange(AddressOf(x_vElement), AddressOf(vComparand), AddressOf(vExchangeWith), false, static_cast<int>(eSuccessModel), static_cast<int>(eFailureModel));
	}
	Element Exchange(Element vOperand, MemoryModel eModel) volatile noexcept {
		Element vRet;
		__atomic_exchange(AddressOf(x_vElement), AddressOf(vOperand), AddressOf(vRet), static_cast<int>(eModel));
		return vRet;
	}

	bool CompareExchange(Element &vComparand, Element vExchangeWith, MemoryModel eModel) volatile noexcept {
		const auto eRealModel = __builtin_constant_p(eModel) ? eModel : kAtomicSeqCst;
		if(eRealModel == kAtomicAcqRel){
			return CompareExchange(vComparand, vExchangeWith, eModel, kAtomicAcquire);
		} else if(eRealModel == kAtomicRelease){
			return CompareExchange(vComparand, vExchangeWith, eModel, kAtomicRelaxed);
		}
		return CompareExchange(vComparand, vExchangeWith, eModel, eModel);
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> AndFetch(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_and_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> FetchAnd(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_and(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> OrFetch(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_or_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> FetchOr(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_or(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> XorFetch(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_xor_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		Element> FetchXor(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_xor(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		Element> AddFetch(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_add_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		Element> FetchAdd(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_add(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		Element> SubFetch(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_sub_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		Element> FetchSub(Element vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_sub(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		Element> AddFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_add_fetch(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		Element> FetchAdd(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_add(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		Element> SubFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_sub_fetch(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		Element> FetchSub(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_sub(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		(std::is_integral<T>::value && !std::is_same<T, bool>::value) || (std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value),
		Element> Increment(MemoryModel eModel) volatile noexcept
	{
		return AddFetch(1, eModel);
	}
	template<typename T = ElementT>
	std::enable_if_t<
		(std::is_integral<T>::value && !std::is_same<T, bool>::value) || (std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value),
		Element> Decrement(MemoryModel eModel) volatile noexcept
	{
		return SubFetch(1, eModel);
	}
};

inline void AtomicFence(MemoryModel eModel) noexcept {
	__atomic_thread_fence(static_cast<int>(eModel));
}

inline void AtomicPause() noexcept {
	__builtin_ia32_pause();
}

}

#endif
