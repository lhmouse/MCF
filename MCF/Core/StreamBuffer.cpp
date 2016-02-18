// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include "FixedSizeAllocator.hpp"

namespace MCF {

namespace {
	using Chunk = Impl_StreamBuffer::Chunk;

	__attribute__((__init_priority__(101)))
	FixedSizeAllocator<sizeof(Chunk)> g_vChunkAllocator;
}

namespace Impl_StreamBuffer {
	void *Chunk::operator new(std::size_t uSize){
		ASSERT(uSize == sizeof(Chunk));
		(void)uSize;
		return g_vChunkAllocator.Allocate();
	}
	void Chunk::operator delete(void *pRaw) noexcept {
		g_vChunkAllocator.Deallocate(pRaw);
	}
}

// 其他非静态成员函数。
int StreamBuffer::Peek() const noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetFirst();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uBegin];
	}
	return nRet;
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
void StreamBuffer::Put(unsigned char by){
	auto pChunk = x_lstChunks.GetLast();
	if(!pChunk || (pChunk->uEnd == sizeof(Chunk::abyData))){
		pChunk = &x_lstChunks.Push(Chunk::FromBeginning());
	}
	pChunk->abyData[pChunk->uEnd] = by;
	++(pChunk->uEnd);
	++x_uSize;
}
int StreamBuffer::Unpeek() const noexcept {
	int nRet = -1;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		nRet = pChunk->abyData[pChunk->uEnd - 1];
	}
	return nRet;
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
void StreamBuffer::Unget(unsigned char by){
	auto pChunk = x_lstChunks.GetFirst();
	if(!pChunk || (0 == pChunk->uBegin)){
		pChunk = &x_lstChunks.Unshift(Chunk::FromEnd());
	}
	--(pChunk->uBegin);
	pChunk->abyData[pChunk->uBegin] = by;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	std::size_t uBytesCopied = 0;
	auto pChunk = x_lstChunks.GetFirst();
	while((uBytesCopied < uSize) && pChunk){
		const auto uBytesToCopy = Min(uSize - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
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
		const auto uBytesToCopy = Min(uSize - uBytesCopied, pChunk->uEnd - pChunk->uBegin);
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
		const auto uBytesToDiscard = Min(uSize - uBytesDiscarded, pChunk->uEnd - pChunk->uBegin);

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
	List<Chunk> lstNewChunks;
	std::size_t uBytesReserved = 0;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		uBytesReserved += pChunk->uEnd - pChunk->uBegin;
	}
	while(uBytesReserved < uSize){
		lstNewChunks.Push(Chunk::FromBeginning());
		uBytesReserved += sizeof(Chunk::abyData);
	}
	x_lstChunks.Splice(nullptr, lstNewChunks);
	if(!pChunk){
		pChunk = x_lstChunks.GetFirst();
	}

	std::size_t uBytesCopied = 0;
	while(uBytesCopied < uSize){
		const auto uBytesToCopy = Min(uSize - uBytesCopied, sizeof(Chunk::abyData) - pChunk->uEnd);
		CopyN(pChunk->abyData + pChunk->uEnd, static_cast<const unsigned char *>(pData) + uBytesCopied, uBytesToCopy);
		pChunk->uEnd += uBytesToCopy;

		uBytesCopied += uBytesToCopy;
		pChunk = x_lstChunks.GetNext(pChunk);
	}
	x_uSize += uBytesCopied;
}
void StreamBuffer::Put(unsigned char by, std::size_t uSize){
	List<Chunk> lstNewChunks;
	std::size_t uBytesReserved = 0;
	auto pChunk = x_lstChunks.GetLast();
	if(pChunk){
		uBytesReserved += pChunk->uEnd - pChunk->uBegin;
	}
	while(uBytesReserved < uSize){
		lstNewChunks.Push(Chunk::FromBeginning());
		uBytesReserved += sizeof(Chunk::abyData);
	}
	x_lstChunks.Splice(nullptr, lstNewChunks);
	if(!pChunk){
		pChunk = x_lstChunks.GetFirst();
	}

	std::size_t uBytesCopied = 0;
	while(uBytesCopied < uSize){
		const auto uBytesToCopy = Min(uSize - uBytesCopied, sizeof(Chunk::abyData) - pChunk->uEnd);
		FillN(pChunk->abyData + pChunk->uEnd, uBytesToCopy, by);
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
		const auto uBytesToCut = Min(uSize - uBytesCut, pChunk->uEnd - pChunk->uBegin);
		if(uBytesToCut < pChunk->uEnd - pChunk->uBegin){
			const auto pOldChunk = pChunk;
			pChunk = x_lstChunks.Emplace(pChunk, Chunk::FromBeginning());
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

}
