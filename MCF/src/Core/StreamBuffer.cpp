// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "StreamBuffer.hpp"
#include "Assert.hpp"
#include "FixedSizeAllocator.hpp"
#include <cstring>

namespace MCF {

namespace {
	constexpr unsigned kChunkSize = 3;

	struct Chunk {
		static void *operator new(std::size_t uSize);
		static void operator delete(void *pRaw) noexcept;

		Chunk *pPrev;
		Chunk *pNext;

		unsigned uBegin;
		unsigned uEnd;
		unsigned char abyData[kChunkSize];
	};

	FixedSizeAllocator<sizeof(Chunk)> g_vChunkAllocator;

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
	auto *restrict pChunk = static_cast<const Chunk *>(rhs.x_pFirst);
	while(pChunk){
		const auto pNext = pChunk->pNext;
		Put(pChunk->abyData + pChunk->uBegin, pChunk->uEnd - pChunk->uBegin);
		pChunk = pNext;
	}
}

void StreamBuffer::Clear() noexcept {
	auto *restrict pChunk = static_cast<const Chunk *>(x_pFirst);
	while(pChunk){
		const auto pNext = pChunk->pNext;
		delete pChunk;
		pChunk = pNext;
	}

	x_pFirst = nullptr;
	x_pLast  = nullptr;
	x_uSize  = 0;
}

int StreamBuffer::PeekFront() const noexcept {
	int nRet = -1;

	auto *restrict pChunk = static_cast<const Chunk *>(x_pFirst);
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];
	}

	return nRet;
}
int StreamBuffer::PeekBack() const noexcept {
	int nRet = -1;

	auto *restrict pChunk = static_cast<const Chunk *>(x_pLast);
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

	auto *restrict pChunk = static_cast<Chunk *>(x_pFirst);
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
				x_pLast = nullptr;
			}
			x_pFirst = pNext;
		}
		// pChunk = pNext;
	}

	return nRet;
}
bool StreamBuffer::Discard() noexcept {
	return Get() >= 0;
}
void StreamBuffer::Put(unsigned char byData){
	auto *restrict pChunk = static_cast<Chunk *>(x_pLast);
	if(!pChunk || (pChunk->uEnd == kChunkSize)){
		const auto pNext = new Chunk;

		if(pChunk){
			pChunk->pNext = pNext;
		} else {
			x_pFirst = pNext;
		}
		x_pLast = pNext;

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

	auto *restrict pChunk = static_cast<Chunk *>(x_pLast);
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
				x_pFirst = nullptr;
			}
			x_pLast = pPrev;
		}
		// pChunk = pPrev;
	}

	return nRet;
}
void StreamBuffer::Unget(unsigned char byData){
	auto *restrict pChunk = static_cast<Chunk *>(x_pFirst);
	if(!pChunk || (pChunk->uBegin == 0)){
		const auto pPrev = new Chunk;

		if(pChunk){
			pChunk->pPrev = pPrev;
		} else {
			x_pLast = pPrev;
		}
		x_pFirst = pPrev;

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

	auto *restrict pChunk = static_cast<Chunk *>(x_pFirst);
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

	auto *restrict pChunk = static_cast<Chunk *>(x_pFirst);
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
				x_pLast = nullptr;
			}
			x_pFirst = pNext;
		}
		pChunk = pNext;
	}

	return static_cast<std::size_t>(pbyWrite - pbyBegin);
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	std::size_t uBytesDiscarded = 0;

	auto *restrict pChunk = static_cast<Chunk *>(x_pFirst);
	while((uBytesDiscarded != uSize) && pChunk){
		const auto uBytesRemaining = uSize - uBytesDiscarded;
		auto uBytesToCopy = pChunk->uEnd - pChunk->uBegin;
		if(uBytesToCopy > uBytesRemaining){
			uBytesToCopy = static_cast<unsigned>(uBytesRemaining);
		}
		uBytesDiscarded += uBytesToCopy;
		pChunk->uBegin += uBytesToCopy;
		x_uSize -= uBytesToCopy;

		const auto pNext = pChunk->pNext;
		if(pChunk->uBegin == pChunk->uEnd){
			delete pChunk;

			if(pNext){
				pNext->pPrev = nullptr;
			} else {
				x_pLast = nullptr;
			}
			x_pFirst = pNext;
		}
		pChunk = pNext;
	}

	return uBytesDiscarded;
}
void StreamBuffer::Put(unsigned char byData, std::size_t uSize){
	Chunk *pSpFirst = nullptr, *pSpLast = nullptr;
	std::size_t uCapacityAvail = 0;

	const auto pLast = static_cast<Chunk *>(x_pLast);
	if(pLast){
		uCapacityAvail += kChunkSize - pLast->uEnd;
	}
	if(uCapacityAvail < uSize){
		pSpFirst = new Chunk;
		pSpLast = pSpFirst;

		pSpFirst->pPrev  = pLast;
		pSpFirst->pNext  = nullptr;
		pSpFirst->uBegin = 0;
		pSpFirst->uEnd   = pSpFirst->uBegin;

		uCapacityAvail += kChunkSize;

		try {
			while(uCapacityAvail < uSize){
				const auto pChunk = pSpLast;
				const auto pNext = new Chunk;

				pSpLast->pNext = pNext;
				pSpLast = pNext;

				pNext->pPrev  = pChunk;
				pNext->pNext  = nullptr;
				pNext->uBegin = 0;
				pNext->uEnd   = pNext->uBegin;

				uCapacityAvail += kChunkSize;
			}
		} catch(...){
			auto pChunk = pSpFirst;
			do {
				const auto pNext = pChunk->pNext;
				delete pChunk;
				pChunk = pNext;
			} while(pChunk);

			throw;
		}
	}

	if(pSpFirst){
		if(pLast){
			pLast->pNext = pSpFirst;
		} else {
			x_pFirst = pSpFirst;
		}
		// pSpFirst->pPrev = pLast;
		x_pLast = pSpLast;
	}

	auto *restrict pChunk = pSpFirst;
	if(pLast){
		pChunk = pLast;
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
	Chunk *pSpFirst = nullptr, *pSpLast = nullptr;
	std::size_t uCapacityAvail = 0;

	const auto pLast = static_cast<Chunk *>(x_pLast);
	if(pLast){
		uCapacityAvail += kChunkSize - pLast->uEnd;
	}
	if(uCapacityAvail < uSize){
		pSpFirst = new Chunk;
		pSpLast = pSpFirst;

		pSpFirst->pPrev  = pLast;
		pSpFirst->pNext  = nullptr;
		pSpFirst->uBegin = 0;
		pSpFirst->uEnd   = pSpFirst->uBegin;

		uCapacityAvail += kChunkSize;

		try {
			while(uCapacityAvail < uSize){
				const auto pChunk = pSpLast;
				const auto pNext = new Chunk;

				pSpLast->pNext = pNext;
				pSpLast = pNext;

				pNext->pPrev  = pChunk;
				pNext->pNext  = nullptr;
				pNext->uBegin = 0;
				pNext->uEnd   = pNext->uBegin;

				uCapacityAvail += kChunkSize;
			}
		} catch(...){
			auto pChunk = pSpFirst;
			do {
				const auto pNext = pChunk->pNext;
				delete pChunk;
				pChunk = pNext;
			} while(pChunk);

			throw;
		}
	}

	if(pSpFirst){
		if(pLast){
			pLast->pNext = pSpFirst;
		} else {
			x_pFirst = pSpFirst;
		}
		// pSpFirst->pPrev = pLast;
		x_pLast = pSpLast;
	}

	auto *restrict pChunk = pSpFirst;
	if(pLast){
		pChunk = pLast;
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

//StreamBuffer StreamBuffer::CutOff(std::size_t uOffsetEnd);
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	MCF_ASSERT(&rhs != this);

	const auto pSpFirst = static_cast<Chunk *>(rhs.x_pFirst);
	const auto pSpLast  = static_cast<Chunk *>(rhs.x_pLast);
	if(pSpFirst){
		const auto pLast = static_cast<Chunk *>(x_pLast);
		if(pLast){
			pLast->pNext = pSpFirst;
		} else {
			x_pFirst = pSpFirst;
		}
		pSpFirst->pPrev = pLast;
		x_pLast = pSpLast;

		x_uSize += rhs.x_uSize;

		rhs.x_pFirst = nullptr;
		rhs.x_pLast  = nullptr;
		rhs.x_uSize  = 0;
	}
}

}
