// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_DATA_BUFFER_HPP_
#define MCF_DATA_BUFFER_HPP_

#include "../Core/Utilities.hpp"
#include <array>
#include <vector>
#include <deque>
#include <functional>
#include <iterator>
#include <cstddef>

namespace MCF {

class DataBufferReadIterator;
class DataBufferWriteIterator;

class DataBuffer {
public:
	typedef DataBufferReadIterator	ReadIterator;
	typedef DataBufferWriteIterator	WriteIterator;

private:
	std::array<unsigned char, 256u> xm_arrSmall;
	std::size_t xm_uSmallSize;
	std::deque<std::vector<unsigned char>> xm_deqLarge;
	std::size_t xm_uSize;

public:
	DataBuffer() noexcept;
	DataBuffer(const DataBuffer &rhs);
	DataBuffer(DataBuffer &&rhs) noexcept;
	DataBuffer &operator=(const DataBuffer &rhs);
	DataBuffer &operator=(DataBuffer &&rhs) noexcept;

public:
	bool IsEmpty() const noexcept;
	std::size_t GetSize() const noexcept;
	void Clear() noexcept;

	// 追加 uSize 个字节。
	void Insert(const void *pData, std::size_t uSize);
	// 要么就从头部读取并删除 uSize 个字节并返回 true，要么就返回 false。
	// 没有只读取一半的情况。
	bool ExtractNoThrow(void *pData, std::size_t uSize) noexcept;
	// 和上面类似，但是如果数据少于请求的字节数会抛出异常 MCF::Exception(ERROR_HANDLE_EOF)。
	void Extract(void *pData, std::size_t uSize);

	void Append(const DataBuffer &dbufOther);
	void Append(DataBuffer &&dbufOther);
	void Swap(DataBuffer &dbufOther) noexcept;

	void Traverse(std::function<void (const unsigned char *, std::size_t)> fnCallback) const;
	void Traverse(std::function<void (unsigned char *, std::size_t)> fnCallback);

	ReadIterator GetReadIterator() noexcept;
	static constexpr ReadIterator GetReadEnd() noexcept;
	// 返回一个和任何有效迭代器都不相等的迭代器，用于从输入流中一直读取。
	// 如果读到流的末尾会引发异常。
	static constexpr ReadIterator GetReadForeverEnd() noexcept;
	WriteIterator GetWriteIterator() noexcept;
};

class DataBufferReadIterator
	: public std::iterator<std::input_iterator_tag, unsigned char>
{
	friend DataBuffer;

private:
	static constexpr const auto xFOREVER = (DataBuffer *)(std::intptr_t)-1;

private:
	DataBuffer *const xm_pdbufOwner;

private:
	explicit constexpr DataBufferReadIterator(DataBuffer *pdbufOwner) noexcept
		: xm_pdbufOwner(pdbufOwner)
	{
	}

public:
	constexpr DataBufferReadIterator() noexcept
		: xm_pdbufOwner(nullptr)
	{
	}
	explicit constexpr DataBufferReadIterator(DataBuffer &dbufOwner) noexcept
		: xm_pdbufOwner(&dbufOwner)
	{
	}

public:
	bool operator==(const DataBufferReadIterator &rhs) const noexcept {
		if((xm_pdbufOwner == xFOREVER) || (rhs.xm_pdbufOwner == xFOREVER)){
			return false;
		}
		if(rhs.xm_pdbufOwner){
			if(xm_pdbufOwner){
				return xm_pdbufOwner == rhs.xm_pdbufOwner;
			} else {
				return rhs.xm_pdbufOwner->IsEmpty();
			}
		} else {
			if(xm_pdbufOwner){
				return xm_pdbufOwner->IsEmpty();
			} else {
				return true;
			}
		}
	}
	bool operator!=(const DataBufferReadIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	DataBufferReadIterator &operator++() noexcept {
		return *this;
	}
	DataBufferReadIterator operator++(int) noexcept {
		return *this;
	}

	unsigned char operator*(){
		unsigned char by;
		xm_pdbufOwner->Extract(&by, sizeof(by));
		return by;
	}
};

class DataBufferWriteIterator
	: public std::iterator<std::output_iterator_tag, unsigned char>
{
	friend DataBuffer;

private:
	DataBuffer *const xm_pdbufOwner;

public:
	explicit constexpr DataBufferWriteIterator(DataBuffer &dbufOwner) noexcept
		: xm_pdbufOwner(&dbufOwner)
	{
	}

public:
	bool operator==(const DataBufferWriteIterator &rhs) const noexcept {
		return xm_pdbufOwner == rhs.xm_pdbufOwner;
	}
	bool operator!=(const DataBufferWriteIterator &rhs) const noexcept {
		return !(*this == rhs);
	}
	DataBufferWriteIterator &operator++() noexcept {
		return *this;
	}
	DataBufferWriteIterator operator++(int) noexcept {
		return *this;
	}

	DataBufferWriteIterator &operator*() noexcept {
		return *this;
	}
	DataBufferWriteIterator &operator=(unsigned char by){
		xm_pdbufOwner->Insert(&by, sizeof(by));
		return *this;
	}
};

inline DataBuffer::ReadIterator DataBuffer::GetReadIterator() noexcept {
	return ReadIterator(*this);
}
inline constexpr DataBuffer::ReadIterator DataBuffer::GetReadEnd() noexcept {
	return ReadIterator();
}
inline constexpr DataBuffer::ReadIterator DataBuffer::GetReadForeverEnd() noexcept {
	return ReadIterator(ReadIterator::xFOREVER);
}
inline DataBuffer::WriteIterator DataBuffer::GetWriteIterator() noexcept {
	return WriteIterator(*this);
}

}

#endif
