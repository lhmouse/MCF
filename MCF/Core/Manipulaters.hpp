// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

// 这是用于 UniqueHandle 和 SharedHandle 的支持文件。
// 请勿直接包含。

#ifndef __MCF_MANIPULATERS_HPP__
#define __MCF_MANIPULATERS_HPP__

#include <new>
#include <cstddef>

namespace MCF {

namespace __MCF {
	template<typename VALUE_T>
	class NormalManipulater {
	public:
		template<typename OTHER_VALUE_T>
		struct Rebind {
			typedef NormalManipulater<OTHER_VALUE_T> Type;
		};
	private:
		VALUE_T xm_Val;
	public:
		NormalManipulater(VALUE_T Val){
			xm_Val = Val;
		}

		NormalManipulater(NormalManipulater &&) = delete;
		void operator=(NormalManipulater &&) = delete;
	public:
		VALUE_T Get() const noexcept {
			return xm_Val;
		}
		operator VALUE_T() const noexcept {
			return Get();
		}
		void operator=(VALUE_T NewVal) noexcept {
			xm_Val = NewVal;
		}
		VALUE_T Exchange(VALUE_T NewVal) noexcept {
			const VALUE_T OldVal = xm_Val;
			xm_Val = NewVal;
			return OldVal;
		}
		VALUE_T operator++() noexcept {
			return ++xm_Val;
		}
		VALUE_T operator--() noexcept {
			return --xm_Val;
		}
		bool CompareExchange(VALUE_T Comparand, VALUE_T NewVal) noexcept {
			const bool bRet = (xm_Val == Comparand);
			xm_Val = bRet ? NewVal : xm_Val;
			return bRet;
		}
	};

	template<typename VALUE_T>
	class AtomicManipulater {
	public:
		template<typename OTHER_VALUE_T>
		struct Rebind {
			typedef AtomicManipulater<OTHER_VALUE_T> Type;
		};
	private:
		VALUE_T xm_Val;
	public:
		AtomicManipulater(VALUE_T Val){
			__atomic_store_n(&xm_Val, Val, __ATOMIC_SEQ_CST);
		}

		AtomicManipulater(AtomicManipulater &&) = delete;
		void operator=(AtomicManipulater &&) = delete;
	public:
		VALUE_T Get() const noexcept {
			return __atomic_load_n(&xm_Val, __ATOMIC_SEQ_CST);
		}
		explicit operator VALUE_T() const noexcept {
			return Get();
		}
		void operator=(VALUE_T NewVal) noexcept {
			__atomic_store_n(&xm_Val, NewVal, __ATOMIC_SEQ_CST);
		}
		VALUE_T Exchange(VALUE_T NewVal) noexcept {
			return __atomic_exchange_n(&xm_Val, NewVal, __ATOMIC_SEQ_CST);
		}
		VALUE_T operator++() noexcept {
			return __atomic_add_fetch(&xm_Val, 1, __ATOMIC_SEQ_CST);
		}
		VALUE_T operator--() noexcept {
			return __atomic_sub_fetch(&xm_Val, 1, __ATOMIC_SEQ_CST);
		}
		bool CompareExchange(VALUE_T Comparand, VALUE_T NewVal) noexcept {
			return __atomic_compare_exchange_n(&xm_Val, &Comparand, NewVal, true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
		}
	};
}

}

#endif
