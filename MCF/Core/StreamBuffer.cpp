// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Thread/SpinLock.hpp"

namespace MCF {

struct StreamBuffer::$Chunk final {
	static SpinLock s_splPoolMutex;
	static $Chunk *__restrict__ s_pPoolHead;

	static void *operator new(std::size_t uSize){
		ASSERT(uSize == sizeof($Chunk));

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
		const auto pPooled = static_cast<$Chunk *>(pRaw);

		const auto uLockValue = s_splPoolMutex.Lock();
		pPooled->pPrev = s_pPoolHead;
		s_pPoolHead = pPooled;
		s_splPoolMutex.Unlock(uLockValue);
	}

	__attribute__((__destructor__(101)))
	static void PoolDestructor() noexcept {
		while(s_pPoolHead){
			const auto pPooled = s_pPoolHead;
			s_pPoolHead = pPooled->pPrev;
			::operator delete(pPooled);
		}
	}

	$Chunk *pPrev;
	$Chunk *pNext;
	unsigned uBegin;
	unsigned uEnd;
	unsigned char abyData[0x100];
};

SpinLock StreamBuffer::$Chunk::s_splPoolMutex;
StreamBuffer::$Chunk *__restrict__ StreamBuffer::$Chunk::s_pPoolHead = nullptr;

unsigned char *StreamBuffer::ChunkEnumerator::GetBegin() const noexcept {
	ASSERT($pChunk);

	return $pChunk->abyData + $pChunk->uBegin;
}
unsigned char *StreamBuffer::ChunkEnumerator::GetEnd() const noexcept {
	ASSERT($pChunk);

	return $pChunk->abyData + $pChunk->uEnd;
}

StreamBuffer::ChunkEnumerator &StreamBuffer::ChunkEnumerator::operator++() noexcept {
	ASSERT($pChunk);

	$pChunk = $pChunk->pNext;
	return *this;
}

const unsigned char *StreamBuffer::ConstChunkEnumerator::GetBegin() const noexcept {
	ASSERT($pChunk);

	return $pChunk->abyData + $pChunk->uBegin;
}
const unsigned char *StreamBuffer::ConstChunkEnumerator::GetEnd() const noexcept {
	ASSERT($pChunk);

	return $pChunk->abyData + $pChunk->uEnd;
}

StreamBuffer::ConstChunkEnumerator &StreamBuffer::ConstChunkEnumerator::operator++() noexcept {
	ASSERT($pChunk);

	$pChunk = $pChunk->pNext;
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
	if($uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = $pFirst;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uBegin];
		}
		pChunk = pChunk->pNext;
	} while(nRet < 0);
	return nRet;
}
int StreamBuffer::PeekBack() const noexcept {
	if($uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = $pLast;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uEnd - 1];
		}
		pChunk = pChunk->pPrev;
	} while(nRet < 0);
	return nRet;
}

void StreamBuffer::Clear() noexcept {
	while($pFirst){
		const auto pChunk = $pFirst;
		$pFirst = pChunk->pNext;
		delete pChunk;
	}
	$pLast = nullptr;
	$uSize = 0;
}

