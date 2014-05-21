// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "DataBuffer.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/Exception.hpp"
#include <cstring>
using namespace MCF;

// 构造函数和析构函数。
DataBuffer::DataBuffer() noexcept
	: xm_uSmallSize	(0)
	, xm_uSize		(0)
{
}
DataBuffer::DataBuffer(const DataBuffer &rhs)
	: xm_arrSmall	(rhs.xm_arrSmall)
	, xm_uSmallSize	(rhs.xm_uSmallSize)
	, xm_deqLarge	(rhs.xm_deqLarge)
	, xm_uSize		(rhs.xm_uSize)
{
}
DataBuffer::DataBuffer(DataBuffer &&rhs) noexcept
	: xm_arrSmall	(std::move(rhs.xm_arrSmall))
	, xm_uSmallSize	(rhs.xm_uSmallSize)
	, xm_deqLarge	(std::move(rhs.xm_deqLarge))
	, xm_uSize		(rhs.xm_uSize)
{
	rhs.Clear();
}
DataBuffer &DataBuffer::operator=(const DataBuffer &rhs){
	xm_arrSmall		= rhs.xm_arrSmall;
	xm_uSmallSize	= rhs.xm_uSmallSize;
	xm_deqLarge		= rhs.xm_deqLarge;
	xm_uSize		= rhs.xm_uSize;

	return *this;
}
DataBuffer &DataBuffer::operator=(DataBuffer &&rhs) noexcept {
	xm_arrSmall		= std::move(rhs.xm_arrSmall);
	xm_uSmallSize	= rhs.xm_uSmallSize;
	xm_deqLarge		= std::move(rhs.xm_deqLarge);
	xm_uSize		= rhs.xm_uSize;

	rhs.Clear();

	return *this;
}

// 其他非静态成员函数。
bool DataBuffer::IsEmpty() const noexcept {
	return xm_uSize == 0;
}
std::size_t DataBuffer::GetSize() const noexcept {
	return xm_uSize;
}
void DataBuffer::Clear() noexcept {
	xm_uSmallSize = 0;
	xm_deqLarge.clear();
	xm_uSize = 0;
}

void DataBuffer::Insert(const void *pData, std::size_t uSize){
	if(uSize == 0){
		return;
	}
	const auto pbyData = (const unsigned char *)pData;

	do {
		if(xm_deqLarge.empty()){
			if(xm_arrSmall.size() - xm_uSmallSize >= uSize){
				std::memcpy(xm_arrSmall.data() + xm_uSmallSize, pbyData, uSize);
				xm_uSmallSize += uSize;
				break;
			}
			xm_deqLarge.emplace_back();
			xm_deqLarge.back().reserve(std::max(xm_arrSmall.size(), uSize));
		}
		auto &vecLast = xm_deqLarge.back();
		const auto uLastSize = vecLast.size();
		if(vecLast.capacity() - uLastSize >= uSize){
			vecLast.insert(vecLast.end(), pbyData, pbyData + uSize);
			break;
		}
		xm_deqLarge.emplace_back();
		xm_deqLarge.back().reserve(std::max(xm_arrSmall.size(), uSize));
		xm_deqLarge.back().assign(pbyData, pbyData + uSize);
	} while(false);

	xm_uSize += uSize;
}
bool DataBuffer::ExtractNoThrow(void *pData, std::size_t uSize) noexcept {
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
				pbyWrite = std::copy(xm_arrSmall.begin(), xm_arrSmall.begin() + (std::ptrdiff_t)uBytesRemaining, pbyWrite);
				std::copy(xm_arrSmall.begin() + (std::ptrdiff_t)uSize, xm_arrSmall.end(), xm_arrSmall.begin());
				xm_uSmallSize -= uSize;
				break;
			}
			pbyWrite = std::copy(xm_arrSmall.begin(), xm_arrSmall.begin() + (std::ptrdiff_t)xm_uSmallSize, pbyWrite);
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
void DataBuffer::Extract(void *pData, std::size_t uSize){
	if(!ExtractNoThrow(pData, uSize)){
		MCF_THROW(ERROR_HANDLE_EOF, L"遇到意外的文件尾。");
	}
}

void DataBuffer::Append(const DataBuffer &dbufOther){
	dbufOther.Traverse([this](const unsigned char *pbyData, std::size_t uSize){
		this->Insert(pbyData, uSize);
	});
}
void DataBuffer::Append(DataBuffer &&dbufOther){
	Insert(dbufOther.xm_arrSmall.data(), dbufOther.xm_uSmallSize);
	for(auto &vecCur : dbufOther.xm_deqLarge){
		xm_deqLarge.emplace_back(std::move(vecCur));
	}
	dbufOther.Clear();
}
void DataBuffer::Swap(DataBuffer &dbufOther) noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		std::swap(xm_arrSmall,		dbufOther.xm_arrSmall);
		std::swap(xm_uSmallSize,	dbufOther.xm_uSmallSize);
		std::swap(xm_deqLarge,		dbufOther.xm_deqLarge);
		std::swap(xm_uSize,			dbufOther.xm_uSize);
	}
	ASSERT_NOEXCEPT_END
}

void DataBuffer::Traverse(std::function<void (const unsigned char *, std::size_t)> fnCallback) const {
	if(xm_uSmallSize > 0){
		fnCallback(xm_arrSmall.data(), xm_uSmallSize);
	}
	for(const auto &vecCur : xm_deqLarge){
		fnCallback(vecCur.data(), vecCur.size());
	}
}
void DataBuffer::Traverse(std::function<void (unsigned char *, std::size_t)> fnCallback){
	if(xm_uSmallSize > 0){
		fnCallback(xm_arrSmall.data(), xm_uSmallSize);
	}
	for(auto &vecCur : xm_deqLarge){
		fnCallback(vecCur.data(), vecCur.size());
	}
}
