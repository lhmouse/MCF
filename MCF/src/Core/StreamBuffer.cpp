// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "StreamBuffer.hpp"
#include "Assert.hpp"
#include "FixedSizeAllocator.hpp"
#include <cstring>

namespace MCF {

constexpr unsigned kChunkSize = 256;

namespace Impl_StreamBuffer {
	struct Chunk {
		static void *operator new(std::size_t uSize);
		static void operator delete(void *pRaw) noexcept;

		Chunk *pPrev;
		Chunk *pNext;

		unsigned uBegin;
		unsigned uEnd;
		unsigned char abyData[kChunkSize];
	};

	namespace {
		FixedSizeAllocator<sizeof(Chunk)> g_vChunkAllocator;
	}

	static void *Chunk::operator new(std::size_t uSize){
		MCF_ASSERT(uSize == sizeof(Chunk));

		return g_vChunkAllocator.Allocate();
	}
	static void Chunk::operator delete(void *pRaw) noexcept {
		g_vChunkAllocator.Deallocate(pRaw);
	}
}

StreamBuffer::StreamBuffer(const StreamBuffer &rhs)
	: StreamBuffer()
{
	auto *pChunk = rhs.x_pHead;
	while(pChunk){
		const auto pNext = pChunk->pNext;
		Put(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin);
		pChunk = pNext;
	}
}

void StreamBuffer::Clear() noexcept {
	auto *pChunk = x_pHead;
	while(pChunk){
		const auto pNext = pChunk->pNext;
		delete pChunk;
		pChunk = pNext;
	}

	x_pHead = nullptr;
	x_pTail  = nullptr;
	x_uSize  = 0;
}

int StreamBuffer::PeekFront() const noexcept {
	int nRet = -1;

	auto *pChunk = x_pHead;
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];
	}

	return nRet;
}
int StreamBuffer::PeekBack() const noexcept {
	int nRet = -1;

	auto *pChunk = x_pTail;
	if(!pChunk){
		nRet = pChunk->abyData[pChunk->uEnd - 1];
	}

	return nRet;
}

