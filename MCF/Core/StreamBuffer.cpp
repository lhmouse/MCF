// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include "FixedSizeAllocator.hpp"

namespace MCF {

FixedSizeAllocator<List<StreamBuffer::X_Chunk, StreamBuffer::X_ChunkAllocator>::kNodeSize> StreamBuffer::X_ChunkAllocator::s_vPool;

void *StreamBuffer::X_ChunkAllocator::operator()(std::size_t uSize){
	ASSERT(uSize == (List<X_Chunk, X_ChunkAllocator>::kNodeSize));
	(void)uSize;
	return s_vPool.Allocate();
}
void StreamBuffer::X_ChunkAllocator::operator()(void *pBlock) noexcept {
	return s_vPool.Deallocate(pBlock);
}

int StreamBuffer::PeekFront() const noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetFirst();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];
	}
	return nRet;
}
int StreamBuffer::PeekBack() const noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uEnd - 1];
	}
	return nRet;
}

int StreamBuffer::Peek() noexcept {
	return PeekFront();
}
int StreamBuffer::Get() noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetFirst();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];

		++(pChunk->uBegin);
		if(pChunk->uBegin == pChunk->uEnd){
			x_lstChunks.Shift();
		}
		--x_uSize;
	}
	return nRet;
}
bool StreamBuffer::Discard() noexcept {
	return Get() >= 0;
}
void StreamBuffer::Put(unsigned char byData){
	auto pChunk = x_lstChunks.GetLast();
	if(!pChunk || (pChunk->uEnd == sizeof(X_Chunk::abyData))){
		pChunk = &x_lstChunks.Push(X_Chunk::FromBeginning());
	}
	pChunk->abyData[pChunk->uEnd] = byData;
	++(pChunk->uEnd);
	++x_uSize;
}

