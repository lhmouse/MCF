// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "Utilities.hpp"
#include <cstring>
using namespace MCF;

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_uSmallSize	(0)
	, xm_uSize		(0)
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

// 其他非静态成员函数。
bool StreamBuffer::IsEmpty() const noexcept {
	return xm_uSize == 0;
}
std::size_t StreamBuffer::GetSize() const noexcept {
	return xm_uSize;
}
void StreamBuffer::Clear() noexcept {
	xm_uSmallSize = 0;
	xm_deqLarge.clear();
	xm_uSize = 0;
}

bool StreamBuffer::Extract(void *pData, std::size_t uSize) noexcept {
	if(xm_uSize < uSize){
		return false;
	}

	auto pbyWrite = (unsigned char *)pData;
	if(xm_uSmallSize >= uSize){
		std::copy_n(xm_abySmall, uSize, pbyWrite);
		if(xm_uSmallSize > uSize){
			std::copy(xm_abySmall + uSize, xm_abySmall + xm_uSmallSize, xm_abySmall);
		}
		xm_uSmallSize -= uSize;
	} else {
		const auto pbyEnd = pbyWrite + uSize;
		pbyWrite = std::copy_n(xm_abySmall, xm_uSmallSize, pbyWrite);
		xm_uSmallSize = 0;

		for(;;){
			ASSERT(!xm_deqLarge.empty());
			auto &vecBlock = xm_deqLarge.front();

			const auto uRemaining = (std::size_t)(pbyEnd - pbyWrite);
			if(vecBlock.GetSize() >= uRemaining){
				pbyWrite = std::copy_n(vecBlock.GetBegin(), uRemaining, pbyWrite);
				if(vecBlock.GetSize() > uRemaining){
					std::copy(vecBlock.GetBegin() + uRemaining, vecBlock.GetEnd(), vecBlock.GetBegin());
					vecBlock.TruncateFromEnd(uRemaining);
				} else {
					xm_deqLarge.pop_front();
				}
				break;
			}
			pbyWrite = std::copy_n(vecBlock.GetBegin(), vecBlock.GetSize(), pbyWrite);
			xm_deqLarge.pop_front();
		}
	}
	xm_uSize -= uSize;
	return true;
}
void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	auto pbyRead = (const unsigned char *)pData;
	const auto uSmallRemaining = sizeof(xm_abySmall) - xm_uSmallSize;
	if(xm_deqLarge.empty() && (uSmallRemaining >= uSize)){
		std::copy_n(pbyRead, uSize, xm_abySmall + xm_uSmallSize);
		xm_uSmallSize += uSize;
	} else {
		if(xm_deqLarge.empty() || (xm_deqLarge.back().GetCapacity() - xm_deqLarge.back().GetSize() < uSize)){
			xm_deqLarge.emplace_back();
			xm_deqLarge.back().Reserve(std::max(sizeof(xm_abySmall), uSize));
		}
		xm_deqLarge.back().CopyToEnd(pbyRead, uSize);
	}
	xm_uSize += uSize;
}

