// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "StreamBuffer.hpp"
#include "_CheckedSizeArithmetic.hpp"
#include "Assert.hpp"

namespace MCF {

struct StreamBuffer::X_ChunkHeader {
	static X_ChunkHeader *Create(std::size_t uMinCapacity, X_ChunkHeader *pPrev, X_ChunkHeader *pNext, bool bBackward){
		const auto uCapacity = uMinCapacity | 1024;
		const auto uOrigin = bBackward ? uCapacity : 0;
		const auto pChunk = static_cast<X_ChunkHeader *>(::operator new(Impl_CheckedSizeArithmetic::Add(sizeof(X_ChunkHeader), uCapacity)));
		pChunk->uCapacity = uCapacity;
		pChunk->pPrev     = pPrev;
		pChunk->pNext     = pNext;
		pChunk->uBegin    = uOrigin;
		pChunk->uEnd      = uOrigin;
		return pChunk;
	}
	static void Destroy(X_ChunkHeader *pChunk) noexcept {
		::operator delete(pChunk);
	}

	std::size_t uCapacity;

	X_ChunkHeader *pPrev;
	X_ChunkHeader *pNext;

	std::size_t uBegin;
	std::size_t uEnd;
	__extension__ unsigned char abyData[];
};

StreamBuffer::StreamBuffer(const StreamBuffer &vOther){
	auto pChunk = vOther.x_pFirst;
	if(!pChunk){
		return;
	}
	const auto pIntegral = X_ChunkHeader::Create(vOther.x_uSize, nullptr, nullptr, false);
	while(pChunk){
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		std::memcpy(pIntegral->abyData + pIntegral->uEnd, pChunk->abyData + pChunk->uBegin, uAvail);
		pIntegral->uEnd += uAvail;
		pChunk = pChunk->pNext;
	}
	x_pLast  = pIntegral;
	x_pFirst = pIntegral;
	x_uSize  = vOther.x_uSize;
}
StreamBuffer::~StreamBuffer(){
	Clear();
}

void StreamBuffer::Clear() noexcept {
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto pNext = pChunk->pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	x_pLast  = nullptr;
	x_pFirst = nullptr;
	x_uSize  = 0;
}

int StreamBuffer::PeekFront() const noexcept {
	int nRead = -1;
	auto pChunk = x_pFirst;
	while(pChunk){
		if(pChunk->uEnd != pChunk->uBegin){
			nRead = pChunk->abyData[pChunk->uBegin];
			break;
		}
		const auto pNext = pChunk->pNext;
		pChunk = pNext;
	}
	return nRead;
}
int StreamBuffer::Get() noexcept {
	int nRead = -1;
	auto pChunk = x_pFirst;
	while(pChunk){
		if(pChunk->uEnd != pChunk->uBegin){
			nRead = pChunk->abyData[pChunk->uBegin];
			pChunk->uBegin += 1;
			x_uSize -= 1;
			break;
		}
		const auto pNext = pChunk->pNext;
		(pNext ? pNext->pPrev : x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	return nRead;
}
bool StreamBuffer::Discard() noexcept {
	bool bDiscarded = false;
	auto pChunk = x_pFirst;
	while(pChunk){
		if(pChunk->uEnd != pChunk->uBegin){
			bDiscarded = true;
			pChunk->uBegin += 1;
			x_uSize -= 1;
			break;
		}
		const auto pNext = pChunk->pNext;
		(pNext ? pNext->pPrev : x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	return bDiscarded;
}
void StreamBuffer::Put(unsigned char byData){
	auto pChunk = x_pLast;
	auto pPrev = pChunk;
	if(pChunk && (pChunk->uCapacity == pChunk->uEnd)){
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(pChunk->uCapacity > uAvail){
			std::memmove(pChunk->abyData, pChunk->abyData + pChunk->uBegin, uAvail);
			pChunk->uBegin = 0;
			pChunk->uEnd = uAvail;
		} else {
			pChunk = nullptr;
		}
	}
	if(!pChunk){
		const auto pNext = X_ChunkHeader::Create(1, pPrev, nullptr, false);
		(pPrev ? pPrev->pNext : x_pFirst) = pNext;
		x_pLast = pNext;
		pChunk = pNext;
	}
	pChunk->abyData[pChunk->uEnd] = byData;
	pChunk->uEnd += 1;
	x_uSize += 1;
}
int StreamBuffer::PeekBack() const noexcept {
	int nRead = -1;
	auto pChunk = x_pLast;
	while(pChunk){
		if(pChunk->uEnd != pChunk->uBegin){
			nRead = pChunk->abyData[pChunk->uEnd - 1];
			break;
		}
		pChunk = pChunk->pPrev;
	}
	return nRead;
}
int StreamBuffer::Unput() noexcept {
	int nRead = -1;
	auto pChunk = x_pLast;
	while(pChunk){
		if(pChunk->uEnd != pChunk->uBegin){
			pChunk->uEnd -= 1;
			nRead = pChunk->abyData[pChunk->uEnd];
			x_uSize -= 1;
			break;
		}
		const auto pPrev = pChunk->pPrev;
		(pPrev ? pPrev->pNext : x_pFirst) = nullptr;
		x_pLast = pPrev;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pPrev;
	}
	return nRead;
}
void StreamBuffer::Unget(unsigned char byData){
	auto pChunk = x_pFirst;
	auto pNext = pChunk;
	if(pChunk && (pChunk->uBegin == 0)){
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(pChunk->uCapacity > uAvail){
			std::memmove(pChunk->abyData + pChunk->uBegin + (pChunk->uCapacity - pChunk->uEnd), pChunk->abyData + pChunk->uBegin, uAvail);
			pChunk->uBegin = pChunk->uCapacity - uAvail;
			pChunk->uEnd = pChunk->uCapacity;
		} else {
			pChunk = nullptr;
		}
	}
	if(!pChunk){
		const auto pPrev = X_ChunkHeader::Create(1, nullptr, pNext, true);
		(pNext ? pNext->pPrev : x_pLast) = pPrev;
		x_pFirst = pPrev;
		pChunk = pPrev;
	}
	pChunk->uBegin -= 1;
	pChunk->abyData[pChunk->uBegin] = byData;
	x_uSize += 1;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	std::size_t uTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uRemaining = uSize - uTotal;
		if(uRemaining == 0){
			break;
		}
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(uAvail >= uRemaining){
			std::memcpy(static_cast<unsigned char *>(pData) + uTotal, pChunk->abyData + pChunk->uBegin, uRemaining);
			uTotal += uRemaining;
			break;
		}
		std::memcpy(static_cast<unsigned char *>(pData) + uTotal, pChunk->abyData + pChunk->uBegin, uAvail);
		uTotal += uAvail;
		const auto pNext = pChunk->pNext;
		pChunk = pNext;
	}
	return uTotal;
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	std::size_t uTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uRemaining = uSize - uTotal;
		if(uRemaining == 0){
			break;
		}
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(uAvail >= uRemaining){
			std::memcpy(static_cast<unsigned char *>(pData) + uTotal, pChunk->abyData + pChunk->uBegin, uRemaining);
			pChunk->uBegin += uRemaining;
			uTotal += uRemaining;
			x_uSize -= uRemaining;
			break;
		}
		std::memcpy(static_cast<unsigned char *>(pData) + uTotal, pChunk->abyData + pChunk->uBegin, uAvail);
		pChunk->uBegin += uAvail;
		uTotal += uAvail;
		x_uSize -= uAvail;
		const auto pNext = pChunk->pNext;
		(pNext ? pNext->pPrev : x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	return uTotal;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	std::size_t uTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uRemaining = uSize - uTotal;
		if(uRemaining == 0){
			break;
		}
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(uAvail >= uRemaining){
			pChunk->uBegin += uRemaining;
			uTotal += uRemaining;
			x_uSize -= uRemaining;
			break;
		}
		pChunk->uBegin += uAvail;
		uTotal += uAvail;
		x_uSize -= uAvail;
		const auto pNext = pChunk->pNext;
		(pNext ? pNext->pPrev : x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	return uTotal;
}
void StreamBuffer::Put(unsigned char byData, std::size_t uSize){
	auto pChunk = x_pLast;
	auto pPrev = pChunk;
	if(pChunk && (pChunk->uCapacity - pChunk->uEnd < uSize)){
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(pChunk->uCapacity - uAvail >= uSize){
			std::memmove(pChunk->abyData, pChunk->abyData + pChunk->uBegin, uAvail);
			pChunk->uBegin = 0;
			pChunk->uEnd = uAvail;
		} else {
			pChunk = nullptr;
		}
	}
	if(!pChunk){
		const auto pNext = X_ChunkHeader::Create(uSize, pPrev, nullptr, false);
		(pPrev ? pPrev->pNext : x_pFirst) = pNext;
		pChunk = pNext;
		x_pLast = pNext;
	}
	std::memset(pChunk->abyData + pChunk->uEnd, byData, uSize);
	pChunk->uEnd += uSize;
	x_uSize += uSize;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	auto pChunk = x_pLast;
	auto pPrev = pChunk;
	if(pChunk && (pChunk->uCapacity - pChunk->uEnd < uSize)){
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(pChunk->uCapacity - uAvail >= uSize){
			std::memmove(pChunk->abyData, pChunk->abyData + pChunk->uBegin, uAvail);
			pChunk->uBegin = 0;
			pChunk->uEnd = uAvail;
		} else {
			pChunk = nullptr;
		}
	}
	if(!pChunk){
		const auto pNext = X_ChunkHeader::Create(uSize, pPrev, nullptr, false);
		(pPrev ? pPrev->pNext : x_pFirst) = pNext;
		x_pLast = pNext;
		pChunk = pNext;
	}
	std::memcpy(pChunk->abyData + pChunk->uEnd, pData, uSize);
	pChunk->uEnd += uSize;
	x_uSize += uSize;
}

void *StreamBuffer::Squash(){
	auto pChunk = x_pFirst;
	if(!pChunk){
		return nullptr;
	}
	if(pChunk != x_pLast){
		StreamBuffer(*this).Swap(*this);
		pChunk = x_pFirst;
	}
	return pChunk->abyData + pChunk->uBegin;
}

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
	std::size_t uTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uRemaining = uSize - uTotal;
		if(uRemaining == 0){
			break;
		}
		const auto uAvail = pChunk->uEnd - pChunk->uBegin;
		if(uAvail >= uRemaining){
			if(uAvail > uRemaining){
				const auto pPrev = pChunk->pPrev;
				const auto pNext = pChunk;
				pChunk = X_ChunkHeader::Create(uRemaining, pPrev, pNext, false);
				std::memcpy(pChunk->abyData, pNext->abyData + pNext->uBegin, uRemaining);
				pChunk->uEnd = uRemaining;
				pNext->uBegin += uRemaining;
				(pPrev ? pPrev->pNext : x_pFirst) = pChunk;
				pNext->pPrev = pChunk;
			}
			uTotal += uRemaining;
			pChunk = pChunk->pNext;
			break;
		}
		uTotal += uAvail;
		const auto pNext = pChunk->pNext;
		pChunk = pNext;
	}
	StreamBuffer vHead;
	if(uTotal != 0){
		const auto pLastCut = std::exchange(pChunk ? pChunk->pPrev : x_pLast, nullptr);
		const auto pFirstCut = std::exchange(x_pFirst, pChunk);
		pLastCut->pNext = nullptr;
		x_uSize -= uTotal;
		vHead.x_pLast  = pLastCut;
		vHead.x_pFirst = pFirstCut;
		vHead.x_uSize  = uTotal;
	}
	return vHead;
}
void StreamBuffer::Splice(StreamBuffer &vOther) noexcept {
	MCF_ASSERT(&vOther != this);

	const auto pFirstAdd = std::exchange(vOther.x_pFirst, nullptr);
	if(!pFirstAdd){
		return;
	}
	const auto pLast = std::exchange(x_pLast, std::exchange(vOther.x_pLast, nullptr));
	(pLast ? pLast->pNext : x_pFirst) = pFirstAdd;
	pFirstAdd->pPrev = pLast;
	x_uSize += std::exchange(vOther.x_uSize, 0);
}

bool StreamBuffer::EnumerateChunk(const void **ppData, std::size_t *puSize, StreamBuffer::EnumerationCookie &vCookie) const noexcept {
	const auto pChunk = vCookie.x_pPrev ? vCookie.x_pPrev->pNext : x_pFirst;
	vCookie.x_pPrev = pChunk;
	if(!pChunk){
		return false;
	}
	if(ppData){
		*ppData = pChunk->abyData + pChunk->uBegin;
	}
	if(puSize){
		*puSize = pChunk->uEnd - pChunk->uBegin;
	}
	return true;
}
bool StreamBuffer::EnumerateChunk(void **ppData, std::size_t *puSize, StreamBuffer::EnumerationCookie &vCookie) noexcept {
	const auto pChunk = vCookie.x_pPrev ? vCookie.x_pPrev->pNext : x_pFirst;
	vCookie.x_pPrev = pChunk;
	if(!pChunk){
		return false;
	}
	if(ppData){
		*ppData = pChunk->abyData + pChunk->uBegin;
	}
	if(puSize){
		*puSize = pChunk->uEnd - pChunk->uBegin;
	}
	return true;
}

}