int StreamBuffer::Unput() noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uEnd - 1];

		--(pChunk->uEnd);
		if(pChunk->uBegin == pChunk->uEnd){
			x_lstChunks.Pop();
		}
		--x_uSize;
	}
	return nRet;
}
void StreamBuffer::Unget(unsigned char byData){
	auto pChunk = x_lstChunks.GetFirst();
	if(!pChunk || (0 == pChunk->uBegin)){
		pChunk = &x_lstChunks.Unshift(X_Chunk::FromEnd());
	}
	--(pChunk->uBegin);
	pChunk->abyData[pChunk->uBegin] = byData;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) noexcept {
	std::size_t uBytesCopied = 0;
	auto pChunk = x_lstChunks.GetFirst();
	while((uBytesCopied < uSize) && pChunk){
		const auto uBytesToCopy = static_cast<unsigned>(Min(uSize - uBytesCopied, pChunk->uEnd - pChunk->uBegin));
		CopyN(static_cast<unsigned char *>(pData) + uBytesCopied, pChunk->abyData + pChunk->uBegin, uBytesToCopy);

		uBytesCopied += uBytesToCopy;
		pChunk = x_lstChunks.GetNext(pChunk);
	}
	return uBytesCopied;
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	std::size_t uBytesCopied = 0;
	auto pChunk = x_lstChunks.GetFirst();
	while((uBytesCopied < uSize) && pChunk){
		const auto uBytesToCopy = static_cast<unsigned>(Min(uSize - uBytesCopied, pChunk->uEnd - pChunk->uBegin));
		CopyN(static_cast<unsigned char *>(pData) + uBytesCopied, pChunk->abyData + pChunk->uBegin, uBytesToCopy);

		pChunk->uBegin += uBytesToCopy;
		if(pChunk->uBegin == pChunk->uEnd){
			x_lstChunks.Shift();
		}
		uBytesCopied += uBytesToCopy;
		pChunk = x_lstChunks.GetFirst();
	}
	x_uSize -= uBytesCopied;
	return uBytesCopied;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	std::size_t uBytesDiscarded = 0;
	auto pChunk = x_lstChunks.GetFirst();
	while((uBytesDiscarded < uSize) && pChunk){
		const auto uBytesToDiscard = static_cast<unsigned>(Min(uSize - uBytesDiscarded, pChunk->uEnd - pChunk->uBegin));

		pChunk->uBegin += uBytesToDiscard;
		if(pChunk->uBegin == pChunk->uEnd){
			x_lstChunks.Shift();
		}
		uBytesDiscarded += uBytesToDiscard;
		pChunk = x_lstChunks.GetFirst();
	}
	x_uSize -= uBytesDiscarded;
	return uBytesDiscarded;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	List<X_Chunk, X_ChunkAllocator> lstNewChunks;
	std::size_t uBytesReserved = 0;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		uBytesReserved += sizeof(X_Chunk::abyData) - pChunk->uEnd;
	}
	while(uBytesReserved < uSize){
		lstNewChunks.Push(X_Chunk::FromBeginning());
		uBytesReserved += sizeof(X_Chunk::abyData);
	}
	x_lstChunks.Splice(nullptr, lstNewChunks);
	if(!pChunk){
		pChunk = x_lstChunks.GetFirst();
	}

	std::size_t uBytesCopied = 0;
	while(uBytesCopied < uSize){
		const auto uBytesToCopy = static_cast<unsigned>(Min(uSize - uBytesCopied, sizeof(X_Chunk::abyData) - pChunk->uEnd));
		CopyN(pChunk->abyData + pChunk->uEnd, static_cast<const unsigned char *>(pData) + uBytesCopied, uBytesToCopy);
		pChunk->uEnd += uBytesToCopy;

		uBytesCopied += uBytesToCopy;
		pChunk = x_lstChunks.GetNext(pChunk);
	}
	x_uSize += uBytesCopied;
}
void StreamBuffer::Put(unsigned char byData, std::size_t uSize){
	List<X_Chunk, X_ChunkAllocator> lstNewChunks;
	std::size_t uBytesReserved = 0;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		uBytesReserved += sizeof(X_Chunk::abyData) - pChunk->uEnd;
	}
	while(uBytesReserved < uSize){
		lstNewChunks.Push(X_Chunk::FromBeginning());
		uBytesReserved += sizeof(X_Chunk::abyData);
	}
	x_lstChunks.Splice(nullptr, lstNewChunks);
	if(!pChunk){
		pChunk = x_lstChunks.GetFirst();
	}

	std::size_t uBytesCopied = 0;
	while(uBytesCopied < uSize){
		const auto uBytesToCopy = static_cast<unsigned>(Min(uSize - uBytesCopied, sizeof(X_Chunk::abyData) - pChunk->uEnd));
		FillN(pChunk->abyData + pChunk->uEnd, uBytesToCopy, byData);
		pChunk->uEnd += uBytesToCopy;

		uBytesCopied += uBytesToCopy;
		pChunk = x_lstChunks.GetNext(pChunk);
	}
	x_uSize += uBytesCopied;
}

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
	std::size_t uBytesCut = 0;
	auto pChunk = x_lstChunks.GetFirst();
	while((uBytesCut < uSize) && pChunk){
		const auto uBytesToCut = static_cast<unsigned>(Min(uSize - uBytesCut, pChunk->uEnd - pChunk->uBegin));
		if(uBytesToCut < pChunk->uEnd - pChunk->uBegin){
			const auto pOldChunk = pChunk;
			pChunk = x_lstChunks.Emplace(pChunk, X_Chunk::FromBeginning());
			CopyN(pChunk->abyData + pChunk->uEnd, pOldChunk->abyData + pOldChunk->uBegin, uBytesToCut);
			pChunk->uEnd += uBytesToCut;
			pOldChunk->uBegin += uBytesToCut;
		}

		uBytesCut += uBytesToCut;
		pChunk = x_lstChunks.GetNext(pChunk);
	}
	x_uSize -= uBytesCut;

	StreamBuffer sbufRet;
	sbufRet.x_lstChunks.Splice(nullptr, x_lstChunks, x_lstChunks.GetFirst(), pChunk);
	sbufRet.x_uSize += uBytesCut;
	return sbufRet;
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	x_lstChunks.Splice(nullptr, rhs.x_lstChunks);
	x_uSize += rhs.x_uSize;
	rhs.x_uSize = 0;
}

void StreamBuffer::Dump(Vector<unsigned char> &vecData) const {
	vecData.ReserveMore(GetSize());
	Iterate(
		[&](auto pbyData, auto uSize){
			vecData.UncheckedAppend(pbyData, pbyData + uSize);
			return true;
		});
}

}
