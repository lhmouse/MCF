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

class StreamBuffer::Chunk {
public:
	enum : std::size_t {
		CHUNK_SIZE = 0x100
	};

private:
	static Mutex xs_vPoolMutex;
	static List<Chunk> xs_lstPool;

public:
	static Chunk &PushPooled(List<Chunk> &lstDst){
		typename List<Chunk>::Node *pNode;
		{
			const auto vLock = xs_vPoolMutex.GetLock();
			if(!xs_lstPool.IsEmpty()){
				pNode = xs_lstPool.GetFirst();
				lstDst.Splice(nullptr, xs_lstPool, xs_lstPool.GetFirst());
				goto jDone;
			}
		}
		pNode = lstDst.Push();

	jDone:
#ifndef NDEBUG
		__builtin_memset(&pNode->GetElement(), 0xCC, sizeof(Chunk));
#endif
		return pNode->GetElement();
	}
	static void PopPooled(List<Chunk> &lstSrc) noexcept {
		ASSERT(!lstSrc.IsEmpty());

		const auto vLock = xs_vPoolMutex.GetLock();
		xs_lstPool.Splice(nullptr, lstSrc, lstSrc.GetLast());
	}
	static Chunk &UnshiftPooled(List<Chunk> &lstDst){
		typename List<Chunk>::Node *pNode;
		{
			const auto vLock = xs_vPoolMutex.GetLock();
			if(!xs_lstPool.IsEmpty()){
				pNode = xs_lstPool.GetFirst();
				lstDst.Splice(lstDst.GetFirst(), xs_lstPool, xs_lstPool.GetFirst());
				goto jDone;
			}
		}
		pNode = lstDst.Unshift();

	jDone:
#ifndef NDEBUG
		__builtin_memset(&pNode->GetElement(), 0xCC, sizeof(Chunk));
#endif
		return pNode->GetElement();
	}
	static void ShiftPooled(List<Chunk> &lstSrc) noexcept {
		ASSERT(!lstSrc.IsEmpty());

		const auto vLock = xs_vPoolMutex.GetLock();
		xs_lstPool.Splice(nullptr, lstSrc, lstSrc.GetFirst());
	}
	static void ClearPooled(List<Chunk> &lstSrc) noexcept {
		if(lstSrc.IsEmpty()){
			return;
		}
		const auto vLock = xs_vPoolMutex.GetLock();
		xs_lstPool.Splice(nullptr, lstSrc);
	}

public:
	unsigned m_uRead;
	unsigned m_uWrite;
	unsigned char m_abyData[CHUNK_SIZE];
};

Mutex						StreamBuffer::Chunk::xs_vPoolMutex;
List<StreamBuffer::Chunk>	StreamBuffer::Chunk::xs_lstPool;

