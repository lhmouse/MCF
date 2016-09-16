// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FIXED_SIZE_ALLOCATOR_HPP_
#define MCF_CORE_FIXED_SIZE_ALLOCATOR_HPP_

#include "../Config.hpp"
#include "Atomic.hpp"
#include "DefaultAllocator.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

template<std::size_t kElementSizeT, class BackingAllocatorT = DefaultAllocator>
struct FixedSizeAllocator {
public:
	enum : std::size_t {
		kElementSize = kElementSizeT,
	};

	using BackingAllocator = BackingAllocatorT;

private:
	union X_Block {
		X_Block *pNext;
		unsigned char abyData[kElementSize];
	};
	struct alignas(2 * alignof(void *)) X_Control {
		X_Block *pFirst;
		X_Block *pLast;
	};

private:
	Atomic<X_Control> x_vControl;

public:
	constexpr FixedSizeAllocator() noexcept
		: x_vControl(X_Control{ nullptr, nullptr })
	{
	}
	~FixedSizeAllocator(){
		Recycle();
	}

	FixedSizeAllocator(const FixedSizeAllocator &) noexcept = delete;
	FixedSizeAllocator &operator=(const FixedSizeAllocator &) noexcept = delete;

private:
	void X_Attach(const X_Control &vNewControl) noexcept {
		auto vControl = x_vControl.Load(kAtomicRelaxed);
		do {
			vNewControl.pLast->pNext = vControl.pFirst;
		} while(!x_vControl.CompareExchange(vControl, vNewControl, kAtomicRelaxed));
	}
	X_Control X_Detach() noexcept {
		return x_vControl.Exchange(X_Control{ nullptr, nullptr }, kAtomicRelaxed);
	}

	X_Block *X_PooledAllocate() noexcept {
		const auto vControl = X_Detach();
		const auto pBlock = vControl.pFirst;
		if(pBlock){
			const auto pNext = pBlock->pNext;
			if(pNext){
				X_Attach(X_Control{ pNext, vControl.pLast });
			}
		}
		return pBlock;
	}
	void X_PooledDeallocate(X_Block *pBlock) noexcept {
		X_Attach(X_Control{ pBlock, pBlock });
	}

public:
	__attribute__((__malloc__))
	void *Allocate(){
		auto pBlock = X_PooledAllocate();
		if(!pBlock){
			pBlock = static_cast<X_Block *>(BackingAllocator()(sizeof(X_Block)));
		}
#ifndef NDEBUG
		__builtin_memset(pBlock, 0xD7, sizeof(X_Block));
#endif
		return pBlock;
	}
	void *AllocateNothrow() noexcept {
		auto pBlock = X_PooledAllocate();
		if(!pBlock){
			pBlock = static_cast<X_Block *>(BackingAllocator()(std::nothrow, sizeof(X_Block)));
			if(!pBlock){
				return nullptr;
			}
		}
#ifndef NDEBUG
		__builtin_memset(pBlock, 0xD7, sizeof(X_Block));
#endif
		return pBlock;
	}
	void Deallocate(void *pRaw) noexcept {
		if(!pRaw){
			return;
		}
		X_PooledDeallocate(static_cast<X_Block *>(pRaw));
	}

	void Recycle() noexcept {
		const auto vControl = X_Detach();
		auto pBlock = vControl.pFirst;
		while(pBlock){
			const auto pNext = pBlock->pNext;
			BackingAllocator()(static_cast<void *>(pBlock));
			pBlock = pNext;
		}
	}
	void Adopt(FixedSizeAllocator &rhs) noexcept {
		const auto vControl = rhs.X_Detach();
		if(vControl.pFirst){
			X_Attach(vControl);
		}
	}
	void Adopt(FixedSizeAllocator &&rhs) noexcept {
		Adopt(rhs);
	}
};

}

#endif
