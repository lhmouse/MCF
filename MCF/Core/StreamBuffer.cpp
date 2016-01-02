// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Thread/Mutex.hpp"

namespace MCF {

struct StreamBuffer::X_Chunk final {
	static Mutex             s_mtxPoolMutex;
	static X_Chunk *restrict s_pPoolHead;

	static void *operator new(std::size_t uSize){
		ASSERT(uSize == sizeof(X_Chunk));

		{
			const Mutex::UniqueLock vLock(s_mtxPoolMutex);

			const auto pChunk = s_pPoolHead;
			if(pChunk){
				s_pPoolHead = pChunk->pPrev;
				return pChunk;
			}
		}
		return ::operator new(uSize);
	}
	static void operator delete(void *pRaw) noexcept {
		if(!pRaw){
			return;
		}

		const Mutex::UniqueLock vLock(s_mtxPoolMutex);

		const auto pChunk = static_cast<X_Chunk *>(pRaw);
		pChunk->pPrev = s_pPoolHead;
		s_pPoolHead = pChunk;
	}

	__attribute__((__destructor__(101)))
	static void PoolDestructor() noexcept {
		while(s_pPoolHead){
			const auto pChunk = s_pPoolHead;
			s_pPoolHead = pChunk->pPrev;
			::operator delete(pChunk);
		}
	}