class StreamBuffer::TraverseContext
	: public List<StreamBuffer::Chunk>::Node
{
};

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
	Chunk::ClearPooled(xm_lstBuffers);
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
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			return vBuffer.m_abyData[vBuffer.m_uRead];
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
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			const int nRet = vBuffer.m_abyData[vBuffer.m_uRead];
			++vBuffer.m_uRead;
			--xm_uSize;
			return nRet;
		}
		Chunk::ShiftPooled(xm_lstBuffers);
	}
}
void StreamBuffer::Put(unsigned char by){
	const auto pNode = xm_lstBuffers.GetLast();
	if(pNode && (pNode->GetElement().m_uWrite < Chunk::CHUNK_SIZE)){
		auto &vBuffer = pNode->GetElement();
		vBuffer.m_abyData[vBuffer.m_uWrite] = by;
		++vBuffer.m_uWrite;
		++xm_uSize;
		return;
	}
	auto &vBuffer = Chunk::PushPooled(xm_lstBuffers);
	vBuffer.m_abyData[0] = by;
	vBuffer.m_uRead = 0;
	vBuffer.m_uWrite = 1;
	++xm_uSize;
}
int StreamBuffer::Unput() noexcept {
	if(xm_uSize == 0){
		return -1;
	}
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetLast()->GetElement();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			--vBuffer.m_uWrite;
			const int nRet = vBuffer.m_abyData[vBuffer.m_uWrite];
			--xm_uSize;
			return nRet;
		}
		Chunk::PopPooled(xm_lstBuffers);
	}
}
void StreamBuffer::Unget(unsigned char by){
	const auto pNode = xm_lstBuffers.GetFirst();
	if(pNode && (pNode->GetElement().m_uRead > 0)){
		auto &vBuffer = pNode->GetElement();
		--vBuffer.m_uRead;
		vBuffer.m_abyData[vBuffer.m_uRead] = by;
		++xm_uSize;
		return;
	}
	auto &vBuffer = Chunk::UnshiftPooled(xm_lstBuffers);
	vBuffer.m_abyData[Chunk::CHUNK_SIZE - 1] = by;
	vBuffer.m_uRead = Chunk::CHUNK_SIZE - 1;
	vBuffer.m_uWrite = Chunk::CHUNK_SIZE;
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
	const auto uRet = Min(xm_uSize, uSize);
	if(uRet == 0){
		return 0;
	}
	auto pbyWrite = (unsigned char *)pData;
	std::size_t uCopied = 0;
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetFirst()->GetElement();
		const auto uToCopy = Min(uRet - uCopied, vBuffer.m_uWrite - vBuffer.m_uRead);
		std::memcpy(pbyWrite, vBuffer.m_abyData + vBuffer.m_uRead, uToCopy);
		pbyWrite += uToCopy;
		vBuffer.m_uRead += uToCopy;
		xm_uSize -= uToCopy;
		uCopied += uToCopy;
		if(uCopied == uRet){
			break;
		}
		Chunk::ShiftPooled(xm_lstBuffers);
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
		const std::size_t uToDrop = Min(uRet - uCopied, vBuffer.m_uWrite - vBuffer.m_uRead);
		vBuffer.m_uRead += uToDrop;
		xm_uSize -= uToDrop;
		uCopied += uToDrop;
		if(uCopied == uRet){
			break;
		}
		Chunk::ShiftPooled(xm_lstBuffers);
	}
	return uRet;
}
void StreamBuffer::Put(const void *pData, std::size_t uSize){
	auto pbyRead = (unsigned char *)pData;
	std::size_t uCopied = 0;
	auto pNode = xm_lstBuffers.GetLast();
	if(pNode && (pNode->GetElement().m_uWrite < Chunk::CHUNK_SIZE)){
		auto &vBuffer = pNode->GetElement();
		const auto uToCopy = Min(uSize - uCopied, Chunk::CHUNK_SIZE - vBuffer.m_uWrite);
		std::memcpy(vBuffer.m_abyData + vBuffer.m_uWrite, pbyRead, uToCopy);
		vBuffer.m_uWrite += uToCopy;
		pbyRead += uToCopy;
		xm_uSize += uToCopy;
		uCopied += uToCopy;
	}
	while(uCopied < uSize){
		auto &vBuffer = Chunk::PushPooled(xm_lstBuffers);
		const auto uToCopy = Min(uSize - uCopied, Chunk::CHUNK_SIZE);
		std::memcpy(vBuffer.m_abyData, pbyRead, uToCopy);
		vBuffer.m_uRead = 0;
		vBuffer.m_uWrite = uToCopy;
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

StreamBuffer StreamBuffer::CutOff(std::size_t uSize){
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
			const auto uAvail = vBuffer.m_uWrite - vBuffer.m_uRead;
			if(uRemaining < uAvail){
				auto &vNewBuffer = Chunk::PushPooled(sbufRet.xm_lstBuffers);
				vNewBuffer.m_uRead = 0;
				vNewBuffer.m_uWrite = uRemaining;
				std::memcpy(vNewBuffer.m_abyData, vBuffer.m_abyData + vBuffer.m_uRead, uRemaining);
				vBuffer.m_uRead += uRemaining;
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
	const typename List<Chunk>::Node *pNode = !pContext ? xm_lstBuffers.GetFirst() : pContext->GetNext();
	for(;;){
		if(!pNode){
			return false;
		}
		const auto &vBuffer = pNode->GetElement();
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
	typename List<Chunk>::Node *pNode = !pContext ? xm_lstBuffers.GetFirst() : pContext->GetNext();
	for(;;){
		if(!pNode){
			return false;
		}
		auto &vBuffer = pNode->GetElement();
		if(vBuffer.m_uRead < vBuffer.m_uWrite){
			pContext = static_cast<TraverseContext *>(pNode);
			vBlock.first = vBuffer.m_abyData + vBuffer.m_uRead;
			vBlock.second = vBuffer.m_uWrite - vBuffer.m_uRead;
			return true;
		}
		pNode = pNode->GetNext();
	}
}
