// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "../Core/Utilities.hpp"
#include <cstring>
using namespace MCF;

// 嵌套类定义。
class StreamBuffer::xDisposableBuffer {
private:
	unsigned short xm_ushRead;
	unsigned short xm_ushWrite;
	unsigned char xm_abyData[0x400];

public:
	xDisposableBuffer() noexcept
		: xm_ushRead	(0)
		, xm_ushWrite	(0)
	{
	}
	xDisposableBuffer(const xDisposableBuffer &rhs) noexcept
		: xm_ushRead	(rhs.xm_ushRead)
		, xm_ushWrite	(rhs.xm_ushWrite)
	{
		Copy(xm_abyData + xm_ushRead, rhs.xm_abyData + rhs.xm_ushRead, rhs.xm_abyData + rhs.xm_ushWrite);
	}
	xDisposableBuffer &operator=(const xDisposableBuffer &rhs) noexcept {
		xm_ushRead	= rhs.xm_ushRead;
		xm_ushWrite	= rhs.xm_ushWrite;
		Copy(xm_abyData + xm_ushRead, rhs.xm_abyData + rhs.xm_ushRead, rhs.xm_abyData + rhs.xm_ushWrite);
		return *this;
	}

public:
	std::size_t GetSize() const noexcept {
		return (std::size_t)(xm_ushWrite - xm_ushRead);
	}
	std::size_t GetFree() const noexcept {
		return sizeof(xm_abyData) - (std::size_t)xm_ushWrite;
	}
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	bool IsFull() const noexcept {
		return GetFree() == 0;
	}
	void Clear() noexcept {
		xm_ushRead	= 0;
		xm_ushWrite	= 0;
	}

	unsigned char Get() noexcept {
		ASSERT(!IsEmpty());

		return xm_abyData[xm_ushRead++];
	}
	void Put(unsigned char by) noexcept {
		ASSERT(!IsFull());

		xm_abyData[xm_ushWrite++] = by;
	}

	const unsigned char *GetBegin() const noexcept {
		return xm_abyData + xm_ushRead;
	}
	const unsigned char *GetEnd() const noexcept {
		return xm_abyData + xm_ushWrite;
	}
	unsigned char *GetBegin() noexcept {
		return xm_abyData + xm_ushRead;
	}
	unsigned char *GetEnd() noexcept {
		return xm_abyData + xm_ushWrite;
	}

	std::size_t CopyOut(unsigned char *&pbyOutput, std::size_t &uToCopy) const noexcept {
		const std::size_t uBytesToCopy = Min(GetSize(), uToCopy);
		pbyOutput = CopyN(pbyOutput, xm_abyData + xm_ushRead, uBytesToCopy).first;
		uToCopy -= uBytesToCopy;
		return uBytesToCopy;
	}
	std::size_t MoveOut(unsigned char *&pbyOutput, std::size_t &uToCopy) noexcept {
		const auto uBytesCopied = CopyOut(pbyOutput, uToCopy);
		xm_ushRead += uBytesCopied;
		return uBytesCopied;
	}
	std::size_t MoveIn(const unsigned char *&pbyInput, std::size_t &uToCopy) noexcept {
		const std::size_t uBytesToCopy = Min(GetFree(), uToCopy);
		pbyInput = CopyN(xm_abyData + xm_ushWrite, pbyInput, uBytesToCopy).second;
		uToCopy -= uBytesToCopy;
		xm_ushWrite += uBytesToCopy;
		return uBytesToCopy;
	}
	std::size_t Transfer(xDisposableBuffer &dst, std::size_t &uToTransfer) noexcept {
		const std::size_t uBytesToTransfer = Min(GetSize(), uToTransfer, dst.GetFree());
		CopyN(dst.xm_abyData + dst.xm_ushWrite, xm_abyData + xm_ushRead, uBytesToTransfer);
		dst.xm_ushWrite += uBytesToTransfer;
		xm_ushRead += uBytesToTransfer;
		uToTransfer -= uBytesToTransfer;
		return uBytesToTransfer;
	}
	std::size_t Discard(std::size_t &uToDiscard) noexcept {
		const std::size_t uBytesToDiscard = Min(GetSize(), uToDiscard);
		uToDiscard -= uBytesToDiscard;
		xm_ushRead += uBytesToDiscard;
		return uBytesToDiscard;
	}