int StreamBuffer::Get() noexcept {
	if($uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = $pFirst;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			nRet = pChunk->abyData[pChunk->uBegin];
			++(pChunk->uBegin);
		}
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete $pFirst;
			$pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				$pLast = nullptr;
			}
		}
	} while(nRet < 0);
	$uSize -= 1;
	return nRet;
}
void StreamBuffer::Put(unsigned char by){
	std::size_t uLastChunkAvail = 0;
	if($pLast){
		uLastChunkAvail = sizeof($pLast->abyData) - $pLast->uEnd;
	}
	$Chunk *pLastChunk = nullptr;
	if(uLastChunkAvail != 0){
		pLastChunk = $pLast;
	} else {
		auto pChunk = new $Chunk;
		pChunk->pNext = nullptr;
		// pChunk->pPrev = nullptr;
		pChunk->uBegin = 0;
		pChunk->uEnd = 0;

		if($pLast){
			$pLast->pNext = pChunk;
		} else {
			$pFirst = pChunk;
		}
		pChunk->pPrev = $pLast;
		$pLast = pChunk;

		if(!pLastChunk){
			pLastChunk = pChunk;
		}
	}

	auto pChunk = pLastChunk;
	pChunk->abyData[pChunk->uEnd] = by;
	++(pChunk->uEnd);
	++$uSize;
}
int StreamBuffer::Unput() noexcept {
	if($uSize == 0){
		return -1;
	}

	int nRet = -1;
	auto pChunk = $pLast;
	do {
		if(pChunk->uEnd != pChunk->uBegin){
			--(pChunk->uEnd);
			nRet = pChunk->abyData[pChunk->uEnd];
		}
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pPrev;
			delete $pLast;
			$pLast = pChunk;

			if(pChunk){
				pChunk->pNext = nullptr;
			} else {
				$pFirst = nullptr;
			}
		}
	} while(nRet < 0);
	$uSize -= 1;
	return nRet;
}
void StreamBuffer::Unget(unsigned char by){
	std::size_t uFirstChunkAvail = 0;
	if($pFirst){
		uFirstChunkAvail = $pFirst->uBegin;
	}
	$Chunk *pFirstChunk = nullptr;
	if(uFirstChunkAvail != 0){
		pFirstChunk = $pFirst;
	} else {
		auto pChunk = new $Chunk;
		// pChunk->pNext = nullptr;
		pChunk->pPrev = nullptr;
		pChunk->uBegin = sizeof(pChunk->abyData);
		pChunk->uEnd = sizeof(pChunk->abyData);

		if($pFirst){
			$pFirst->pPrev = pChunk;
		} else {
			$pLast = pChunk;
		}
		pChunk->pNext = $pFirst;
		$pFirst = pChunk;

		if(!pFirstChunk){
			pFirstChunk = pChunk;
		}
	}

	auto pChunk = pFirstChunk;
	--(pChunk->uBegin);
	pChunk->abyData[pChunk->uBegin] = by;
	++$uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	const auto uBytesToCopy = Min(uSize, $uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = $pFirst;
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
	const auto uBytesToCopy = Min(uSize, $uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = $pFirst;
	do {
		const auto pbyWrite = static_cast<unsigned char *>(pData) + uBytesCopied;
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
		std::memcpy(pbyWrite, pChunk->abyData + pChunk->uBegin, uBytesToCopyThisTime);
		uBytesCopied += uBytesToCopyThisTime;
		pChunk->uBegin += uBytesToCopyThisTime;
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete $pFirst;
			$pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				$pLast = nullptr;
			}
		}
	} while(uBytesCopied < uBytesToCopy);
	$uSize -= uBytesToCopy;
	return uBytesCopied;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	const auto uBytesToCopy = Min(uSize, $uSize);
	if(uBytesToCopy == 0){
		return 0;
	}

	std::size_t uBytesCopied = 0;
	auto pChunk = $pFirst;
	do {
		const auto uBytesToCopyThisTime = Min(uBytesToCopy - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
		uBytesCopied += uBytesToCopyThisTime;
		pChunk->uBegin += uBytesToCopyThisTime;
		if(pChunk->uBegin == pChunk->uEnd){
			pChunk = pChunk->pNext;
			delete $pFirst;
			$pFirst = pChunk;

			if(pChunk){
				pChunk->pPrev = nullptr;
			} else {
				$pLast = nullptr;
			}
		}
	} while(uBytesCopied < uBytesToCopy);
	$uSize -= uBytesToCopy;
	return uBytesCopied;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	const auto uBytesToCopy = uSize;
	if(uBytesToCopy == 0){
		return;
	}

	std::size_t uLastChunkAvail = 0;
	if($pLast){
		uLastChunkAvail = sizeof($pLast->abyData) - $pLast->uEnd;
	}
	$Chunk *pLastChunk = nullptr;
	if(uLastChunkAvail != 0){
		pLastChunk = $pLast;
	}
	if(uBytesToCopy > uLastChunkAvail){
		const auto uNewChunks = (uBytesToCopy - uLastChunkAvail - 1) / sizeof(pLastChunk->abyData) + 1;
		ASSERT(uNewChunks != 0);

		auto pChunk = new $Chunk;
		pChunk->pNext = nullptr;
		pChunk->pPrev = nullptr;
		pChunk->uBegin = 0;
		pChunk->uEnd = 0;

		auto pToSpliceFirst = pChunk, pToSpliceLast = pChunk;
		try {
			for(std::size_t i = 1; i < uNewChunks; ++i){
				pChunk = new $Chunk;
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
		if($pLast){
			$pLast->pNext = pToSpliceFirst;
		} else {
			$pFirst = pToSpliceFirst;
		}
		pToSpliceFirst->pPrev = $pLast;
		$pLast = pToSpliceLast;

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
	$uSize += uBytesToCopy;
}
void StreamBuffer::Put(const char *pszData){
	Put(pszData, std::strlen(pszData));
}

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
	StreamBuffer sbufRet;

	const auto uBytesToCopy = Min(uSize, $uSize);
	if(uBytesToCopy == 0){
		return sbufRet;
	}

	if($uSize <= uBytesToCopy){
		sbufRet.Swap(*this);
		return sbufRet;
	}

	std::size_t uBytesCopied = 0;
	auto pCutEnd = $pFirst;
	for(;;){
		const auto uBytesRemaining = uBytesToCopy - uBytesCopied;
		const auto uBytesAvail = pCutEnd->uEnd - pCutEnd->uBegin;
		if(uBytesRemaining <= uBytesAvail){
			if(uBytesRemaining == uBytesAvail){
				pCutEnd = pCutEnd->pNext;
			} else {
				const auto pChunk = new $Chunk;
				pChunk->pNext = pCutEnd;
				pChunk->pPrev = pCutEnd->pPrev;
				pChunk->uBegin = 0;
				pChunk->uEnd = uBytesRemaining;

				std::memcpy(pChunk->abyData, pCutEnd->abyData + pCutEnd->uBegin, uBytesRemaining);
				pCutEnd->uBegin += uBytesRemaining;

				if(pCutEnd->pPrev){
					pCutEnd->pPrev->pNext = pChunk;
				} else {
					$pFirst = pChunk;
				}
				pCutEnd->pPrev = pChunk;
			}
			break;
		}
		uBytesCopied += uBytesAvail;
		pCutEnd = pCutEnd->pNext;
	}

	const auto pCutFirst = $pFirst;
	const auto pCutLast = pCutEnd->pPrev;
	pCutLast->pNext = nullptr;
	pCutEnd->pPrev = nullptr;

	$pFirst = pCutEnd;
	$uSize -= uBytesToCopy;

	sbufRet.$pFirst = pCutFirst;
	sbufRet.$pLast = pCutLast;
	sbufRet.$uSize = uBytesToCopy;
	return sbufRet;
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	if(&rhs == this){
		return;
	}
	if(!rhs.$pFirst){
		return;
	}

	if($pLast){
		$pLast->pNext = rhs.$pFirst;
	} else {
		$pFirst = rhs.$pFirst;
	}
	rhs.$pFirst->pPrev = $pLast;
	$pLast = rhs.$pLast;
	$uSize += rhs.$uSize;

	rhs.$pFirst = nullptr;
	rhs.$pLast = nullptr;
	rhs.$uSize = 0;
}

}
