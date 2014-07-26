// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "Utilities.hpp"
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

namespace {

inline void AllocPooled(auto &lstData, auto &itTail, auto &lstPool){
	if(lstPool.empty()){
		itTail = lstData.emplace_after(itTail);
	} else {
		lstData.splice_after(itTail, lstPool, lstPool.before_begin());
		++itTail;
		itTail->Clear();
	}
}
inline void AllocPooled(auto &lstData, auto &itTail, auto &lstPool, auto &&vBuffer){
	if(lstPool.empty()){
		itTail = lstData.emplace_after(itTail, std::move(vBuffer));
	} else {
		lstData.splice_after(itTail, lstPool, lstPool.before_begin());
		++itTail;
		*itTail = std::move(vBuffer);
	}
}
inline void DeallocPooled(auto &lstData, auto &itTail, auto &lstPool) noexcept {
	lstPool.splice_after(lstPool.before_begin(), lstData, lstData.before_begin());
	if(lstData.empty()){
		itTail = lstData.before_begin();
	}
}

}

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_itTail	(xm_lstData.before_begin())
	, xm_uSize	(0)
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
	xm_lstPool.splice_after(xm_lstPool.before_begin(), xm_lstData);
	xm_itTail = xm_lstData.before_begin();
	xm_uSize = 0;
}

int StreamBuffer::Get() noexcept {
	int nRet = -1;
	while(!xm_lstData.empty()){
		if(!xm_lstData.front().IsEmpty()){
			nRet = xm_lstData.front().Get();
			--xm_uSize;
			break;
		}
		DeallocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}
	return nRet;
}
int StreamBuffer::Peek() const noexcept {
	for(const auto &vCur : xm_lstData){
		if(!vCur.IsEmpty()){
			return vCur.GetBegin()[0];
		}
	}
	return -1;
}

void StreamBuffer::Put(unsigned char by){
	if(xm_lstData.empty() || xm_itTail->IsFull()){
		AllocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}

ASSERT_NOEXCEPT_BEGIN
	xm_itTail->Put(by);
	++xm_uSize;
ASSERT_NOEXCEPT_END
}

bool StreamBuffer::Extract(void *pData, std::size_t uSize) noexcept {
	if(uSize > xm_uSize){
		return false;
	}
	auto pbyWrite = (unsigned char *)pData;
	auto uRemaining = uSize;
	for(;;){
		ASSERT(!xm_lstData.empty());

		xm_uSize -= xm_lstData.front().MoveOut(pbyWrite, uRemaining);
		if(uRemaining == 0){
			break;
		}
		ASSERT(xm_lstData.front().IsEmpty());
		DeallocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}
	return true;
}
bool StreamBuffer::CopyOut(void *pData, std::size_t uSize) const noexcept {
	if(uSize > xm_uSize){
		return false;
	}
	auto pbyWrite = (unsigned char *)pData;
	auto uRemaining = uSize;
	for(auto itCur = xm_lstData.begin(); ; ++itCur){
		ASSERT(itCur != xm_lstData.end());

		itCur->CopyOut(pbyWrite, uRemaining);
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
		ASSERT(!xm_lstData.empty());

		xm_uSize -= xm_lstData.front().Discard(uRemaining);
		if(uRemaining == 0){
			break;
		}
		ASSERT(xm_lstData.front().IsEmpty());
		DeallocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}
	return true;
}
void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	if(xm_lstData.empty() || xm_itTail->IsFull()){
		AllocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}
	auto itCur = xm_itTail;
	auto uRemaining = uSize;
	for(;;){
		const auto uFree = xm_itTail->GetFree();
		if(uRemaining <= uFree){
			break;
		}
		uRemaining -= uFree;
		AllocPooled(xm_lstData, xm_itTail, xm_lstPool);
	}

ASSERT_NOEXCEPT_BEGIN
	auto pbyRead = (const unsigned char *)pData;
	uRemaining = uSize;
	while(uRemaining != 0){
		xm_uSize += itCur->MoveIn(pbyRead, uRemaining);
		++itCur;
	}
ASSERT_NOEXCEPT_END
}

bool StreamBuffer::CutOut(StreamBuffer &sbufHead, std::size_t uSize){
	if(uSize > xm_uSize){
		return false;
	}

	StreamBuffer sbufTemp;
	if(this != &sbufHead){
		xm_lstPool.splice_after(xm_lstPool.before_begin(), sbufHead.xm_lstPool);
	}
	auto itNewTail = sbufTemp.xm_lstData.before_begin();
	auto itSpliceEnd = xm_lstData.begin();
	auto uRemaining = uSize;
	while(uRemaining != 0){
		ASSERT(itSpliceEnd != xm_lstData.end());

		const auto uCurSize = itSpliceEnd->GetSize();
		if(uRemaining < uCurSize){
			AllocPooled(sbufTemp.xm_lstData, sbufTemp.xm_itTail, xm_lstPool);
			itNewTail = sbufTemp.xm_itTail;
			break;
		}
		itNewTail = itSpliceEnd;
		++itSpliceEnd;
		uRemaining -= uCurSize;
	}

	sbufTemp.xm_lstData.splice_after(
		sbufTemp.xm_lstData.before_begin(),
		xm_lstData, xm_lstData.before_begin(), itSpliceEnd
	);
	sbufTemp.xm_itTail = itNewTail;
	if(uRemaining != 0){
		ASSERT(itNewTail != sbufTemp.xm_lstData.before_begin());

		xm_lstData.front().Transfer(*itNewTail, uRemaining);
		ASSERT(uRemaining == 0);
	}
	xm_uSize -= uSize;
	sbufTemp.xm_uSize += uSize;

	sbufHead = std::move(sbufTemp);
	return true;
}
void StreamBuffer::Append(const StreamBuffer &rhs){
	xBufferList lstTemp(rhs.xm_lstData);
	auto itNewTail = lstTemp.before_begin();

	for(const auto &vCur : rhs.xm_lstData){
		if(!vCur.IsEmpty()){
			AllocPooled(lstTemp, itNewTail, xm_lstPool, vCur);
		}
	}

	xm_lstData.splice_after(xm_itTail, lstTemp);
	xm_itTail = itNewTail;
	xm_uSize += rhs.xm_uSize;
}
void StreamBuffer::Append(StreamBuffer &&rhs) noexcept {
	if(&rhs == this){
		return Append(rhs);
	}

	xm_lstData.splice_after(xm_itTail, rhs.xm_lstData);
	xm_itTail = rhs.xm_itTail;
	xm_lstPool.splice_after(xm_lstPool.before_begin(), rhs.xm_lstPool);
	xm_uSize += rhs.xm_uSize;

	rhs.xm_itTail = rhs.xm_lstData.before_begin();
	rhs.xm_uSize = 0;
}

void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	std::swap(xm_lstData,	rhs.xm_lstData);
	std::swap(xm_itTail,	rhs.xm_itTail);
	std::swap(xm_lstPool,	rhs.xm_lstPool);
	std::swap(xm_uSize,		rhs.xm_uSize);
}

void StreamBuffer::Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const {
	for(const auto &vCur : xm_lstData){
		if(!vCur.IsEmpty()){
			fnCallback(vCur.GetBegin(), vCur.GetSize());
		}
	}
}
void StreamBuffer::Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback){
	for(auto &vCur : xm_lstData){
		if(!vCur.IsEmpty()){
			fnCallback(vCur.GetBegin(), vCur.GetSize());
		}
	}
}