	void Swap(xDisposableBuffer &rhs) noexcept {
		if((xm_ushRead >= rhs.xm_ushWrite) || (rhs.xm_ushRead >= xm_ushWrite)){
			Copy(
				xm_abyData + rhs.xm_ushRead,
				rhs.xm_abyData + rhs.xm_ushRead,
				rhs.xm_abyData + rhs.xm_ushWrite
			);
			Copy(
				rhs.xm_abyData + xm_ushRead,
				xm_abyData + xm_ushRead,
				xm_abyData + xm_ushWrite
			);
		} else {
			decltype(xm_abyData) abyTemp;

			Copy(
				abyTemp + rhs.xm_ushRead,
				rhs.xm_abyData + rhs.xm_ushRead,
				rhs.xm_abyData + rhs.xm_ushWrite
			);
			Copy(
				rhs.xm_abyData + xm_ushRead,
				xm_abyData + xm_ushRead,
				xm_abyData + xm_ushWrite
			);
			Copy(
				xm_abyData + rhs.xm_ushRead,
				abyTemp + rhs.xm_ushRead,
				abyTemp + rhs.xm_ushWrite
			);
		}
		std::swap(xm_ushRead, rhs.xm_ushRead);
		std::swap(xm_ushWrite, rhs.xm_ushWrite);
	}
};

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_uSize	(0)
{
}
StreamBuffer::StreamBuffer(const void *pData, std::size_t uSize)
	: StreamBuffer()
{
	Insert(pData, uSize);
}
StreamBuffer::StreamBuffer(const StreamBuffer &rhs)
	: StreamBuffer()
{
	Append(rhs);
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
StreamBuffer::~StreamBuffer() noexcept {
}

// 其他非静态成员函数。
bool StreamBuffer::IsEmpty() const noexcept {
	return xm_uSize == 0;
}
std::size_t StreamBuffer::GetSize() const noexcept {
	return xm_uSize;
}
void StreamBuffer::Clear() noexcept {
	xm_lstBuffers.Clear();
	xm_uSize = 0;
}

int StreamBuffer::Get() noexcept {
	int nRet = -1;
	while(!xm_lstBuffers.IsEmpty()){
		auto &vBuffer = xm_lstBuffers.GetBegin()->GetElement();
		if(!vBuffer.IsEmpty()){
			nRet = vBuffer.Get();
			--xm_uSize;
			break;
		}
		xm_lstBuffers.Shift();
	}
	return nRet;
}
int StreamBuffer::Peek() const noexcept {
	int nRet = -1;
	for(auto pNode = xm_lstBuffers.GetBegin(); pNode; pNode = pNode->GetNext()){
		const auto &vBuffer = pNode->GetElement();
		if(!vBuffer.IsEmpty()){
			nRet = vBuffer.GetBegin()[0];
			break;
		}
	}
	return nRet;
}

void StreamBuffer::Put(unsigned char by){
	if(xm_lstBuffers.IsEmpty() || xm_lstBuffers.GetRBegin()->GetElement().IsFull()){
		xm_lstBuffers.Push();
	}
	xm_lstBuffers.GetRBegin()->GetElement().Put(by);
	++xm_uSize;
}

bool StreamBuffer::Extract(void *pData, std::size_t uSize) noexcept {
	if(uSize > xm_uSize){
		return false;
	}
	auto pbyWrite = (unsigned char *)pData;
	auto uRemaining = uSize;
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetBegin()->GetElement();
		xm_uSize -= vBuffer.MoveOut(pbyWrite, uRemaining);
		if(uRemaining == 0){
			break;
		}
		ASSERT(vBuffer.IsEmpty());
		xm_lstBuffers.Shift();
	}
	return true;
}
bool StreamBuffer::CopyOut(void *pData, std::size_t uSize) const noexcept {
	if(uSize > xm_uSize){
		return false;
	}
	auto pbyWrite = (unsigned char *)pData;
	auto uRemaining = uSize;
	auto pNode = xm_lstBuffers.GetBegin();
	for(;;){
		ASSERT(pNode);

		pNode->GetElement().CopyOut(pbyWrite, uRemaining);
		if(uRemaining == 0){
			break;
		}
	}
	return true;
}
bool StreamBuffer::Discard(std::size_t uSize) noexcept {
	if(uSize > xm_uSize){
		return false;
	}
	auto uRemaining = uSize;
	for(;;){
		ASSERT(!xm_lstBuffers.IsEmpty());

		auto &vBuffer = xm_lstBuffers.GetBegin()->GetElement();
		xm_uSize -= vBuffer.Discard(uRemaining);
		if(uRemaining == 0){
			break;
		}
		ASSERT(vBuffer.IsEmpty());

		xm_lstBuffers.Shift();
	}
	return true;
}
void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	if(xm_lstBuffers.IsEmpty() || xm_lstBuffers.GetRBegin()->GetElement().IsFull()){
		xm_lstBuffers.Push();
	}
	auto pNode = xm_lstBuffers.GetRBegin();
	auto uRemaining = uSize;
	for(;;){
		const auto uFree = xm_lstBuffers.GetRBegin()->GetElement().GetFree();
		if(uRemaining <= uFree){
			break;
		}
		uRemaining -= uFree;
		xm_lstBuffers.Push();
	}

