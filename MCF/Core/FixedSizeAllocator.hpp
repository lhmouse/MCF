// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FIXED_SIZE_ALLOCATOR_HPP_
#define MCF_CORE_FIXED_SIZE_ALLOCATOR_HPP_

#include "../Thread/Atomic.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

template<std::size_t kSizeT>
struct FixedSizeAllocator {
public:
	enum : std::size_t {
		kSize = kSizeT,
	};

	union Block {
		Block *pNext;
		unsigned char abyData[kSize];
		std::max_align_t vAlignment;
	};

private:
	struct alignas(2 * alignof(void *)) X_Control {
		Block *pFirst;
		Block *pLast;
	};
	Atomic<X_Control> x_vControl;

public:
	constexpr FixedSizeAllocator() noexcept
		: x_vControl(X_Control{ nullptr, nullptr })
	{
	}
	~FixedSizeAllocator(){
		Clear();
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

public:
	__attribute__((__flatten__))
	void *Allocate(){
		const auto vControl = X_Detach();
		const auto pBlock = vControl.pFirst;
		if(!pBlock){
			return ::operator new(sizeof(Block));
		}
		const auto pNext = pBlock->pNext;
		if(pNext){
			const auto vNewControl = X_Control{ pNext, vControl.pLast };
			X_Attach(vNewControl);
		}
		return pBlock;
	}
	__attribute__((__flatten__))
	void Deallocate(void *pRaw) noexcept {
		const auto pBlock = static_cast<Block *>(pRaw);
		if(!pBlock){
			return;
		}
		const auto vControl = X_Control{ pBlock, pBlock };
		X_Attach(vControl);
	}
	__attribute__((__flatten__))
	void Clear() noexcept {
		const auto vControl = X_Detach();
		auto pBlock = vControl.pFirst;
		while(pBlock){
			const auto pNext = pBlock->pNext;
			::operator delete(pBlock);
			pBlock = pNext;
		}
	}
	__attribute__((__flatten__))
	void Splice(FixedSizeAllocator &rhs) noexcept {
		const auto vControl = rhs.X_Detach();
		if(!vControl.pFirst){
			return;
		}
		X_Attach(vControl);
	}
};

}

#endif
