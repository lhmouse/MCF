// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/Algorithms.hpp"
#include "../Thread/Mutex.hpp"
using namespace MCF;

constexpr std::size_t CHUNK_SIZE = 0x400;

namespace MCF {

struct StreamBufferChunk {
	unsigned uRead;
	unsigned uWrite;
	unsigned char abyData[CHUNK_SIZE];
};

}

namespace {

using ChunkNode = typename List<StreamBufferChunk>::Node;

Mutex g_mtxPoolMutex;
List<StreamBufferChunk> g_lstPool;

StreamBufferChunk &PushPooled(List<StreamBufferChunk> &lstDst){
	ChunkNode *pNode;
	{
		const auto vLock = g_mtxPoolMutex.GetLock();
		if(!g_lstPool.IsEmpty()){
			pNode = g_lstPool.GetFirst();
			lstDst.Splice(nullptr, g_lstPool, g_lstPool.GetFirst());
			goto jDone;
		}
	}
	pNode = lstDst.Push();

jDone:
#ifndef NDEBUG
	__builtin_memset(&pNode->GetElement(), 0xCC, sizeof(StreamBufferChunk));
#endif
	return pNode->GetElement();
}
void PopPooled(List<StreamBufferChunk> &lstSrc) noexcept {
	ASSERT(!lstSrc.IsEmpty());

	const auto vLock = g_mtxPoolMutex.GetLock();
	g_lstPool.Splice(nullptr, lstSrc, lstSrc.GetLast());
}
StreamBufferChunk &UnshiftPooled(List<StreamBufferChunk> &lstDst){
	ChunkNode *pNode;
	{
		const auto vLock = g_mtxPoolMutex.GetLock();
		if(!g_lstPool.IsEmpty()){
			pNode = g_lstPool.GetFirst();
			lstDst.Splice(lstDst.GetFirst(), g_lstPool, g_lstPool.GetFirst());
			goto jDone;
		}
	}
	pNode = lstDst.Unshift();

jDone:
#ifndef NDEBUG
	__builtin_memset(&pNode->GetElement(), 0xCC, sizeof(StreamBufferChunk));
#endif
	return pNode->GetElement();
}
void ShiftPooled(List<StreamBufferChunk> &lstSrc) noexcept {
	ASSERT(!lstSrc.IsEmpty());

	const auto vLock = g_mtxPoolMutex.GetLock();
	g_lstPool.Splice(nullptr, lstSrc, lstSrc.GetFirst());
}
void ClearPooled(List<StreamBufferChunk> &lstSrc) noexcept {
	if(lstSrc.IsEmpty()){
		return;
	}
	const auto vLock = g_mtxPoolMutex.GetLock();
	g_lstPool.Splice(nullptr, lstSrc);
}

}

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_lstBuffers(), xm_uSize(0)
{
}
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
	for(auto pNode = rhs.xm_lstBuffers.GetFirst(); pNode; pNode = pNode->GetNext()){
		const auto &vBuffer = pNode->GetElement();
		Put(vBuffer.abyData + vBuffer.uRead, vBuffer.uWrite - vBuffer.uRead);
	}
}
StreamBuffer::StreamBuffer(StreamBuffer &&rhs) noexcept
	: StreamBuffer()
{
	Swap(rhs);
}
StreamBuffer &StreamBuffer::operator=(const StreamBuffer &rhs){
	if(&rhs != this){
		StreamBuffer(rhs).Swap(*this);
	}
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
void StreamBuffer::Clear() noexcept {
	ClearPooled(xm_lstBuffers);
	xm_uSize = 0;
}

int StreamBuffer::Peek() const noexcept {
	if(xm_uSize == 0){
		return -1;
	}
	auto pNode = xm_lstBuffers.GetFirst();
	for(;;){
		ASSERT(pNode);

		const auto &vBuffer = pNode->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			return vBuffer.abyData[vBuffer.uRead];
		}
		pNode = pNode->GetNext();
	}
}
int StreamBuffer::Get() noexcept {
	if(xm_uSize == 0){
		return -1;
	}
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetFirst()->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			const int nRet = vBuffer.abyData[vBuffer.uRead];
			++vBuffer.uRead;
			--xm_uSize;
			return nRet;
		}
		ShiftPooled(xm_lstBuffers);
	}
}
void StreamBuffer::Put(unsigned char by){
	const auto pNode = xm_lstBuffers.GetLast();
	if(pNode && (pNode->GetElement().uWrite < CHUNK_SIZE)){
		auto &vBuffer = pNode->GetElement();
		vBuffer.abyData[vBuffer.uWrite] = by;
		++vBuffer.uWrite;
		++xm_uSize;
		return;
	}
	auto &vBuffer = PushPooled(xm_lstBuffers);
	vBuffer.abyData[0] = by;
	vBuffer.uRead = 0;
	vBuffer.uWrite = 1;
	++xm_uSize;
}
int StreamBuffer::Unput() noexcept {
	if(xm_uSize == 0){
		return -1;
	}
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetLast()->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			--vBuffer.uWrite;
			const int nRet = vBuffer.abyData[vBuffer.uWrite];
			--xm_uSize;
			return nRet;
		}
		PopPooled(xm_lstBuffers);
	}
}
void StreamBuffer::Unget(unsigned char by){
	const auto pNode = xm_lstBuffers.GetFirst();
	if(pNode && (pNode->GetElement().uRead > 0)){
		auto &vBuffer = pNode->GetElement();
		--vBuffer.uRead;
		vBuffer.abyData[vBuffer.uRead] = by;
		++xm_uSize;
		return;
	}
	auto &vBuffer = UnshiftPooled(xm_lstBuffers);
	vBuffer.abyData[CHUNK_SIZE - 1] = by;
	vBuffer.uRead = CHUNK_SIZE - 1;
	vBuffer.uWrite = CHUNK_SIZE;
	++xm_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	const auto uRet = Min(xm_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	auto pbyWrite = (unsigned char *)pData;
	std::size_t uCopied = 0;
	auto pNode = xm_lstBuffers.GetFirst();
	for(;;){
		ASSERT(pNode);
		const auto &vBuffer = pNode->GetElement();

		const auto uToCopy = Min(uRet - uCopied, vBuffer.uWrite - vBuffer.uRead);
		std::memcpy(pbyWrite, vBuffer.abyData + vBuffer.uRead, uToCopy);
		pbyWrite += uToCopy;
		uCopied += uToCopy;
		if(uCopied == uRet){
			break;
		}
		pNode = pNode->GetNext();
	}
	return uRet;
}
std::size_t StreamBuffer::Get(void *pData, std::size_t uSize) noexcept {
	const auto uRet = Min(xm_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	auto pbyWrite = (unsigned char *)pData;
	std::size_t uCopied = 0;
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetFirst()->GetElement();
		const auto uToCopy = Min(uRet - uCopied, vBuffer.uWrite - vBuffer.uRead);
		std::memcpy(pbyWrite, vBuffer.abyData + vBuffer.uRead, uToCopy);
		pbyWrite += uToCopy;
		vBuffer.uRead += uToCopy;
		xm_uSize -= uToCopy;
		uCopied += uToCopy;
		if(uCopied == uRet){
			break;
		}
		ShiftPooled(xm_lstBuffers);
	}
	return uRet;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	const auto uRet = Min(xm_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	std::size_t uCopied = 0;
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetFirst()->GetElement();
		const std::size_t uToDrop = Min(uRet - uCopied, vBuffer.uWrite - vBuffer.uRead);
		vBuffer.uRead += uToDrop;
		xm_uSize -= uToDrop;
		uCopied += uToDrop;
		if(uCopied == uRet){
			break;
		}
		ShiftPooled(xm_lstBuffers);
	}
	return uRet;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	auto pbyRead = (unsigned char *)pData;
	std::size_t uCopied = 0;
	auto pNode = xm_lstBuffers.GetLast();
	if(pNode && (pNode->GetElement().uWrite < CHUNK_SIZE)){
		auto &vBuffer = pNode->GetElement();
		const auto uToCopy = Min(uSize - uCopied, CHUNK_SIZE - vBuffer.uWrite);
		std::memcpy(vBuffer.abyData + vBuffer.uWrite, pbyRead, uToCopy);
		vBuffer.uWrite += uToCopy;
		pbyRead += uToCopy;
		xm_uSize += uToCopy;
		uCopied += uToCopy;
	}
	while(uCopied < uSize){
		auto &vBuffer = PushPooled(xm_lstBuffers);
		const auto uToCopy = Min(uSize - uCopied, CHUNK_SIZE);
		std::memcpy(vBuffer.abyData, pbyRead, uToCopy);
		vBuffer.uRead = 0;
		vBuffer.uWrite = uToCopy;
		pbyRead += uToCopy;
		xm_uSize += uToCopy;
		uCopied += uToCopy;
	}
}
void StreamBuffer::Put(const char *pszData){
	char ch;
	while((ch = *(pszData++)) != 0){
		Put((unsigned char)ch);
	}
}

StreamBuffer StreamBuffer::Cut(std::size_t uSize){
	StreamBuffer sbufRet;
	if(xm_uSize <= uSize){
		sbufRet.Swap(*this);
	} else {
		auto pNode = xm_lstBuffers.GetFirst();
		std::size_t uTotal = 0; // 这是 [xm_lstBuffers.GetFirst(), pNode) 的字节数，不含零头。
		while(uTotal < uSize){
			ASSERT(pNode);
			auto &vBuffer = pNode->GetElement();

			const std::size_t uRemaining = uSize - uTotal;
			const auto uAvail = vBuffer.uWrite - vBuffer.uRead;
			if(uRemaining < uAvail){
				auto &vNewBuffer = PushPooled(sbufRet.xm_lstBuffers);
				vNewBuffer.uRead = 0;
				vNewBuffer.uWrite = uRemaining;
				std::memcpy(vNewBuffer.abyData, vBuffer.abyData + vBuffer.uRead, uRemaining);
				vBuffer.uRead += uRemaining;
				sbufRet.xm_uSize += uRemaining;
				xm_uSize -= uRemaining;
				break;
			}
			uTotal += uAvail;
			pNode = pNode->GetNext();
		}
		sbufRet.xm_lstBuffers.Splice(sbufRet.xm_lstBuffers.GetFirst(),
			xm_lstBuffers, xm_lstBuffers.GetFirst(), pNode);
		sbufRet.xm_uSize += uTotal;
		xm_uSize -= uTotal;
	}
	return std::move(sbufRet);
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	if(&rhs == this){
		return;
	}
	xm_lstBuffers.Splice(nullptr, rhs.xm_lstBuffers);
	xm_uSize += rhs.xm_uSize;
	rhs.xm_uSize = 0;
}

bool StreamBuffer::Traverse(const StreamBuffer::TraverseContext *&pContext,
	std::pair<const void *, std::size_t> &vBlock) const noexcept
{
	auto pNode = pContext ? ((const ChunkNode *)pContext)->GetNext() : xm_lstBuffers.GetFirst();
	for(;;){
		if(!pNode){
			return false;
		}
		const auto &vBuffer = pNode->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			pContext = (const TraverseContext *)pNode;
			vBlock.first = vBuffer.abyData + vBuffer.uRead;
			vBlock.second = vBuffer.uWrite - vBuffer.uRead;
			return true;
		}
		pNode = pNode->GetNext();
	}
}
bool StreamBuffer::Traverse(StreamBuffer::TraverseContext *&pContext,
	std::pair<void *, std::size_t> &vBlock) noexcept
{
	auto pNode = pContext ? ((ChunkNode *)pContext)->GetNext() : xm_lstBuffers.GetFirst();
	for(;;){
		if(!pNode){
			return false;
		}
		pContext = (TraverseContext *)pNode;
		auto &vBuffer = pNode->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			vBlock.first = vBuffer.abyData + vBuffer.uRead;
			vBlock.second = vBuffer.uWrite - vBuffer.uRead;
			return true;
		}
		pNode = pNode->GetNext();
	}
}
