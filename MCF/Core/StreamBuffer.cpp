// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Thread/SpinLock.hpp"

namespace MCF {

struct StreamBuffer::xChunk final {
	static SpinLock s_splPoolMutex;
	static xChunk *restrict s_pPoolHead;

	static void *operator new(std::size_t uSize){
		ASSERT(uSize == sizeof(xChunk));

		const auto uLockValue = s_splPoolMutex.Lock();
		const auto pPooled = s_pPoolHead;
		if(!pPooled){
			s_splPoolMutex.Unlock(uLockValue);
			return ::operator new(uSize);
		}
		s_pPoolHead = pPooled->pPrev;
		s_splPoolMutex.Unlock(uLockValue);
		return pPooled;
	}
	static void operator delete(void *pRaw) noexcept {
		if(!pRaw){
			return;
		}
		const auto pPooled = static_cast<xChunk *>(pRaw);

		const auto uLockValue = s_splPoolMutex.Lock();
		pPooled->pPrev = s_pPoolHead;
		s_pPoolHead = pPooled;
		s_splPoolMutex.Unlock(uLockValue);
	}

	__attribute__((__destructor__))
	static void PoolDestructor() noexcept {
		while(s_pPoolHead){
			const auto pPooled = s_pPoolHead;
			s_pPoolHead = pPooled->pPrev;
			::operator delete(pPooled);
		}
	}

