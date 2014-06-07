// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "StreamBuffer.hpp"
#include "Utilities.hpp"
#include <cstring>
using namespace MCF;

// 嵌套类定义。
class StreamBuffer::xBlockHeader {
public:
	static xBlockHeaderPtr Create(std::size_t m_uCapacity);

public:
	xBlockHeaderPtr m_pNext;

	std::size_t m_uRead;
	std::size_t m_uWrite;
	const std::size_t m_uCapacity;
	unsigned char m_abyData[];

private:
	explicit xBlockHeader(std::size_t uCapacity) noexcept
		: m_pNext		(nullptr)
		, m_uRead		(0)
		, m_uWrite		(0)
		, m_uCapacity	(uCapacity)
	{
	}
};

inline StreamBuffer::xBlockHeaderPtr StreamBuffer::xBlockHeader::Create(std::size_t uCapacity){
	struct Helper : public xBlockHeader {
		explicit Helper(std::size_t uCapacity)
			: xBlockHeader(uCapacity)
		{
		}
	};

	if(uCapacity < 4096){
		uCapacity = 4096;
	}
	return xBlockHeaderPtr(
		Construct<Helper>(::operator new(sizeof(Helper) + uCapacity), uCapacity)
	);
}

inline void StreamBuffer::xBlockDeleter::operator()(xBlockHeader *pBlock) noexcept {
	if(pBlock){
		pBlock->~xBlockHeader();
		::operator delete(pBlock);
	}
}

// 构造函数和析构函数。
StreamBuffer::StreamBuffer() noexcept
	: xm_uSmallRead		(0)
	, xm_uSmallWrite	(0)
	, xm_pLargeHead		(nullptr)
	, xm_pLargeTail		(nullptr)
	, xm_uSize			(0)
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
	xm_uSmallRead	= 0;
	xm_uSmallWrite	= 0;

	xm_pLargeHead	= nullptr;
	xm_pLargeTail	= nullptr;
	xm_uSize		= 0;
}

int StreamBuffer::Get() noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		if(xm_uSmallWrite > xm_uSmallRead){
			const int nRet = xm_abySmall[xm_uSmallRead];
			++xm_uSmallRead;
			--xm_uSize;
			return nRet;
		}
		if(xm_pLargeHead){
			for(;;){
				if(xm_pLargeHead->m_uWrite > xm_pLargeHead->m_uRead){
					const int nRet = xm_pLargeHead->m_abyData[xm_pLargeHead->m_uRead];
					++xm_pLargeHead->m_uRead;
					if(xm_pLargeHead->m_uRead == xm_pLargeHead->m_uWrite){
						std::exchange(xm_pLargeHead, std::move(xm_pLargeHead->m_pNext));
					}
					--xm_uSize;
					return nRet;
				}
				std::exchange(xm_pLargeHead, std::move(xm_pLargeHead->m_pNext));

				if(!xm_pLargeHead){
					xm_pLargeTail = nullptr;
					break;
				}
			}
		}
		return -1;
	}
	ASSERT_NOEXCEPT_END
}
int StreamBuffer::Peek() const noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		if(xm_uSmallWrite > xm_uSmallRead){
			return xm_abySmall[xm_uSmallRead];
		}
		for(auto pBlock = xm_pLargeHead.get(); pBlock; pBlock = pBlock->m_pNext.get()){
			if(pBlock->m_uWrite > pBlock->m_uRead){
				return pBlock->m_abyData[pBlock->m_uRead];
			}
		}
		return -1;
	}
	ASSERT_NOEXCEPT_END
}
void StreamBuffer::Put(unsigned char by){
	ASSERT_NOEXCEPT_BEGIN
	{
		if(sizeof(xm_abySmall) > xm_uSmallWrite){
			xm_abySmall[xm_uSmallWrite] = by;
			++xm_uSmallWrite;
			++xm_uSize;
			return;
		}
		if(xm_pLargeTail && (xm_pLargeTail->m_uCapacity > xm_pLargeTail->m_uWrite)){
			xm_pLargeTail->m_abyData[xm_pLargeTail->m_uWrite] = by;
			++xm_pLargeTail->m_uWrite;
			++xm_uSize;
			return;
		}
	}
	ASSERT_NOEXCEPT_END

	auto pNewBlock = xBlockHeader::Create(1);
	pNewBlock->m_abyData[0] = by;
	pNewBlock->m_uWrite = 1;

	ASSERT_NOEXCEPT_BEGIN
	{
		const auto pNewTail = pNewBlock.get();
		if(xm_pLargeHead){
			ASSERT(xm_pLargeTail);

			xm_pLargeTail->m_pNext = std::move(pNewBlock);
		} else {
			xm_pLargeHead = std::move(pNewBlock);
		}
		xm_pLargeTail = pNewTail;

		++xm_uSize;
	}
	ASSERT_NOEXCEPT_END
}

