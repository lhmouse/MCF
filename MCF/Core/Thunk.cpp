// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Containers/MultiIndexedMap.hpp"
#include "../Thread/CriticalSection.hpp"
#include <memory>
#include <exception>
using namespace MCF;

namespace {

typedef MultiIndexedMap<
	std::pair<
		std::shared_ptr<void>,	// Chunk。内存是以 64KiB 的粒度分配的，每一块称为一个 chunk。
		std::size_t 			// Thunk 大小。
		>,
	void *, 					// Thunk 地址。
	std::size_t					// 在空闲 thunk 上等于大小，否则为零。
	> ThunkMap;

std::size_t	g_uPageOffsetBits = 0;
const auto	g_pcsLock = CriticalSection::Create();
ThunkMap	g_mapThunks;

struct ThunkDeallocator {
	void operator()(const void *pThunk) const noexcept {
		STATIC_ASSERT_NOEXCEPT_BEGIN
		{
			auto vLock = g_pcsLock->GetLock();

			auto pCurrentThunk = g_mapThunks.Find<0>(pThunk);
			ASSERT(pCurrentThunk && (pCurrentThunk->GetIndex<1>() == 0));

			const auto pNextThunk = pCurrentThunk->GetNext<0>();
			if(
				pNextThunk &&
				(pNextThunk->GetElement().first == pCurrentThunk->GetElement().first) &&
				(pNextThunk->GetIndex<1>() != 0) &&
				(const unsigned char *)pCurrentThunk->GetIndex<0>() + pCurrentThunk->GetElement().second == pNextThunk->GetIndex<0>()
			){
				// 如果连续的下一个 thunk 也在缓存中，把它合并到当前 thunk。
				pCurrentThunk->GetElement().second += pNextThunk->GetElement().second;
				g_mapThunks.Erase(pNextThunk);
			}
			const auto pPrevThunk = pCurrentThunk->GetPrev<0>();
			if(
				pPrevThunk &&
				(pPrevThunk->GetElement().first == pCurrentThunk->GetElement().first) &&
				(pPrevThunk->GetIndex<1>() != 0) &&
				(const unsigned char *)pPrevThunk->GetIndex<0>() + pPrevThunk->GetElement().second == pCurrentThunk->GetIndex<0>()
			){
				// 如果连续的前一个 thunk 也在缓存中，把当前 thunk 合并到它。
				pPrevThunk->GetElement().second += pCurrentThunk->GetElement().second;
				g_mapThunks.Erase(pCurrentThunk);
				pCurrentThunk = pPrevThunk;
			}
			if(!pCurrentThunk->GetElement().first.unique()){
				g_mapThunks.SetIndex<1>(pCurrentThunk, pCurrentThunk->GetElement().second);

				const auto pbyRoundedBegin = (unsigned char *)(
					(((std::uintptr_t)pCurrentThunk->GetIndex<0>() + (1u << g_uPageOffsetBits) - 1) >> g_uPageOffsetBits)
					<< g_uPageOffsetBits
				);
				const auto pbyRoundedEnd = (unsigned char *)(
					(((std::uintptr_t)pCurrentThunk->GetIndex<0>() + pCurrentThunk->GetElement().second) >> g_uPageOffsetBits)
					<< g_uPageOffsetBits
				);
				if(pbyRoundedBegin != pbyRoundedEnd){
					FORCE_NOEXCEPT_BEGIN
					{
						DWORD dwOldProtect;
						::VirtualProtect(pbyRoundedBegin, (std::size_t)(pbyRoundedEnd - pbyRoundedBegin), PAGE_READONLY, &dwOldProtect);
					}
					FORCE_NOEXCEPT_END
				}
			} else {
				g_mapThunks.Erase(pCurrentThunk);
			}
		}
		STATIC_ASSERT_NOEXCEPT_END
	}
};

}

namespace MCF {

std::shared_ptr<const void> AllocateThunk(const void *pInit, std::size_t uSize){
	std::size_t uThunkSize = uSize + 8;
	uThunkSize = (uThunkSize + 0x0F) & (size_t)-0x10;
	if(uThunkSize < uSize){
		// 无符号溢出。
		throw std::bad_alloc();
	}

	std::shared_ptr<const void> pThunk;
	{
		auto vLock = g_pcsLock->GetLock();

		if(g_uPageOffsetBits == 0){
			SYSTEM_INFO vSystemInfo;
			::GetSystemInfo(&vSystemInfo);
			g_uPageOffsetBits = CountTrailingZeroes(vSystemInfo.dwPageSize) + 1u;
		}

		// 先挖坑，否则后面就没法 noexcept 了。
		auto pNullNode = g_mapThunks.Find<0>(nullptr);
		if(!pNullNode){
			pNullNode = g_mapThunks.Insert(std::make_pair(std::shared_ptr<void>(), 0u), nullptr, 0u);
		}

		auto pCached = g_mapThunks.GetLowerBound<1>(uThunkSize);
		if(!pCached){
			// 如果缓存中没有足够大的 thunk，我们先分配一个新的 chunk。
			const std::size_t uChunkSize = (uThunkSize + 0xFFFF) & (std::size_t)-0x10000;
			if(uChunkSize < uSize){
				throw std::bad_alloc();
			}
			std::shared_ptr<void> pChunk(
				::VirtualAlloc(0, uChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY),
				[](void *pChunk) noexcept { ::VirtualFree(pChunk, 0, MEM_RELEASE); }
			);
			if(!pChunk){
				throw std::bad_alloc();
			}
			pCached = g_mapThunks.Insert(std::make_pair(std::move(pChunk), uChunkSize), pChunk.get(), uChunkSize);
		}
		ASSERT(pCached->GetIndex<1>() >= uThunkSize);

		STATIC_ASSERT_NOEXCEPT_BEGIN
		{
			const auto pbyThunk = (unsigned char *)pCached->GetIndex<0>();
			const auto uSizeRemaining = pCached->GetIndex<1>() - uThunkSize;
			if(uSizeRemaining >= 0x10){
				// 如果剩下的空间还很大，保存成一个新的空闲 thunk。
				pCached->GetElement().second = uThunkSize;
				g_mapThunks.SetIndex<1>(pCached, 0u);

				pNullNode->GetElement() = std::make_pair(pCached->GetElement().first, uSizeRemaining);
				g_mapThunks.SetIndex<0>(pNullNode, pbyThunk + uThunkSize);
				g_mapThunks.SetIndex<1>(pNullNode, uSizeRemaining);
			} else {
				// 否则就不继续切分，当作一个整体。
				uThunkSize = pCached->GetIndex<1>();
				g_mapThunks.SetIndex<1>(pCached, 0u);
			}

			FORCE_NOEXCEPT_BEGIN
			{
				DWORD dwOldProtect;
				// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
				::VirtualProtect(pbyThunk, uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
				auto pbyWrite = pbyThunk;
				pbyWrite = CopyN(pbyWrite, (const unsigned char *)pInit, uSize).first;
				pbyWrite = FillN(pbyWrite, uThunkSize - uSize, 0xCC);
				::VirtualProtect(pbyThunk, uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);
			}
			FORCE_NOEXCEPT_END
		}
		STATIC_ASSERT_NOEXCEPT_END

		pThunk.reset(pCached->GetIndex<0>(), ThunkDeallocator());
	}
	ASSERT(pThunk);

	return std::move(pThunk);
}

}