int StreamBuffer::Peek() const noexcept {
	return PeekFront();
}
int StreamBuffer::Get() noexcept {
	int nRet = -1;

	auto *pChunk = x_pHead;
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];
		++(pChunk->uBegin);
		--x_uSize;

		const auto pNext = pChunk->pNext;
		if(pChunk->uBegin == pChunk->uEnd){
			delete pChunk;

			if(pNext){
				pNext->pPrev = nullptr;
			} else {
				x_pTail = nullptr;
			}
			x_pHead = pNext;
		}
		// pChunk = pNext;
	}

	return nRet;
}
bool StreamBuffer::Discard() noexcept {
	return Get() >= 0;
}
void StreamBuffer::Put(unsigned char byData){
	auto *pChunk = x_pTail;
	if(!pChunk || (pChunk->uEnd == kChunkSize)){
		const auto pNext = new Impl_StreamBuffer::Chunk;

		if(pChunk){
			pChunk->pNext = pNext;
		} else {
			x_pHead = pNext;
		}
		x_pTail = pNext;

		pNext->pPrev  = pChunk;
		pNext->pNext  = nullptr;
		pNext->uBegin = 0;
		pNext->uEnd   = pNext->uBegin;

		pChunk = pNext;
	}

	pChunk->abyData[pChunk->uEnd] = byData;
	++(pChunk->uEnd);
	++x_uSize;
}
int StreamBuffer::Unput() noexcept {
	int nRet = -1;

	auto *pChunk = x_pTail;
	if(pChunk){
		--(pChunk->uEnd);
		nRet = pChunk->abyData[pChunk->uEnd];
		--x_uSize;

		const auto pPrev = pChunk->pPrev;
		if(pChunk->uBegin == pChunk->uEnd){
			delete pChunk;

			if(pPrev){
				pPrev->pNext = nullptr;
			} else {
				x_pHead = nullptr;
			}
			x_pTail = pPrev;
		}
		// pChunk = pPrev;
	}

	return nRet;
}
void StreamBuffer::Unget(unsigned char byData){
	auto *pChunk = x_pHead;
	if(!pChunk || (pChunk->uBegin == 0)){
		const auto pPrev = new Impl_StreamBuffer::Chunk;

		if(pChunk){
			pChunk->pPrev = pPrev;
		} else {
			x_pTail = pPrev;
		}
		x_pHead = pPrev;

		pPrev->pPrev  = nullptr;
		pPrev->pNext  = pChunk;
		pPrev->uBegin = kChunkSize;
		pPrev->uEnd   = pPrev->uBegin;

		pChunk = pPrev;
	}

	--(pChunk->uBegin);
	pChunk->abyData[pChunk->uBegin] = byData;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	const auto pbyBegin = static_cast<unsigned char *>(pData);
	const auto pbyEnd = pbyBegin + uSize;
	auto pbyWrite = pbyBegin;

	auto *pChunk = x_pHead;
	while((pbyWrite != pbyEnd) && pChunk){
		const auto uBytesRemaining = static_cast<std::size_t>(pbyEnd - pbyWrite);
		auto uBytesToCopy = pChunk->uEnd - pChunk->uBegin;
		if(uBytesToCopy > uBytesRemaining){
			uBytesToCopy = static_cast<unsigned>(uBytesRemaining);
		}
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesToCopy);
		pbyWrite += uBytesToCopy;

		const auto pNext = pChunk->pNext;
		pChunk = pNext;
	}

	return static_cast<std::size_t>(pbyWrite - pbyBegin);
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	const auto pbyBegin = static_cast<unsigned char *>(pData);
	const auto pbyEnd = pbyBegin + uSize;
	auto pbyWrite = pbyBegin;

	auto *pChunk = x_pHead;
	while((pbyWrite != pbyEnd) && pChunk){
		const auto uBytesRemaining = static_cast<std::size_t>(pbyEnd - pbyWrite);
		auto uBytesToCopy = pChunk->uEnd - pChunk->uBegin;
		if(uBytesToCopy > uBytesRemaining){
			uBytesToCopy = static_cast<unsigned>(uBytesRemaining);
		}
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesToCopy);
		pbyWrite += uBytesToCopy;
		pChunk->uBegin += uBytesToCopy;
		x_uSize -= uBytesToCopy;

		const auto pNext = pChunk->pNext;
		if(pChunk->uBegin == pChunk->uEnd){
			delete pChunk;

			if(pNext){
				pNext->pPrev = nullptr;
			} else {
				x_pTail = nullptr;
			}
			x_pHead = pNext;
		}
		pChunk = pNext;
	}

	return static_cast<std::size_t>(pbyWrite - pbyBegin);
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	std::size_t uBytesDiscarded = 0;

	auto *pChunk = x_pHead;
	while((uBytesDiscarded != uSize) && pChunk){
		const auto uBytesRemaining = uSize - uBytesDiscarded;
		auto uBytesToDiscard = pChunk->uEnd - pChunk->uBegin;
		if(uBytesToDiscard > uBytesRemaining){
			uBytesToDiscard = static_cast<unsigned>(uBytesRemaining);
		}
		uBytesDiscarded += uBytesToDiscard;
		pChunk->uBegin += uBytesToDiscard;
		x_uSize -= uBytesToDiscard;

		const auto pNext = pChunk->pNext;
		if(pChunk->uBegin == pChunk->uEnd){
			delete pChunk;

			if(pNext){
				pNext->pPrev = nullptr;
			} else {
				x_pTail = nullptr;
			}
			x_pHead = pNext;
		}
		pChunk = pNext;
	}

	return uBytesDiscarded;
}
void StreamBuffer::Put(unsigned char byData, std::size_t uSize){
	Impl_StreamBuffer::Chunk *pSpHead = nullptr, *pSpTail = nullptr;
	std::size_t uCapacityAvail = 0;

	const auto pTail = x_pTail;
	if(pTail){
		uCapacityAvail += kChunkSize - pTail->uEnd;
	}
	if(uCapacityAvail < uSize){
		pSpHead = new Impl_StreamBuffer::Chunk;
		pSpTail = pSpHead;

		pSpHead->pPrev  = pTail;
		pSpHead->pNext  = nullptr;
		pSpHead->uBegin = 0;
		pSpHead->uEnd   = pSpHead->uBegin;

		uCapacityAvail += kChunkSize;

		try {
			while(uCapacityAvail < uSize){
				const auto pChunk = pSpTail;
				const auto pNext = new Impl_StreamBuffer::Chunk;

				pSpTail->pNext = pNext;
				pSpTail = pNext;

				pNext->pPrev  = pChunk;
				pNext->pNext  = nullptr;
				pNext->uBegin = 0;
				pNext->uEnd   = pNext->uBegin;

				uCapacityAvail += kChunkSize;
			}
		} catch(...){
			auto pChunk = pSpHead;
			do {
				const auto pNext = pChunk->pNext;
				delete pChunk;
				pChunk = pNext;
			} while(pChunk);

			throw;
		}
	}

	if(pSpHead){
		if(pTail){
			pTail->pNext = pSpHead;
		} else {
			x_pHead = pSpHead;
		}
		// pSpHead->pPrev = pTail;
		x_pTail = pSpTail;
	}

	auto *pChunk = pSpHead;
	if(pTail){
		pChunk = pTail;
	}
	std::size_t uBytesFilled = 0;
	while(uBytesFilled != uSize){
		const auto uBytesRemaining = uSize - uBytesFilled;
		auto uBytesToCopy = kChunkSize - pChunk->uEnd;
		if(uBytesToCopy > uBytesRemaining){
			uBytesToCopy = static_cast<unsigned>(uBytesRemaining);
		}
		std::memset(pChunk->abyData + pChunk->uEnd, byData, uBytesToCopy);
		pChunk->uEnd += uBytesToCopy;
		x_uSize += uBytesToCopy;

		pChunk = pChunk->pNext;
	}
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	Impl_StreamBuffer::Chunk *pSpHead = nullptr, *pSpTail = nullptr;
	std::size_t uCapacityAvail = 0;

	const auto pTail = x_pTail;
	if(pTail){
		uCapacityAvail += kChunkSize - pTail->uEnd;
	}
	if(uCapacityAvail < uSize){
		pSpHead = new Impl_StreamBuffer::Chunk;
		pSpTail = pSpHead;

		pSpHead->pPrev  = pTail;
		pSpHead->pNext  = nullptr;
		pSpHead->uBegin = 0;
		pSpHead->uEnd   = pSpHead->uBegin;

		uCapacityAvail += kChunkSize;

		try {
			while(uCapacityAvail < uSize){
				const auto pChunk = pSpTail;
				const auto pNext = new Impl_StreamBuffer::Chunk;

				pSpTail->pNext = pNext;
				pSpTail = pNext;

				pNext->pPrev  = pChunk;
				pNext->pNext  = nullptr;
				pNext->uBegin = 0;
				pNext->uEnd   = pNext->uBegin;

				uCapacityAvail += kChunkSize;
			}
		} catch(...){
			auto pChunk = pSpHead;
			do {
				const auto pNext = pChunk->pNext;
				delete pChunk;
				pChunk = pNext;
			} while(pChunk);

			throw;
		}
	}

	if(pSpHead){
		if(pTail){
			pTail->pNext = pSpHead;
		} else {
			x_pHead = pSpHead;
		}
		// pSpHead->pPrev = pTail;
		x_pTail = pSpTail;
	}

	auto *pChunk = pSpHead;
	if(pTail){
		pChunk = pTail;
	}
	const auto pbyBegin = static_cast<const unsigned char *>(pData);
	const auto pbyEnd = pbyBegin + uSize;
	auto pbyRead = pbyBegin;
	while(pbyRead != pbyEnd){
		const auto uBytesRemaining = static_cast<std::size_t>(pbyEnd - pbyRead);
		auto uBytesToCopy = kChunkSize - pChunk->uEnd;
		if(uBytesToCopy > uBytesRemaining){
			uBytesToCopy = static_cast<unsigned>(uBytesRemaining);
		}
		std::memcpy(pChunk->abyData + pChunk->uEnd, pbyRead, uBytesToCopy);
		pChunk->uEnd += uBytesToCopy;
		pbyRead += uBytesToCopy;
		x_uSize += uBytesToCopy;

		pChunk = pChunk->pNext;
	}
}