ASSERT_NOEXCEPT_BEGIN
	auto pbyRead = (const unsigned char *)pData;
	uRemaining = uSize;
	while(uRemaining != 0){
		ASSERT(pNode);

		xm_uSize += pNode->GetElement().MoveIn(pbyRead, uRemaining);
		pNode = pNode->GetNext();
	}
ASSERT_NOEXCEPT_END
}

bool StreamBuffer::CutOut(StreamBuffer &sbufHead, std::size_t uSize){
	if(uSize > xm_uSize){
		return false;
	}

	StreamBuffer sbufTemp;
	auto pSpliceEnd = xm_lstBuffers.GetBegin();
	auto uRemaining = uSize;
	while(uRemaining != 0){
		ASSERT(pSpliceEnd);

		const auto uCurSize = pSpliceEnd->GetElement().GetSize();
		if(uRemaining < uCurSize){
			sbufTemp.xm_lstBuffers.Push();
			break;
		}
		pSpliceEnd = pSpliceEnd->GetNext();
		uRemaining -= uCurSize;
	}
	sbufTemp.xm_lstBuffers.Splice(
		sbufTemp.xm_lstBuffers.GetBegin(),
		xm_lstBuffers, xm_lstBuffers.GetBegin(), pSpliceEnd
	);
	if(uRemaining != 0){
		ASSERT(!xm_lstBuffers.IsEmpty());
		xm_lstBuffers.GetBegin()->GetElement().Transfer(
			sbufTemp.xm_lstBuffers.GetRBegin()->GetElement(), uRemaining
		);

		ASSERT(uRemaining == 0);
	}
	xm_uSize -= uSize;
	sbufTemp.xm_uSize += uSize;

	sbufHead = std::move(sbufTemp);
	return true;
}
void StreamBuffer::Append(const StreamBuffer &rhs){
	xm_lstBuffers.Splice(nullptr, Clone(rhs.xm_lstBuffers));
	xm_uSize += rhs.xm_uSize;
}
void StreamBuffer::Append(StreamBuffer &&rhs) noexcept {
	if(&rhs == this){
		return Append(rhs);
	}

	xm_lstBuffers.Splice(nullptr, rhs.xm_lstBuffers);
	xm_uSize += rhs.xm_uSize;
	rhs.xm_uSize = 0;
}

void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	xm_lstBuffers.Swap(rhs.xm_lstBuffers);
	std::swap(xm_uSize, rhs.xm_uSize);
}

void StreamBuffer::Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const {
	for(auto pNode = xm_lstBuffers.GetBegin(); pNode; pNode = pNode->GetNext()){
		const auto &vBuffer = pNode->GetElement();
		if(!vBuffer.IsEmpty()){
			fnCallback(vBuffer.GetBegin(), vBuffer.GetSize());
		}
	}
}
void StreamBuffer::Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback){
	for(auto pNode = xm_lstBuffers.GetBegin(); pNode; pNode = pNode->GetNext()){
		auto &vBuffer = pNode->GetElement();
		if(!vBuffer.IsEmpty()){
			fnCallback(vBuffer.GetBegin(), vBuffer.GetSize());
		}
	}
}
