// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include "../Thread/Mutex.hpp"

namespace MCF {

class StreamBuffer::xChunk {
public:
	enum : std::size_t {
		CHUNK_SIZE = 0x100
	};

private:
	static Mutex s_vPoolMutex;
	static List<xChunk> s_lstPool;

public:
	static xChunk &PushPooled(List<xChunk> &lstDst){
		typename List<xChunk>::Node *pNode;
		{
			const auto vLock = s_vPoolMutex.GetLock();
			if(!s_lstPool.IsEmpty()){
				pNode = s_lstPool.GetFirst();
				lstDst.Splice(nullptr, s_lstPool, s_lstPool.GetFirst());
				goto jDone;
			}
		}
		pNode = lstDst.Push();

	jDone:
#ifndef NDEBUG
		__builtin_memset(&pNode->Get(), 0xCC, sizeof(xChunk));
#endif
		return pNode->Get();
	}
	static void PopPooled(List<xChunk> &lstSrc) noexcept {
		ASSERT(!lstSrc.IsEmpty());

		const auto vLock = s_vPoolMutex.GetLock();
		s_lstPool.Splice(nullptr, lstSrc, lstSrc.GetLast());
	}
	static xChunk &UnshiftPooled(List<xChunk> &lstDst){
		typename List<xChunk>::Node *pNode;
		{
			const auto vLock = s_vPoolMutex.GetLock();
			if(!s_lstPool.IsEmpty()){
				pNode = s_lstPool.GetFirst();
				lstDst.Splice(lstDst.GetFirst(), s_lstPool, s_lstPool.GetFirst());
				goto jDone;
			}
		}
		pNode = lstDst.Unshift();

	jDone:
#ifndef NDEBUG
		__builtin_memset(&pNode->Get(), 0xCC, sizeof(xChunk));
#endif
		return pNode->Get();
	}
	static void ShiftPooled(List<xChunk> &lstSrc) noexcept {
		ASSERT(!lstSrc.IsEmpty());

		const auto vLock = s_vPoolMutex.GetLock();
		s_lstPool.Splice(nullptr, lstSrc, lstSrc.GetFirst());
	}
	static void ClearPooled(List<xChunk> &lstSrc) noexcept {
		if(lstSrc.IsEmpty()){
			return;
		}
		const auto vLock = s_vPoolMutex.GetLock();
		s_lstPool.Splice(nullptr, lstSrc);
	}

public:
	unsigned char m_abyData[CHUNK_SIZE];
	unsigned m_uRead;
	unsigned m_uWrite;
};

Mutex						StreamBuffer::xChunk::s_vPoolMutex	__attribute__((__init_priority__(101)));
List<StreamBuffer::xChunk>	StreamBuffer::xChunk::s_lstPool		__attribute__((__init_priority__(101)));

class StreamBuffer::TraverseContext
	: public List<StreamBuffer::xChunk>::Node
{
};

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: x_lstBuffers(), x_uSize(0)
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
	for(auto pNode = rhs.x_lstBuffers.GetFirst(); pNode; pNode = pNode->GetNext()){
		const auto &vBuffer = pNode->Get();
		Put(vBuffer.m_abyData + vBuffer.m_uRead, vBuffer.m_uWrite - vBuffer.m_uRead);
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
	xChunk::ClearPooled(x_lstBuffers);
	x_uSize = 0;
}

int StreamBuffer::GetFront() const noexcept {
	if(x_uSize == 0){
		return -1;
	}
	auto pNode = x_lstBuffers.GetFirst();
	for(;;){
		ASSERT(pNode);

		const auto &vBuffer = pNode->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			return vBuffer.m_abyData[vBuffer.m_uRead];
		}
		pNode = pNode->GetNext();
	}
}
int StreamBuffer::GetBack() const noexcept {
	if(x_uSize == 0){
		return -1;
	}
	auto pNode = x_lstBuffers.GetLast();
	for(;;){
		ASSERT(pNode);

		const auto &vBuffer = pNode->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			return vBuffer.m_abyData[vBuffer.m_uWrite - 1];
		}
		pNode = pNode->GetPrev();
	}
}

