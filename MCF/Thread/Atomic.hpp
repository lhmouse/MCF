// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_ATOMIC_HPP_
#define MCF_THREAD_ATOMIC_HPP_

#include <type_traits>
#include <cstddef>

namespace MCF {

enum MemoryModel {
	kAtomicRelaxed	= __ATOMIC_RELAXED,
	kAtomicConsume	= __ATOMIC_CONSUME,
	kAtomicAcquire	= __ATOMIC_ACQUIRE,
	kAtomicRelease	= __ATOMIC_RELEASE,
	kAtomicAcqRel	= __ATOMIC_ACQ_REL,
	kAtomicSeqCst	= __ATOMIC_SEQ_CST,
};

namespace Impl_Atomic {
	template<typename ElementT>
	class AtomicCommon {
	protected:
		volatile ElementT x_vElement;

	protected:
		explicit constexpr AtomicCommon(ElementT vElement) noexcept
			: x_vElement(vElement)
		{
		}

		AtomicCommon(const AtomicCommon &) noexcept = delete;
		AtomicCommon &operator=(const AtomicCommon &) noexcept = delete;

	public:
		ElementT Load(MemoryModel eModel) const volatile noexcept {
			return __atomic_load_n(&this->x_vElement, static_cast<int>(eModel));
		}
		void Store(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			__atomic_store_n(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		bool CompareExchange(ElementT &vComparand, ElementT vExchangeWith, MemoryModel eSuccessModel, MemoryModel eFailureModel) volatile noexcept {
			return __atomic_compare_exchange_n(&this->x_vElement, &vComparand, vExchangeWith, false, static_cast<int>(eSuccessModel), static_cast<int>(eFailureModel));
		}
		ElementT Exchange(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_exchange_n(&this->x_vElement, vOperand, static_cast<int>(eModel));
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
	private:
		using xBase = AtomicCommon<ElementT>;

	public:
		explicit constexpr AtomicInteger(ElementT vElement = 0) noexcept
			: xBase(vElement)
		{
		}

	public:
		ElementT AndFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_and_fetch(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}
		ElementT FetchAnd(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_and(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		ElementT OrFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_or_fetch(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}
		ElementT FetchOr(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_or(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		ElementT XorFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_xor_fetch(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}
		ElementT FetchXor(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_xor(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		ElementT AddFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_add_fetch(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}
		ElementT FetchAdd(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_add(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		ElementT SubFetch(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_sub_fetch(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}
		ElementT FetchSub(ElementT vOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_sub(&this->x_vElement, vOperand, static_cast<int>(eModel));
		}

		ElementT Increment(MemoryModel eModel) volatile noexcept {
			return AddFetch(1, eModel);
		}
		ElementT Decrement(MemoryModel eModel) volatile noexcept {
			return SubFetch(1, eModel);
		}
	};

	template<typename ElementT>
	class AtomicPointerToObject final : public AtomicCommon<ElementT> {
	private:
		using xBase = AtomicCommon<ElementT>;

	public:
		explicit constexpr AtomicPointerToObject(ElementT pElement = nullptr) noexcept
			: xBase(pElement)
		{
		}

	public:
		ElementT AddFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_add_fetch(&this->x_vElement, nOperand, static_cast<int>(eModel));
		}
		ElementT FetchAdd(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_add(&this->x_vElement, nOperand, static_cast<int>(eModel));
		}

		ElementT SubFetch(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_sub_fetch(&this->x_vElement, nOperand, static_cast<int>(eModel));
		}
		ElementT FetchSub(std::ptrdiff_t nOperand, MemoryModel eModel) volatile noexcept {
			return __atomic_fetch_sub(&this->x_vElement, nOperand, static_cast<int>(eModel));
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
	private:
		using xBase = AtomicCommon<ElementT>;

	public:
		explicit constexpr AtomicPointerToNonObject(ElementT pElement = nullptr) noexcept
			: xBase(pElement)
		{
		}
	};

	template<typename ElementT>
	class AtomicError final {
		static_assert(((void)sizeof(ElementT), false), "Only integer, enumeration and pointer types are supported.");
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
			Impl_Atomic::AtomicError<ElementT>
		>>;

inline void AtomicFence(MemoryModel eModel) noexcept {
	__atomic_thread_fence(static_cast<int>(eModel));
}

inline void AtomicPause() throw() { // FIXME: g++ 4.9.2 ICE.
	__builtin_ia32_pause();
}

}

#endif
