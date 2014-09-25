// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/MinMax.hpp"
#include "../Utilities/Algorithms.hpp"
#include "../Thread/CriticalSection.hpp"
using namespace MCF;

constexpr std::size_t CHUNK_SIZE = 0x400;

struct Impl::DisposableBuffer {
	unsigned uRead;
	unsigned uWrite;
	unsigned char abyData[CHUNK_SIZE];
};

namespace {

const auto g_pcsLock = CriticalSection::Create();
VList<Impl::DisposableBuffer> g_lstPool;

auto &PushPooled(VList<Impl::DisposableBuffer> &lstDst){
	VList<Impl::DisposableBuffer>::Node *pNode;
	{
		const auto vLock = g_pcsLock->GetLock();
		if(!g_lstPool.IsEmpty()){
			lstDst.Splice(nullptr, g_lstPool, g_lstPool.GetFirst());
			pNode = lstDst.GetLast();
			goto jDone;
		}
	}
	pNode = lstDst.Push();

jDone:
#ifndef NDEBUG
	__builtin_memset(&pNode->GetElement(), 0xCC, sizeof(Impl::DisposableBuffer));
#endif
	return pNode->GetElement();
}
void ShiftPooled(VList<Impl::DisposableBuffer> &lstSrc){
	ASSERT(!lstSrc.IsEmpty());

	const auto vLock = g_pcsLock->GetLock();
	g_lstPool.Splice(nullptr, lstSrc, lstSrc.GetFirst());
}
void ClearPooled(VList<Impl::DisposableBuffer> &lstSrc){
	if(lstSrc.IsEmpty()){
		return;
	}
	const auto vLock = g_pcsLock->GetLock();
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
	: xm_lstBuffers(rhs.xm_lstBuffers), xm_uSize(rhs.xm_uSize)
{
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
StreamBuffer::~StreamBuffer() noexcept {
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

void StreamBuffer::Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const {
	for(auto pNode = xm_lstBuffers.GetFirst(); pNode; pNode = pNode->GetNext()){
		auto &vBuffer = pNode->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			fnCallback(vBuffer.abyData + vBuffer.uRead, vBuffer.uWrite - vBuffer.uRead);
		}
	}
}
void StreamBuffer::Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback){
	for(auto pNode = xm_lstBuffers.GetFirst(); pNode; pNode = pNode->GetNext()){
		auto &vBuffer = pNode->GetElement();
		if(vBuffer.uRead < vBuffer.uWrite){
			fnCallback(vBuffer.abyData + vBuffer.uRead, vBuffer.uWrite - vBuffer.uRead);
		}
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