int StreamBuffer::Get() noexcept {
	if(x_uSize == 0){
		return -1;
	}
	for(;;){
		ASSERT(!x_lstBuffers.IsEmpty());

		auto &vBuffer = x_lstBuffers.GetFirst()->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			const int nRet = vBuffer.m_abyData[vBuffer.m_uRead];
			++vBuffer.m_uRead;
			--x_uSize;
			return nRet;
		}
		xChunk::ShiftPooled(x_lstBuffers);
	}
}
void StreamBuffer::Put(unsigned char by){
	const auto pNode = x_lstBuffers.GetLast();
	if(pNode && (pNode->Get().m_uWrite < xChunk::CHUNK_SIZE)){
		auto &vBuffer = pNode->Get();
		vBuffer.m_abyData[vBuffer.m_uWrite] = by;
		++vBuffer.m_uWrite;
		++x_uSize;
		return;
	}
	auto &vBuffer = xChunk::PushPooled(x_lstBuffers);
	vBuffer.m_abyData[0] = by;
	vBuffer.m_uRead = 0;
	vBuffer.m_uWrite = 1;
	++x_uSize;
}
int StreamBuffer::Unput() noexcept {
	if(x_uSize == 0){
		return -1;
	}
	for(;;){
		ASSERT(!x_lstBuffers.IsEmpty());

		auto &vBuffer = x_lstBuffers.GetLast()->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			--vBuffer.m_uWrite;
			const int nRet = vBuffer.m_abyData[vBuffer.m_uWrite];
			--x_uSize;
			return nRet;
		}
		xChunk::PopPooled(x_lstBuffers);
	}
}
void StreamBuffer::Unget(unsigned char by){
	const auto pNode = x_lstBuffers.GetFirst();
	if(pNode && (pNode->Get().m_uRead > 0)){
		auto &vBuffer = pNode->Get();
		--vBuffer.m_uRead;
		vBuffer.m_abyData[vBuffer.m_uRead] = by;
		++x_uSize;
		return;
	}
	auto &vBuffer = xChunk::UnshiftPooled(x_lstBuffers);
	vBuffer.m_abyData[xChunk::CHUNK_SIZE - 1] = by;
	vBuffer.m_uRead = xChunk::CHUNK_SIZE - 1;
	vBuffer.m_uWrite = xChunk::CHUNK_SIZE;
	++x_uSize;
}