int StreamBuffer::Get() noexcept {
	int nRet = -1;
	if(xm_uSmallSize > 0){
		nRet = xm_abySmall[0];
		if(xm_uSmallSize > 1){
			std::copy(xm_abySmall + 1, xm_abySmall + xm_uSmallSize, xm_abySmall);
		}
		--xm_uSmallSize;
	} else {
		while(!xm_deqLarge.empty()){
			auto &vecBlock = xm_deqLarge.front();
			if(!vecBlock.IsEmpty()){
				nRet = vecBlock[0];
				if(vecBlock.GetSize() > 1){
					std::copy(vecBlock.GetBegin() + 1, vecBlock.GetEnd(), vecBlock.GetBegin());
					vecBlock.Pop();
				} else {
					xm_deqLarge.pop_front();
				}
				break;
			}
			xm_deqLarge.pop_front();
		}
	}
	--xm_uSize;
	return nRet;
}
int StreamBuffer::Peek() const noexcept {
	if(xm_uSmallSize > 0){
		return xm_abySmall[0];
	}
	for(const auto &vecBlock : xm_deqLarge){
		if(!vecBlock.IsEmpty()){
			return vecBlock[0];
		}
	}
	return -1;
}
void StreamBuffer::Put(unsigned char by){
	if(xm_deqLarge.empty() && (sizeof(xm_abySmall) > xm_uSmallSize)){
		xm_abySmall[xm_uSmallSize] = by;
		++xm_uSmallSize;
	} else {
		if(xm_deqLarge.empty() || (xm_deqLarge.back().GetCapacity() == xm_deqLarge.back().GetSize())){
			xm_deqLarge.emplace_back();
			xm_deqLarge.back().Reserve(sizeof(xm_abySmall));
		}
		xm_deqLarge.back().Push(by);
	}
	++xm_uSize;
}

void StreamBuffer::Append(const StreamBuffer &rhs){
	Vector<unsigned char> vecTemp;
	vecTemp.Reserve(rhs.GetSize());
	rhs.Traverse(
		[&vecTemp](auto pbyData, auto uSize){
			vecTemp.CopyToEnd(pbyData, pbyData + uSize);
		}
	);
	const auto uDeltaSize = vecTemp.GetSize();
	xm_deqLarge.emplace_back(std::move(vecTemp));
	xm_uSize += uDeltaSize;
}
void StreamBuffer::Append(StreamBuffer &&rhs){
	if(&rhs == this){
		Append(rhs);
	} else {
		Insert(rhs.xm_abySmall, rhs.xm_uSmallSize);
		for(auto &vecBlock : rhs.xm_deqLarge){
			if(!vecBlock.IsEmpty()){
				const auto uDeltaSize = vecBlock.GetSize();
				xm_deqLarge.emplace_back(std::move(vecBlock));
				xm_uSize += uDeltaSize;
				rhs.xm_uSize -= uDeltaSize;
				vecBlock.Clear();
			}
		}
		rhs.Clear();
	}
}

void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	std::swap(xm_uSmallSize,		rhs.xm_uSmallSize);
	decltype(xm_abySmall) abyTemp;
	std::memcpy(abyTemp,			xm_abySmall,		xm_uSmallSize);
	std::memcpy(xm_abySmall,		rhs.xm_abySmall,	rhs.xm_uSmallSize);
	std::memcpy(rhs.xm_abySmall,	abyTemp,			xm_uSmallSize);

	std::swap(xm_deqLarge,			rhs.xm_deqLarge);
	std::swap(xm_uSize,				rhs.xm_uSize);
}

void StreamBuffer::Traverse(std::function<void (const unsigned char *, std::size_t)> &fnCallback) const {
	if(xm_uSmallSize > 0){
		fnCallback(xm_abySmall, xm_uSmallSize);
	}
	for(const auto &vecBlock : xm_deqLarge){
		if(!vecBlock.IsEmpty()){
			fnCallback(vecBlock.GetData(), vecBlock.GetSize());
		}
	}
}
void StreamBuffer::Traverse(std::function<void (const unsigned char *, std::size_t)> &&fnCallback) const {
	Traverse(fnCallback);
}
void StreamBuffer::Traverse(std::function<void (unsigned char *, std::size_t)> &fnCallback){
	if(xm_uSmallSize > 0){
		fnCallback(xm_abySmall, xm_uSmallSize);
	}
	for(auto &vecBlock : xm_deqLarge){
		if(!vecBlock.IsEmpty()){
			fnCallback(vecBlock.GetData(), vecBlock.GetSize());
		}
	}
}
void StreamBuffer::Traverse(std::function<void (unsigned char *, std::size_t)> &&fnCallback){
	Traverse(fnCallback);
}
