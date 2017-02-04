// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_UNIQUE_LOCK_BASE_HPP_
#define MCF_THREAD_UNIQUE_LOCK_BASE_HPP_

#include "../Core/AddressOf.hpp"
#include "../Core/Assert.hpp"
#include <utility>
#include <cstdint>

namespace MCF {

template<typename MutexT>
struct DefaultMutexTraits {
	static bool Try(MutexT *pMutex, std::uint64_t u64UntilFastMonoClock){
		return pMutex->Try(u64UntilFastMonoClock);
	}
	static void Lock(MutexT *pMutex){
		pMutex->Lock();
	}
	static void Unlock(MutexT *pMutex){
		pMutex->Unlock();
	}
};

template<typename MutexT, typename TraitsT = DefaultMutexTraits<MutexT>>
class UniqueLock {
public:
	using Mutex  = MutexT;
	using Traits = TraitsT;

private:
	Mutex *x_pMutex;

public:
	constexpr UniqueLock() noexcept
		: x_pMutex(nullptr)
	{
	}
	explicit constexpr UniqueLock(MutexT *pMutex) noexcept
		: x_pMutex(pMutex)
	{
	}
	UniqueLock(MutexT &vMutex, std::uint64_t u64UntilFastMonoClock) noexcept
		: x_pMutex((Traits::Try(AddressOf(vMutex), u64UntilFastMonoClock) ? AddressOf(vMutex) : nullptr))
	{
	}
	explicit UniqueLock(MutexT &vMutex)
		: x_pMutex((Traits::Lock(AddressOf(vMutex)), AddressOf(vMutex)))
	{
	}
	UniqueLock(UniqueLock &&rhs) noexcept
		: x_pMutex(rhs.Release())
	{
	}
	UniqueLock &operator=(UniqueLock &&rhs) noexcept {
		return Reset(std::move(rhs));
	}
	~UniqueLock(){
		const auto pMutex = x_pMutex;
		if(pMutex){
			Traits::Unlock(pMutex);
		}
#ifndef NDEBUG
		__builtin_memset(&x_pMutex, 0xE1, sizeof(x_pMutex));
#endif
	}

public:
	constexpr bool IsNull() const noexcept {
		return x_pMutex == nullptr;
	}

	constexpr Mutex *GetMutex() const noexcept {
		return x_pMutex;
	}
	Mutex *Release() noexcept {
		return std::exchange(x_pMutex, nullptr);
	}

	UniqueLock &Reset() noexcept {
		UniqueLock().Swap(*this);
		return *this;
	}
	UniqueLock &TryReset(MutexT &vMutex, std::uint64_t u64UntilFastMonoClock) noexcept {
		UniqueLock(vMutex, u64UntilFastMonoClock).Swap(*this);
		return *this;
	}
	UniqueLock &Reset(MutexT &vMutex){
		UniqueLock(vMutex).Swap(*this);
		return *this;
	}
	UniqueLock &Reset(UniqueLock &&rhs) noexcept {
		UniqueLock(std::move(rhs)).Swap(*this);
		return *this;
	}

	void Swap(UniqueLock &rhs) noexcept {
		using std::swap;
		swap(x_pMutex, rhs.x_pMutex);
	}

public:
	explicit constexpr operator bool() const noexcept {
		return !IsNull();
	}

	friend void swap(UniqueLock &lhs, UniqueLock &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
