// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "Utilities.hpp"
#include "Exception.hpp"
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
	: xm_abySmall	(rhs.xm_abySmall)
	, xm_uSmallSize	(rhs.xm_uSmallSize)
	, xm_deqLarge	(rhs.xm_deqLarge)
	, xm_uSize		(rhs.xm_uSize)
{
}
StreamBuffer::StreamBuffer(StreamBuffer &&rhs) noexcept
	: xm_abySmall	(std::move(rhs.xm_abySmall))
	, xm_uSmallSize	(rhs.xm_uSmallSize)
	, xm_deqLarge	(std::move(rhs.xm_deqLarge))
	, xm_uSize		(rhs.xm_uSize)
{
	rhs.Clear();
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

void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	if(uSize == 0){
		return;
	}
	const auto pbyData = (const unsigned char *)pData;

	do {
		if(xm_deqLarge.empty()){
			if(xm_abySmall.size() - xm_uSmallSize >= uSize){
				std::memcpy(xm_abySmall.data() + xm_uSmallSize, pbyData, uSize);
				xm_uSmallSize += uSize;
				break;
			}
		} else {
			auto &vecLast = xm_deqLarge.back();
			const auto uLastSize = vecLast.size();
			if(vecLast.capacity() - uLastSize >= uSize){
				vecLast.insert(vecLast.end(), pbyData, pbyData + uSize);
				break;
			}
		}
		xm_deqLarge.emplace_back();
		auto &vecLast = xm_deqLarge.back();
		vecLast.reserve(std::max(xm_abySmall.size(), uSize));
		vecLast.assign(pbyData, pbyData + uSize);
	} while(false);

	xm_uSize += uSize;
}
bool StreamBuffer::ExtractNoThrow(void *pData, std::size_t uSize) noexcept {
	if(xm_uSize < uSize){
		return false;
	}
	if(uSize == 0){
		return true;
	}

	do {
		auto pbyWrite = (unsigned char *)pData;
		auto uBytesRemaining = uSize;
		if(xm_uSmallSize > 0){
			if(xm_uSmallSize >= uSize){
				pbyWrite = std::copy(xm_abySmall.begin(), xm_abySmall.begin() + (std::ptrdiff_t)uBytesRemaining, pbyWrite);
				std::copy(xm_abySmall.begin() + (std::ptrdiff_t)uSize, xm_abySmall.end(), xm_abySmall.begin());
				xm_uSmallSize -= uSize;
				break;
			}
			pbyWrite = std::copy(xm_abySmall.begin(), xm_abySmall.begin() + (std::ptrdiff_t)xm_uSmallSize, pbyWrite);
			uBytesRemaining -= xm_uSmallSize;
			xm_uSmallSize = 0;
		}
		for(;;){
			ASSERT(!xm_deqLarge.empty());

			auto &vecFirst = xm_deqLarge.front();
			const auto uFirstSize = vecFirst.size();
			if(uFirstSize > uBytesRemaining){
				pbyWrite = std::copy(vecFirst.begin(), vecFirst.begin() + (std::ptrdiff_t)uBytesRemaining, pbyWrite);
				vecFirst.erase(
					std::copy(vecFirst.begin() + (std::ptrdiff_t)uBytesRemaining, vecFirst.end(), vecFirst.begin()),
					vecFirst.end()
				);
				break;
			}
			pbyWrite = std::copy(vecFirst.begin(), vecFirst.end(), pbyWrite);
			uBytesRemaining -= uFirstSize;
			xm_deqLarge.pop_front();

			if(uBytesRemaining == 0){
				break;
			}
		}
	} while(false);

	xm_uSize -= uSize;
	return true;
}
void StreamBuffer::Extract(void *pData, std::size_t uSize){
	if(!ExtractNoThrow(pData, uSize)){
		MCF_THROW(ERROR_HANDLE_EOF, L"遇到意外的文件尾。");
	}
}

void StreamBuffer::Append(const StreamBuffer &rhs){
	std::vector<unsigned char> vecTemp;
	vecTemp.reserve(xm_uSize);
	rhs.Traverse(
		[&](auto pbyData, auto uSize){
			vecTemp.insert(vecTemp.end(), pbyData, pbyData + uSize);
			return false;
		}
	);
	xm_deqLarge.emplace_back(std::move(vecTemp));
	xm_uSize += rhs.xm_uSize;
}
void StreamBuffer::Append(StreamBuffer &&rhs){
	if(&rhs == this){
		Append(*this);
	} else {
		Insert(
			rhs.xm_abySmall.data(),
			rhs.xm_uSmallSize
		);
		std::move(
			rhs.xm_deqLarge.begin(),
			rhs.xm_deqLarge.end(),
			std::back_inserter(xm_deqLarge)
		);
		xm_uSize += rhs.xm_uSize;
		rhs.Clear();
	}
}
void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		std::swap(xm_abySmall,		rhs.xm_abySmall);
		std::swap(xm_uSmallSize,	rhs.xm_uSmallSize);
		std::swap(xm_deqLarge,		rhs.xm_deqLarge);
		std::swap(xm_uSize,			rhs.xm_uSize);
	}
	ASSERT_NOEXCEPT_END
}

void StreamBuffer::Traverse(std::function<void (const unsigned char *, std::size_t)> fnCallback) const {
	if(xm_uSmallSize > 0){
		fnCallback(xm_abySmall.data(), xm_uSmallSize);
	}
	for(const auto &vecCur : xm_deqLarge){
		fnCallback(vecCur.data(), vecCur.size());
	}
}
void StreamBuffer::Traverse(std::function<void (unsigned char *, std::size_t)> fnCallback){
	if(xm_uSmallSize > 0){
		fnCallback(xm_abySmall.data(), xm_uSmallSize);
	}
	for(auto &vecCur : xm_deqLarge){
		fnCallback(vecCur.data(), vecCur.size());
	}
}
