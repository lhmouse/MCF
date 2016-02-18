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

namespace Impl_Atomic {
	template<typename ElementT>
	class AtomicCommon {
		static_assert(std::is_object<ElementT>::value,          "Only object types are supported.");
		static_assert(std::is_trivial<ElementT>::value,         "This object type is not trivial.");
		static_assert(sizeof(ElementT)  <= 2 * sizeof(void *),  "The size of this object type is too large.");
		static_assert(alignof(ElementT) <= 2 * alignof(void *), "The alignment of this object type is too strict.");

	protected:
		volatile ElementT x_vElement;

	public:
		explicit constexpr AtomicCommon(ElementT vElement) noexcept
			: x_vElement(vElement)
		{
		}

		AtomicCommon(const AtomicCommon &) noexcept = delete;
		AtomicCommon &operator=(const AtomicCommon &) noexcept = delete;

	public:
		const volatile ElementT &Get() const volatile noexcept {
			return x_vElement;
		}
		volatile ElementT &Get() volatile noexcept {
			return x_vElement;
		}

		ElementT Load(MemoryModel eModel) const volatile noexcept {
			ElementT vRet;
			__atomic_load(AddressOf(x_vElement), &vRet, static_cast<int>(eModel));
			return vRet;
		}
		void Store(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			__atomic_store(AddressOf(x_vElement), &vOperand, static_cast<int>(eModel));
		}

		bool CompareExchange(ElementT &vComparand, ElementT vExchangeWith, MemoryModel eSuccessModel, MemoryModel eFailureModel) volatile noexcept {
			return __atomic_compare_exchange(AddressOf(x_vElement), &vComparand, &vExchangeWith, false, static_cast<int>(eSuccessModel), static_cast<int>(eFailureModel));
		}
		ElementT Exchange(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			ElementT vRet;
			__atomic_exchange(AddressOf(x_vElement), &vOperand, &vRet, static_cast<int>(eModel));
			return vRet;
		}

		bool CompareExchange(ElementT &vComparand, ElementT vExchangeWith, MemoryModel eModel) volatile noexcept {
			switch(__builtin_constant_p(eModel) ? eModel : kAtomicSeqCst){
			case kAtomicAcqRel:
				return CompareExchange(vComparand, vExchangeWith, eModel, kAtomicAcquire);
			case kAtomicRelease:
				return CompareExchange(vComparand, vExchangeWith, eModel, kAtomicRelaxed);
			default:
				return CompareExchange(vComparand, vExchangeWith, eModel, eModel);
			}
		}
	};

	template<typename ElementT>
	class AtomicInteger final : public AtomicCommon<ElementT> {
		static_assert(std::is_integral<ElementT>::value || std::is_enum<ElementT>::value, "!");

	public:
		explicit constexpr AtomicInteger(ElementT vElement = 0) noexcept
			: AtomicCommon<ElementT>(vElement)
		{
		}

	public:
		ElementT AndFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_and_fetch(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}
		ElementT FetchAnd(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_and(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}

		ElementT OrFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_or_fetch(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}
		ElementT FetchOr(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_or(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}

		ElementT XorFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_xor_fetch(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}
		ElementT FetchXor(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_xor(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}

		ElementT AddFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_add_fetch(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}
		ElementT FetchAdd(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_add(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}

		ElementT SubFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_sub_fetch(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}
		ElementT FetchSub(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_sub(AddressOf(this->x_vElement), vOperand, static_cast<int>(eModel));
		}

		ElementT Increment(MemoryModel eModel) volatile noexcept {
			return AddFetch(1, eModel);
		}
		ElementT Decrement(MemoryModel eModel) volatile noexcept {
			return SubFetch(1, eModel);
		}
	};

	template<>
	class AtomicInteger<bool> final : public AtomicCommon<bool> {
	public:
		explicit constexpr AtomicInteger(bool bElement = 0) noexcept
			: AtomicCommon<bool>(bElement)
		{
		}

	public:
		bool AndFetch(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_and_fetch(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}
		bool FetchAnd(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_and(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}

		bool OrFetch(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_or_fetch(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}
		bool FetchOr(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_or(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}

		bool XorFetch(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_xor_fetch(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}
		bool FetchXor(bool bOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_xor(AddressOf(this->x_vElement), bOperand, static_cast<int>(eModel));
		}
	};

	template<typename ElementT>
	class AtomicPointerToObject final : public AtomicCommon<ElementT> {
		static_assert(std::is_pointer<ElementT>::value && std::is_object<std::remove_pointer_t<ElementT>>::value, "!");

	public:
		explicit constexpr AtomicPointerToObject(ElementT pElement = nullptr) noexcept
			: AtomicCommon<ElementT>(pElement)
		{
		}

	public:
		ElementT AddFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_add_fetch(AddressOf(this->x_vElement), nOperand, static_cast<int>(eModel));
		}
		ElementT FetchAdd(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_add(AddressOf(this->x_vElement), nOperand, static_cast<int>(eModel));
		}

		ElementT SubFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_sub_fetch(AddressOf(this->x_vElement), nOperand, static_cast<int>(eModel));
		}
		ElementT FetchSub(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_sub(AddressOf(this->x_vElement), nOperand, static_cast<int>(eModel));
		}

		ElementT Increment(MemoryModel eModel) volatile noexcept {
			return AddFetch(1, eModel);
		}
		ElementT Decrement(MemoryModel eModel) volatile noexcept {
			return SubFetch(1, eModel);
		}
	};

	template<typename ElementT>
	class AtomicPointerToNonObject final : public AtomicCommon<ElementT> {
		static_assert(std::is_pointer<ElementT>::value && !std::is_object<std::remove_pointer_t<ElementT>>::value, "!");

	public:
		explicit constexpr AtomicPointerToNonObject(ElementT pElement = nullptr) noexcept
			: AtomicCommon<ElementT>(pElement)
		{
		}
	};
}

template<typename ElementT>
using Atomic =
	std::conditional_t<std::is_integral<ElementT>::value || std::is_enum<ElementT>::value,
		Impl_Atomic::AtomicInteger<ElementT>,
		std::conditional_t<std::is_pointer<ElementT>::value,
			std::conditional_t<std::is_object<std::remove_pointer_t<ElementT>>::value,
				Impl_Atomic::AtomicPointerToObject<ElementT>,
				Impl_Atomic::AtomicPointerToNonObject<ElementT>>,
			Impl_Atomic::AtomicCommon<ElementT>
		>>;

inline void AtomicFence(MemoryModel eModel) noexcept {
	__atomic_thread_fence(static_cast<int>(eModel));
}

inline void AtomicPause() noexcept {
	__builtin_ia32_pause();
}

}

#endif