bool StreamBuffer::Extract(void *pData, std::size_t uSize) noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		if(uSize > xm_uSize){
			return false;
		}

		auto pbyWrite = (unsigned char *)pData;
		if(xm_uSmallWrite - xm_uSmallRead >= uSize){
			std::memcpy(pbyWrite, xm_abySmall + xm_uSmallRead, uSize);
			xm_uSmallRead += uSize;
			xm_uSize -= uSize;
			return true;
		}

		const auto pbyEnd = pbyWrite + uSize;
		std::memcpy(pbyWrite, xm_abySmall + xm_uSmallRead, xm_uSmallWrite - xm_uSmallRead);
		pbyWrite += (xm_uSmallWrite - xm_uSmallRead);
		xm_uSmallRead = xm_uSmallWrite;

		for(;;){
			ASSERT(xm_pLargeHead);

			const auto uRemaining = (std::size_t)(pbyEnd - pbyWrite);
			const auto uBlockSize = xm_pLargeHead->m_uWrite - xm_pLargeHead->m_uRead;
			if(uBlockSize >= uRemaining){
				std::memcpy(pbyWrite, xm_pLargeHead->m_abyData + xm_pLargeHead->m_uRead, uSize);
				xm_pLargeHead->m_uRead += uSize;
				if(xm_pLargeHead->m_uRead == xm_pLargeHead->m_uWrite){
					std::exchange(xm_pLargeHead, std::move(xm_pLargeHead->m_pNext));
				}
				xm_uSize -= uSize;
				return true;
			}
			std::memcpy(pbyWrite, xm_pLargeHead->m_abyData + xm_pLargeHead->m_uRead, uBlockSize);
			pbyWrite += uBlockSize;
			std::exchange(xm_pLargeHead, std::move(xm_pLargeHead->m_pNext));
		}
	}
	ASSERT_NOEXCEPT_END
}
void StreamBuffer::Insert(const void *pData, std::size_t uSize){
	const auto pbyRead = (const unsigned char *)pData;

	ASSERT_NOEXCEPT_BEGIN
	{
		if(sizeof(xm_abySmall) - xm_uSmallWrite >= uSize){
			std::memcpy(xm_abySmall + xm_uSmallWrite, pbyRead, uSize);
			xm_uSmallWrite += uSize;
			xm_uSize += uSize;
			return;
		}
		if(xm_pLargeTail && (xm_pLargeTail->m_uCapacity - xm_pLargeTail->m_uWrite >= uSize)){
			std::memcpy(xm_pLargeTail->m_abyData + xm_pLargeTail->m_uWrite, pbyRead, uSize);
			xm_pLargeTail->m_uWrite += uSize;
			xm_uSize += uSize;
			return;
		}
	}
	ASSERT_NOEXCEPT_END

	auto pNewBlock = xBlockHeader::Create(uSize);
	std::memcpy(pNewBlock->m_abyData, pbyRead, uSize);
	pNewBlock->m_uWrite = uSize;

	ASSERT_NOEXCEPT_BEGIN
	{
		const auto pNewTail = pNewBlock.get();
		if(xm_pLargeHead){
			ASSERT(xm_pLargeTail);

			xm_pLargeTail->m_pNext = std::move(pNewBlock);
		} else {
			xm_pLargeHead = std::move(pNewBlock);
		}
		xm_pLargeTail = pNewTail;

		xm_uSize += uSize;
	}
	ASSERT_NOEXCEPT_END
}

