// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

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
	unsigned char xm_abyData[0x400u];

public:
	xDisposableBuffer()
		: xm_ushRead	(0)
		, xm_ushWrite	(0)
	{
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
		xm_ushRead = 0;
		xm_ushWrite = 0;
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

	std::size_t CopyOut(unsigned char *&pbyOutput, const unsigned char *pbyEnd) noexcept {
		const std::size_t uBytesToCopy = std::min(GetSize(), (std::size_t)(pbyEnd - pbyOutput));
		pbyOutput = CopyN(pbyOutput, xm_abyData + xm_ushRead, uBytesToCopy).first;
		xm_ushRead += uBytesToCopy;
		return uBytesToCopy;
	}
	std::size_t CopyIn(const unsigned char *&pbyInput, const unsigned char *pbyEnd) noexcept {
		const std::size_t uBytesToCopy = std::min(GetFree(), (std::size_t)(pbyEnd - pbyInput));
		pbyInput = CopyN(xm_abyData + xm_ushWrite, pbyInput, uBytesToCopy).second;
		xm_ushWrite += uBytesToCopy;
		return uBytesToCopy;
	}
	std::size_t Transfer(xDisposableBuffer &dst, std::size_t &uToTransfer) noexcept {
		const std::size_t uBytesToTransfer = std::min(std::min(GetSize(), uToTransfer), dst.GetFree());
		CopyN(dst.xm_abyData + dst.xm_ushWrite, xm_abyData + xm_ushRead, uBytesToTransfer);
		dst.xm_ushWrite += uBytesToTransfer;
		xm_ushRead += uBytesToTransfer;
		uToTransfer -= uBytesToTransfer;
		return uBytesToTransfer;
	}
	std::size_t Discard(std::size_t &uToDiscard) noexcept {
		const std::size_t uBytesToDiscard = std::min(GetSize(), uToDiscard);
		uToDiscard -= uBytesToDiscard;
		xm_ushRead += uBytesToDiscard;
		return uBytesToDiscard;
	}

	void Swap(xDisposableBuffer &rhs) noexcept {
		if((xm_ushRead >= rhs.xm_ushWrite) || (rhs.xm_ushRead >= xm_ushWrite)){
			Copy(xm_abyData + rhs.xm_ushRead,	rhs.xm_abyData + rhs.xm_ushRead,	rhs.xm_abyData + rhs.xm_ushWrite);
			Copy(rhs.xm_abyData + xm_ushRead,	xm_abyData + xm_ushRead,			xm_abyData + xm_ushWrite);
		} else {
			decltype(xm_abyData) abyTemp;

			Copy(abyTemp + rhs.xm_ushRead,		rhs.xm_abyData + rhs.xm_ushRead,	rhs.xm_abyData + rhs.xm_ushWrite);
			Copy(rhs.xm_abyData + xm_ushRead,	xm_abyData + xm_ushRead,			xm_abyData + xm_ushWrite);
			Copy(xm_abyData + rhs.xm_ushRead,	abyTemp + rhs.xm_ushRead,			abyTemp + rhs.xm_ushWrite);
		}
		std::swap(xm_ushRead, rhs.xm_ushRead);
		std::swap(xm_ushWrite, rhs.xm_ushWrite);
	}
};

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_uSize(0)
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
	xm_lstData.clear();
	xm_uSize = 0;
}

int StreamBuffer::Get() noexcept {
ASSERT_NOEXCEPT_BEGIN
	int nRet = -1;
	while(!xm_lstData.empty()){
		if(!xm_lstData.front().IsEmpty()){
			nRet = xm_lstData.front().Get();
			break;
		}
		xm_lstData.pop_front();
	}
	--xm_uSize;
	return nRet;
ASSERT_NOEXCEPT_END
}
int StreamBuffer::Peek() const noexcept {
ASSERT_NOEXCEPT_BEGIN
	for(const auto &vCur : xm_lstData){
		if(!vCur.IsEmpty()){
			return vCur.GetBegin()[0];
		}
	}
	return -1;
ASSERT_NOEXCEPT_END
}

