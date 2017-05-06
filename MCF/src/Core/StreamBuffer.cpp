// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "StreamBuffer.hpp"
#include "MinMax.hpp"
#include "_CheckedSizeArithmetic.hpp"
#include "Assert.hpp"

namespace MCF {

struct StreamBuffer::X_ChunkHeader {
	static X_ChunkHeader *Create(std::size_t uMinCapacity, X_ChunkHeader *pPrev, X_ChunkHeader *pNext, bool bBackward){
		const auto uCapacity = Max(uMinCapacity, 1024u);
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
	const auto pNewChunk = X_ChunkHeader::Create(vOther.x_uSize, nullptr, nullptr, false);
	for(auto pChunk = vOther.x_pFirst; pChunk; pChunk = pChunk->pNext){
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		std::memcpy(pNewChunk->abyData + pNewChunk->uEnd, pChunk->abyData + pChunk->uBegin, uBytesAvail);
		pNewChunk->uEnd += uBytesAvail;
	}
	x_pLast  = pNewChunk;
	x_pFirst = pNewChunk;
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
	const auto pChunk = x_pFirst;
	if(!pChunk){
		return -1;
	}
	const auto byData = pChunk->abyData[pChunk->uBegin];
	return byData;
}
int StreamBuffer::PeekBack() const noexcept {
	const auto pChunk = x_pLast;
	if(!pChunk){
		return -1;
	}
	const auto byData = pChunk->abyData[pChunk->uEnd - 1];
	return byData;
}

int StreamBuffer::Peek() const noexcept {
	auto pChunk = x_pFirst;
	if(!pChunk){
		return -1;
	}
	const auto byData = pChunk->abyData[pChunk->uBegin];
	return byData;
}
int StreamBuffer::Get() noexcept {
	auto pChunk = x_pFirst;
	if(!pChunk){
		return -1;
	}
	const auto byData = pChunk->abyData[pChunk->uBegin];
	if(++pChunk->uBegin == pChunk->uEnd){
		const auto pNext = pChunk->pNext;
		*(pNext ? &(pNext->pPrev) : &x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	--x_uSize;
	return byData;
}
bool StreamBuffer::Discard() noexcept {
	auto pChunk = x_pFirst;
	if(!pChunk){
		return false;
	}
	if(++pChunk->uBegin == pChunk->uEnd){
		const auto pNext = pChunk->pNext;
		*(pNext ? &(pNext->pPrev) : &x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
	}
	--x_uSize;
	return true;
}
void StreamBuffer::Put(unsigned char byData){
	auto pChunk = x_pLast;
	if(!pChunk || (pChunk->uCapacity - pChunk->uEnd < 1)){
		const auto pPrev = pChunk;
		pChunk = X_ChunkHeader::Create(1, pPrev, nullptr, false);
		x_pLast = pChunk;
		*(pPrev ? &(pPrev->pNext) : &x_pFirst) = pChunk;
	}
	pChunk->abyData[pChunk->uEnd] = byData;
	++pChunk->uEnd;
	++x_uSize;
}
int StreamBuffer::Unput() noexcept {
	auto pChunk = x_pLast;
	if(!pChunk){
		return -1;
	}
	const auto byData = pChunk->abyData[pChunk->uEnd - 1];
	if(--pChunk->uEnd == pChunk->uBegin){
		const auto pPrev = pChunk->pPrev;
		x_pLast = pPrev;
		*(pPrev ? &(pPrev->pNext) : &x_pFirst) = nullptr;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pPrev;
	}
	--x_uSize;
	return byData;
}
void StreamBuffer::Unget(unsigned char byData){
	auto pChunk = x_pFirst;
	if(!pChunk || (pChunk->uBegin < 1)){
		const auto pNext = pChunk;
		pChunk = X_ChunkHeader::Create(1, nullptr, pNext, true);
		*(pNext ? &(pNext->pPrev) : &x_pLast) = pChunk;
		x_pFirst = pChunk;
	}
	pChunk->abyData[pChunk->uBegin - 1] = byData;
	--pChunk->uBegin;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	std::size_t uBytesTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uBytesRemaining = uSize - uBytesTotal;
		if(uBytesRemaining == 0){
			break;
		}
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining < uBytesAvail){
			std::memcpy(static_cast<char *>(pData) + uBytesTotal, pChunk->abyData + pChunk->uBegin, uBytesRemaining);
			uBytesTotal += uBytesRemaining;
			break;
		}
		std::memcpy(static_cast<char *>(pData) + uBytesTotal, pChunk->abyData + pChunk->uBegin, uBytesAvail);
		const auto pNext = pChunk->pNext;
		pChunk = pNext;
		uBytesTotal += uBytesAvail;
	}
	return uBytesTotal;
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	std::size_t uBytesTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uBytesRemaining = uSize - uBytesTotal;
		if(uBytesRemaining == 0){
			break;
		}
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining < uBytesAvail){
			std::memcpy(static_cast<char *>(pData) + uBytesTotal, pChunk->abyData + pChunk->uBegin, uBytesRemaining);
			pChunk->uBegin += uBytesRemaining;
			uBytesTotal += uBytesRemaining;
			break;
		}
		std::memcpy(static_cast<char *>(pData) + uBytesTotal, pChunk->abyData + pChunk->uBegin, uBytesAvail);
		const auto pNext = pChunk->pNext;
		*(pNext ? &(pNext->pPrev) : &x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
		uBytesTotal += uBytesAvail;
	}
	x_uSize -= uBytesTotal;
	return uBytesTotal;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	std::size_t uBytesTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uBytesRemaining = uSize - uBytesTotal;
		if(uBytesRemaining == 0){
			break;
		}
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining < uBytesAvail){
			pChunk->uBegin += uBytesRemaining;
			uBytesTotal += uBytesRemaining;
			break;
		}
		const auto pNext = pChunk->pNext;
		*(pNext ? &(pNext->pPrev) : &x_pLast) = nullptr;
		x_pFirst = pNext;
		X_ChunkHeader::Destroy(pChunk);
		pChunk = pNext;
		uBytesTotal += uBytesAvail;
	}
	x_uSize -= uBytesTotal;
	return uBytesTotal;
}
void StreamBuffer::Put(unsigned char byData, std::size_t uSize){
	auto pChunk = x_pLast;
	if(!pChunk || (pChunk->uCapacity - pChunk->uEnd < uSize)){
		const auto pPrev = pChunk;
		pChunk = X_ChunkHeader::Create(uSize, pPrev, nullptr, false);
		x_pLast = pChunk;
		*(pPrev ? &(pPrev->pNext) : &x_pFirst) = pChunk;
	}
	std::memset(pChunk->abyData + pChunk->uEnd, byData, uSize);
	pChunk->uEnd += uSize;
	x_uSize += uSize;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	auto pChunk = x_pLast;
	if(!pChunk || (pChunk->uCapacity - pChunk->uEnd < uSize)){
		const auto pPrev = pChunk;
		pChunk = X_ChunkHeader::Create(uSize, pPrev, nullptr, false);
		x_pLast = pChunk;
		*(pPrev ? &(pPrev->pNext) : &x_pFirst) = pChunk;
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
		const auto pNewChunk = X_ChunkHeader::Create(x_uSize, nullptr, nullptr, false);
		while(pChunk){
			const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
			std::memcpy(pNewChunk->abyData + pNewChunk->uEnd, pChunk->abyData + pChunk->uBegin, uBytesAvail);
			pNewChunk->uEnd += uBytesAvail;
			const auto pNext = pChunk->pNext;
			X_ChunkHeader::Destroy(pChunk);
			pChunk = pNext;
		}
		x_pLast  = pNewChunk;
		x_pFirst = pNewChunk;
		pChunk = pNewChunk;
	}
	return pChunk->abyData + pChunk->uBegin;
}