void StreamBuffer::Append(const StreamBuffer &rhs){
	const auto uDeltaSize = rhs.GetSize();
	auto pNewBlock = xBlockHeader::Create(uDeltaSize);
	rhs.Traverse(
		[&](auto pbyData, auto uSize) noexcept {
			ASSERT(pNewBlock->m_uCapacity - pNewBlock->m_uWrite >= uSize);

			std::memcpy(pNewBlock->m_abyData + pNewBlock->m_uWrite, pbyData, uSize);
			pNewBlock->m_uWrite += uSize;
		}
	);

	ASSERT_NOEXCEPT_BEGIN
	{
		const auto pNewTail = pNewBlock.get();
		if(xm_pLargeHead){
			ASSERT(xm_pLargeTail);

			xm_pLargeTail->m_pNext = std::move(pNewBlock);
		} else {
			xm_pLargeHead = std::move(pNewBlock);
		}
		xm_pLargeTail = pNewTail;

		xm_uSize += uDeltaSize;
	}
	ASSERT_NOEXCEPT_END
}
void StreamBuffer::Append(StreamBuffer &&rhs){
	if(&rhs == this){
		return Append(rhs);
	}

	const auto uSmallSize = rhs.xm_uSmallWrite - rhs.xm_uSmallRead;
	Insert(rhs.xm_abySmall, uSmallSize);

	ASSERT_NOEXCEPT_BEGIN
	{
		if(xm_pLargeHead){
			ASSERT(xm_pLargeTail);

			xm_pLargeTail->m_pNext = std::move(rhs.xm_pLargeHead);
		} else {
			xm_pLargeHead = std::move(rhs.xm_pLargeHead);
		}
		xm_pLargeTail = rhs.xm_pLargeTail;

		xm_uSize += (rhs.xm_uSize - uSmallSize);
		rhs.Clear();
	}
	ASSERT_NOEXCEPT_END
}

void StreamBuffer::Swap(StreamBuffer &rhs) noexcept {
	ASSERT_NOEXCEPT_BEGIN
	{
		std::swap(xm_abySmall,		rhs.xm_abySmall);
		std::swap(xm_uSmallRead,	rhs.xm_uSmallRead);
		std::swap(xm_uSmallWrite,	rhs.xm_uSmallWrite);

		std::swap(xm_pLargeHead,	rhs.xm_pLargeHead);
		std::swap(xm_pLargeTail,	rhs.xm_pLargeTail);
		std::swap(xm_uSize,			rhs.xm_uSize);
	}
	ASSERT_NOEXCEPT_END
}

void StreamBuffer::Traverse(const std::function<void (const unsigned char *, std::size_t)> &fnCallback) const {
	if(xm_uSmallWrite > xm_uSmallRead){
		fnCallback(xm_abySmall + xm_uSmallRead, xm_uSmallWrite - xm_uSmallRead);
	}
	for(auto pBlock = xm_pLargeHead.get(); pBlock; pBlock = pBlock->m_pNext.get()){
		if(pBlock->m_uWrite > pBlock->m_uRead){
			fnCallback(pBlock->m_abyData + pBlock->m_uRead, pBlock->m_uWrite - pBlock->m_uRead);
		}
	}
}
void StreamBuffer::Traverse(const std::function<void (unsigned char *, std::size_t)> &fnCallback){
	if(xm_uSmallWrite > xm_uSmallRead){
		fnCallback(xm_abySmall + xm_uSmallRead, xm_uSmallWrite - xm_uSmallRead);
	}
	for(auto pBlock = xm_pLargeHead.get(); pBlock; pBlock = pBlock->m_pNext.get()){
		if(pBlock->m_uWrite > pBlock->m_uRead){
			fnCallback(pBlock->m_abyData + pBlock->m_uRead, pBlock->m_uWrite - pBlock->m_uRead);
		}
	}
}