StreamBuffer StreamBuffer::CutOff(std::size_t uOffsetEnd){
	std::size_t uBytesCut = 0;
	const auto pCutBegin = x_pHead;
	auto pCutEnd = pCutBegin;
	while((uBytesCut != uOffsetEnd) && pCutEnd){
		const auto uBytesRemaining = uOffsetEnd - uBytesCut;
		auto uBytesToCut = pCutEnd->uEnd - pCutEnd->uBegin;
		if(uBytesToCut > uBytesRemaining){
			const auto pSplit = new Impl_StreamBuffer::Chunk;
			const auto pNext = pCutEnd->pNext;
			const auto uBytesLeft = static_cast<unsigned>(uBytesToCut - uBytesRemaining);

			pSplit->pPrev  = pCutEnd;
			pSplit->pNext  = pNext;
			pSplit->uBegin = 0;
			pSplit->uEnd   = uBytesLeft;

			std::memcpy(pSplit->abyData, pCutEnd->abyData + pCutEnd->uBegin + uBytesRemaining, uBytesLeft);
			pCutEnd->uEnd -= uBytesLeft;

			pCutEnd->pNext = pSplit;
			(pNext ? pNext->pPrev : x_pTail) = pSplit;

			uBytesToCut = static_cast<unsigned>(uBytesRemaining);
		}
		uBytesCut += uBytesToCut;

		const auto pNext = pCutEnd->pNext;
		pCutEnd = pNext;
	}

	StreamBuffer vHead;
	if(pCutBegin != pCutEnd){
		MCF_DEBUG_CHECK(pCutBegin);

		const auto pBeforeEnd = std::exchange(pCutEnd ? pCutEnd->pPrev : x_pTail, nullptr);

		vHead.x_pHead = pCutBegin;
		vHead.x_pTail = pBeforeEnd;
		x_pHead = pCutEnd;
		// pCutBegin->pPrev = nullptr;
		pBeforeEnd->pNext = nullptr;

		vHead.x_uSize = uBytesCut;
		x_uSize -= uBytesCut;
	}
	return vHead;
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	MCF_DEBUG_CHECK(&rhs != this);

	const auto pSpHead = rhs.x_pHead;
	if(pSpHead){
		const auto pSpTail = rhs.x_pTail;
		const auto pTail = x_pTail;
		(pTail ? pTail->pNext : x_pHead) = pSpHead;
		pSpHead->pPrev = pTail;
		x_pTail = pSpTail;

		rhs.x_pHead = nullptr;
		rhs.x_pTail  = nullptr;

		x_uSize += rhs.x_uSize;
		rhs.x_uSize = 0;
	}
}

}