	X_Chunk *pPrev;
	X_Chunk *pNext;
	std::size_t uBegin;
	std::size_t uEnd;
	unsigned char abyData[0x100];
};

Mutex                           StreamBuffer::X_Chunk::s_mtxPoolMutex;
StreamBuffer::X_Chunk *restrict StreamBuffer::X_Chunk::s_pPoolHead     = nullptr;

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
	for(auto ce = rhs.EnumerateFirstChunk(); ce; ++ce){
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
	rhs.Swap(*this);
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

	int nRet = -1;
	auto pChunk = x_pFirst;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uBegin];
		}
		pChunk = pChunk->pNext;
	} while(nRet < 0);
	return nRet;
}
int StreamBuffer::PeekBack() const noexcept {
	if(x_uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = x_pLast;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uEnd - 1];
		}
		pChunk = pChunk->pPrev;
	} while(nRet < 0);
	return nRet;
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

	int nRet = -1;
	auto pChunk = x_pFirst;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uBegin];
			++(pChunk->uBegin);
		}
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete x_pFirst;
			x_pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				x_pLast = nullptr;
			}
		}
	} while(nRet < 0);
	x_uSize -= 1;
	return nRet;
}
void StreamBuffer::Put(unsigned char by){
	std::size_t uLastChunkAvail = 0;
	if(x_pLast){
		uLastChunkAvail = sizeof(x_pLast->abyData) - x_pLast->uEnd;
	}
	X_Chunk *pLastChunk = nullptr;
	if(uLastChunkAvail != 0){
		pLastChunk = x_pLast;
	} else {
		auto pChunk = new X_Chunk;
		pChunk->pNext = nullptr;
		// pChunk->pPrev = nullptr;
		pChunk->uBegin = 0;
		pChunk->uEnd = 0;

		if(x_pLast){
			x_pLast->pNext = pChunk;
		} else {
			x_pFirst = pChunk;
		}
		pChunk->pPrev = x_pLast;
		x_pLast = pChunk;

		if(!pLastChunk){
			pLastChunk = pChunk;
		}
	}

	auto pChunk = pLastChunk;
	pChunk->abyData[pChunk->uEnd] = by;
	++(pChunk->uEnd);
	++x_uSize;
}
int StreamBuffer::Unput() noexcept {
	if(x_uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = x_pLast;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			--(pChunk->uEnd);
			nRet = pChunk->abyData[pChunk->uEnd];
		}
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pPrev;
			delete x_pLast;
			x_pLast = pChunk;

			if(pChunk){
				pChunk->pNext = nullptr;
			} else {
				x_pFirst = nullptr;
			}
		}
	} while(nRet < 0);
	x_uSize -= 1;
	return nRet;
}
void StreamBuffer::Unget(unsigned char by){
	std::size_t uFirstChunkAvail = 0;
	if(x_pFirst){
		uFirstChunkAvail = x_pFirst->uBegin;
	}
	X_Chunk *pFirstChunk = nullptr;
	if(uFirstChunkAvail != 0){
		pFirstChunk = x_pFirst;
	} else {
		auto pChunk = new X_Chunk;
		// pChunk->pNext = nullptr;
		pChunk->pPrev = nullptr;
		pChunk->uBegin = sizeof(pChunk->abyData);
		pChunk->uEnd = sizeof(pChunk->abyData);

		if(x_pFirst){
			x_pFirst->pPrev = pChunk;
		} else {
			x_pLast = pChunk;
		}
		pChunk->pNext = x_pFirst;
		x_pFirst = pChunk;

		if(!pFirstChunk){
			pFirstChunk = pChunk;
		}
	}

	auto pChunk = pFirstChunk;
	--(pChunk->uBegin);
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
	do {
		const auto pbyWrite = static_cast<unsigned char *>(pData) + uBytesCopied;
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesToCopyThisTime);
		uBytesCopied += uBytesToCopyThisTime;
		pChunk = pChunk->pNext;
	} while(uBytesCopied < uBytesToCopy);
	return uBytesCopied;
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = x_pFirst;
	do {
		const auto pbyWrite = static_cast<unsigned char *>(pData) + uBytesCopied;
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesToCopyThisTime);
		uBytesCopied += uBytesToCopyThisTime;
		pChunk->uBegin += uBytesToCopyThisTime;
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete x_pFirst;
			x_pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				x_pLast = nullptr;
			}
		}
	} while(uBytesCopied < uBytesToCopy);
	x_uSize -= uBytesToCopy;
	return uBytesCopied;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	const auto uBytesToCopy = Min(uSize, x_uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = x_pFirst;
	do {
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
		uBytesCopied += uBytesToCopyThisTime;
		pChunk->uBegin += uBytesToCopyThisTime;
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete x_pFirst;
			x_pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				x_pLast = nullptr;
			}
		}
	} while(uBytesCopied < uBytesToCopy);
	x_uSize -= uBytesToCopy;
	return uBytesCopied;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	const auto uBytesToCopy = uSize;
	if(uBytesToCopy == 0){
		return;
	}

	std::size_t uLastChunkAvail = 0;
	if(x_pLast){
		uLastChunkAvail = sizeof(x_pLast->abyData) - x_pLast->uEnd;
	}
	X_Chunk *pLastChunk = nullptr;
	if(uLastChunkAvail != 0){
		pLastChunk = x_pLast;
	}
	if(uBytesToCopy > uLastChunkAvail){
		const auto uNewChunks = (uBytesToCopy - uLastChunkAvail - 1) / sizeof(pLastChunk->abyData) + 1;
		ASSERT(uNewChunks != 0);

		auto pChunk = new X_Chunk;
		pChunk->pNext = nullptr;
		pChunk->pPrev = nullptr;
		pChunk->uBegin = 0;
		pChunk->uEnd = 0;

		auto pToSpliceFirst = pChunk, pToSpliceLast = pChunk;
		try {
			for(std::size_t i = 1; i < uNewChunks; ++i){
				pChunk = new X_Chunk;
				pChunk->pNext = nullptr;
				pChunk->pPrev = pToSpliceLast;
				pChunk->uBegin = 0;
				pChunk->uEnd = 0;

				pToSpliceLast->pNext = pChunk;
				pToSpliceLast = pChunk;
			}
		} catch(...){
			do {
				pChunk = pToSpliceFirst;
				pToSpliceFirst = pChunk->pNext;
				delete pChunk;
			} while(pToSpliceFirst);

			throw;
		}
		if(x_pLast){
			x_pLast->pNext = pToSpliceFirst;
		} else {
			x_pFirst = pToSpliceFirst;
		}
		pToSpliceFirst->pPrev = x_pLast;
		x_pLast = pToSpliceLast;

		if(!pLastChunk){
			pLastChunk = pToSpliceFirst;
		}
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = pLastChunk;
	do {
		const auto pbyRead = static_cast<const unsigned char *>(pData) + uBytesCopied;
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, sizeof(pChunk->abyData) - pChunk->uEnd);
		std::memcpy(pChunk->abyData + pChunk->uEnd, pbyRead, uBytesToCopyThisTime);
		pChunk->uEnd += uBytesToCopyThisTime;
		uBytesCopied += uBytesToCopyThisTime;
		pChunk = pChunk->pNext;
	} while(uBytesCopied < uBytesToCopy);
	x_uSize += uBytesToCopy;
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
				const auto pChunk = new X_Chunk;
				pChunk->pNext = pCutEnd;
				pChunk->pPrev = pCutEnd->pPrev;
				pChunk->uBegin = 0;
				pChunk->uEnd = uBytesRemaining;

				std::memcpy(pChunk->abyData, pCutEnd->abyData + pCutEnd->uBegin, uBytesRemaining);
				pCutEnd->uBegin += uBytesRemaining;

				if(pCutEnd->pPrev){
					pCutEnd->pPrev->pNext = pChunk;
				} else {
					x_pFirst = pChunk;
				}
				pCutEnd->pPrev = pChunk;
			}
			break;
		}
		uBytesCopied += uBytesAvail;
		pCutEnd = pCutEnd->pNext;
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
