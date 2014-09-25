// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Thunk.hpp"
#include "../Utilities/StaticAssertNoexcept.hpp"
#include "../Utilities/CountLeadingTrailingZeroes.hpp"
#include "../Utilities/Algorithms.hpp"
#include "../Containers/MultiIndexMap.hpp"
#include "../Thread/CriticalSection.hpp"
#include <memory>
#include <exception>
using namespace MCF;

namespace {

struct ThunkItem {
	std::shared_ptr<void> pChunk;	// Chunk。内存是以 64KiB 的粒度分配的，每一块称为一个 chunk。
	std::size_t uThunkSize;
	void *pThunk;
	std::size_t uFreeSize;			// 在空闲 thunk 上等于大小，否则为零。
};

typedef MultiIndexMap<ThunkItem,
	UniqueOrderedMemberIndex<ThunkItem, void *, &ThunkItem::pThunk>,
	MultiOrderedMemberIndex<ThunkItem, std::size_t, &ThunkItem::uFreeSize>
	> ThunkMap;

const auto NULL_THUNK = ThunkItem();

std::size_t	g_uPageOffsetBits = 0;
const auto	g_pcsLock = CriticalSection::Create();
ThunkMap	g_mapThunks;

struct ThunkDeallocator {
	void operator()(const void *pThunk) const noexcept {
		STATIC_ASSERT_NOEXCEPT_BEGIN
		{
			auto vLock = g_pcsLock->GetLock();

			auto pCurrentThunk = g_mapThunks.Find<0>(pThunk);
			ASSERT(pCurrentThunk && (pCurrentThunk->uFreeSize == 0));

			const auto pNextThunk = pCurrentThunk->GetNext<0>();
			if(pNextThunk && (pNextThunk->pChunk == pCurrentThunk->pChunk) && (pNextThunk->uFreeSize != 0) &&
				(const unsigned char *)pCurrentThunk->pThunk + pCurrentThunk->uThunkSize == pNextThunk->pThunk)
			{
				// 如果连续的下一个 thunk 也在缓存中，把它合并到当前 thunk。
				pCurrentThunk->uThunkSize += pNextThunk->uThunkSize;
				g_mapThunks.Erase(pNextThunk);
			}
			const auto pPrevThunk = pCurrentThunk->GetPrev<0>();
			if(pPrevThunk && (pPrevThunk->pChunk == pCurrentThunk->pChunk) && (pPrevThunk->uFreeSize != 0) &&
				(const unsigned char *)pPrevThunk->pThunk + pPrevThunk->uThunkSize == pCurrentThunk->pThunk)
			{
				// 如果连续的前一个 thunk 也在缓存中，把当前 thunk 合并到它。
				pPrevThunk->uThunkSize += pCurrentThunk->uThunkSize;
				g_mapThunks.Erase(pCurrentThunk);
				pCurrentThunk = pPrevThunk;
			}
			if(!pCurrentThunk->pChunk.unique()){
				FORCE_NOEXCEPT_BEGIN
				{
					g_mapThunks.SetKey<1>(pCurrentThunk, pCurrentThunk->uThunkSize);
				}
				FORCE_NOEXCEPT_END

				const auto pbyRoundedBegin = (unsigned char *)(
					(((std::uintptr_t)pCurrentThunk->pThunk + (1u << g_uPageOffsetBits) - 1)
						>> g_uPageOffsetBits) << g_uPageOffsetBits);
				const auto pbyRoundedEnd = (unsigned char *)(
					(((std::uintptr_t)pCurrentThunk->pThunk + pCurrentThunk->uThunkSize)
						>> g_uPageOffsetBits) << g_uPageOffsetBits);
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
		// 无符号进位。
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
		auto pNullNode = g_mapThunks.GetLowerBound<0>(nullptr);
		if(!pNullNode || (pNullNode->pThunk != nullptr)){
			pNullNode = g_mapThunks.InsertWithHints(
				std::make_tuple(pNullNode, nullptr), NULL_THUNK).first;
		}

		auto pCached = g_mapThunks.GetLowerBound<1>(uThunkSize);
		if(!pCached){
			// 如果缓存中没有足够大的 thunk，我们先分配一个新的 chunk。
			const std::size_t uChunkSize = (uThunkSize + 0xFFFF) & (std::size_t)-0x10000;
			if(uChunkSize < uSize){
				throw std::bad_alloc();
			}
			ThunkItem vThunk;
			vThunk.pChunk.reset(::VirtualAlloc(0, uChunkSize, MEM_COMMIT | MEM_RESERVE, PAGE_READONLY),
				[](void *pChunk) noexcept { ::VirtualFree(pChunk, 0, MEM_RELEASE); });
			if(!vThunk.pChunk){
				throw std::bad_alloc();
			}
			vThunk.uThunkSize = uChunkSize;
			vThunk.pThunk = vThunk.pChunk.get();
			vThunk.uFreeSize = uChunkSize;
			pCached = g_mapThunks.Insert(std::move(vThunk)).first;
		}
		ASSERT(pCached->uFreeSize >= uThunkSize);

		const auto pbyThunk = (unsigned char *)pCached->pThunk;
		const auto uSizeRemaining = pCached->uFreeSize - uThunkSize;
		if(uSizeRemaining >= 0x10){
			// 如果剩下的空间还很大，保存成一个新的空闲 thunk。
			pCached->uThunkSize = uThunkSize;
			g_mapThunks.SetKey<1>(pCached, 0);

			pNullNode->pChunk = pCached->pChunk;
			pNullNode->uThunkSize = uSizeRemaining;
			g_mapThunks.SetKey<0>(pNullNode, pbyThunk + uThunkSize);
			g_mapThunks.SetKey<1>(pNullNode, uSizeRemaining);
		} else {
			// 否则就不继续切分，当作一个整体。
			uThunkSize = pCached->uFreeSize;
			g_mapThunks.SetKey<1>(pCached, 0);
		}

		DWORD dwOldProtect;
		// 由于其他 thunk 可能共享了当前内存页，所以不能设置为 PAGE_READWRITE。
		::VirtualProtect(pbyThunk, uThunkSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
		auto pbyWrite = pbyThunk;
		pbyWrite = CopyN(pbyWrite, (const unsigned char *)pInit, uSize).first;
		pbyWrite = FillN(pbyWrite, uThunkSize - uSize, 0xCC);
		::VirtualProtect(pbyThunk, uThunkSize, PAGE_EXECUTE_READ, &dwOldProtect);

		pThunk.reset(pCached->pThunk, ThunkDeallocator());
	}
	ASSERT(pThunk);

	return std::move(pThunk);
}

}
