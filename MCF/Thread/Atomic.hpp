// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_ATOMIC_HPP_
#define MCF_THREAD_ATOMIC_HPP_

#include <type_traits>

namespace MCF {

enum class MemoryModel {
	kRelaxed	= __ATOMIC_RELAXED,
	kConsume	= __ATOMIC_CONSUME,
	kAcquire	= __ATOMIC_ACQUIRE,
	kRelease	= __ATOMIC_RELEASE,
	kAcqRel		= __ATOMIC_ACQ_REL,
	kSeqCst		= __ATOMIC_SEQ_CST,
};

template<typename T>
inline T AtomicLoad(volatile T &ref, MemoryModel model) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	return __atomic_load_n(&ref, static_cast<int>(model));
}
template<typename T>
inline void AtomicStore(volatile T &ref, std::common_type_t<T> val, MemoryModel model) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	__atomic_store_n(&ref, val, static_cast<int>(model));
}

template<typename T>
inline T AtomicAdd(volatile T &ref, std::common_type_t<T> val, MemoryModel model) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	return __atomic_add_fetch(&ref, val, static_cast<int>(model));
}
template<typename T>
inline T AtomicSub(volatile T &ref, std::common_type_t<T> val, MemoryModel model) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	return __atomic_sub_fetch(&ref, val, static_cast<int>(model));
}

template<typename T>
inline bool AtomicCompareExchange(volatile T &ref, std::common_type_t<T> &cmp, std::common_type_t<T> xchg, MemoryModel succ, MemoryModel fail) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	return __atomic_compare_exchange_n(&ref, &cmp, xchg, false, static_cast<int>(succ), static_cast<int>(fail));
}
template<typename T>
inline T AtomicExchange(volatile T &ref, std::common_type_t<T> val, MemoryModel model) noexcept {
	static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "Only integers and pointers are supported.");
	return __atomic_exchange_n(&ref, val, static_cast<int>(model));
}

inline void AtomicFence(MemoryModel model) noexcept {
	__atomic_thread_fence(static_cast<int>(model));
}

inline void AtomicPause() throw() { // FIXME: g++ 4.9.2 ICE.
	__builtin_ia32_pause();
}

template<typename T>
inline T AtomicIncrement(volatile T &ref, MemoryModel model) noexcept {
	return AtomicAdd(ref, 1, model);
}
template<typename T>
inline T AtomicDecrement(volatile T &ref, MemoryModel model) noexcept {
	return AtomicSub(ref, 1, model);
}

template<typename T>
inline bool AtomicCompareExchange(volatile T &ref, std::common_type_t<T> &cmp, std::common_type_t<T> xchg, MemoryModel model) noexcept {
	if(model == MemoryModel::kAcqRel){
		return AtomicCompareExchange(ref, cmp, xchg, model, MemoryModel::kAcquire);
	} else if(model == MemoryModel::kRelease){
		return AtomicCompareExchange(ref, cmp, xchg, model, MemoryModel::kRelaxed);
	} else {
		return AtomicCompareExchange(ref, cmp, xchg, model, model);
	}
}

}

#endif