std::size_t StreamBuffer::Peek(void *pData, std::size_t uSize) const noexcept {
	const auto uRet = Min(x_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	auto pbyWrite = (unsigned char *)pData;
	std::size_t uCopied = 0;
	auto pNode = x_lstBuffers.GetFirst();
	for(;;){
		ASSERT(pNode);
		const auto &vBuffer = pNode->Get();

		const auto uToCopy = Min(uRet - uCopied, vBuffer.m_uWrite - vBuffer.m_uRead);
		std::memcpy(pbyWrite, vBuffer.m_abyData + vBuffer.m_uRead, uToCopy);
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
	const auto uRet = Min(x_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	auto pbyWrite = (unsigned char *)pData;
	std::size_t uCopied = 0;
	for(;;){
		ASSERT(!x_lstBuffers.IsEmpty());

		auto &vBuffer = x_lstBuffers.GetFirst()->Get();
		const auto uToCopy = Min(uRet - uCopied, vBuffer.m_uWrite - vBuffer.m_uRead);
		std::memcpy(pbyWrite, vBuffer.m_abyData + vBuffer.m_uRead, uToCopy);
		pbyWrite += uToCopy;
		vBuffer.m_uRead += uToCopy;
		x_uSize -= uToCopy;
		uCopied += uToCopy;
		if(uCopied == uRet){
			break;
		}
		xChunk::ShiftPooled(x_lstBuffers);
	}
	return uRet;
}
std::size_t StreamBuffer::Discard(std::size_t uSize) noexcept {
	const auto uRet = Min(x_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	std::size_t uCopied = 0;
	for(;;){
		ASSERT(!x_lstBuffers.IsEmpty());

		auto &vBuffer = x_lstBuffers.GetFirst()->Get();
		const std::size_t uToDrop = Min(uRet - uCopied, vBuffer.m_uWrite - vBuffer.m_uRead);
		vBuffer.m_uRead += uToDrop;
		x_uSize -= uToDrop;
		uCopied += uToDrop;
		if(uCopied == uRet){
			break;
		}
		xChunk::ShiftPooled(x_lstBuffers);
	}
	return uRet;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	auto pbyRead = (unsigned char *)pData;
	std::size_t uCopied = 0;
	auto pNode = x_lstBuffers.GetLast();
	if(pNode && (pNode->Get().m_uWrite < xChunk::CHUNK_SIZE)){
		auto &vBuffer = pNode->Get();
		const auto uToCopy = Min(uSize - uCopied, xChunk::CHUNK_SIZE - vBuffer.m_uWrite);
		std::memcpy(vBuffer.m_abyData + vBuffer.m_uWrite, pbyRead, uToCopy);
		vBuffer.m_uWrite += uToCopy;
		pbyRead += uToCopy;
		x_uSize += uToCopy;
		uCopied += uToCopy;
	}
	while(uCopied < uSize){
		auto &vBuffer = xChunk::PushPooled(x_lstBuffers);
		const auto uToCopy = Min(uSize - uCopied, xChunk::CHUNK_SIZE);
		std::memcpy(vBuffer.m_abyData, pbyRead, uToCopy);
		vBuffer.m_uRead = 0;
		vBuffer.m_uWrite = uToCopy;
		pbyRead += uToCopy;
		x_uSize += uToCopy;
		uCopied += uToCopy;
	}
}
void StreamBuffer::Put(const char *pszData){
	char ch;
	while((ch = *(pszData++)) != 0){
		Put((unsigned char)ch);
	}
}

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
	StreamBuffer sbufRet;
	if(x_uSize <= uSize){
		sbufRet.Swap(*this);
	} else {
		auto pNode = x_lstBuffers.GetFirst();
		std::size_t uTotal = 0; // 这是 [x_lstBuffers.GetFirst(), pNode) 的字节数，不含零头。
		while(uTotal < uSize){
			ASSERT(pNode);
			auto &vBuffer = pNode->Get();

			const std::size_t uRemaining = uSize - uTotal;
			const auto uAvail = vBuffer.m_uWrite - vBuffer.m_uRead;
			if(uRemaining < uAvail){
				auto &vNewBuffer = xChunk::PushPooled(sbufRet.x_lstBuffers);
				vNewBuffer.m_uRead = 0;
				vNewBuffer.m_uWrite = uRemaining;
				std::memcpy(vNewBuffer.m_abyData, vBuffer.m_abyData + vBuffer.m_uRead, uRemaining);
				vBuffer.m_uRead += uRemaining;
				sbufRet.x_uSize += uRemaining;
				x_uSize -= uRemaining;
				break;
			}
			uTotal += uAvail;
			pNode = pNode->GetNext();
		}
		sbufRet.x_lstBuffers.Splice(sbufRet.x_lstBuffers.GetFirst(),
			x_lstBuffers, x_lstBuffers.GetFirst(), pNode);
		sbufRet.x_uSize += uTotal;
		x_uSize -= uTotal;
	}
	return sbufRet;
}
void StreamBuffer::Splice(StreamBuffer &rhs) noexcept {
	if(&rhs == this){
		return;
	}
	x_lstBuffers.Splice(nullptr, rhs.x_lstBuffers);
	x_uSize += rhs.x_uSize;
	rhs.x_uSize = 0;
}

bool StreamBuffer::Traverse(const StreamBuffer::TraverseContext *&pContext,
	std::pair<const void *, std::size_t> &vBlock) const noexcept
{
	const List<xChunk>::Node *pNode = pContext ? pContext->GetNext() : x_lstBuffers.GetFirst();
	for(;;){
		if(!pNode){
			return false;
		}
		const auto &vBuffer = pNode->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			pContext = static_cast<const TraverseContext *>(pNode);
			vBlock.first = vBuffer.m_abyData + vBuffer.m_uRead;
			vBlock.second = vBuffer.m_uWrite - vBuffer.m_uRead;
			return true;
		}
		pNode = pNode->GetNext();
	}
}
bool StreamBuffer::Traverse(StreamBuffer::TraverseContext *&pContext,
	std::pair<void *, std::size_t> &vBlock) noexcept
{
	List<xChunk>::Node *pNode = pContext ? pContext->GetNext() : x_lstBuffers.GetFirst();
	for(;;){
		if(!pNode){
			return false;
		}
		auto &vBuffer = pNode->Get();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			pContext = static_cast<TraverseContext *>(pNode);
			vBlock.first = vBuffer.m_abyData + vBuffer.m_uRead;
			vBlock.second = vBuffer.m_uWrite - vBuffer.m_uRead;
			return true;
		}
		pNode = pNode->GetNext();
	}
}

}