	xChunk *pPrev;
	xChunk *pNext;
	unsigned uBegin;
	unsigned uEnd;
	unsigned char abyData[10];
};

SpinLock StreamBuffer::xChunk::s_splPoolMutex;
StreamBuffer::xChunk *restrict StreamBuffer::xChunk::s_pPoolHead = nullptr;

unsigned char *StreamBuffer::ChunkEnumerator::GetBegin() const noexcept {
	ASSERT(x_pChunk);

	return x_pChunk->abyData + x_pChunk->uBegin;
}
unsigned char *StreamBuffer::ChunkEnumerator::GetEnd() const noexcept {
	ASSERT(x_pChunk);

	return x_pChunk->abyData + x_pChunk->uEnd;
}

StreamBuffer::ChunkEnumerator &StreamBuffer::ChunkEnumerator::operator++() noexcept {
	ASSERT(x_pChunk);

	x_pChunk = x_pChunk->pNext;
	return *this;
}

const unsigned char *StreamBuffer::ConstChunkEnumerator::GetBegin() const noexcept {
	ASSERT(x_pChunk);

	return x_pChunk->abyData + x_pChunk->uBegin;
}
const unsigned char *StreamBuffer::ConstChunkEnumerator::GetEnd() const noexcept {
	ASSERT(x_pChunk);

	return x_pChunk->abyData + x_pChunk->uEnd;
}

StreamBuffer::ConstChunkEnumerator &StreamBuffer::ConstChunkEnumerator::operator++() noexcept {
	ASSERT(x_pChunk);

	x_pChunk = x_pChunk->pNext;
	return *this;
}

// 构造函数和析构函数。
StreamBuffer::StreamBuffer(const void *pData, std::size_t uSize)
	: StreamBuffer()
{
	Put(pData, uSize);
}
StreamBuffer::StreamBuffer(const char *pszData)
	: StreamBuffer()
{
	Put(pszData);
}
StreamBuffer::StreamBuffer(const StreamBuffer &rhs)
	: StreamBuffer()
{
	for(auto ce = rhs.GetChunkEnumerator(); ce; ++ce){
		Put(ce.GetData(), ce.GetSize());
	}
}
StreamBuffer::StreamBuffer(StreamBuffer &&rhs) noexcept
	: StreamBuffer()
{
	Swap(rhs);
}
StreamBuffer &StreamBuffer::operator=(const StreamBuffer &rhs){
	StreamBuffer(rhs).Swap(*this);
	return *this;
}
StreamBuffer &StreamBuffer::operator=(StreamBuffer &&rhs) noexcept {
	StreamBuffer(std::move(rhs)).Swap(*this);
	return *this;
}
StreamBuffer::~StreamBuffer(){
	Clear();
}

// 其他非静态成员函数。
int StreamBuffer::PeekFront() const noexcept {
	if(x_uSize == 0){
		return -1;
	}
	auto pChunk = x_pFirst;
	for(;;){
		if(pChunk->uBegin < pChunk->uEnd){
			return pChunk->abyData[pChunk->uBegin];
		}
		pChunk = pChunk->pNext;
	}
}
int StreamBuffer::PeekBack() const noexcept {
	if(x_uSize == 0){
		return -1;
	}
	auto pChunk = x_pLast;
	for(;;){
		if(pChunk->uBegin < pChunk->uEnd){
			return pChunk->abyData[pChunk->uEnd - 1];
		}
		pChunk = pChunk->pPrev;
	}
}

void StreamBuffer::Clear() noexcept {
	while(x_pFirst){
		const auto pChunk = x_pFirst;
		x_pFirst = pChunk->pNext;
		delete pChunk;
	}
	x_pLast = nullptr;
	x_uSize = 0;
}

int StreamBuffer::Get() noexcept {
	if(x_uSize == 0){
		return -1;
	}

	auto pChunk = x_pFirst;
	for(;;){
		if(pChunk->uBegin < pChunk->uEnd){
			const int nRet = pChunk->abyData[pChunk->uBegin];
			++pChunk->uBegin;
			--x_uSize;
			return nRet;
		}
		pChunk = pChunk->pNext;

		delete x_pFirst;
		x_pFirst = pChunk;
		pChunk->pPrev = nullptr;
	}
}
void StreamBuffer::Put(unsigned char by){
	xChunk *pChunk;
	if(x_pLast && (x_pLast->uEnd < sizeof(x_pLast->abyData))){
		pChunk = x_pLast;
	} else {
		pChunk = new xChunk;
		pChunk->pNext = nullptr;
		// pChunk->pPrev = nullptr;
		pChunk->uBegin = 0;
		pChunk->uEnd = pChunk->uBegin;

		if(x_pLast){
			x_pLast->pNext = pChunk;
		} else {
			x_pFirst = pChunk;
		}
		pChunk->pPrev = x_pLast;
		x_pLast = pChunk;
	}
	pChunk->abyData[pChunk->uEnd] = by;
	++pChunk->uEnd;
	++x_uSize;
}
int StreamBuffer::Unput() noexcept {
	if(x_uSize == 0){
		return -1;
	}

	auto pChunk = x_pLast;
	for(;;){
		if(pChunk->uBegin < pChunk->uEnd){
			--pChunk->uEnd;
			const int nRet = pChunk->abyData[pChunk->uEnd];
			--x_uSize;
			return nRet;
		}
		pChunk = pChunk->pPrev;

		delete x_pLast;
		x_pLast = pChunk;
		pChunk->pNext = nullptr;
	}
}
void StreamBuffer::Unget(unsigned char by){
	xChunk *pChunk;
	if(x_pFirst && (0 < x_pFirst->uBegin)){
		pChunk = x_pFirst;
	} else {
		pChunk = new xChunk;
		// pChunk->pNext = nullptr;
		pChunk->pPrev = nullptr;
		pChunk->uBegin = sizeof(x_pLast->abyData);
		pChunk->uEnd = pChunk->uBegin;

		if(x_pFirst){
			x_pFirst->pPrev = pChunk;
		} else {
			x_pLast = pChunk;
		}
		pChunk->pNext = x_pFirst;
		x_pFirst = pChunk;
	}
	--pChunk->uBegin;
	pChunk->abyData[pChunk->uBegin] = by;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = x_pFirst;
	for(;;){
		const auto pbyWrite = static_cast<unsigned char *>(pData) + uBytesCopied;
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining <= uBytesAvail){
			std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesRemaining);
			return uBytesToCopy;
		}
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesAvail);
		uBytesCopied += uBytesAvail;
		pChunk = pChunk->pNext;
	}
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = x_pFirst;
	for(;;){
		const auto pbyWrite = static_cast<unsigned char *>(pData) + uBytesCopied;
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining <= uBytesAvail){
			std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesRemaining);
			pChunk->uBegin += uBytesRemaining;
			x_uSize -= uBytesToCopy;
			return uBytesToCopy;
		}
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesAvail);
		pChunk->uBegin += uBytesAvail;
		uBytesCopied += uBytesAvail;
		pChunk = pChunk->pNext;

		delete x_pFirst;
		x_pFirst = pChunk;
		pChunk->pPrev = nullptr;
	}
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = x_pFirst;
	for(;;){
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = pChunk->uEnd - pChunk->uBegin;
		if(uBytesRemaining <= uBytesAvail){
			pChunk->uBegin += uBytesRemaining;
			x_uSize -= uBytesToCopy;
			return uBytesToCopy;
		}
		pChunk->uBegin += uBytesAvail;
		uBytesCopied += uBytesAvail;
		pChunk = pChunk->pNext;

		delete x_pFirst;
		x_pFirst = pChunk;
		pChunk->pNext = nullptr;
	}
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	const auto uBytesToCopy = uSize;
	if(uBytesToCopy == 0){
		return;
	}

	xChunk *pLastChunk = nullptr;
	std::size_t uLastChunkAvail = 0;
	if(x_pLast){
		pLastChunk = x_pLast;
		uLastChunkAvail = sizeof(x_pLast->abyData) - x_pLast->uEnd;
	}
	if(uBytesToCopy > uLastChunkAvail){
		const auto uNewChunks = (uBytesToCopy - uLastChunkAvail - 1) / sizeof(x_pLast->abyData) + 1;
		ASSERT(uNewChunks != 0);

		xChunk *pToSpliceFirst = nullptr, *pToSpliceLast = nullptr;
		try {
			for(std::size_t i = 0; i < uNewChunks; ++i){
				auto pChunk = new xChunk;
				pChunk->pNext = nullptr;
				// pChunk->pPrev = nullptr;
				pChunk->uBegin = 0;
				pChunk->uEnd = pChunk->uBegin;

				if(pToSpliceLast){
					pToSpliceLast->pNext = pChunk;
				} else {
					pToSpliceFirst = pChunk;
				}
				pChunk->pPrev = pToSpliceLast;
				pToSpliceLast = pChunk;
			}
		} catch(...){
			while(pToSpliceFirst){
				const auto pChunk = pToSpliceFirst;
				pToSpliceFirst = pChunk->pNext;
				delete pChunk;
			}
			throw;
		}
		if(x_pLast){
			x_pLast->pNext = pToSpliceFirst;
		} else {
			x_pFirst = pToSpliceFirst;
			pLastChunk = pToSpliceFirst;
		}
		pToSpliceFirst->pPrev = x_pLast;
		x_pLast = pToSpliceLast;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = pLastChunk;
	for(;;){
		const auto pbyRead = static_cast<const unsigned char *>(pData) + uBytesCopied;
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = sizeof(x_pLast->abyData) - x_pLast->uEnd;
		if(uBytesRemaining <= uBytesAvail){
			std::memcpy(pChunk->abyData + pChunk->uEnd, pbyRead, uBytesRemaining);
			pChunk->uEnd += uBytesRemaining;
			x_uSize += uBytesToCopy;
			return;
		}
		std::memcpy(pChunk->abyData + pChunk->uEnd, pbyRead, uBytesAvail);
		pChunk->uEnd += uBytesAvail;
		uBytesCopied += uBytesAvail;
		pChunk = pChunk->pNext;
	}
}
void StreamBuffer::Put(const char *pszData){
	Put(pszData, std::strlen(pszData));
}

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
	StreamBuffer sbufRet;

	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return sbufRet;
	}

	if(x_uSize <= uBytesToCopy){
		sbufRet.Swap(*this);
		return sbufRet;
	}

	std::size_t uBytesCopied = 0;
	auto pCutEnd = x_pFirst;
	for(;;){
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = pCutEnd->uEnd - pCutEnd->uBegin;
		if(uBytesRemaining <= uBytesAvail){
			if(uBytesRemaining == uBytesAvail){
				pCutEnd = pCutEnd->pNext;
			} else {
				const auto pChunk = new xChunk;
				pChunk->pNext = pCutEnd;
				pChunk->pPrev = pCutEnd->pPrev;
				pChunk->uBegin = 0;
				// pChunk->uEnd = pChunk->uBegin;

				std::memcpy(pChunk->abyData, pCutEnd->abyData + pCutEnd->uBegin, uBytesRemaining);
				pChunk->uEnd = uBytesRemaining;
				pCutEnd->uBegin += uBytesRemaining;

				if(pCutEnd->pPrev){
					pCutEnd->pPrev->pNext = pChunk;
				} else {
					x_pFirst = pChunk;
				}
				pCutEnd->pPrev = pChunk;
			}

			const auto pCutFirst = x_pFirst;
			const auto pCutLast = pCutEnd->pPrev;

			pCutLast->pNext = nullptr;
			pCutEnd->pPrev = nullptr;

			x_pFirst = pCutEnd;
			x_uSize -= uBytesToCopy;

			sbufRet.x_pFirst = pCutFirst;
			sbufRet.x_pLast = pCutLast;
			sbufRet.x_uSize = uBytesToCopy;

			return sbufRet;
		}
		uBytesCopied += uBytesAvail;
		pCutEnd = pCutEnd->pNext;
	}
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	if(&rhs == this){
		return;
	}
	if(!rhs.x_pFirst){
		return;
	}

	if(x_pLast){
		x_pLast->pNext = rhs.x_pFirst;
	} else {
		x_pFirst = rhs.x_pFirst;
	}
	rhs.x_pFirst->pPrev = x_pLast;
	x_pLast = rhs.x_pLast;
	x_uSize += rhs.x_uSize;

	rhs.x_pFirst = nullptr;
	rhs.x_pLast = nullptr;
	rhs.x_uSize = 0;
}

}