void StreamBuffer::Put(unsigned char by){
	if(xm_lstData.empty() || xm_lstData.back().IsFull()){
		xm_lstData.emplace_back();
	}

ASSERT_NOEXCEPT_BEGIN
	xm_lstData.back().Put(by);
	++xm_uSize;
ASSERT_NOEXCEPT_END
}

bool StreamBuffer::Extract(void *pData, std::size_t uSize) noexcept {
ASSERT_NOEXCEPT_BEGIN
	if(uSize > xm_uSize){
		return false;
	}

	if(pData){
		auto pbyWrite = (unsigned char *)pData;
		const auto pbyEnd = pbyWrite + uSize;
		for(;;){
			ASSERT(!xm_lstData.empty());

			xm_uSize -= xm_lstData.front().CopyOut(pbyWrite, pbyEnd);
			if(pbyWrite == pbyEnd){
				break;
			}

			ASSERT(xm_lstData.front().IsEmpty());
			xm_lstData.pop_front();
		}
	} else {
		std::size_t uToDiscard = uSize;
		for(;;){
			ASSERT(!xm_lstData.empty());

			xm_uSize -= xm_lstData.front().Discard(uToDiscard);
			if(uToDiscard == 0){
				break;
			}

			ASSERT(xm_lstData.front().IsEmpty());
			xm_lstData.pop_front();
		}
	}
	return true;
ASSERT_NOEXCEPT_END
}
void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	const auto pbyEnd = pbyRead + uSize;

	decltype(xm_lstData) lstTemp;
	while(pbyRead != pbyEnd){
		lstTemp.emplace_back();

	ASSERT_NOEXCEPT_BEGIN
		xm_uSize += lstTemp.back().CopyIn(pbyRead, pbyEnd);
		xm_lstData.splice(xm_lstData.end(), lstTemp);
	ASSERT_NOEXCEPT_END
	}
}

bool StreamBuffer::CutOut(StreamBuffer &sbufHead, std::size_t uSize){
	if(uSize > xm_uSize){
		return false;
	}

	StreamBuffer sbufTemp;
	auto uRemaining = uSize;
	if(uRemaining != 0){
		std::size_t uToSplice = 0;
		auto itLast = xm_lstData.begin();
		for(;;){
			ASSERT(itLast != xm_lstData.end());

			const auto uLastSize = itLast->GetSize();
			if(uRemaining - uToSplice < uLastSize){
				break;
			}
			uToSplice += uLastSize;
			++itLast;
		}

	ASSERT_NOEXCEPT_BEGIN
		sbufTemp.xm_lstData.splice(sbufTemp.xm_lstData.end(), xm_lstData, xm_lstData.begin(), itLast);
		uRemaining -= uToSplice;
		xm_uSize -= uToSplice;
		sbufTemp.xm_uSize += uToSplice;
	ASSERT_NOEXCEPT_END

		if(uRemaining != 0){
			sbufTemp.xm_lstData.emplace_back();

		ASSERT_NOEXCEPT_BEGIN
			const auto uTransferred = itLast->Transfer(sbufTemp.xm_lstData.back(), uRemaining);
			xm_uSize -= uTransferred;
			sbufTemp.xm_uSize += uTransferred;
		ASSERT_NOEXCEPT_END

			ASSERT(uRemaining == 0);
		}
	}
	sbufHead = std::move(sbufTemp);
	return true;
}
void StreamBuffer::Append(const StreamBuffer &rhs){
	decltype(xm_lstData) lstTemp(rhs.xm_lstData);

ASSERT_NOEXCEPT_BEGIN
	xm_lstData.splice(xm_lstData.end(), lstTemp);
	xm_uSize += rhs.xm_uSize;
ASSERT_NOEXCEPT_END
}
void StreamBuffer::Append(StreamBuffer &&rhs) noexcept {
	if(&rhs == this){
		return Append(rhs);
	}

	decltype(xm_lstData) lstTemp;

ASSERT_NOEXCEPT_BEGIN
	xm_lstData.splice(xm_lstData.end(), lstTemp);
	xm_lstData.splice(xm_lstData.end(), rhs.xm_lstData);
	xm_uSize += rhs.xm_uSize;
	rhs.Clear();
ASSERT_NOEXCEPT_END
}

void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	std::swap(xm_lstData, rhs.xm_lstData);
	std::swap(xm_uSize, rhs.xm_uSize);
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