StreamBuffer StreamBuffer::CutOff(std::size_t uOffsetEnd){
	std::size_t uBytesTotal = 0;
	auto pChunk = x_pFirst;
	while(pChunk){
		const auto uBytesRemaining = uOffsetEnd - uBytesTotal;
		if(uBytesRemaining == 0){
			break;
		}
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining < uBytesAvail){
			const auto pPrev = pChunk->pPrev;
			const auto pNext = pChunk;
			pChunk = X_ChunkHeader::Create(uBytesRemaining, pPrev, pNext, false);
			std::memcpy(pChunk->abyData + pChunk->uEnd, pNext->abyData + pNext->uBegin, uBytesRemaining);
			pChunk->uEnd += uBytesRemaining;
			pNext->uBegin += uBytesRemaining;
			*(pPrev ? &(pPrev->pNext) : &x_pFirst) = pChunk;
			pNext->pPrev = pChunk;
			pChunk = pNext;
			uBytesTotal += uBytesRemaining;
			break;
		}
		const auto pNext = pChunk->pNext;
		pChunk = pNext;
		uBytesTotal += uBytesAvail;
	}

	StreamBuffer vHead;
	if(uBytesTotal != 0){
		const auto pLastCut = std::exchange(*(pChunk ? &(pChunk->pPrev) : &x_pLast), nullptr);
		pLastCut->pNext = nullptr;
		const auto pFirstCut = std::exchange(x_pFirst, pChunk);
		vHead.x_pLast  = pLastCut;
		vHead.x_pFirst = pFirstCut;
		vHead.x_uSize  = uBytesTotal;
		x_uSize -= uBytesTotal;
	}
	return vHead;
}
void StreamBuffer::Splice(StreamBuffer &vOther) noexcept {
	MCF_ASSERT(&vOther != this);

	const auto pFirstAdd = std::exchange(vOther.x_pFirst, nullptr);
	if(!pFirstAdd){
		return;
	}
	const auto pLastAdd  = std::exchange(vOther.x_pLast, nullptr);
	const auto uSizeAdd  = std::exchange(vOther.x_uSize, 0);

	const auto pLast = x_pLast;
	x_pLast = pLastAdd;
	*(pLast ? &(pLast->pNext) : &x_pFirst) = pFirstAdd;
	pFirstAdd->pPrev = pLast;
	x_uSize += uSizeAdd;
}

}
