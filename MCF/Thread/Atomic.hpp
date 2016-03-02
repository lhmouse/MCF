// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_ATOMIC_HPP_
#define MCF_THREAD_ATOMIC_HPP_

#include <type_traits>
#include <cstddef>
#include "../Utilities/AddressOf.hpp"

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
	static_assert(std::is_object<ElementT>::value,          "Only object types are supported.");
	static_assert(std::is_trivial<ElementT>::value,         "This object type is not trivial.");
	static_assert(sizeof(ElementT)  <= 2 * sizeof(void *),  "The size of this object type is too large.");
	static_assert(alignof(ElementT) <= 2 * alignof(void *), "The alignment of this object type is too strict.");

protected:
	volatile ElementT x_vElement;

public:
	explicit constexpr Atomic(ElementT vElement = ElementT()) noexcept
		: x_vElement(vElement)
	{
	}

	Atomic(const Atomic &) noexcept = delete;
	Atomic &operator=(const Atomic &) noexcept = delete;

public:
	const volatile ElementT &Get() const volatile noexcept {
		return x_vElement;
	}
	volatile ElementT &Get() volatile noexcept {
		return x_vElement;
	}

	ElementT Load(MemoryModel eModel) const volatile noexcept {
		ElementT vRet;
		__atomic_load(AddressOf(x_vElement), AddressOf(vRet), static_cast<int>(eModel));
		return vRet;
	}
	void Store(ElementT vOperand, MemoryModel eModel) volatile noexcept {
		__atomic_store(AddressOf(x_vElement), AddressOf(vOperand), static_cast<int>(eModel));
	}

	bool CompareExchange(ElementT &vComparand, ElementT vExchangeWith, MemoryModel eSuccessModel, MemoryModel eFailureModel) volatile noexcept {
		return __atomic_compare_exchange(AddressOf(x_vElement), AddressOf(vComparand), AddressOf(vExchangeWith), false, static_cast<int>(eSuccessModel), static_cast<int>(eFailureModel));
	}
	ElementT Exchange(ElementT vOperand, MemoryModel eModel) volatile noexcept {
		ElementT vRet;
		__atomic_exchange(AddressOf(x_vElement), AddressOf(vOperand), AddressOf(vRet), static_cast<int>(eModel));
		return vRet;
	}

	bool CompareExchange(ElementT &vComparand, ElementT vExchangeWith, MemoryModel eModel) volatile noexcept {
		constexpr auto eRealModel = __builtin_constant_p(eModel) ? eModel : kAtomicSeqCst;
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
		ElementT> AndFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_and_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		ElementT> FetchAnd(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_and(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		ElementT> OrFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_or_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		ElementT> FetchOr(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_or(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		ElementT> XorFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_xor_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value,
		ElementT> FetchXor(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_xor(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		ElementT> AddFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_add_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		ElementT> FetchAdd(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_add(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		ElementT> SubFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_sub_fetch(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_integral<T>::value && !std::is_same<T, bool>::value,
		ElementT> FetchSub(ElementT vOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_sub(AddressOf(x_vElement), vOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		ElementT> AddFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_add_fetch(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		ElementT> FetchAdd(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_add(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		ElementT> SubFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_sub_fetch(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}
	template<typename T = ElementT>
	std::enable_if_t<
		std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value,
		ElementT> FetchSub(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept
	{
		return __atomic_fetch_sub(AddressOf(x_vElement), nOperand, static_cast<int>(eModel));
	}

	template<typename T = ElementT>
	std::enable_if_t<
		(std::is_integral<T>::value && !std::is_same<T, bool>::value) || (std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value),
		ElementT> Increment(MemoryModel eModel) volatile noexcept
	{
		return AddFetch(1, eModel);
	}
	template<typename T = ElementT>
	std::enable_if_t<
		(std::is_integral<T>::value && !std::is_same<T, bool>::value) || (std::is_pointer<T>::value && std::is_object<std::remove_pointer_t<T>>::value),
		ElementT> Decrement(MemoryModel eModel) volatile noexcept
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
